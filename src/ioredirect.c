#include "command_utils.h"
#include "redirect.h"
#include "stdlib.h"
#include "stdio.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "background.h"

//allows redirect commands to be in defined format
struct redirectCommand {
    char* command;
    char* infilePath; //freed by caller - derived from caLLLER
    char* outfilePath; //freed by caller - derived from caller
};

void freeRedirectCommand(struct redirectCommand* comm) {
    free(comm->command);
}

//free allocated memory
void cleanRedirect(struct redirectCommand* redirects) {

    freeRedirectCommand(redirects);


    free(redirects);
}

//you can just pass the direct command sequence here - we will parse it
//we do assume it has at least ONE I/O redirect in it tho
//we will validate command structure
int redirectInput( tokenlist* tokens ,  backgroundProcs* backgroundProcesses , int sendtoBack ) {

    int numCommands = 1; //TODO make accurate
    struct redirectCommand* command = parseInput(tokens->items , tokens->size , numCommands);

    char** args = getArgs(tokens->items , tokens->size);

    int procId = fork();

    if(procId < 0) {
        printf("fatal error executing command");
        exit(1);
    }
    else if(procId == 0 ) { //inside child

        int infileDesc = -1;
        int outfileDesc = -1;

        if(command->infilePath != NULL) {

            infileDesc = open(command->infilePath , O_RDONLY); //TODO: error if file does not exist

            if(infileDesc == -1) { //error case
                
                cleanRedirect(command);

                for(int i = 0 ; i < tokens->size ; i++) {
                    free(args[i]);
                }

                free(args);

                _exit(5); //exit child process
            }

            dup2(infileDesc , STDIN_FILENO);

            
        }

        if(command->outfilePath != NULL) {
            
            outfileDesc = open(command->outfilePath , O_WRONLY | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );

            //TODO: error check

            dup2(outfileDesc , STDOUT_FILENO);

        }

        execv(command->command , args);
        
    }
    else {
        
        int* processStat = (int*) malloc(sizeof(int));

        if(sendtoBack == 0) {

            waitpid(procId , processStat , 0); //TODO: refactor for background execution

            if(WEXITSTATUS(*processStat) == 5) {
                printf("Error: unknown file or directory\n");
            }

        }
        else {

            addBackgroundProcess(backgroundProcesses , procId , tokens);

            if(waitpid(procId , processStat , WNOHANG) < 0) {
                printf("Error: Background process %d exited with error...\n" , procId);
            }
        }

        free(processStat);

        cleanRedirect(command); //free mem

        for(int i = 0 ; i < tokens->size ; i++) {
            free(args[i]);
        }

        free(args);
    }

    return 1; //TODO: adjust for background proccessing
    
}

//expects commands to be in order of input
//expects first element in input to be a command
struct redirectCommand* parseInput(char** input , int inputLength , int numCommands) {

    //we use an array of structs as a placeholder in case we need to support multiple commands
    struct redirectCommand* redirectCommand = (struct redirectCommand*) malloc(sizeof( struct redirectCommand ) * 1);
    char* firstCommandPath;

    redirectCommand->command = NULL;
    redirectCommand->infilePath = NULL;
    redirectCommand->outfilePath = NULL;

    if(!command_exists(input[0])) {
        printf("fatal parse error"); //we should never get here, it is expected that input contains a starting command when passed
        exit(1);
    }


    firstCommandPath = get_command_path(input[0]); //first argument should be a command

    //TODO: generalize
    redirectCommand->command = firstCommandPath;

    for(int i = 0 ; i < inputLength ; i++) { //assumes only one command in the pipe for now TODO: expand
        
        if( i == 0){
            continue;
        }


        if( strcmp(input[i], ">") == 0 || strcmp(input[i] , "<") == 0 ) {

            if(i + 1 >= inputLength) { //error state
                return NULL;
            }

            if(strcmp(input[i], ">") == 0) { //TODO: logic here breaks if i add a > || < at the end of input
                
                i++;

                redirectCommand->outfilePath = input[i]; //TODO: make support multiple commands

            }
            else if(strcmp(input[i] , "<") == 0) {

                i++;

                redirectCommand->infilePath = input[i]; //TODO: make support multiple commands
            }
        }
    }

    return redirectCommand;
}

