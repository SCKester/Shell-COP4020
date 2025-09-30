#include "lexer.h"
#include "prompt.h"
#include "path_search.h"
#include "ExternalCommand.h"
#include "command_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <unistd.h>  
#include <redirect.h>      	  
#include <sys/wait.h>        
#include <errno.h>      
#include "piping.h" 
#include "background.h"    
#include "InternalCommand.h"

int main() {

	backgroundProcs backProcs;

	backProcs.numActiveBackgroundProcesses = 0;
	backProcs.numBackgroundProcessesTotal = 0;

	for(int i = 0 ; i < 10 ; i++) {
		backProcs.activeBackgroundProcessPIDs[i] = 0;
		backProcs.activeBackgroundProcessesJobNums[i] = 0;
	}

	while (1) {

		prompt();

		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();

		tokenlist *tokens = get_tokens(input);

		expand_env_var(tokens);

        //turns ~ -> $HOME and ~/dir -> $HOME/dir.
        expand_tilde(tokens);

		if (tokens->size > 0 && is_internal_command(tokens)) {
    		int rc = run_internal_command(tokens, &backProcs); // cd/exit
    		if (rc == -1) { // exit
        		free(input);
        		free_tokens(tokens);
        		return 0; // end shell
    		}
    		if (rc == 1) ic_history_record(input);  // record only if success
    		free(input);
    		free_tokens(tokens);
    		continue; // skip external path
		}

        if (tokens->size > 0 && tokens->items[0]) {   

            const char *cmd = tokens->items[0];   

			if( strcmp(cmd , "jobs") == 0 ) { //user asking for jobs

				listBackgroundProcesses(&backProcs);
				goto end;
			}

            if (!is_builtin(cmd)) {      

                char *resolved = search_path_for_command(cmd); 

                if (resolved) {    

                    free(resolved);    

                } else {

					free(resolved);

                    // If it's a built-in we print nothing here; if not found in PATH, say so.
                    if (strchr(cmd, '/') == NULL)                 
                        printf("[resolve] command not found: %s\n", cmd); 
						goto end;
                }
            }
        }

		int status;

		for(int i = 0 ; i < 10; i++) {

			if(backProcs.activeBackgroundProcessPIDs[i] == 0){
				continue;
			}

			int sta = waitpid(backProcs.activeBackgroundProcessPIDs[i] , &status , WNOHANG);

			if(sta > 0) { //process has returned
				removeBackgroundProcess(&backProcs , backProcs.activeBackgroundProcessesJobNums[i]);
			}
		}

		int hasPiping = 0;
		int hasRedirects = 0;
		int sendToBack = 0; //sned current command to background

		for(int i = 0 ; i < tokens->size ; i++) {
			if(strcmp(tokens->items[i] , "<" ) == 0 || strcmp(tokens->items[i] , ">") == 0) {
				hasRedirects = 1;
			}

			if(strcmp(tokens->items[i] , "|") == 0) {
				hasPiping = 1;
			}

			if(strcmp(tokens->items[i] , "&") == 0) {
				sendToBack = 1;
				removeAmpersand(tokens); //remove special
			}
		}

		if (tokens->size > 0 && is_internal_command(tokens)) {
    		ic_history_record(input);                 // track for exit history
    		int rc = run_internal_command(tokens, &backProcs);
    		if (rc == -1) {                           // exit
     		   free(input);
    		    free_tokens(tokens);
    		    break;
   			}
   		 	free(input);
   		 	free_tokens(tokens);
    		continue;                                 // skip external path/exec
		}

		int ok = 0;
		if (hasPiping) { pipeCommands(tokens, &backProcs, sendToBack); ok = 1; }
		else if (hasRedirects) { redirectInput(tokens, &backProcs, sendToBack); ok = 1; }
		else { ok = ExternalCommand(tokens, &backProcs, sendToBack); }
		if (ok) ic_history_record(input);

		end:free(input);
		free_tokens(tokens);

		
	}

	return 0;
}

char *get_input(void) {
	char *buffer = NULL;
	int bufsize = 0;
	char line[5];
	while (fgets(line, 5, stdin) != NULL)
	{
		int addby = 0;
		char *newln = strchr(line, '\n');
		if (newln != NULL)
			addby = newln - line;
		else
			addby = 5 - 1;
		buffer = (char *)realloc(buffer, bufsize + addby);
		memcpy(&buffer[bufsize], line, addby);
		bufsize += addby;
		if (newln != NULL)
			break;
	}
	buffer = (char *)realloc(buffer, bufsize + 1);
	buffer[bufsize] = 0;
	return buffer;
}

tokenlist *new_tokenlist(void) {
	tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = (char **)malloc(sizeof(char *));
	tokens->items[0] = NULL; /* make NULL terminated */
	return tokens;
}

void add_token(tokenlist *tokens, char *item) {
	int i = tokens->size;

	tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *)malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);

	tokens->size += 1;
}

tokenlist *get_tokens(char *input) {
	char *buf = (char *)malloc(strlen(input) + 1);
	strcpy(buf, input);
	tokenlist *tokens = new_tokenlist();
	char *tok = strtok(buf, " ");
	while (tok != NULL)
	{
		add_token(tokens, tok);
		tok = strtok(NULL, " ");
	}
	free(buf);
	return tokens;
}

void free_tokens(tokenlist *tokens) {
	for (int i = 0; i < tokens->size; i++)
		free(tokens->items[i]);
	free(tokens->items);
	free(tokens);
}

