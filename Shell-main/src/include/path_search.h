#pragma once
#include <stdbool.h>

/* Returns a newly-allocated absolute path to the command if found.
If cmd contains '/', returns a copy of cmd (caller frees).
If not found, returns NULL.*/
char *search_path_for_command(const char *cmd);

bool is_builtin(const char *cmd); 
