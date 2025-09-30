#include "path_search.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   // access()
#include <sys/stat.h> // stat()
#include <limits.h>   // PATH_MAX
#include <stdio.h>    // fprintf (errors), optional

// Join dir + / + cmd safely.
// Returns malloc'd string or NULL on failure.
static char *join_path(const char *dir, const char *cmd) {    
    size_t ld = strlen(dir), lc = strlen(cmd);
    // allocate: dir + "/" + cmd + '\0'
    char *out = (char *)malloc(ld + 1 + lc + 1);
    if (!out) return NULL;
    memcpy(out, dir, ld);
    out[ld] = '/';
    memcpy(out + ld + 1, cmd, lc + 1);
    return out;
}

bool is_builtin(const char *cmd) {
    if (!cmd) return false;
    return (strcmp(cmd, "cd") == 0) ||
           (strcmp(cmd, "exit") == 0) ||
           (strcmp(cmd, "jobs") == 0);
}

// Search $PATH for an executable named cmd.                    
char *search_path_for_command(const char *cmd) {
    if (!cmd || *cmd == '\0') return NULL;

    // If the command includes /, do not search PATH;            
    // use it as-is. Return a copy.
    if (strchr(cmd, '/')) {
        char *copy = (char *)malloc(strlen(cmd) + 1);
        if (copy) strcpy(copy, cmd);
        return copy;
    }

    if (is_builtin(cmd)) return NULL;

    const char *path = getenv("PATH");
    if (!path || *path == '\0') return NULL;

    char *paths = (char *)malloc(strlen(path) + 1);
    if (!paths) return NULL;
    strcpy(paths, path);

    char *dir = strtok(paths, ":");
    while (dir) {
        // Empty entry in PATH means current directory.       
        const char *use_dir = (*dir == '\0') ? "." : dir;

        char *candidate = join_path(use_dir, cmd);
        if (candidate) {
            // Check if file exists and is executable.               
            if (access(candidate, X_OK) == 0) {
                free(paths);
                return candidate; // caller frees
            }
            free(candidate);
        }
        dir = strtok(NULL, ":");
    }

    free(paths);
    return NULL; // not found                                      
}
