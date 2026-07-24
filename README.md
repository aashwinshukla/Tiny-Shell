# Tiny Shell

A minimal command-line shell written in C from scratch. Tiny Shell implements a small but practical set of Unix-style commands and runs as a standalone executable on both Linux/macOS and Windows.

---

## Features

- Clean read-validate-execute loop
- 11 built-in commands (see below)
- In-session command history
- Cross-platform support (Windows + Unix/Linux)
- Modular codebase split across logical files

---

## Commands

| Command   | Description                                              |
|-----------|----------------------------------------------------------|
| `./help`  | Show the list of all available commands                  |
| `pwd`     | Print the current working directory                      |
| `ls`      | List files and folders in the current directory          |
| `date`    | Display the current system date and time                 |
| `whoami`  | Show the name of the currently logged-in user            |
| `mkdir`   | Create a new directory (prompts for a name)              |
| `echo`    | Print a line of text you type (like Unix echo)           |
| `clear`   | Clear the terminal screen                                |
| `cd`      | Change the current working directory                     |
| `history` | Display all commands entered in the current session      |
| `exit`    | Quit Tiny Shell                                          |

> After typing `./help`, type `./expl` to see a description of each command.

---

## Project Structure

```
Tiny-Shell/
│
├── include/                  # Header files
│   ├── globals.h             # extern declarations for all global variables
│   └── commands.h            # Function declarations for commands and utilities
│
├── src/                      # Source files
│   ├── main.c                # Entry point — welcome banner + main loop
│   ├── globals.c             # Definitions of all global variables
│   ├── commands.c            # Implementation of every shell command
│   └── utils.c               # check(), help(), defination(), history helpers
│
├── Tiny-shell-code/          # Original single-file prototype (reference only)
│   └── main.c
│
├── Makefile                  # Build system
├── LICENSE
└── README.md
```

---

## Build & Run

### Linux / macOS

```bash
make
./tiny-shell
```

### Windows (MinGW / MSYS2)

```bash
mingw32-make
./tiny-shell.exe
```

### Manual compilation (no make)

```bash
gcc -Wall -std=c11 -Iinclude src/main.c src/globals.c src/commands.c src/utils.c -o tiny-shell
```

---

## How It Works

1. The shell prints a prompt (`tiny-shell>`) and waits for input.
2. `check()` validates the input against the known command set.
3. If valid, the command is logged to history and dispatched to its handler function.
4. The handler runs and control returns to the prompt.
5. Typing `exit` sets the loop flag to `false` and the program exits cleanly.

---

## Author

Built by Arjun as a systems programming project to understand how shells work at the C level.

---

## License

This project is licensed under the terms in the [LICENSE](LICENSE) file.
