// Copyright 2016,2024 by Mark Malek
// See LICENSE for license information.
/*
 * RobotArm.c
 *
 * Requires libusb-1.0 (sudo apt install libusb-1.0-0-dev)
 */

#include "robotarm.h"

#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>

#define ROBOTARM_USB_VENDOR   0x1267
#define ROBOTARM_USB_PRODUCT       0

// Timeout for the actual USB control command
#define COMMAND_TIMEOUT_MS 500

/// OPAQUE grouping of context needed for the robot arm
struct RobotArmHandle
{
    // allocated in libusb_init()
    // So as a library, we don't interfere with other uses of libusb
    libusb_context* libusb_context;

    // allocated by libusb_open
    // open handle to the robot arm device
    libusb_device_handle* device_handle;

    // Current state of the arm.  Saving the entire
    // state allows us to control the light without
    // messing up the motion, and vice-versa.
    // This is the actual buffer that is sent on the
    // USB control channel.
    // Byte 0: as documented in RobotArmMotions
    // Byte 1: as documented in RobotArmMotions
    // Byte 2: light status, per RobotArmLight
    unsigned char state[3];
};

// Private; sends the control message based on the current state buffer
static int robotarm_send(struct RobotArmHandle* arm)
{
    return libusb_control_transfer(
            arm->device_handle,
            0x40,   //uint8_t   bmRequestType = LIBUSB_ENDPOINT_OUT,LIBUSB_REQUEST_TYPE_VENDOR,LIBUSB_RECIPIENT_DEVICE
            6,      //uint8_t   bRequest = ??
            0x100,  //uint16_t  wValue = ??
            0,      //uint16_t  wIndex = ??
            arm->state,
            sizeof(arm->state),
            COMMAND_TIMEOUT_MS
        );
}

struct RobotArmHandle* robotarm_find()
{
    // Initialize libusb
    libusb_context* context;
    int ret = libusb_init(&context); // allocates
    if (ret < 0)
    {
        fprintf(stderr, "libusb_init error %d: %s\n", ret, libusb_strerror(ret));
        return NULL;
    }
    // 'context' is allocated; must libusb_exit() from here on error

    // Get all connected USB devices
    libusb_device** devices;
    ssize_t count = libusb_get_device_list(context, &devices);
    if (count < 0)
    {
        fprintf(stderr, "libusb_get_device_list error %ld\n", count);
        libusb_exit(context);
        return NULL;
    }
    // 'devices' is allocated; must libusb_free_device_list() from here on

    // Scan the list for matching vendor/product
    libusb_device* foundDevice = NULL;
    for (ssize_t i=0; i<count; ++i)
    {
        libusb_device* dev = devices[i];
        struct libusb_device_descriptor desc;
        ret = libusb_get_device_descriptor(dev, &desc);
        if (ret < 0)
        {
            fprintf(stderr, "libusb_get_device_descriptor fatal error %d, %s\n", ret, libusb_strerror(ret));
            libusb_free_device_list(devices, 1);
            libusb_exit(context);
            return NULL;
        }

        if (desc.idVendor == ROBOTARM_USB_VENDOR
            && desc.idProduct == ROBOTARM_USB_PRODUCT)
        {
            foundDevice = dev;
            break;
        }
    }
    // 'foundDevice' lives in devices map; do not free

    if (foundDevice == NULL)
    {
        libusb_free_device_list(devices, 1);
        libusb_exit(context);
        return NULL;
    }

    // Open the device
    struct libusb_device_handle* handle = NULL;
    ret = libusb_open(foundDevice, &handle);
    if (ret != 0)
    {
        fprintf(stderr, "libusb_open error %d %s\n", ret, libusb_strerror(ret));
        libusb_free_device_list(devices, 1);
        libusb_exit(context);
        return NULL;
    }

    // We're ready to rock; allocate and transfer ownership of handle to
    // RobotArmHandle struct.
    RobotArmHandle* arm = (RobotArmHandle*)malloc(sizeof(RobotArmHandle));
    arm->libusb_context = context;
    arm->device_handle = handle;
    // Initialize state and ensure robot matches state
    arm->state[0] = 0;
    arm->state[1] = 0;
    arm->state[2] = (unsigned char)ROBOTARM_LIGHT_ON; // this is the default in the arm
    robotarm_send(arm);

    // libusb doc says it's ok to free devices after open
    libusb_free_device_list(devices, 1);

    return arm;
}

void robotarm_close(struct RobotArmHandle* arm)
{
    // Just in case
    robotarm_stop(arm);
    robotarm_set_light(arm, ROBOTARM_LIGHT_OFF);

    // Close and free everything in the struct
    libusb_close(arm->device_handle);
    libusb_exit(arm->libusb_context);

    free(arm);
}

void robotarm_move(struct RobotArmHandle* arm, unsigned int motions)
{
    arm->state[0] = motions & 0xFF;
    arm->state[1] = (motions >> 8) & 0xFF;
    // state[2] of light unchanged
    int ret = robotarm_send(arm);
    if (ret < 0)
    {
        fprintf(stderr, "command failed: %d %s\n", ret, libusb_strerror(ret));
        // Try and stop to be safe
        robotarm_stop(arm);
    }
}

void robotarm_move_timed(struct RobotArmHandle* arm, unsigned int motions, unsigned int durationMs)
{
    robotarm_move(arm, motions);
    usleep(durationMs * 1000); // TODO: not portable
    robotarm_stop(arm);
}

void robotarm_stop(struct RobotArmHandle* arm)
{
    arm->state[0] = 0;
    arm->state[1] = 0;
    // state[2] of light unchanged
    int ret = robotarm_send(arm);
    if (ret < 0)
    {
        fprintf(stderr, "aieee! stop failed. You're on your own now. %d %s\n", ret, libusb_strerror(ret));
    }
}

void robotarm_set_light(struct RobotArmHandle* arm, enum RobotArmLight onoff)
{
    arm->state[2] = (unsigned char)onoff;
    int ret = robotarm_send(arm);
    if (ret < 0)
    {
        fprintf(stderr, "setting light failed: %d %s\n", ret, libusb_strerror(ret));
    }
}

