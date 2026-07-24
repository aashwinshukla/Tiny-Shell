/*
 * ui.h
 * ----
 * Declarations for the OpenGL/GLFW terminal UI layer.
 *
 * The UI renders a dark terminal window using OpenGL primitives and
 * a bitmap font. It provides:
 *   - A styled header bar with the Tiny Shell title
 *   - A scrollable output pane showing command results
 *   - An input line at the bottom with a blinking cursor
 *   - Colour-coded text (prompt, output, errors)
 *
 * The shell logic (check, dispatch, commands) is completely separate —
 * ui.c calls into commands.c/utils.c the same way main.c did before.
 */

#ifndef UI_H
#define UI_H

/* Maximum characters in a single output line */
#define UI_LINE_MAX_LEN  256

/* Maximum lines kept in the scrollback buffer */
#define UI_MAX_LINES     512

/* Maximum characters the user can type in one input */
#define UI_INPUT_MAX_LEN 255

/* Colour tags used when appending output lines */
typedef enum {
    COLOR_NORMAL  = 0,   /* white  — standard output            */
    COLOR_PROMPT  = 1,   /* green  — the "tiny-shell>" prompt   */
    COLOR_ERROR   = 2,   /* red    — error messages             */
    COLOR_INFO    = 3,   /* cyan   — help / info text           */
    COLOR_SUCCESS = 4    /* yellow — success confirmations      */
} LineColor;

/*
 * ui_init()
 * ---------
 * Creates the GLFW window, initialises OpenGL, loads the bitmap font,
 * and sets up the output buffer. Must be called before any other ui_ function.
 * Returns 1 on success, 0 on failure.
 */
int ui_init(const char *title, int width, int height);

/*
 * ui_run()
 * --------
 * Enters the main render + event loop. Blocks until the window is closed
 * or the user types 'exit'. This replaces the while(enter_command) loop
 * that was previously in main.c.
 */
void ui_run(void);

/*
 * ui_shutdown()
 * -------------
 * Destroys the GLFW window and frees all UI resources. Call after ui_run()
 * returns.
 */
void ui_shutdown(void);

/*
 * ui_print()
 * ----------
 * Appends a line of text to the scrollback buffer with the given colour tag.
 * Called by command functions instead of printf().
 */
void ui_print(const char *text, LineColor color);

/*
 * ui_print_prompt()
 * -----------------
 * Appends the styled "tiny-shell> " prompt line to the output buffer.
 */
void ui_print_prompt(void);

#endif /* UI_H */
