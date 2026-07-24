/*
 * globals.h
 * ---------
 * Declares all global variables shared across the project.
 * Each .c file that needs these variables should include this header.
 *
 * Using 'extern' here means the actual memory is allocated in globals.c,
 * and every other file just gets a reference to it.
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdbool.h>

/* Buffer that stores the command the user types in */
extern char command[256];

/* Buffer used to capture the ./expl follow-up input after ./help */
extern char explain[10];

/* Loop flag — set to false when the user types 'exit' */
extern bool enter_command;

/* Stores up to 100 past commands for the history feature */
extern char history_list[100][256];

/* Tracks how many commands have been recorded so far */
extern int history_count;

#endif /* GLOBALS_H */
