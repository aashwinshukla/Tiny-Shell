# Tiny Shell

A minimal command-line shell written in C from scratch, with a custom **OpenGL/GLFW terminal UI**.

Instead of running inside your system terminal, Tiny Shell opens its own graphical window — a dark, styled terminal rendered with OpenGL 2D primitives and a bitmap font. All input, output, scrolling, and the blinking cursor are handled entirely in OpenGL.

---

## Screenshot

```
╔══════════════════════════════════════════════════╗
║              Tiny Shell  v1.0                    ║  ← gradient header
╠══════════════════════════════════════════════════╣
║  ========================================        ║
║         Welcome to Tiny Shell                    ║  ← colour-coded output
║  ========================================        ║
║  Type  ./help   to see all commands.             ║
║                                                  ║
║  tiny-shell> ls                                  ║
║  Makefile  README.md  include  src               ║
║                                                  ║
╠══════════════════════════════════════════════════╣
║  tiny-shell> _                                   ║  ← input bar + cursor
╚══════════════════════════════════════════════════╝
```

---

## Features

- OpenGL window — no system terminal required
- Gradient header bar with title
- Scrollable output pane (Up / Down / Page Up / Page Down)
- Colour-coded text (green prompt, cyan info, red errors, yellow success)
- Blinking block cursor
- In-session command history
- Cross-platform: Windows (MinGW), Linux, macOS

---

## Commands

| Command   | Description                                              |
|-----------|----------------------------------------------------------|
| `./help`  | Show the list of all available commands                  |
| `./expl`  | Show a brief description of each command                 |
| `pwd`     | Print the current working directory                      |
| `ls`      | List files and folders in the current directory          |
| `date`    | Display the current system date and time                 |
| `whoami`  | Show the name of the currently logged-in user            |
| `mkdir`   | Create a new directory (prompts for a name)              |
| `echo`    | Print a line of text you type                            |
| `clear`   | Clear the output pane                                    |
| `cd`      | Change the current working directory                     |
| `history` | Display all commands entered this session                |
| `exit`    | Quit Tiny Shell                                          |

---

## Project Structure

```
Tiny-Shell/
│
├── include/                  # Header files
│   ├── globals.h             # extern declarations for all global variables
│   ├── commands.h            # Function prototypes for commands + utilities
│   └── ui.h                  # OpenGL UI API declarations
│
├── src/                      # Source files
│   ├── main.c                # Entry point — init UI, run loop, shutdown
│   ├── globals.c             # Global variable definitions
│   ├── commands.c            # Shell command implementations
│   ├── utils.c               # check(), help(), defination(), history
│   └── ui.c                  # Full OpenGL/GLFW terminal UI
│
├── Tiny-shell-code/          # Original single-file prototype (reference only)
│   └── main.c                # ← do NOT compile this
│
├── Makefile                  # Cross-platform build system
├── LICENSE
└── README.md
```

---

## Dependencies

| Library   | Purpose                          |
|-----------|----------------------------------|
| GLFW 3    | Window creation + input events   |
| OpenGL    | 2D rendering                     |
| freeglut  | Bitmap font rendering            |

### Install dependencies

**Windows — MinGW/MSYS2**
```bash
pacman -S mingw-w64-x86_64-glfw mingw-w64-x86_64-freeglut
```

**Ubuntu / Debian**
```bash
sudo apt install libglfw3-dev freeglut3-dev libgl-dev
```

**macOS (Homebrew)**
```bash
brew install glfw freeglut
```

---

## Build & Run

```bash
# Linux / macOS
make
./tiny-shell

# Windows (MinGW/MSYS2)
mingw32-make
./tiny-shell.exe

# Manual (no make)
gcc -Wall -std=c11 -Iinclude \
    src/main.c src/globals.c src/commands.c src/utils.c src/ui.c \
    -lglfw3 -lfreeglut -lopengl32 -lgdi32 -lwinmm \
    -o tiny-shell
```

---

## How It Works

```
main()
  └── ui_init()          Creates GLFW window, prints welcome banner
  └── ui_run()           Main loop:
        ├── glfwPollEvents()
        ├── char_callback()  → appends typed chars to input buffer
        ├── key_callback()   → Enter submits, Backspace deletes, arrows scroll
        │     └── on_enter()
        │           ├── add_to_history()
        │           └── dispatch()  → routes to ui_run_pwd / ls / etc.
        └── render()
              ├── draw_output()   scrollback buffer with colour tags
              ├── draw_header()   gradient title bar
              └── draw_footer()   input bar + blinking cursor
  └── ui_shutdown()      Destroys window, frees GLFW
```

---

## Author

Built by Arjun as a systems + graphics programming project — combining a custom shell implementation with a hand-written OpenGL terminal renderer.

---

## License

This project is licensed under the terms in the [LICENSE](LICENSE) file.
