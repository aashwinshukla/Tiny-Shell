# ============================================================
# Makefile — Tiny Shell
# ============================================================
#
# Dependencies:
#   Windows (MinGW/MSYS2):
#     pacman -S mingw-w64-x86_64-glfw mingw-w64-x86_64-freeglut
#
#   Ubuntu / Debian:
#     sudo apt install libglfw3-dev freeglut3-dev libgl-dev
#
#   macOS (Homebrew):
#     brew install glfw freeglut
#
# Usage:
#   make            -> build (output: tiny-shell  or  tiny-shell.exe)
#   make clean      -> remove compiled output
#   make rebuild    -> clean then build fresh
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Iinclude

# ---------- Platform detection ----------
ifeq ($(OS),Windows_NT)
    TARGET   = tiny-shell.exe
    # MinGW/MSYS2 pkg-config names
    GL_FLAGS = -lglfw3 -lfreeglut -lopengl32 -lgdi32 -lwinmm
    RM       = del /Q
else
    TARGET   = tiny-shell
    UNAME   := $(shell uname -s)
    ifeq ($(UNAME),Darwin)
        # macOS — frameworks
        GL_FLAGS = $(shell pkg-config --libs glfw3) \
                   -framework OpenGL -framework GLUT
    else
        # Linux
        GL_FLAGS = $(shell pkg-config --libs glfw3) \
                   -lGL -lglut -lm
    endif
    RM = rm -f
endif

LDFLAGS = $(GL_FLAGS)

# ---------- Sources ----------
SRCS = src/main.c     \
       src/globals.c  \
       src/commands.c \
       src/utils.c    \
       src/ui.c

OBJS = $(SRCS:.c=.o)

# ---------- Targets ----------
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
	@echo ""
	@echo "  Build successful -> $(TARGET)"
	@echo ""

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
	@echo "Cleaned."

rebuild: clean all

.PHONY: all clean rebuild
