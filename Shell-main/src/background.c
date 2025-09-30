#include "background.h"
#include "lexer.h"
#include "string.h"
#include <stdio.h>

//command expected to be full command as string
void incrementBackProcs( backgroundProcs* processes , int pid , char* command) {

    processes->numBackgroundProcessesTotal++;

    processes->numActiveBackgroundProcesses++;

    //if no more than 10 are active at a time, the calling process always knows which job num it is 
    //by simply saving the numBackgroundProcessesTotal and then % 10 and print array val
    //could also use pids here optionally
    processes->activeBackgroundProcessesJobNums[ processes->numBackgroundProcessesTotal % 10 ] = processes->numBackgroundProcessesTotal;
    processes->activeBackgroundProcessPIDs[ processes->numBackgroundProcessesTotal % 10 ] = pid;

    processes->activeBackgroundProcessesNames[processes->numBackgroundProcessesTotal % 10] = command;
}

//simply removes the specified background proccess from record and prints its details to stdout
//jobNum is the relative jobnum (the counter from start of program)
void removeBackgroundProcess(backgroundProcs* processes , int jobNum) {

    processes->numActiveBackgroundProcesses--;

    int ind;

    for(int i = 0 ; i < 10 ; i++) {
        if(jobNum == processes->activeBackgroundProcessesJobNums[i]) {
            ind = i;
        }
    }

    printf("[%d]  + %d done %s\n" , 
        processes->activeBackgroundProcessesJobNums[ind] , 
        processes->activeBackgroundProcessPIDs[ind] , 
        processes->activeBackgroundProcessesNames[ind]
    );

    free(processes->activeBackgroundProcessesNames[ind]);
    processes->activeBackgroundProcessPIDs[ind] = 0;
    processes->activeBackgroundProcessesJobNums[ind] = 0;
}

//adds a background process to the tracker and prints its detailsa to stdout
void addBackgroundProcess(backgroundProcs* processes , int pid , tokenlist* tokens) {

    char* printableCommand = (char*) malloc(sizeof(char) * 200);

    for(int i = 0 ; i < tokens->size ; i++) {

        if(i == 0 ){
            strcpy(printableCommand , tokens->items[i]);
        }
        else{
            strcat(printableCommand , tokens->items[i]);
        }
        
        strcat(printableCommand , " ");
    }


    incrementBackProcs(processes , pid , printableCommand);

    printf("[%d] %d" , 
        processes->activeBackgroundProcessesJobNums[ processes->numBackgroundProcessesTotal % 10] , 
        pid
    );

    printf("\n");
}

void reportRunningProcess(backgroundProcs* processes , int jobNum) {
    int ind;

    for(int i = 0 ; i < 10 ; i++) {
        if(jobNum == processes->activeBackgroundProcessesJobNums[i]) {
            ind = i;
        }
    }

    printf("[%d]  + %d running %s\n" , 
        processes->activeBackgroundProcessesJobNums[ind] , 
        processes->activeBackgroundProcessPIDs[ind] , 
        processes->activeBackgroundProcessesNames[ind]
    );
  
}

void listBackgroundProcesses(backgroundProcs* processes) {

    for(int i = 0 ; i < 10 ; i++) {

        if(processes->activeBackgroundProcessesJobNums[i] != 0 ) { //process is active
            
            reportRunningProcess(processes ,  processes->activeBackgroundProcessesJobNums[i]);
        }
    }
}
//removes an ampersand from the input tokens, do not call if no ampersand present
//returns tokenlist in format expected by execv for args
void removeAmpersand(tokenlist* tokens) {

    int hasAmpersand = 0;
    
    for(int i = 0 ; i < tokens->size ; i++) {
        hasAmpersand = strcmp(tokens->items[i] , "&") == 0 ? 1 : 0; //set flag to 1 if ampersand found
    }

    if(hasAmpersand == 0 ) {
        return;
    }

    int numAmpersands = 0;

    for(int i = 0 ; i < tokens->size ; i++) {
        if(strcmp(tokens->items[i] , "&") == 0 ) {
            numAmpersands++;
        }
    }

    char** newTokens = (char**) malloc(sizeof(char*) * tokens->size - numAmpersands);
    int ind = 0;

    for(int i = 0 ; i < tokens->size ; i++) {

        if(strcmp(tokens->items[i] , "&") != 0 ) {

            char* token = (char*) malloc(sizeof(char) * strlen(tokens->items[i]) + 1); //allocate new string space

            strcpy(token , tokens->items[i]);

            newTokens[ind] = token;

            ind++;
        }
    }

    for(int i = 0 ; i < tokens->size ; i++) {
        free(tokens->items[i]);
    }

    free(tokens->items);

    tokens->items = newTokens;
    tokens->size = tokens->size - numAmpersands;
}

