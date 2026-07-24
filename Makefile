# ============================================================
# Makefile — Tiny Shell
# ============================================================
# Usage:
#   make          -> build the shell (output: tiny-shell)
#   make clean    -> remove compiled output
#   make rebuild  -> clean then build fresh
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Iinclude
TARGET  = tiny-shell
SRCS    = src/main.c src/commands.c src/utils.c src/globals.c
OBJS    = $(SRCS:.c=.o)

# Default target — compile everything
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
	@echo "Build successful -> $(TARGET)"

# Pattern rule: compile each .c to a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Remove all compiled artifacts
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "Cleaned build artifacts."

# Full rebuild from scratch
rebuild: clean all

.PHONY: all clean rebuild
