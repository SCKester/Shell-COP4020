#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include "InternalCommand.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     
#include <errno.h>
#include <sys/wait.h>
#include <limits.h>   
#include "background.h" 

#define IC_MAX_CMDLEN 200
#define _POSIX_C_SOURCE 200809L
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// last 3 valid commands
static char hist[3][IC_MAX_CMDLEN + 1];
static int  hist_count = 0;

void ic_history_record(const char *line) {
    if (!line || !*line) return;
    size_t n = strlen(line);
    if (n > IC_MAX_CMDLEN) n = IC_MAX_CMDLEN;
    int slot = hist_count % 3;
    memcpy(hist[slot], line, n);
    hist[slot][n] = '\0';
    hist_count++;
}

static void print_history_on_exit(void) {
    if (hist_count == 0) { printf("No valid commands.\n"); return; }
    if (hist_count < 3) {
        int last = (hist_count - 1) % 3;
        printf("%s\n", hist[last]);
        return;
    }
    int last  = (hist_count - 1) % 3;
    int mid   = (hist_count - 2) % 3;
    int first = (hist_count - 3) % 3;
    printf("%s\n%s\n%s\n", hist[first], hist[mid], hist[last]);
}

// cd
static int do_cd(tokenlist *tokens) {
    if (tokens->size == 1) {
        const char *home = getenv("HOME");
        if (!home) { fprintf(stderr, "cd: HOME not set\n"); return 0; }
        if (chdir(home) != 0) { perror("cd"); return 0; }
    } else if (tokens->size == 2) {
        const char *target = tokens->items[1];
        if (!target || !*target) { fprintf(stderr, "cd: invalid target\n"); return 0; }
        if (chdir(target) != 0) { perror("cd"); return 0; }
    } else {
        fprintf(stderr, "cd: too many arguments\n");
        return 0;
    }

    // update PWD so prompt/env stays in sync
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd))) setenv("PWD", cwd, 1);
    return 1; /* success */
}

static void wait_all_background(backgroundProcs *procs) {
    int status;
    for (int i = 0; i < 10; i++) {
        pid_t pid = procs->activeBackgroundProcessPIDs[i];
        if (pid > 0) {
            while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {}
            removeBackgroundProcess(procs , procs->activeBackgroundProcessesJobNums[i]);
        }
    }
}

int is_internal_command(tokenlist *tokens) {
    if (!tokens || tokens->size == 0 || !tokens->items[0]) return 0;
    const char *cmd = tokens->items[0];
    return (strcmp(cmd, "cd") == 0) || (strcmp(cmd, "exit") == 0);
}

int run_internal_command(tokenlist *tokens, backgroundProcs *procs) {
    const char *cmd = tokens->items[0];

    if (strcmp(cmd, "cd") == 0)
        return do_cd(tokens);                 /* 1 on success, 0 on fail */

    if (strcmp(cmd, "exit") == 0) {
        wait_all_background(procs);           /* per spec */
        print_history_on_exit();              /* per spec */
        return -1;                            /* signal to exit shell */
    }

    return 0;
}
