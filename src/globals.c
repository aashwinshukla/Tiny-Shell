/*
 * globals.c
 * ---------
 * Defines (allocates memory for) all global variables declared in globals.h.
 * Only this file should define these — all other files use 'extern' via globals.h.
 */

#include <stdbool.h>
#include "../include/globals.h"

/* The command buffer — sized at 256 to safely hold any supported command */
char command[256];

/* Secondary input buffer for reading ./expl after ./help */
char explain[10];

/* Controls the main loop; set to false to exit the shell */
bool enter_command = true;

/* 2D array storing up to 100 commands, each up to 255 chars */
char history_list[100][256];

/* Starts at 0, increments with every valid command entered */
int history_count = 0;
