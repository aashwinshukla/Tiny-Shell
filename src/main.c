/*
 * main.c
 * ------
 * Entry point for Tiny Shell.
 *
 * All it does now is:
 *   1. Initialise the OpenGL/GLFW terminal UI
 *   2. Hand control to the UI loop (which handles input + dispatch)
 *   3. Shut down cleanly when the loop exits
 *
 * Shell logic  → src/commands.c, src/utils.c
 * Global state → src/globals.c  (declared in include/globals.h)
 * UI layer     → src/ui.c       (declared in include/ui.h)
 */

#include <stdio.h>
#include "../include/ui.h"

int main(void) {

    /* Create the 900×600 terminal window */
    if (!ui_init("Tiny Shell", 900, 600)) {
        fprintf(stderr, "Failed to initialise Tiny Shell UI.\n");
        return 1;
    }

    /* Run until the user types 'exit' or closes the window */
    ui_run();

    /* Clean up GLFW resources */
    ui_shutdown();

    return 0;
}
