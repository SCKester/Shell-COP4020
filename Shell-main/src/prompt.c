#include "prompt.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

void expand_env_var(tokenlist * tokens) {
    for (int i = 0; i < tokens->size; i++) {
        char *token = tokens->items[i]; // pointer to the current token

        if (token[0] == '$') {  // Check if it starts with '$'
            char * env_var = &token[1]; // pointer to the string after the '$' (IE - USER)

            char * value = getenv(env_var); // get the environment of the environment variable 

            if(value != NULL) {
                free(tokens->items[i]);

                tokens->items[i] = malloc(strlen(value) + 1);

                strcpy(tokens->items[i], value);
            }
        }
}

}

void prompt() {
    char *env = getenv("PWD");
    char *name = getenv("USER"); 
    char *machine = getenv("MACHINE");

    printf("%s@%s:%s> ", name, machine, env);


    //THESE SHOULD NOT BE HERE - HUGH
    //char input[1024];
    //fgets(input, sizeof(input), stdin);
}

/*
 - If ~ , replace with $HOME
 - If starts with ~/ , replace with $HOME + (token+1)
 
 -If $HOME is unset, token unchanged.
 -Only applies when ~ first.
 -frees the old token string and replaces it with a new allocation.
*/
void expand_tilde(tokenlist *tokens) { 
    char *home = getenv("HOME");       
    if (!home) return;                 //Nothing to do if HOME is not set

    for (size_t i = 0; i < tokens->size; i++) {   
        char *tok = tokens->items[i];             
        if (!tok || tok[0] != '~') continue;      

        if (tok[1] == '\0') { // token is exactly ~          
            char *rep = (char *)malloc(strlen(home) + 1);
            if (!rep) continue;                               
            strcpy(rep, home);                                 
            free(tokens->items[i]);                          
            tokens->items[i] = rep;                            
        } else if (tok[1] == '/') { // starts with ~/"        
            const char *rest = tok + 1; // points to /...     
            size_t len = strlen(home) + strlen(rest) + 1;        
            char *rep = (char *)malloc(len);                     
            if (!rep) continue;                                 
            strcpy(rep, home);                                 
            strcat(rep, rest);                                 
            free(tokens->items[i]);                             
            tokens->items[i] = rep;                             
        }
        
    }
} // Part 3