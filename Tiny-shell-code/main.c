/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║              THIS FOLDER IS THE ORIGINAL PROTOTYPE           ║
 * ╚══════════════════════════════════════════════════════════════╝
 *
 * This was the first single-file implementation of Tiny Shell —
 * everything crammed into one main.c as a starting point.
 *
 * The project has since been properly structured. The active source
 * code now lives in:
 *
 *   src/main.c       ← entry point + main loop
 *   src/commands.c   ← all command implementations
 *   src/utils.c      ← check, help, history, defination
 *   src/globals.c    ← global variable definitions
 *   src/ui.c         ← OpenGL/GLFW terminal UI
 *
 *   include/globals.h   ← shared state declarations
 *   include/commands.h  ← function prototypes
 *   include/ui.h        ← UI function prototypes
 *
 * To build and run the project:
 *
 *   make
 *   ./tiny-shell
 *
 * This file is kept for historical reference only.
 * Do NOT compile this file — it will not build.
 */
