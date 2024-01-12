// Copyright 2016,2024 by Mark Malek
// See LICENSE for license information.
/*
 * RobotArmTest.c
 *
 * A simple terminal-based program to control the robot
 * arm.  Also a good example of usage.
 *
 * Link with -lncurses as well as the -lusb-1.0 needed by robotarm.h
 */


#include <ncurses.h>

#include "robotarm.h"

int main()
{
    RobotArmHandle* arm = robotarm_find();
    if (arm == NULL)
    {
        fprintf(stderr, "No robot arm found\n");
        return -1;
    }

    // init ncurses and print instructions
    initscr();
    printw("** Simple Robot Arm Controller **\n");
    printw("Press 'q' to quit. For best results, do not hold down keys\n");
    printw("\n");
    printw("      [8]fwd     Wrist:  Elbow:  Shouldr: Grip:     Light:  \n");
    printw("CCW[4]   [6]CW    [7]up   [9]up   [0]fwd   [+]open   [*]on  \n");
    printw("      [2]back     [1]dn   [3]dn   [.]back  [-]close  [/]off \n");
    refresh();

    // unbuffered input mode
    noecho();
    cbreak();

    int ch = -1;
    while ((ch=getch()) >= 0 && ch != 'q')
    {
        if (ch == '*')
        {
            robotarm_set_light(arm, ROBOTARM_LIGHT_ON);
            continue;
        }
        if (ch == '/')
        {
            robotarm_set_light(arm, ROBOTARM_LIGHT_OFF);
            continue;
        }
        unsigned int mv = 0;
        switch (ch)
        {
        case '8': mv = ROBOTARM_SHOULDER_FWD | ROBOTARM_ELBOW_UP; break;
        case '2': mv = ROBOTARM_SHOULDER_BACK | ROBOTARM_ELBOW_DOWN; break;
        case '4': mv = ROBOTARM_BASE_CCW; break;
        case '6': mv = ROBOTARM_BASE_CW; break;
        case '7': mv = ROBOTARM_WRIST_UP; break;
        case '1': mv = ROBOTARM_WRIST_DOWN; break;
        case '9': mv = ROBOTARM_ELBOW_UP; break;
        case '3': mv = ROBOTARM_ELBOW_DOWN; break;
        case '0': mv = ROBOTARM_SHOULDER_FWD; break;
        case '.': mv = ROBOTARM_SHOULDER_BACK; break;
        case '+': mv = ROBOTARM_GRIP_OPEN; break;
        case '-': mv = ROBOTARM_GRIP_CLOSE; break;
        default: continue;
        }
        robotarm_move_timed(arm, mv, 250);
    }

    //cleanup
    endwin();
    robotarm_close(arm);
}




