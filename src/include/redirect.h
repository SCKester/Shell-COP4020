#ifndef REDIRECT_H
#define REDIRECT_H

#include "lexer.h"
#include "background.h"

struct redirectCommand;
void freeRedirectCommand(struct redirectCommand* comm);
void cleanRedirect(struct redirectCommand* redirects);
int redirectInput( tokenlist* tokens ,  backgroundProcs* backgroundProcesses , int sendtoBack );
struct redirectCommand* parseInput(char** input , int inputLength , int numCommands);

#endif