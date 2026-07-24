/*
 * main.c
 * ------
 * Entry point for Tiny Shell.
 *
 * This file is pure C and is intentionally tiny.
 * All it does is:
 *   1. Call ui_init()  — creates the ImGui window
 *   2. Call ui_run()   — runs until the user types 'exit'
 *   3. Call ui_shutdown() — cleans up
 *
 * The actual shell logic lives in:
 *   src/ui.cpp       — ImGui window, input handling, command dispatch
 *   src/commands.c   — command implementations (fallback/reference)
 *   src/utils.c      — check(), help(), history helpers
 *   src/globals.c    — global variable storage
 */

#include <stdio.h>
#include "../include/ui.h"

int main(void) {

    if (!ui_init()) {
        fprintf(stderr, "Failed to initialise Tiny Shell UI.\n");
        return 1;
    }

    ui_run();

    ui_shutdown();
    return 0;
}
