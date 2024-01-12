// Copyright 2016,2024 by Mark Malek
// See LICENSE for license information.
/*
 * RobotArm.h
 *
 * A library for controlling the OWI Robotic Arm Edge.
 * Requires libusb-1.0.  Link with -lusb-1.0
 * Reverse-engineered from a reverse-engineered blog posting at
 * http://notbrainsurgery.livejournal.com/38622.html
 *
 * Note that ordinarily root privileges are required to open USB devices.
 */
#ifndef ROBOTARM_H_
#define ROBOTARM_H_

// Forward declaration and convenience typedef of the handle type for the
// robot arm.  This structure should be treated as opaque by clients.
struct RobotArmHandle;
typedef struct RobotArmHandle RobotArmHandle;

/// Find the (first) robot arm by scanning the available USB devices.
/// If one is found, it is returned ready to go.  The light is on by default.
/// \return An open handle to the robot if one is found, else NULL.  The handle
///         that is returned must be freed later with robotarm_close().
struct RobotArmHandle* robotarm_find();

/// Close the robot arm handle and release all resources associated with it.
/// After this call, the handle is no longer usable.
/// \param arm A handle acquired via robotarm_find().
void robotarm_close(struct RobotArmHandle* arm);

/// The set of motions that can be done by the robot arm, for use with
/// robotarm_move().  They can be ORed together to run multiple movements
/// simultaneously.
enum RobotArmMotions
{                                  //| Byte0  | Byte1  |
    ROBOTARM_STOP = 0,             //|        |        |
    ROBOTARM_GRIP_CLOSE = 0x01,    //|      01|        |
    ROBOTARM_GRIP_OPEN = 0x02,     //|      10|        |
    ROBOTARM_WRIST_UP = 0x04,      //|    01  |        |
    ROBOTARM_WRIST_DOWN = 0x08,    //|    10  |        |
    ROBOTARM_ELBOW_UP = 0x10,      //|  01    |        |
    ROBOTARM_ELBOW_DOWN = 0x20,    //|  10    |        |
    ROBOTARM_SHOULDER_BACK = 0x40, //|01      |        |
    ROBOTARM_SHOULDER_FWD = 0x80,  //|10      |        |
    ROBOTARM_BASE_CW = 0x0100,     //|        |      01|
    ROBOTARM_BASE_CCW = 0x0200,    //|        |      10|
};

/// Start the robot arm moving with the given motion(s).  Warning: this will
/// move the arm and it will keep moving until you send a different motion, or
/// call robotarm_stop().
/// \param arm Handle to the robot.
/// \param motions One of the RobotArmMotions values or several ORed together.
void robotarm_move(struct RobotArmHandle* arm, unsigned int motions);

/// Move the robot arm with the given motion(s), for the given time.
/// Will block/sleep for the duration of the motion.
/// \postcondition All motion is stopped, as if by calling robotarm_stop(),
///                even if other motions were previously in progress.
/// \param arm Handle to the robot.
/// \param motions One of the RobotArmMotions values or several ORed together.
/// \param durationMs Time in milliseconds to do the given motion.
void robotarm_move_timed(struct RobotArmHandle* arm, unsigned int motions, unsigned int durationMs);

/// Stop any movement of the robot arm.  Equivalent to passing a 0 to
/// robotarm_move().
void robotarm_stop(struct RobotArmHandle* arm);

/// Possible values of the light; obviously on or off.
enum RobotArmLight
{
    ROBOTARM_LIGHT_OFF = 0,
    ROBOTARM_LIGHT_ON = 1
};

/// Set the state of the light to on/off.  The light will stay that way, even
/// after moving or stopping the arm.
void robotarm_set_light(struct RobotArmHandle* arm, enum RobotArmLight onoff);


#endif /* ROBOTARM_H_ */
