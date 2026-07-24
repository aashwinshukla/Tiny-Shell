/*
 * main.c
 * ------
 * Entry point for Tiny Shell.
 *
 * Tiny Shell is a minimal command-line shell written in C.
 * It supports a fixed set of commands and runs a simple read-execute loop:
 *
 *   1. Print a prompt
 *   2. Read the user's input
 *   3. Validate it with check()
 *   4. Execute the matching command function
 *   5. Repeat until the user types 'exit'
 *
 * All global state lives in src/globals.c (declared in include/globals.h).
 * Command implementations live in src/commands.c.
 * Utility functions (check, help, history, etc.) live in src/utils.c.
 */

#include <stdio.h>
#include <string.h>
#include "../include/globals.h"
#include "../include/commands.h"

int main() {

    /* ── Welcome banner ── */
    printf("\n========================================\n");
    printf("           Welcome to Tiny Shell        \n");
    printf("========================================\n");
    printf("  Type './help' to see all commands.\n\n");

    /* ── Main read-execute loop ── */
    while (enter_command) {

        /* Prompt the user */
        printf("tiny-shell> ");
        scanf(" %255s", command);

        /* Only proceed if the command is in the known set */
        if (check(command)) {

            /* Log every valid command to history before running it */
            add_to_history(command);

            /* ── Dispatch to the correct handler ── */

            if (strcmp(command, "./help") == 0) {
                /* Show the command list, then optionally show descriptions */
                help();
                scanf(" %9s", explain);
                if (strcmp(explain, "./expl") == 0) {
                    defination();
                }
            }
            else if (strcmp(command, "pwd")     == 0) { run_pwd();     }
            else if (strcmp(command, "date")    == 0) { run_date();    }
            else if (strcmp(command, "ls")      == 0) { run_ls();      }
            else if (strcmp(command, "whoami")  == 0) { run_whoami();  }
            else if (strcmp(command, "mkdir")   == 0) { run_mkdir();   }
            else if (strcmp(command, "echo")    == 0) { run_echo();    }
            else if (strcmp(command, "clear")   == 0) { run_clear();   }
            else if (strcmp(command, "cd")      == 0) { run_cd();      }
            else if (strcmp(command, "history") == 0) { run_history(); }
            else if (strcmp(command, "exit")    == 0) {
                run_exit();
                enter_command = false; /* break the loop cleanly */
            }
        }
    }

    return 0;
}
