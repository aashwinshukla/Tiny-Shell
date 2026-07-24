/*
 * commands.c
 * ----------
 * Implements every shell command that Tiny Shell supports.
 * Each function maps 1-to-1 with a command the user can type.
 *
 * Cross-platform notes:
 *   - mkdir()  has a different signature on Windows vs Unix
 *   - clear    uses "cls" on Windows, "clear" on Unix/Linux
 *   - whoami   reads USERNAME on Windows, USER on Unix/Linux
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <direct.h>    /* _getcwd, _chdir, _mkdir on Windows */
    #include <windows.h>
    #define getcwd  _getcwd
    #define chdir   _chdir
#else
    #include <unistd.h>    /* getcwd, chdir on Unix/Linux */
    #include <dirent.h>    /* opendir, readdir, closedir */
    #include <sys/stat.h>  /* mkdir on Unix/Linux */
#endif

#include "../include/globals.h"
#include "../include/commands.h"

/* ------------------------------------------------------------------ */

/*
 * run_pwd()
 * ---------
 * Retrieves the current working directory using getcwd() and prints it.
 * Buffer size of 1024 bytes is enough for any realistic path.
 */
void run_pwd() {
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n\n", cwd);
    } else {
        perror("pwd error");
    }
}

/* ------------------------------------------------------------------ */

/*
 * run_date()
 * ----------
 * Gets the current calendar time via time() and converts it to a
 * human-readable string with ctime(). ctime() appends a newline, so
 * we add just one more blank line for spacing.
 */
void run_date() {
    time_t now = time(NULL);
    printf("%s\n", ctime(&now));
}

/* ------------------------------------------------------------------ */

/*
 * run_ls()
 * --------
 * Opens the current directory (".") with opendir() and iterates over
 * every entry. Hidden files (those starting with '.') are skipped to
 * keep the output clean, matching typical 'ls' behavior.
 *
 * Note: On Windows, dirent.h support depends on the build environment
 * (e.g. MinGW/MSYS2 provide it; MSVC does not).
 */
void run_ls() {
#ifdef _WIN32
    /* Use Windows FindFirstFile / FindNextFile API */
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile("*", &ffd);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("ls error: could not open directory.\n\n");
        return;
    }

    do {
        /* Skip hidden/system entries that start with '.' */
        if (ffd.cFileName[0] != '.') {
            printf("%s  ", ffd.cFileName);
        }
    } while (FindNextFile(hFind, &ffd) != 0);

    FindClose(hFind);
    printf("\n\n");
#else
    DIR *dir = opendir(".");
    struct dirent *entry;

    if (dir == NULL) {
        perror("ls error");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            printf("%s  ", entry->d_name);
        }
    }
    printf("\n\n");

    closedir(dir);
#endif
}

/* ------------------------------------------------------------------ */

/*
 * run_whoami()
 * ------------
 * Reads the username from environment variables.
 * Windows sets USERNAME; Unix/Linux sets USER.
 * Falls back to "Unknown user" if neither variable is found.
 */
void run_whoami() {
    char *username = getenv("USERNAME"); /* Windows */

    if (username == NULL) {
        username = getenv("USER");       /* Unix/Linux */
    }

    if (username != NULL) {
        printf("%s\n\n", username);
    } else {
        printf("Unknown user\n\n");
    }
}

/* ------------------------------------------------------------------ */

/*
 * run_mkdir()
 * -----------
 * Prompts the user for a directory name, then attempts to create it.
 * On Unix/Linux, 0777 sets full permissions (subject to umask).
 * On Windows, _mkdir() does not take a permissions argument.
 */
void run_mkdir() {
    char dirname[256];
    printf("Enter directory name: ");
    scanf(" %255s", dirname);

#ifdef _WIN32
    if (_mkdir(dirname) == 0) {
#else
    if (mkdir(dirname, 0777) == 0) {
#endif
        printf("Directory '%s' created successfully.\n\n", dirname);
    } else {
        perror("mkdir error");
        printf("\n");
    }
}

/* ------------------------------------------------------------------ */

/*
 * run_echo()
 * ----------
 * Reads a full line of text (including spaces) from stdin and prints it.
 * getchar() is called first to consume the newline left behind by the
 * previous scanf() in the main loop; without this, fgets would return
 * immediately with an empty string.
 */
void run_echo() {
    char text[512];
    printf("Enter text: ");

    getchar(); /* consume the trailing newline from the last scanf */

    if (fgets(text, sizeof(text), stdin) != NULL) {
        printf("%s\n", text);
    }
}

/* ------------------------------------------------------------------ */

/*
 * run_clear()
 * -----------
 * Clears the terminal screen.
 * Uses "cls" on Windows and "clear" on Unix/Linux.
 */
void run_clear() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* ------------------------------------------------------------------ */

/*
 * run_cd()
 * --------
 * Prompts for a directory path and changes the process's working directory
 * using chdir() (or _chdir on Windows via the macro defined above).
 * Subsequent pwd and ls calls will reflect the new location.
 */
void run_cd() {
    char path[512];
    printf("Enter directory path: ");
    scanf(" %511s", path);

    if (chdir(path) != 0) {
        perror("cd error");
        printf("\n");
    } else {
        printf("Changed directory to: %s\n\n", path);
    }
}

/* ------------------------------------------------------------------ */

/*
 * run_exit()
 * ----------
 * Prints a farewell message. The main loop checks for "exit" and sets
 * enter_command = false after calling this, ending the program cleanly.
 */
void run_exit() {
    printf("\nExiting Tiny Shell... Goodbye!\n\n");
}
