#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAX_LINE 80 /* The maximum length command */
#define HISTORY_LIMIT 10 /* The maximum history length */

// Parsed command representation
#define EXEC    1
#define EXIT    2
#define CDIR    3
#define HISTORY 4

struct history {
    int count;
    char bufs[HISTORY_LIMIT][MAX_LINE];
};

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

char* getHistory(struct history *his, int index) {
    if ((index > his->count) || (index < his->count - HISTORY_LIMIT) || (index < 0)) {
        return 0;
    }
    return his->bufs[index % HISTORY_LIMIT];
}

int printHistory(struct history *his) {
    if (his->count == 0) {
        printf("No commands in history.\n");
        return 1;
    }
    int i;
    for (i = his->count - 1; i >= his->count - HISTORY_LIMIT; i--) {
        char* buf = getHistory(his, i);
        if (buf) {
            printf("%d %s\n", i + 1, buf);
        }
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

int saveCmd(char *buf, struct history *his) {
    strcpy(his->bufs[(his->count) % HISTORY_LIMIT], buf);
    his->count = his->count + 1;
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
    if (strcmp(execCmd->args[0], "history") == 0) {
        execCmd->type = HISTORY;
    }
    return (struct cmd*)execCmd;
}

int runCmd(struct cmd* cmd, int *should_run, struct history *his) {
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
    case HISTORY:
        his->count--;
        printHistory(his);
        break;
    }
    return 0;
}

int main(void) {
    static char buf[MAX_LINE];
    struct history his;
    his.count = 0;
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
        saveCmd(buf, &his);
        struct cmd* cmd = parseCmd(buf);
        runCmd(cmd, &should_run, &his);
    }
    return 0;
}
