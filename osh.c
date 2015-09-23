#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAX_LINE 80 /* The maximum length command */

char whitespace[] = " \t\r\n\v";

int isWhitespace(char ch) {
    return strchr(whitespace, ch) != NULL;
}

int handleAmpersand(char *arg) {
    int len = strlen(arg);
    if (arg[len - 1] == '&') {
        arg[len - 1] = '\0';
        return 1;
    }
    return 0;
}

int getCmd(char *buf, int buf_size) {
    memset(buf, 0, buf_size);
    gets(buf);
    if (buf[0] == 0) {
        return -1;
    }
    if (strlen(buf) > MAX_LINE) {
        perror("getCmd");
        return -1;
    }
    return 0;
}

int parseCmd(char *buf, char **args, int *include_ampersand) {
    int N = strlen(buf);
    if (N > MAX_LINE) {
        printf("Input is too long.");
    }
    int buf_p = 0;
    int arg_p = 0;
    while (buf_p < N) {
        while ( (buf_p < N) && (isWhitespace(buf[buf_p])) ) {
            buf[buf_p] = '\0';
            buf_p++;
        }
        if (buf_p >= N) {
            break;
        }
        args[arg_p] = buf + buf_p;
        arg_p++;
        while (!isWhitespace(buf[buf_p])) {
            buf_p++;
        }
    }
    *include_ampersand = handleAmpersand(args[arg_p - 1]);
    if (strlen(args[arg_p - 1]) == 0) {
        args[arg_p - 1] = NULL;
    } else {
        args[arg_p] = NULL;
    }
    return 0;
}

int execCmd(char **args, int include_ampersand) {
    if (!fork()) {
        execvp(args[0], args);
    } else {
        if (!include_ampersand) {
            wait(NULL);
        }
    }
    return 0;
}

int main(void) {
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    static char buf[MAX_LINE];
    int should_run = 1; /* flag to determine when to exit program */

    int i = 0;
    int include_ampersand = 0;
    for (i = 0; i < (MAX_LINE/2 + 1); i++) {
        args[i] = malloc(MAX_LINE);
    }

    while (should_run) {
        printf("osh>");
        fflush(stdout);
        /*
         * After reading user input, the steps are:
         * (1) fork a child process using fork()
         * (2) the child process will invoke execvp()
         * (3) if command included &, parent will invoke wait() */
        if (getCmd(buf, sizeof(buf)) < 0) {
            return 0;
        }
        parseCmd(buf, args, &include_ampersand);
        execCmd(args, include_ampersand);
    }
    return 0;
}
