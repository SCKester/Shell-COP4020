#pragma once
#include <stdbool.h>

/*Returns 1 if the command exists
  Returns 0 otherwise.*/ 
int command_exists(const char *cmd);

/*Returns a path to the command if it
  exists in $PATH or has '' in its name.
  Returns NULL if not found.
  Caller must free the returned string. */
char *get_command_path(const char *cmd);


//returns arguments to a command in the style execv expects of length input length
//unused slots are NULL
char** getArgs(char** input , int inputLength); 