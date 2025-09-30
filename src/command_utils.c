
#include "command_utils.h"
#include "path_search.h"
#include <stdlib.h>
#include <string.h>


// Return 1 if a command exists and is executable.

int command_exists(const char *cmd) {
    if (!cmd || *cmd == '\0') return 0;

    // Builtins count as "existing"
    if (is_builtin(cmd)) return 1;

    char *path = search_path_for_command(cmd);
    if (path) {
        free(path);
        return 1;
    }
    return 0;
}

 // Return path to a command or NULL.

 //CALLER MUST FREE
char *get_command_path(const char *cmd) {
    if (!cmd || *cmd == '\0') return NULL;

    if (is_builtin(cmd)) return NULL;

    return search_path_for_command(cmd);  // may be NULL
}

//returns array of length inputLength in format that execv expects - unused slots are NULL
//Must be freed by caller
char** getArgs(char** input , int inputLength) {

    char** args = (char**) malloc(sizeof(char*) * inputLength);

    char* commandPath = get_command_path(input[0]);

    args[0] = commandPath;

    int argCount = 1;

    for(int j = 0 ; j < inputLength ; j++) { //TODO: BAD

        if(j == 0) {
            continue;
        }

        if(strcmp(input[j], ">") == 0 || strcmp(input[j] , "<") == 0 || strcmp(input[j] , "|") == 0) {
            j++;
            break;
        }

        int len = strlen(input[j]) + 1;

        args[argCount] = (char*) malloc(sizeof(char) * (len));

        strcpy(args[argCount] , input[j]);

        argCount++;
    }

    args[argCount] = NULL;

    for(int i = argCount + 1 ; i < inputLength ; i++) {
        args[i] = NULL;
    }

    return args;
}
