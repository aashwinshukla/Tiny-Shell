/*
 * ui.c
 * ----
 * OpenGL/GLFW terminal UI for Tiny Shell.
 *
 * Renders a dark terminal window with:
 *   - A gradient header bar (title + version)
 *   - A scrollable output pane with colour-coded text
 *   - A bottom input bar with blinking cursor
 *   - Keyboard input handling (printable chars, backspace, enter)
 *
 * Text is drawn using GLFW's built-in bitmap font via glutBitmapCharacter.
 * We use GLUT/freeglut for font rendering only — GLFW handles the window.
 *
 * Coordinate system: bottom-left origin (standard OpenGL 2D ortho).
 *
 * Dependencies: GLFW3, OpenGL, freeglut (or GLUT)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
  #include <windows.h>
#endif

#include <GLFW/glfw3.h>
#include <GL/gl.h>

/* freeglut / GLUT for bitmap font rendering */
#ifdef _WIN32
  #include <GL/freeglut.h>
#else
  #include <GL/glut.h>
#endif

#include "../include/ui.h"
#include "../include/globals.h"
#include "../include/commands.h"

/* ================================================================
 *  Internal types
 * ================================================================ */

/* One line in the scrollback buffer */
typedef struct {
    char      text[UI_LINE_MAX_LEN];
    LineColor color;
} OutputLine;

/* ================================================================
 *  Internal state
 * ================================================================ */

static GLFWwindow  *g_window       = NULL;
static int          g_win_w        = 900;
static int          g_win_h        = 600;

/* Scrollback buffer */
static OutputLine   g_lines[UI_MAX_LINES];
static int          g_line_count   = 0;
static int          g_scroll_offset = 0;   /* lines scrolled up from bottom */

/* Current input being typed */
static char         g_input[UI_INPUT_MAX_LEN + 1] = {0};
static int          g_input_len   = 0;

/* Cursor blink state */
static double       g_blink_time  = 0.0;
static int          g_cursor_vis  = 1;

/* Layout constants (pixels) */
#define HEADER_H      48
#define FOOTER_H      36
#define LINE_H        18
#define MARGIN_X      12
#define FONT_W        9    /* approximate width of GLUT_BITMAP_9_BY_15 */

/* ================================================================
 *  Colour helpers
 * ================================================================ */

/* Set the GL draw colour for a given LineColor tag */
static void set_line_color(LineColor c) {
    switch (c) {
        case COLOR_PROMPT:  glColor3f(0.35f, 0.95f, 0.35f); break; /* green  */
        case COLOR_ERROR:   glColor3f(0.95f, 0.30f, 0.30f); break; /* red    */
        case COLOR_INFO:    glColor3f(0.30f, 0.85f, 0.95f); break; /* cyan   */
        case COLOR_SUCCESS: glColor3f(0.95f, 0.85f, 0.20f); break; /* yellow */
        default:            glColor3f(0.90f, 0.90f, 0.90f); break; /* white  */
    }
}

/* Draw a filled rectangle (bottom-left origin) */
static void draw_rect(float x, float y, float w, float h,
                      float r, float g, float b, float a) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
        glVertex2f(x,     y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x,     y + h);
    glEnd();
}

/* Draw a gradient rectangle — top colour blends into bottom colour */
static void draw_rect_gradient(float x, float y, float w, float h,
                                float r1,float g1,float b1,   /* bottom */
                                float r2,float g2,float b2) { /* top    */
    glBegin(GL_QUADS);
        glColor3f(r1, g1, b1); glVertex2f(x,     y);
        glColor3f(r1, g1, b1); glVertex2f(x + w, y);
        glColor3f(r2, g2, b2); glVertex2f(x + w, y + h);
        glColor3f(r2, g2, b2); glVertex2f(x,     y + h);
    glEnd();
}

/* ================================================================
 *  Text rendering
 * ================================================================ */

/*
 * draw_string()
 * Renders a null-terminated string at pixel position (x, y)
 * using GLUT's 9x15 bitmap font.  y is the baseline.
 */
static void draw_string(float x, float y, const char *str) {
    glRasterPos2f(x, y);
    for (const char *c = str; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, (unsigned char)*c);
    }
}

/* Measure pixel width of a string in the 9x15 font */
static int string_width(const char *str) {
    int w = 0;
    for (const char *c = str; *c != '\0'; c++) {
        w += glutBitmapWidth(GLUT_BITMAP_9_BY_15, (unsigned char)*c);
    }
    return w;
}

/* ================================================================
 *  Output buffer helpers
 * ================================================================ */

/*
 * ui_print()
 * ----------
 * Public API — appends one line to the scrollback buffer.
 * Long lines are word-wrapped at the window's text width.
 * If the buffer is full the oldest line is dropped (ring behaviour).
 */
void ui_print(const char *text, LineColor color) {
    /* Max chars that fit in the output pane */
    int max_chars = (g_win_w - 2 * MARGIN_X) / FONT_W;
    if (max_chars < 1) max_chars = 80;

    int len = (int)strlen(text);

    /* If it fits on one line, just append */
    if (len <= max_chars) {
        int idx = g_line_count % UI_MAX_LINES;
        strncpy(g_lines[idx].text, text, UI_LINE_MAX_LEN - 1);
        g_lines[idx].text[UI_LINE_MAX_LEN - 1] = '\0';
        g_lines[idx].color = color;
        g_line_count++;
        return;
    }

    /* Otherwise split into chunks */
    int start = 0;
    while (start < len) {
        int chunk = (len - start < max_chars) ? (len - start) : max_chars;
        int idx   = g_line_count % UI_MAX_LINES;
        strncpy(g_lines[idx].text, text + start, chunk);
        g_lines[idx].text[chunk] = '\0';
        g_lines[idx].color = color;
        g_line_count++;
        start += chunk;
    }
}

/* Append the styled prompt line */
void ui_print_prompt(void) {
    ui_print("tiny-shell> ", COLOR_PROMPT);
}

/* Append a blank separator line */
static void ui_newline(void) {
    ui_print("", COLOR_NORMAL);
}

/* ================================================================
 *  Command dispatch (mirrors the loop that was in main.c)
 * ================================================================ */

/*
 * Redirect printf output into the UI buffer.
 * We capture output by temporarily redirecting stdout to a pipe,
 * running the command, then reading the pipe into ui_print().
 *
 * On Windows we use _popen/_pclose; on Unix popen/pclose.
 * For simplicity each command handler below calls ui_print directly
 * instead of using printf, so we don't need pipe capture.
 */

/* Forward-declare the UI-aware command wrappers defined below */
static void ui_run_pwd(void);
static void ui_run_date(void);
static void ui_run_ls(void);
static void ui_run_whoami(void);
static void ui_run_mkdir(void);
static void ui_run_echo(void);
static void ui_run_cd(void);
static void ui_run_history(void);
static void ui_run_help(void);
static void ui_run_defination(void);

/*
 * dispatch()
 * ----------
 * Takes the current g_input string, validates it, logs it, runs it.
 * Returns 0 if the user typed 'exit', 1 otherwise.
 */
static int dispatch(const char *cmd) {

    if (strcmp(cmd, "exit") == 0) {
        ui_print("Exiting Tiny Shell... Goodbye!", COLOR_SUCCESS);
        ui_newline();
        return 0;
    }

    if (strcmp(cmd, "./help") == 0)    { ui_run_help();     return 1; }
    if (strcmp(cmd, "./expl") == 0)    { ui_run_defination(); return 1; }
    if (strcmp(cmd, "pwd") == 0)       { ui_run_pwd();      return 1; }
    if (strcmp(cmd, "date") == 0)      { ui_run_date();     return 1; }
    if (strcmp(cmd, "ls") == 0)        { ui_run_ls();       return 1; }
    if (strcmp(cmd, "whoami") == 0)    { ui_run_whoami();   return 1; }
    if (strcmp(cmd, "mkdir") == 0)     { ui_run_mkdir();    return 1; }
    if (strcmp(cmd, "echo") == 0)      { ui_run_echo();     return 1; }
    if (strcmp(cmd, "clear") == 0) {
        g_line_count   = 0;
        g_scroll_offset = 0;
        ui_print("Screen cleared.", COLOR_INFO);
        return 1;
    }
    if (strcmp(cmd, "cd") == 0)        { ui_run_cd();       return 1; }
    if (strcmp(cmd, "history") == 0)   { ui_run_history();  return 1; }

    /* Unknown command */
    char err[UI_LINE_MAX_LEN];
    snprintf(err, sizeof(err), "[ERROR] Unknown command: '%s'  (type ./help)", cmd);
    ui_print(err, COLOR_ERROR);
    return 1;
}

/* ================================================================
 *  UI-aware command implementations
 *  (same logic as commands.c but output goes to ui_print instead of printf)
 * ================================================================ */

#ifdef _WIN32
  #include <direct.h>
  #include <windows.h>
  #define getcwd  _getcwd
  #define chdir   _chdir
#else
  #include <unistd.h>
  #include <dirent.h>
  #include <sys/stat.h>
#endif

/* -- pwd --------------------------------------------------------- */
static void ui_run_pwd(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        ui_print(cwd, COLOR_NORMAL);
    } else {
        ui_print("[ERROR] Could not get current directory.", COLOR_ERROR);
    }
    ui_newline();
}

/* -- date -------------------------------------------------------- */
static void ui_run_date(void) {
    time_t now = time(NULL);
    char  *s   = ctime(&now);
    /* ctime adds '\n', strip it */
    char   buf[64];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    int l = (int)strlen(buf);
    if (l > 0 && buf[l - 1] == '\n') buf[l - 1] = '\0';
    ui_print(buf, COLOR_NORMAL);
    ui_newline();
}

/* -- ls ---------------------------------------------------------- */
static void ui_run_ls(void) {
    char line[UI_LINE_MAX_LEN] = {0};
    int  pos = 0;

#ifdef _WIN32
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile("*", &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        ui_print("[ERROR] Could not open directory.", COLOR_ERROR);
        return;
    }
    do {
        if (ffd.cFileName[0] != '.') {
            int nlen = (int)strlen(ffd.cFileName);
            if (pos + nlen + 2 >= UI_LINE_MAX_LEN) {
                ui_print(line, COLOR_NORMAL);
                memset(line, 0, sizeof(line));
                pos = 0;
            }
            pos += snprintf(line + pos, sizeof(line) - pos, "%s  ", ffd.cFileName);
        }
    } while (FindNextFile(hFind, &ffd));
    FindClose(hFind);
#else
    DIR *dir = opendir(".");
    struct dirent *entry;
    if (!dir) { ui_print("[ERROR] ls failed.", COLOR_ERROR); return; }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        int nlen = (int)strlen(entry->d_name);
        if (pos + nlen + 2 >= UI_LINE_MAX_LEN) {
            ui_print(line, COLOR_NORMAL);
            memset(line, 0, sizeof(line));
            pos = 0;
        }
        pos += snprintf(line + pos, sizeof(line) - pos, "%s  ", entry->d_name);
    }
    closedir(dir);
#endif

    if (pos > 0) ui_print(line, COLOR_NORMAL);
    ui_newline();
}

/* -- whoami ------------------------------------------------------ */
static void ui_run_whoami(void) {
    char *u = getenv("USERNAME");
    if (!u) u = getenv("USER");
    ui_print(u ? u : "Unknown user", COLOR_NORMAL);
    ui_newline();
}

/* ----------------------------------------------------------------
 * mkdir, echo, cd:
 * These commands need user input after the initial command.
 * We handle them with a two-phase approach:
 *   Phase 0 — command entered → show prompt for argument
 *   Phase 1 — argument entered → execute
 * The g_pending_* variables track this state.
 * ---------------------------------------------------------------- */
typedef enum {
    PENDING_NONE = 0,
    PENDING_MKDIR,
    PENDING_ECHO,
    PENDING_CD
} PendingCmd;

static PendingCmd g_pending = PENDING_NONE;

static void ui_run_mkdir(void) {
    ui_print("Enter directory name:", COLOR_INFO);
    g_pending = PENDING_MKDIR;
}

static void ui_run_echo(void) {
    ui_print("Enter text:", COLOR_INFO);
    g_pending = PENDING_ECHO;
}

static void ui_run_cd(void) {
    ui_print("Enter directory path:", COLOR_INFO);
    g_pending = PENDING_CD;
}

/* Execute the pending command once the argument is supplied */
static void execute_pending(const char *arg) {
    char buf[UI_LINE_MAX_LEN];

    switch (g_pending) {
        case PENDING_MKDIR: {
#ifdef _WIN32
            int r = _mkdir(arg);
#else
            int r = mkdir(arg, 0777);
#endif
            if (r == 0) {
                snprintf(buf, sizeof(buf), "Directory '%s' created.", arg);
                ui_print(buf, COLOR_SUCCESS);
            } else {
                snprintf(buf, sizeof(buf), "[ERROR] Could not create '%s'.", arg);
                ui_print(buf, COLOR_ERROR);
            }
            break;
        }
        case PENDING_ECHO:
            ui_print(arg, COLOR_NORMAL);
            break;

        case PENDING_CD:
            if (chdir(arg) == 0) {
                snprintf(buf, sizeof(buf), "Changed directory to: %s", arg);
                ui_print(buf, COLOR_SUCCESS);
            } else {
                snprintf(buf, sizeof(buf), "[ERROR] Cannot cd to '%s'.", arg);
                ui_print(buf, COLOR_ERROR);
            }
            break;

        default: break;
    }

    g_pending = PENDING_NONE;
    ui_newline();
}

/* -- history ----------------------------------------------------- */
static void ui_run_history(void) {
    ui_print("------- Command History -------", COLOR_INFO);
    if (history_count == 0) {
        ui_print("  No commands yet.", COLOR_NORMAL);
    } else {
        char buf[UI_LINE_MAX_LEN];
        for (int i = 0; i < history_count; i++) {
            snprintf(buf, sizeof(buf), "  %2d: %s", i + 1, history_list[i]);
            ui_print(buf, COLOR_NORMAL);
        }
    }
    ui_print("-------------------------------", COLOR_INFO);
    ui_newline();
}

/* -- help -------------------------------------------------------- */
static void ui_run_help(void) {
    ui_print("--------------Command List--------------", COLOR_INFO);
    ui_print("  1.  ./help", COLOR_NORMAL);
    ui_print("  2.  pwd",    COLOR_NORMAL);
    ui_print("  3.  ls",     COLOR_NORMAL);
    ui_print("  4.  date",   COLOR_NORMAL);
    ui_print("  5.  whoami", COLOR_NORMAL);
    ui_print("  6.  mkdir",  COLOR_NORMAL);
    ui_print("  7.  echo",   COLOR_NORMAL);
    ui_print("  8.  exit",   COLOR_NORMAL);
    ui_print("  9.  clear",  COLOR_NORMAL);
    ui_print("  10. cd",     COLOR_NORMAL);
    ui_print("  11. history",COLOR_NORMAL);
    ui_print("----------------------------------------", COLOR_INFO);
    ui_print("  Type './expl' for descriptions.", COLOR_INFO);
    ui_newline();
}

/* -- defination -------------------------------------------------- */
static void ui_run_defination(void) {
    ui_print("-----------Command Descriptions----------", COLOR_INFO);
    ui_print("  ./help   - Show the list of all available commands", COLOR_NORMAL);
    ui_print("  pwd      - Print the current working directory",     COLOR_NORMAL);
    ui_print("  ls       - List files and folders here",             COLOR_NORMAL);
    ui_print("  date     - Display current system date and time",    COLOR_NORMAL);
    ui_print("  whoami   - Show the logged-in username",             COLOR_NORMAL);
    ui_print("  mkdir    - Create a new directory",                  COLOR_NORMAL);
    ui_print("  echo     - Print a line of text",                    COLOR_NORMAL);
    ui_print("  exit     - Quit Tiny Shell",                         COLOR_NORMAL);
    ui_print("  clear    - Clear the terminal screen",               COLOR_NORMAL);
    ui_print("  cd       - Change the working directory",            COLOR_NORMAL);
    ui_print("  history  - Show all commands entered this session",  COLOR_NORMAL);
    ui_print("-----------------------------------------", COLOR_INFO);
    ui_newline();
}

/* ================================================================
 *  Input handling
 * ================================================================ */

/*
 * on_enter()
 * ----------
 * Called when the user presses Enter in the input bar.
 * Echoes the typed text, then dispatches or handles pending state.
 */
static int g_running = 1;  /* set to 0 to close the window */

static void on_enter(void) {
    if (g_input_len == 0) return;

    /* Echo what was typed, styled as prompt+command */
    char echo_buf[UI_LINE_MAX_LEN];
    snprintf(echo_buf, sizeof(echo_buf), "tiny-shell> %s", g_input);
    ui_print(echo_buf, COLOR_PROMPT);

    if (g_pending != PENDING_NONE) {
        /* We're waiting for an argument (mkdir name, echo text, cd path) */
        execute_pending(g_input);
    } else {
        /* Log to history then dispatch */
        add_to_history(g_input);
        int keep_running = dispatch(g_input);
        if (!keep_running) g_running = 0;
    }

    /* Clear input buffer */
    memset(g_input, 0, sizeof(g_input));
    g_input_len    = 0;
    g_scroll_offset = 0; /* snap to bottom after every command */
}

/*
 * key_callback()
 * --------------
 * GLFW keyboard callback. Handles:
 *   - Printable ASCII → append to input buffer
 *   - Backspace       → delete last character
 *   - Enter           → submit command
 *   - Up/Down arrows  → scroll output
 *   - Page Up/Down    → fast scroll
 */
static void key_callback(GLFWwindow *window, int key, int scancode,
                         int action, int mods) {
    (void)scancode; (void)mods; (void)window;
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    if (key == GLFW_KEY_ENTER) {
        on_enter();
        return;
    }

    if (key == GLFW_KEY_BACKSPACE) {
        if (g_input_len > 0) {
            g_input[--g_input_len] = '\0';
        }
        return;
    }

    /* Scroll output pane */
    int visible_lines = (g_win_h - HEADER_H - FOOTER_H) / LINE_H;
    if (key == GLFW_KEY_UP)        { g_scroll_offset++;                    return; }
    if (key == GLFW_KEY_DOWN)      { if (g_scroll_offset > 0) g_scroll_offset--; return; }
    if (key == GLFW_KEY_PAGE_UP)   { g_scroll_offset += visible_lines / 2; return; }
    if (key == GLFW_KEY_PAGE_DOWN) {
        g_scroll_offset -= visible_lines / 2;
        if (g_scroll_offset < 0) g_scroll_offset = 0;
        return;
    }
}

/*
 * char_callback()
 * ---------------
 * GLFW Unicode character callback — fires for every printable character.
 * This correctly handles shifted keys, accents, etc.
 */
static void char_callback(GLFWwindow *window, unsigned int codepoint) {
    (void)window;
    if (g_input_len >= UI_INPUT_MAX_LEN) return;
    /* Only accept printable ASCII for now */
    if (codepoint >= 32 && codepoint <= 126) {
        g_input[g_input_len++] = (char)codepoint;
        g_input[g_input_len]   = '\0';
    }
}

/* Resize callback — update viewport and layout */
static void framebuffer_size_callback(GLFWwindow *window, int w, int h) {
    (void)window;
    g_win_w = w;
    g_win_h = h;
    glViewport(0, 0, w, h);
}

/* ================================================================
 *  Render
 * ================================================================ */

/*
 * draw_header()
 * -------------
 * Renders a gradient bar across the top of the window containing
 * the shell title and a decorative accent line.
 */
static void draw_header(void) {
    /* Gradient: dark teal at bottom → slightly lighter at top */
    draw_rect_gradient(0, g_win_h - HEADER_H, g_win_w, HEADER_H,
                       0.05f, 0.18f, 0.22f,   /* bottom */
                       0.08f, 0.28f, 0.35f);  /* top    */

    /* Accent line at the bottom edge of the header */
    draw_rect(0, g_win_h - HEADER_H, g_win_w, 2,
              0.20f, 0.80f, 0.80f, 1.0f);

    /* Title text — centred */
    const char *title = "  Tiny Shell  v1.0";
    int tw = string_width(title);
    glColor3f(0.90f, 0.98f, 0.98f);
    draw_string((float)(g_win_w / 2 - tw / 2),
                (float)(g_win_h - HEADER_H + 16),
                title);

    /* Subtle subtitle */
    glColor3f(0.50f, 0.80f, 0.80f);
    draw_string((float)MARGIN_X,
                (float)(g_win_h - HEADER_H + 6),
                "type ./help for commands");
}

/*
 * draw_footer()
 * -------------
 * Renders the input bar at the bottom: background, prompt label,
 * the typed text, and a blinking block cursor.
 */
static void draw_footer(void) {
    /* Background */
    draw_rect(0, 0, g_win_w, FOOTER_H,
              0.07f, 0.07f, 0.10f, 1.0f);

    /* Top border */
    draw_rect(0, FOOTER_H - 2, g_win_w, 2,
              0.20f, 0.80f, 0.80f, 1.0f);

    /* Prompt label */
    glColor3f(0.35f, 0.95f, 0.35f);
    float px = MARGIN_X;
    draw_string(px, 10.0f, "tiny-shell> ");
    px += (float)string_width("tiny-shell> ");

    /* Typed input */
    glColor3f(0.95f, 0.95f, 0.95f);
    draw_string(px, 10.0f, g_input);
    px += (float)string_width(g_input);

    /* Blinking cursor block */
    if (g_cursor_vis) {
        draw_rect(px, 4, FONT_W, LINE_H - 2,
                  0.35f, 0.95f, 0.35f, 0.85f);
    }
}

/*
 * draw_output()
 * -------------
 * Renders visible lines from the scrollback buffer between the header
 * and footer. Newest lines appear at the bottom; scroll up with arrows.
 */
static void draw_output(void) {
    /* Output area boundaries */
    float area_bottom = FOOTER_H + 4;
    float area_top    = (float)(g_win_h - HEADER_H - 4);
    int   visible     = (int)((area_top - area_bottom) / LINE_H);

    /* The line index to start drawing from (bottom = most recent) */
    int start = g_line_count - visible - g_scroll_offset;
    if (start < 0) start = 0;
    int end = start + visible;
    if (end > g_line_count) end = g_line_count;

    /* Clamp scroll so we can't scroll past the top */
    int max_scroll = g_line_count - visible;
    if (max_scroll < 0) max_scroll = 0;
    if (g_scroll_offset > max_scroll) g_scroll_offset = max_scroll;

    /* Draw each visible line bottom-up */
    for (int i = end - 1; i >= start; i--) {
        int   buf_idx = i % UI_MAX_LINES;
        float y = area_bottom + (float)((end - 1 - i) * LINE_H);
        set_line_color(g_lines[buf_idx].color);
        draw_string((float)MARGIN_X, y, g_lines[buf_idx].text);
    }

    /* Scroll indicator on the right edge when not at bottom */
    if (g_scroll_offset > 0) {
        float ind_y = area_bottom +
                      (float)(g_scroll_offset * (area_top - area_bottom)) /
                      (float)(g_line_count > 1 ? g_line_count : 1);
        draw_rect((float)(g_win_w - 6), ind_y, 4, 40,
                  0.20f, 0.80f, 0.80f, 0.7f);
    }
}

/*
 * render()
 * --------
 * Full frame render: background → output → header → footer.
 */
static void render(void) {
    /* Dark background */
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Set up 2D ortho projection (bottom-left origin) */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, g_win_w, 0, g_win_h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    draw_output();
    draw_header();
    draw_footer();
}

/* ================================================================
 *  Public API
 * ================================================================ */

/*
 * ui_init()
 * ---------
 * Initialises GLFW, creates the window, registers callbacks,
 * and prints the welcome banner into the output buffer.
 */
int ui_init(const char *title, int width, int height) {
    g_win_w = width;
    g_win_h = height;

    if (!glfwInit()) {
        fprintf(stderr, "[ui] glfwInit() failed.\n");
        return 0;
    }

    /* Request a plain OpenGL 2.1 context — compatible with all drivers */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    g_window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!g_window) {
        fprintf(stderr, "[ui] glfwCreateWindow() failed.\n");
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(g_window);
    glfwSwapInterval(1); /* vsync */

    /* Register callbacks */
    glfwSetKeyCallback(g_window,             key_callback);
    glfwSetCharCallback(g_window,            char_callback);
    glfwSetFramebufferSizeCallback(g_window, framebuffer_size_callback);

    /* Initialise freeglut font system (no window needed) */
    int   argc = 0;
    glutInit(&argc, NULL);

    /* Print welcome banner into the output buffer */
    ui_print("========================================", COLOR_INFO);
    ui_print("         Welcome to Tiny Shell          ", COLOR_SUCCESS);
    ui_print("========================================", COLOR_INFO);
    ui_print("  Type  ./help   to see all commands.  ", COLOR_NORMAL);
    ui_print("  Type  ./expl   for descriptions.     ", COLOR_NORMAL);
    ui_print("  Use   Up/Down  to scroll output.     ", COLOR_NORMAL);
    ui_print("========================================", COLOR_INFO);
    ui_newline();

    return 1;
}

/*
 * ui_run()
 * --------
 * Main loop: poll events, update cursor blink, render, swap buffers.
 * Returns when the window is closed or the user types 'exit'.
 */
void ui_run(void) {
    while (!glfwWindowShouldClose(g_window) && g_running) {
        glfwPollEvents();

        /* Cursor blink: toggle every 0.5 s */
        double now = glfwGetTime();
        if (now - g_blink_time >= 0.5) {
            g_cursor_vis  = !g_cursor_vis;
            g_blink_time  = now;
        }

        render();
        glfwSwapBuffers(g_window);
    }
}

/*
 * ui_shutdown()
 * -------------
 * Destroys the window and terminates GLFW.
 */
void ui_shutdown(void) {
    if (g_window) {
        glfwDestroyWindow(g_window);
        g_window = NULL;
    }
    glfwTerminate();
}
