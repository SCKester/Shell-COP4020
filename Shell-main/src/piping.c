#include <stdio.h>
#include <unistd.h>
#include "piping.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "lexer.h"
#include <string.h>
#include "command_utils.h"
#include "background.h"


//returns pointer to array of child pids
//the array commands contains the commands to be executed with piping in order , num commnads in the number of them
//WILL NEED TO MODIFY TO SUPPORT BACHGROUND PROCESSES
//args is a 2-d array of arguments to the respective commands. empty array if none. same length as commands
void createChildProcesses(char** commands , const int numCommands , char*** args , tokenlist* tokens , backgroundProcs* backgroundProcesses , int sendtoBack) {

    int pipes[2][2];

    int pids[numCommands]; 

	// create read and write end of pipe
	pipe(pipes[0]);
	pipe(pipes[1]);


    for( int i = 0 ; i < numCommands ; i++) {

        //printf("%s\n" , args[i][0]);

        int pid = fork();

        pids[i] = pid;

        if(pid < 0) {
            printf("Error: unable to execute command\n");
            exit(1);
        }

        if(pid == 0 ) {

            if(i > 0 ) {
                dup2(pipes[i-1][0] , STDIN_FILENO);
            }
            
            if( i < numCommands - 1) {

                dup2(pipes[i][1] , STDOUT_FILENO);
            }


            for(int j = 0 ; j < 2 ; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            int success = execv(commands[i] , args[i]);

            if(success == -1) {
                printf("Error: Unable to execute");
                exit(1);
            }
        }

    }

    for(int j = 0 ; j < 2 ; j++) { //oh my god the parent has to close them too
        close(pipes[j][0]);
        close(pipes[j][1]);
    }

    if(sendtoBack == 0){
        for(int i = 0 ; i < numCommands ; i++) {
            waitpid(pids[i] , NULL , 0);
        }
    }
    else {

        for(int i = 0 ; i < numCommands ; i++) {

            waitpid(pids[i] , NULL , WNOHANG);
        }

        addBackgroundProcess(backgroundProcesses , pids[numCommands - 1] , tokens);
    }

    return;

}

void pipeCommands(tokenlist* tokens , backgroundProcs* backgroundProcesses , int sendtoBack) {

    int numCommands = 0;
    char** commands = (char**) malloc(sizeof(char*) * tokens->size); //array of strings, each one is a command
    char*** args = (char***) malloc(sizeof(char**) * tokens->size); //array of array of strings in format execv expects

    for(int k = 0 ; k < tokens->size ; k++) {
        args[k] = NULL;
    }

    //get all commands
    for(int i = 0 ; i < tokens->size ; i++) {

        char* command;

        if( i == 0 ) {

            command = get_command_path(tokens->items[i]);
            
            commands[i] = command;

            numCommands++; 

            continue;
        }

        if(strcmp(tokens->items[i] , "|") == 0 ) { //next token is command assumed

            command = get_command_path(tokens->items[i + 1]);

            if(command == NULL) {
                printf("command not found...\n");
                goto end;
            }
            
            commands[numCommands] = command;
            numCommands++;
        }

    }

    //get arguments
    int startIndex = 0;

    for(int i = 0 ; i < numCommands ; i++){

        char** argArr = (char**) malloc(sizeof(char*) * tokens->size);

        for(int k = 0 ; k < tokens->size; k++){ //init to null
            argArr[k] = NULL;
        }

        for(int k = startIndex ; k < tokens->size ; k++) { //copy arguments

            if(k - startIndex == 0 ) { //use command path as first for execv
                argArr[k - startIndex] = commands[i];
                continue;
            }

            if(strcmp(tokens->items[k] , "|") == 0 ) { //stop when pipe
                startIndex = ++k;
                break;
            }

            argArr[k - startIndex] = tokens->items[k]; //copy reference to arg
        }

        args[i] = argArr; //arguments for command go in parrallel array
        
    }

    createChildProcesses(commands , numCommands , args , tokens, backgroundProcesses , sendtoBack);

    end:for(int i = 0 ; i < numCommands ; i++) {
        free(commands[i]);
    }

    free(commands);

    for(int i = 0 ; i < tokens->size ; i++) {
        free(args[i]); //all strings themselves are freed at end of loop
    }

    free(args);
}
