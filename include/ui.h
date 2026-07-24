/*
 * ui.h
 * ----
 * Public interface for the Dear ImGui terminal UI.
 *
 * Any file that wants to start, run, or print to the UI
 * just includes this header and calls these three functions.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ui_init()
 * ---------
 * Creates the Win32 window, sets up DirectX 9, and initialises ImGui.
 * Prints the welcome banner into the output buffer.
 * Returns 1 on success, 0 on failure.
 */
int ui_init(void);

/*
 * ui_run()
 * --------
 * Enters the render loop. Blocks here until the user types 'exit'
 * or closes the window. All input handling and command dispatch
 * happens inside this loop.
 */
void ui_run(void);

/*
 * ui_shutdown()
 * -------------
 * Cleans up ImGui, DirectX 9, and the Win32 window.
 * Call this after ui_run() returns.
 */
void ui_shutdown(void);

#ifdef __cplusplus
}
#endif
