# Copyright 2016,2024 by Mark Malek
# See LICENSE for license information.
# Build the roboticarm library and test application.
# The library depends on libusb-1.0 (sudo apt install libusb-1.0-0-dev)
# The test application additionally depends on ncurses (apt-get install libncurses5-dev)

CC=gcc
CFLAGS=-O0 -g3 -Wall -Wextra -Werror -Wconversion -fmessage-length=0

TARGS=libroboticarm.a RobotArmTest

.PHONY: all clean
all: $(TARGS)

clean:
	rm -f $(TARGS)


libroboticarm.a: robotarm.c robotarm.h
	$(CC) $(CFLAGS) -c -fPIC robotarm.c -o $@

RobotArmTest: robotarmtest.c libroboticarm.a
	$(CC) $(CFLAGS) robotarmtest.c libroboticarm.a -lusb-1.0 -lncurses -o $@
