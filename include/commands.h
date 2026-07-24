/*
 * commands.h
 * ----------
 * Declares all shell command functions and utility functions.
 * Include this header in any file that needs to call these functions.
 */

#ifndef COMMANDS_H
#define COMMANDS_H

/* ── Utility / UI functions (implemented in src/utils.c) ── */

/* Validates the input against the known command set. Returns 1 if valid, 0 if not. */
int  check(const char *cmd);

/* Prints the list of available commands */
void help();

/* Prints a brief description of what each command does */
void defination();

/* Appends a command string to the history list */
void add_to_history(const char *cmd);

/* Prints the full command history */
void run_history();


/* ── Command implementations (implemented in src/commands.c) ── */

/* Prints the current working directory */
void run_pwd();

/* Prints the current date and time */
void run_date();

/* Lists files and directories in the current directory */
void run_ls();

/* Prints the current logged-in username */
void run_whoami();

/* Prompts for a name and creates a new directory */
void run_mkdir();

/* Prompts for text and prints it back (like Unix echo) */
void run_echo();

/* Clears the terminal screen */
void run_clear();

/* Prompts for a path and changes the working directory */
void run_cd();

/* Prints a goodbye message before the shell exits */
void run_exit();

#endif /* COMMANDS_H */
