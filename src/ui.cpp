/*
 * ui.cpp
 * ------
 * Dear ImGui terminal UI for Tiny Shell.
 * Backend: Win32 window + DirectX 9 renderer.
 *
 * No external library installation needed — ImGui source files are
 * bundled in third_party/imgui/ and compiled alongside this file.
 *
 * What this file does:
 *   - Opens a Win32 window (pure Windows API, no GLFW needed)
 *   - Sets up a DirectX 9 device to draw into that window
 *   - Hands DirectX 9 to ImGui so ImGui can draw widgets
 *   - Renders a terminal-style layout every frame:
 *       * A dark scrollable output area showing command results
 *       * A styled header bar
 *       * An input box at the bottom where you type commands
 *   - On Enter: validates the command, logs it to history, runs it
 */

/* ── Windows + DirectX 9 headers ─────────────────────────────────── */
#include <windows.h>
#include <d3d9.h>

/* ── ImGui core ──────────────────────────────────────────────────── */
#include "../third_party/imgui/imgui.h"
#include "../third_party/imgui/imgui_impl_win32.h"
#include "../third_party/imgui/imgui_impl_dx9.h"

/* ── Our own shell headers ───────────────────────────────────────── */
/*
 * globals.h and commands.h are C headers but we're in a .cpp file,
 * so we wrap them in extern "C" to tell the C++ compiler not to
 * mangle their names (C++ name-mangles functions; C does not).
 */
extern "C" {
    #include "../include/globals.h"
    #include "../include/commands.h"
}
#include "../include/ui.h"

/* ── Standard library ────────────────────────────────────────────── */
#include <cstring>
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>

/* ════════════════════════════════════════════════════════════════════
 *  Output buffer
 *  Each line the shell produces is stored here as a coloured string.
 *  ImGui reads this every frame and draws them in the output pane.
 * ════════════════════════════════════════════════════════════════════ */

/* Colour for a line of output */
enum LineColor {
    COL_NORMAL  = 0,   /* white  */
    COL_PROMPT  = 1,   /* green  */
    COL_ERROR   = 2,   /* red    */
    COL_INFO    = 3,   /* cyan   */
    COL_SUCCESS = 4    /* yellow */
};

/* One entry in the scrollback buffer */
struct OutputLine {
    std::string text;
    LineColor   color;
};

static std::vector<OutputLine> g_output;   /* the scrollback buffer    */
static bool                    g_scroll_to_bottom = false; /* snap flag */

/* Append one line to the output buffer */
static void print_line(const char *text, LineColor color) {
    g_output.push_back({ std::string(text), color });
    g_scroll_to_bottom = true;   /* after any new output, scroll down  */
}

/* Shorthand helpers */
static void print_normal (const char *s) { print_line(s, COL_NORMAL);  }
static void print_info   (const char *s) { print_line(s, COL_INFO);    }
static void print_error  (const char *s) { print_line(s, COL_ERROR);   }
static void print_success(const char *s) { print_line(s, COL_SUCCESS); }
static void print_prompt (const char *s) { print_line(s, COL_PROMPT);  }
static void newline      ()              { print_line("", COL_NORMAL);  }

/* Convert a LineColor tag to an ImGui RGBA colour */
static ImVec4 line_color_to_imvec(LineColor c) {
    switch (c) {
        case COL_PROMPT:  return ImVec4(0.35f, 0.95f, 0.35f, 1.0f); /* green  */
        case COL_ERROR:   return ImVec4(0.95f, 0.35f, 0.35f, 1.0f); /* red    */
        case COL_INFO:    return ImVec4(0.30f, 0.85f, 0.95f, 1.0f); /* cyan   */
        case COL_SUCCESS: return ImVec4(0.95f, 0.85f, 0.20f, 1.0f); /* yellow */
        default:          return ImVec4(0.90f, 0.90f, 0.90f, 1.0f); /* white  */
    }
}

/* ════════════════════════════════════════════════════════════════════
 *  Pending command state
 *
 *  Some commands (mkdir, echo, cd) need a second input from the user
 *  after the initial command word.  Instead of blocking (like scanf
 *  would), we set g_pending and show a custom prompt next frame.
 *  When the user hits Enter again, execute_pending() handles it.
 * ════════════════════════════════════════════════════════════════════ */

enum PendingCmd { PENDING_NONE, PENDING_MKDIR, PENDING_ECHO, PENDING_CD };
static PendingCmd g_pending = PENDING_NONE;

/* ════════════════════════════════════════════════════════════════════
 *  UI-aware command implementations
 *  Same logic as commands.c but output goes to print_line() instead
 *  of printf() so it appears inside the ImGui window.
 * ════════════════════════════════════════════════════════════════════ */

#ifdef _WIN32
  #include <direct.h>
  #define getcwd  _getcwd
  #define chdir   _chdir
#else
  #include <unistd.h>
  #include <dirent.h>
  #include <sys/stat.h>
#endif

static void cmd_pwd() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)))
        print_normal(cwd);
    else
        print_error("[ERROR] Could not get current directory.");
    newline();
}

static void cmd_date() {
    time_t now = time(NULL);
    char  *s   = ctime(&now);
    char   buf[64];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    /* strip the trailing newline ctime adds */
    int l = (int)strlen(buf);
    if (l > 0 && buf[l-1] == '\n') buf[l-1] = '\0';
    print_normal(buf);
    newline();
}

static void cmd_ls() {
    std::string line;
    WIN32_FIND_DATAA ffd;
    HANDLE h = FindFirstFileA("*", &ffd);
    if (h == INVALID_HANDLE_VALUE) {
        print_error("[ERROR] Could not list directory.");
        return;
    }
    do {
        if (ffd.cFileName[0] != '.') {
            if (!line.empty()) line += "   ";
            line += ffd.cFileName;
            /* wrap every ~80 chars */
            if ((int)line.size() > 80) {
                print_normal(line.c_str());
                line.clear();
            }
        }
    } while (FindNextFileA(h, &ffd));
    FindClose(h);
    if (!line.empty()) print_normal(line.c_str());
    newline();
}

static void cmd_whoami() {
    char *u = getenv("USERNAME");
    if (!u) u = getenv("USER");
    print_normal(u ? u : "Unknown user");
    newline();
}

static void cmd_history() {
    print_info("------- Command History -------");
    if (history_count == 0) {
        print_normal("  No commands yet.");
    } else {
        char buf[300];
        for (int i = 0; i < history_count; i++) {
            snprintf(buf, sizeof(buf), "  %2d: %s", i + 1, history_list[i]);
            print_normal(buf);
        }
    }
    print_info("-------------------------------");
    newline();
}

static void cmd_help() {
    print_info("--------------Command List--------------");
    print_normal("  ./help   show this list");
    print_normal("  ./expl   describe each command");
    print_normal("  pwd      current directory");
    print_normal("  ls       list files");
    print_normal("  date     current date/time");
    print_normal("  whoami   your username");
    print_normal("  mkdir    create a directory");
    print_normal("  echo     print text");
    print_normal("  clear    clear the output");
    print_normal("  cd       change directory");
    print_normal("  history  command history");
    print_normal("  exit     quit");
    print_info("----------------------------------------");
    newline();
}

static void cmd_expl() {
    print_info("-----------Command Descriptions----------");
    print_normal("  ./help   - Show the list of all commands");
    print_normal("  pwd      - Print the current directory path");
    print_normal("  ls       - List files and folders here");
    print_normal("  date     - Show current date and time");
    print_normal("  whoami   - Show your username");
    print_normal("  mkdir    - Create a new directory");
    print_normal("  echo     - Print a line of text");
    print_normal("  exit     - Quit Tiny Shell");
    print_normal("  clear    - Clear the output pane");
    print_normal("  cd       - Change working directory");
    print_normal("  history  - Show commands entered this session");
    print_info("-----------------------------------------");
    newline();
}

/* ════════════════════════════════════════════════════════════════════
 *  Command dispatch
 *  Called every time the user presses Enter.
 *  Returns false if the shell should exit, true otherwise.
 * ════════════════════════════════════════════════════════════════════ */

static bool g_running = true;

/*
 * execute_pending()
 * -----------------
 * The user just provided the argument for a two-step command
 * (mkdir needs a name, echo needs text, cd needs a path).
 */
static void execute_pending(const char *arg) {
    char buf[512];
    switch (g_pending) {
        case PENDING_MKDIR: {
            int r = _mkdir(arg);
            if (r == 0) {
                snprintf(buf, sizeof(buf), "Directory '%s' created.", arg);
                print_success(buf);
            } else {
                snprintf(buf, sizeof(buf), "[ERROR] Could not create '%s'.", arg);
                print_error(buf);
            }
            break;
        }
        case PENDING_ECHO:
            print_normal(arg);
            break;
        case PENDING_CD:
            if (chdir(arg) == 0) {
                snprintf(buf, sizeof(buf), "Changed directory to: %s", arg);
                print_success(buf);
            } else {
                snprintf(buf, sizeof(buf), "[ERROR] Cannot cd to '%s'.", arg);
                print_error(buf);
            }
            break;
        default: break;
    }
    g_pending = PENDING_NONE;
    newline();
}

/*
 * dispatch()
 * ----------
 * Routes a validated command string to the right handler.
 */
static void dispatch(const char *cmd) {
    /* Echo what was typed with a green prompt prefix */
    char echo_buf[300];
    snprintf(echo_buf, sizeof(echo_buf), "tiny-shell> %s", cmd);
    print_prompt(echo_buf);

    if      (strcmp(cmd, "exit")    == 0) { print_success("Goodbye!"); g_running = false; }
    else if (strcmp(cmd, "./help")  == 0) { cmd_help();    }
    else if (strcmp(cmd, "./expl")  == 0) { cmd_expl();    }
    else if (strcmp(cmd, "pwd")     == 0) { cmd_pwd();     }
    else if (strcmp(cmd, "date")    == 0) { cmd_date();    }
    else if (strcmp(cmd, "ls")      == 0) { cmd_ls();      }
    else if (strcmp(cmd, "whoami")  == 0) { cmd_whoami();  }
    else if (strcmp(cmd, "history") == 0) { cmd_history(); }
    else if (strcmp(cmd, "clear")   == 0) { g_output.clear(); }
    else if (strcmp(cmd, "mkdir")   == 0) { print_info("Enter directory name:"); g_pending = PENDING_MKDIR; }
    else if (strcmp(cmd, "echo")    == 0) { print_info("Enter text:");           g_pending = PENDING_ECHO;  }
    else if (strcmp(cmd, "cd")      == 0) { print_info("Enter directory path:"); g_pending = PENDING_CD;    }
    else {
        char err[300];
        snprintf(err, sizeof(err), "[ERROR] Unknown command: '%s'  (type ./help)", cmd);
        print_error(err);
    }
}

/*
 * on_enter()
 * ----------
 * Called when the user presses Enter in the input box.
 * Handles both normal commands and the pending argument state.
 */
static void on_enter(char *input_buf) {
    /* Trim leading/trailing whitespace */
    char *s = input_buf;
    while (*s == ' ') s++;
    int l = (int)strlen(s);
    while (l > 0 && s[l-1] == ' ') s[--l] = '\0';

    if (l == 0) { input_buf[0] = '\0'; return; }

    if (g_pending != PENDING_NONE) {
        /* Second-step input for mkdir / echo / cd */
        char echo_buf[300];
        snprintf(echo_buf, sizeof(echo_buf), "  > %s", s);
        print_normal(echo_buf);
        execute_pending(s);
    } else {
        /* Normal command — log to history then dispatch */
        add_to_history(s);
        dispatch(s);
    }

    input_buf[0] = '\0';   /* clear the input box */
}

/* ════════════════════════════════════════════════════════════════════
 *  DirectX 9 setup helpers
 * ════════════════════════════════════════════════════════════════════ */

static LPDIRECT3D9       g_d3d        = NULL;
static LPDIRECT3DDEVICE9 g_d3d_device = NULL;
static D3DPRESENT_PARAMETERS g_d3dpp  = {};

/*
 * CreateDeviceD3D()
 * -----------------
 * Creates the DirectX 9 device attached to our window.
 * The device is what actually draws pixels on screen.
 */
static bool CreateDeviceD3D(HWND hWnd) {
    g_d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!g_d3d) return false;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed               = TRUE;
    g_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat       = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE; /* vsync */

    HRESULT hr = g_d3d->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &g_d3dpp, &g_d3d_device);

    if (FAILED(hr)) return false;
    return true;
}

static void CleanupDeviceD3D() {
    if (g_d3d_device) { g_d3d_device->Release(); g_d3d_device = NULL; }
    if (g_d3d)        { g_d3d->Release();         g_d3d        = NULL; }
}

/* Called when the window is resized — recreate the swap chain */
static void ResetDevice() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    g_d3d_device->Reset(&g_d3dpp);
    ImGui_ImplDX9_CreateDeviceObjects();
}

/* Win32 message handler — ImGui needs to see mouse/keyboard events */
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg,
                               WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_SIZE:
            if (g_d3d_device && wParam != SIZE_MINIMIZED) {
                g_d3dpp.BackBufferWidth  = LOWORD(lParam);
                g_d3dpp.BackBufferHeight = HIWORD(lParam);
                ResetDevice();
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

/* ════════════════════════════════════════════════════════════════════
 *  Public API — ui_init, ui_run, ui_shutdown
 * ════════════════════════════════════════════════════════════════════ */

static HWND      g_hwnd      = NULL;
static WNDCLASSEXW g_wc      = {};

extern "C" int ui_init(void) {
    /* Register a Win32 window class */
    g_wc = { sizeof(g_wc), CS_CLASSDC, WndProc, 0L, 0L,
             GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
             L"TinyShell", NULL };
    RegisterClassExW(&g_wc);

    /* Create the window — 900x600, centred-ish */
    g_hwnd = CreateWindowW(
        L"TinyShell", L"Tiny Shell",
        WS_OVERLAPPEDWINDOW,
        100, 100, 900, 620,
        NULL, NULL, g_wc.hInstance, NULL);

    if (!CreateDeviceD3D(g_hwnd)) {
        CleanupDeviceD3D();
        UnregisterClassW(g_wc.lpszClassName, g_wc.hInstance);
        return 0;
    }

    ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hwnd);

    /* Initialise ImGui */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    /* Dark theme */
    ImGui::StyleColorsDark();

    /* Tweak colours to feel more like a terminal */
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding    = 0.0f;
    style.FrameRounding     = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.Colors[ImGuiCol_WindowBg]       = ImVec4(0.06f, 0.06f, 0.09f, 1.0f);
    style.Colors[ImGuiCol_FrameBg]        = ImVec4(0.10f, 0.10f, 0.14f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.15f, 0.20f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive]  = ImVec4(0.05f, 0.20f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg]    = ImVec4(0.06f, 0.06f, 0.09f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab]  = ImVec4(0.20f, 0.70f, 0.70f, 0.6f);

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX9_Init(g_d3d_device);

    /* Welcome banner */
    print_info("========================================");
    print_success("         Welcome to Tiny Shell         ");
    print_info("========================================");
    print_normal("  Type  ./help   to see all commands.");
    print_normal("  Type  ./expl   for descriptions.");
    print_info("========================================");
    newline();

    return 1;
}

extern "C" void ui_run(void) {
    static char input_buf[256] = {};

    MSG msg;
    while (g_running) {
        /* Process all pending Windows messages (keyboard, mouse, resize…) */
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) g_running = false;
        }
        if (!g_running) break;

        /* ── Start new ImGui frame ── */
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        /* ── Get the actual window client size ── */
        RECT rc;
        GetClientRect(g_hwnd, &rc);
        float win_w = (float)(rc.right  - rc.left);
        float win_h = (float)(rc.bottom - rc.top);

        /* ── Full-screen borderless ImGui window ── */
        ImGui::SetNextWindowPos (ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(win_w, win_h));
        ImGui::Begin("##main",
            NULL,
            ImGuiWindowFlags_NoTitleBar        |
            ImGuiWindowFlags_NoResize          |
            ImGuiWindowFlags_NoMove            |
            ImGuiWindowFlags_NoScrollbar       |
            ImGuiWindowFlags_NoSavedSettings);

        /* ── Header bar ── */
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.22f, 0.28f, 1.0f));
        ImGui::BeginChild("##header", ImVec2(0, 38), false,
                          ImGuiWindowFlags_NoScrollbar);
        ImGui::SetCursorPos(ImVec2(12, 10));
        ImGui::TextColored(ImVec4(0.9f,1.0f,1.0f,1.0f), "Tiny Shell  v1.0");
        ImGui::SameLine(0, 40);
        ImGui::TextColored(ImVec4(0.5f,0.8f,0.8f,1.0f),
                           "type ./help for commands");
        ImGui::EndChild();
        ImGui::PopStyleColor();

        /* ── Output scrollback pane ── */
        float footer_h = 38.0f;                    /* height of input bar  */
        float output_h = win_h - 38 - footer_h - 18; /* remaining space    */

        ImGui::BeginChild("##output", ImVec2(0, output_h), false);

        for (auto &line : g_output) {
            ImGui::TextColored(line_color_to_imvec(line.color),
                               "%s", line.text.c_str());
        }

        /* Auto-scroll to bottom when new output arrives */
        if (g_scroll_to_bottom) {
            ImGui::SetScrollHereY(1.0f);
            g_scroll_to_bottom = false;
        }
        ImGui::EndChild();

        ImGui::Separator();

        /* ── Input bar ── */
        /*
         * We want the input box to always have keyboard focus so the user
         * can just start typing without clicking first.
         */
        bool reclaim_focus = false;

        /* Show a different prompt when waiting for a second argument */
        const char *prompt_label = (g_pending != PENDING_NONE)
                                   ? "  >  ##input"
                                   : "tiny-shell>  ##input";

        ImGui::PushItemWidth(-1);
        ImGui::PushStyleColor(ImGuiCol_FrameBg,
                              ImVec4(0.10f, 0.10f, 0.14f, 1.0f));

        /* InputText returns true when Enter is pressed */
        if (ImGui::InputText(prompt_label, input_buf, sizeof(input_buf),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            on_enter(input_buf);
            reclaim_focus = true;
        }
        ImGui::PopStyleColor();
        ImGui::PopItemWidth();

        /* Keep keyboard focus on the input box every frame */
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1);

        ImGui::End();

        /* ── Render ── */
        g_d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

        D3DCOLOR clear_col = D3DCOLOR_RGBA(15, 15, 22, 255); /* dark bg */
        g_d3d_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                            clear_col, 1.0f, 0);

        if (g_d3d_device->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_d3d_device->EndScene();
        }
        g_d3d_device->Present(NULL, NULL, NULL, NULL);
    }
}

extern "C" void ui_shutdown(void) {
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(g_hwnd);
    UnregisterClassW(g_wc.lpszClassName, g_wc.hInstance);
}
