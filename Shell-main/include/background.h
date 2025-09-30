#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "lexer.h"

typedef struct {
    int numBackgroundProcessesTotal;

    int numActiveBackgroundProcesses;

    int activeBackgroundProcessesJobNums[10]; //job nums

    int activeBackgroundProcessPIDs[10]; //parralell array of pids

    char* activeBackgroundProcessesNames[10]; //array of strings that are the commands
} backgroundProcs;

void removeAmpersand(tokenlist* tokens);

void incrementBackProcs( backgroundProcs* processes , int pid , char* command);

void addBackgroundProcess(backgroundProcs* processes , int pid , tokenlist* tokens);

void removeBackgroundProcess(backgroundProcs* processes , int jobNum);

void reportRunningProcess(backgroundProcs* processes , int jobNum);

void listBackgroundProcesses(backgroundProcs* processes);

#endif