#include "../include/ExternalCommand.h"  
#include "../include/path_search.h"
#include "../include/command_utils.h"
#include "../include/background.h"
#include <unistd.h>        
#include <sys/wait.h>      
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// CHANGED: was void ExternalCommand
int ExternalCommand(tokenlist *tokens, backgroundProcs *procs, int sendToBack) {  
    if (!tokens || tokens->size == 0 || !tokens->items[0]) return 0;             

    const char *cmd = tokens->items[0];

    // Builtins are not executed here
    if (is_builtin(cmd)) return 0;

    // Resolve path
    char *prog_path = search_path_for_command(cmd);
    if (!prog_path) {
        if (strchr(cmd, '/') == NULL)
            fprintf(stderr, "command not found: %s\n", cmd);
        return 0;                                                                
    }

    // Build argv for execv
    char **args = (char **)malloc(sizeof(char *) * (tokens->size + 1));
    if (!args) { free(prog_path); return 0; }                                    

    for (int i = 0; i < tokens->size; i++) {
        if (i == 0) {
            args[i] = prog_path;          // argv[0] = resolved path
        } else {
            args[i] = tokens->items[i];   // pointer copy is fine
        }
    }
    args[tokens->size] = NULL;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        free(args);
        free(prog_path);
        return 0;                                  
    }

    if (pid == 0) {
        // Child
        execv(prog_path, args);
        // If execv returns, it failed
        perror("execv");
        _exit(127);
    }

    // Parent
    int status = 0;

    if (sendToBack) {                                                           
        // Track background job if your API supports it
        // If your signature differs, adjust this call.
        addBackgroundProcess(procs, pid, tokens);                               

        // Non-blocking check just to surface immediate failures
        if (waitpid(pid, &status, WNOHANG) < 0) {
            printf("Error: Background process %d exited with error...\n", (int)pid);
            free(args);
            free(prog_path);
            return 0; // fail
        }
        // Started successfully in background
        free(args);
        free(prog_path);
        return 1;                                                               
    } else {
        // Foreground: wait
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            free(args);
            free(prog_path);
            return 0; // fail
        }

        int ok = (WIFEXITED(status) && WEXITSTATUS(status) == 0);            
        free(args);
        free(prog_path);
        return ok ? 1 : 0; // success only on exit code 0
    }
}
