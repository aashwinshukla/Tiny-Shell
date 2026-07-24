/*
 * utils.c
 * -------
 * Contains utility and UI functions:
 *   - check()         : validates user input against the known command set
 *   - help()          : shows the command list
 *   - defination()    : shows a brief description of each command
 *   - add_to_history(): logs a command into the history array
 *   - run_history()   : prints all logged commands
 */

#include <stdio.h>
#include <string.h>
#include "../include/globals.h"
#include "../include/commands.h"

/*
 * check()
 * -------
 * Compares the input string against every supported command.
 * Returns 1 if it matches a known command, 0 otherwise.
 * Also prints an error message for unknown input.
 */
int check(const char *cmd) {
    if (strcmp(cmd, "./help")   == 0 ||
        strcmp(cmd, "./expl")   == 0 ||  /* shows descriptions of each command */
        strcmp(cmd, "pwd")      == 0 ||
        strcmp(cmd, "date")     == 0 ||
        strcmp(cmd, "ls")       == 0 ||
        strcmp(cmd, "whoami")   == 0 ||
        strcmp(cmd, "mkdir")    == 0 ||
        strcmp(cmd, "echo")     == 0 ||
        strcmp(cmd, "exit")     == 0 ||
        strcmp(cmd, "clear")    == 0 ||
        strcmp(cmd, "cd")       == 0 ||
        strcmp(cmd, "history")  == 0) {
        return 1;
    }

    /* Unknown command — inform the user */
    printf("\n[ERROR] Unknown command: '%s'\n", cmd);
    printf("Use './help' to see the list of available commands.\n\n");
    return 0;
}

/*
 * help()
 * ------
 * Prints a numbered list of all supported commands.
 * After the list, prompts the user to type ./expl for detailed descriptions.
 */
void help() {
    printf("\n--------------Command List--------------\n");
    printf("  1.  ./help\n");
    printf("  2.  pwd\n");
    printf("  3.  ls\n");
    printf("  4.  date\n");
    printf("  5.  whoami\n");
    printf("  6.  mkdir\n");
    printf("  7.  echo\n");
    printf("  8.  exit\n");
    printf("  9.  clear\n");
    printf("  10. cd\n");
    printf("  11. history\n");
    printf("----------------------------------------\n");
    printf("Type './expl' for command descriptions, or any key to continue.\n\n");
}

/*
 * defination()
 * ------------
 * Prints a one-line description for every supported command.
 * Reached by typing './expl' after the './help' prompt.
 */
void defination() {
    printf("\n-----------Command Descriptions----------\n");
    printf("  ./help   - Show the list of all available commands\n");
    printf("  pwd      - Print the current working directory path\n");
    printf("  ls       - List all files and folders in the current directory\n");
    printf("  date     - Display the current system date and time\n");
    printf("  whoami   - Show the name of the currently logged-in user\n");
    printf("  mkdir    - Create a new directory (prompts for a name)\n");
    printf("  echo     - Print a line of text that you type (like Unix echo)\n");
    printf("  exit     - Quit the Tiny Shell program\n");
    printf("  clear    - Clear the terminal screen\n");
    printf("  cd       - Change the current working directory\n");
    printf("  history  - Display all commands entered in this session\n");
    printf("-----------------------------------------\n\n");
}

/*
 * add_to_history()
 * ----------------
 * Saves the given command string into the history array.
 * Silently ignores new entries once the 100-command limit is reached.
 */
void add_to_history(const char *cmd) {
    if (history_count < 100) {
        strncpy(history_list[history_count], cmd, 255);
        history_list[history_count][255] = '\0'; /* ensure null-termination */
        history_count++;
    }
}

/*
 * run_history()
 * -------------
 * Prints every command stored in history_list with its index number.
 * Shows a message if no commands have been recorded yet.
 */
void run_history() {
    printf("\n------- Command History -------\n");

    if (history_count == 0) {
        printf("  No commands in history yet.\n");
    } else {
        for (int i = 0; i < history_count; i++) {
            printf("  %2d: %s\n", i + 1, history_list[i]);
        }
    }

    printf("-------------------------------\n\n");
}
