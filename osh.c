#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAX_LINE 80 /* The maximum length command */

// Parsed command representation
#define EXEC 1
#define EXIT 2
#define CDIR 3

struct cmd {
    int type;
    int include_ampersand;
};

struct execCmd {
    int type;
    int include_ampersand;
    char *args[MAX_LINE / 2 + 1];
};

struct exitCmd {
    int type;
    int include_ampersand;
    char *args[MAX_LINE / 2 + 1];
};

struct cdirCmd {
    int type;
    int include_ampersand;
    char *args[MAX_LINE / 2 + 1];
};

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

struct cmd* parseCmd(char *buf) {
    int N = strlen(buf);
    if (N > MAX_LINE) {
        printf("Input is too long.");
    }
    struct execCmd *execCmd = malloc(sizeof(struct execCmd));
    execCmd->type = EXEC;
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
        execCmd->args[arg_p] = buf + buf_p;
        arg_p++;
        while (!isWhitespace(buf[buf_p])) {
            buf_p++;
        }
    }
    execCmd->include_ampersand = handleAmpersand(execCmd->args[arg_p - 1]);
    if (strlen(execCmd->args[arg_p - 1]) == 0) {
        execCmd->args[arg_p - 1] = NULL;
    } else {
        execCmd->args[arg_p] = NULL;
    }
    if (strcmp(execCmd->args[0], "exit") == 0) {
        execCmd->type = EXIT;
    }
    if (strcmp(execCmd->args[0], "cd") == 0) {
        execCmd->type = CDIR;
    }
    return (struct cmd*)execCmd;
}

int runCmd(struct cmd* cmd, int *should_run) {
    struct execCmd* execCmd;
    struct cdirCmd* cdirCmd;

    switch (cmd->type) {
    case EXEC:
        execCmd = (struct execCmd*)cmd;
        if (!fork()) {
            execvp(execCmd->args[0], execCmd->args);
        } else {
            if (!execCmd->include_ampersand) {
                wait(NULL);
            }
        }
        break;
    case EXIT:
        *should_run = 0;
        break;
    case CDIR:
        cdirCmd = (struct cdirCmd*)cmd;
        chdir(cdirCmd->args[1]);
        break;
    }
    return 0;
}

int main(void) {
    static char buf[MAX_LINE];
    int should_run = 1; /* flag to determine when to exit program */

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
        struct cmd* cmd = parseCmd(buf);
        runCmd(cmd, &should_run);
    }
    return 0;
}
