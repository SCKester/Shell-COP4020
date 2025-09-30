#ifndef PIPING_H
#define PIPING_H

#include "lexer.h"
#include "background.h"

void cleanProcesses(int* processes , int processCount , int** pipes , int pipesCount);

void createChildProcesses(char** commands , const int numCommands , char*** args , tokenlist* tokens , backgroundProcs* backgroundProcesses , int sendtoBack);

int** createPipes( int numProcesses);

void pipeCommands(tokenlist* tokens , backgroundProcs* backgroundProcesses , int sendtoBack);

#endif