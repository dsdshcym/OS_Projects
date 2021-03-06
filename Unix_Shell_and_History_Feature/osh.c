#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#define MAX_LINE 80 /* The maximum length command */
#define HISTORY_LIMIT 10 /* The maximum history length */
#define HISTORY_CAPACITY 11 /* The maximum history length */

// Parsed command representation
#define EXEC    1
#define EXIT    2
#define CDIR    3
#define HISTORY 4
#define EXECHIS 5

struct history {
    int count;
    char bufs[HISTORY_CAPACITY][MAX_LINE];
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

struct execHisCmd {
    int type;
    int include_ampersand;
    char *args[MAX_LINE / 2 + 1];
};

char* getHistory(struct history *his, int index);
int printHistory(struct history *his);
int execHistory(struct execHisCmd *execHisCmd, struct history *his, int *should_run);
int getCmd(char **buf);
int saveCmd(char *buf, struct history *his);
struct cmd* parseCmd(char *buf);
int runCmd(struct cmd* cmd, int *should_run, struct history *his);

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
    if ((index > his->count) || (index <= his->count - HISTORY_LIMIT) || (index <= 0)) {
        return 0;
    }
    return his->bufs[(index - 1) % HISTORY_CAPACITY];
}

int printHistory(struct history *his) {
    if (his->count == 0) {
        printf("No commands in history.\n");
        return 1;
    }
    int i;
    for (i = his->count; i > his->count - HISTORY_LIMIT; i--) {
        char* buf = getHistory(his, i);
        if (buf) {
            printf("%d %s\n", i, buf);
        }
    }
    return 0;
}

int execHistory(struct execHisCmd *execHisCmd, struct history *his, int *should_run) {
    int len = strlen(execHisCmd->args[0]);
    if (len == 0) {
        printf("Syntex error.\n");
        return 1;
    }
    int cmd_index = -1;
    int is_bang = execHisCmd->args[0][0] == '!';
    if (is_bang) {
        if (len > 1) {
            printf("Syntex error.\n");
            return 1;
        }
        cmd_index = his->count;
    } else {
        cmd_index = atoi(execHisCmd->args[0]);
    }
    char *buf = getHistory(his, cmd_index);
    if (buf) {
        printf("%s\n", buf);
        char *new_buf = malloc(sizeof(*buf));
        strcpy(new_buf, buf);
        saveCmd(new_buf, his);
        runCmd(parseCmd(new_buf), should_run, his);
    }
    else {
        printf("No commands in history.\n");
        return 2;
    }
    return 0;
}

int getCmd(char **buf) {
    char* input, shell_prompt[100];
    snprintf(shell_prompt, sizeof(shell_prompt), "%s:%s osh$ ", getenv("USER"), getcwd(NULL, 1024));
    input = readline(shell_prompt);
    if (!input) {
        exit(0);
    }
    add_history(input);
    *buf = input;
    return 0;
}

int saveCmd(char *buf, struct history *his) {
    strcpy(his->bufs[(his->count) % HISTORY_CAPACITY], buf);
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
    if (execCmd->args[0][0] == '!') {
        execCmd->type = EXECHIS;
        execCmd->args[0]++;
    }
    return (struct cmd*)execCmd;
}

int runCmd(struct cmd* cmd, int *should_run, struct history *his) {
    struct execCmd* execCmd;
    struct cdirCmd* cdirCmd;

    switch (cmd->type) {
    case EXEC:
        execCmd = (struct execCmd*)cmd;
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
        }
        if (pid == 0) {
            if (execvp(execCmd->args[0], execCmd->args) == -1) {
                perror("execvp");
            };
        }
        if (pid > 0) {
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
        if (chdir(cdirCmd->args[1]) < 0) {
            perror("cd");
        };
        break;
    case HISTORY:
        his->count--;
        printHistory(his);
        break;
    case EXECHIS:
        his->count--;
        execHistory((struct execHisCmd*)cmd, his, should_run);
        break;
    }
    return 0;
}

int main(void) {
    char* buf = NULL;
    struct history his;
    his.count = 0;
    int should_run = 1; /* flag to determine when to exit program */

    while (should_run) {
        fflush(stdout);
        /*
         * After reading user input, the steps are:
         * (1) fork a child process using fork()
         * (2) the child process will invoke execvp()
         * (3) if command included &, parent will invoke wait() */
        getCmd(&buf);
        saveCmd(buf, &his);
        struct cmd* cmd = parseCmd(buf);
        runCmd(cmd, &should_run, &his);
        free(buf);
    }
    return 0;
}
