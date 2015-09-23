#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAX_LINE 80 /* The maximum length command */

char whitespace[] = " \t\r\n\v";

int isWhitespace(char ch) {
    return strchr(whitespace, ch) != NULL;
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

int parseCmd(char *buf, char **args, int *args_count) {
    int N = strlen(buf);
    if (N > MAX_LINE) {
        printf("Input is too long.");
    }
    int buf_p = 0;
    int arg_p = 0;
    while (buf_p < N) {
        while (isWhitespace(buf[buf_p])) {
            buf_p++;
        }
        if (buf_p >= N) {
            break;
        }
        int arg_start = buf_p;
        while (!isWhitespace(buf[buf_p])) {
            buf_p++;
        }
        int arg_len = buf_p - arg_start + 1;
        strncpy(args[arg_p], buf + arg_start, arg_len + 1);
        args[arg_p][arg_len] = '\0';
        arg_p++;
    }
    args[arg_p] = NULL;
    *args_count = arg_p;
    return 0;
}

int execCmd(char **args, int args_count) {
    int i;
    for (i = 0; i < args_count; i++) {
        printf("%s", args[i]);
    }
    if (!fork()) {
        execvp(args[0], args);
        /* char *a[] = {"ls", "/", NULL}; */
        /* execvp("ls", a); */
    } else {
        wait(NULL);
        args[args_count] = malloc(MAX_LINE);
    }
    return 0;
}

int main(void) {
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    static char buf[MAX_LINE];
    int should_run = 1; /* flag to determine when to exit program */

    int i = 0;
    int args_count = 0;
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
        parseCmd(buf, args, &args_count);
        execCmd(args, args_count);
    }
    return 0;
}
