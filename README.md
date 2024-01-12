# libroboticarm
C Library for controlling the OWI Robotic Arm Edge

This is a simple library I made a while ago to control the toy OWI Robotic Arm
Edge via USB.  It was a "toy" project but I recently saw that they still sell 
these things so thought I would throw this out there.

This is a portable library written in C and uses the libusb-1.0.so library
for USB access.  Note this usually requires root access on Linux.  This library
can also work on Windows if you get the same libusb library available there.

>![WARNING]
>I have not tested this in a long time, and it was probably on an older model 
> that I no longer have.  There may be bugs.  Fixes are welcome.
