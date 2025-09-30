#pragma once
#include "path_search.h"
#include "lexer.h"
#include "background.h"

// CHANGED: return int (1=success, 0=fail) instead of void 
int ExternalCommand(tokenlist *tokens, backgroundProcs *procs, int sendToBack);  // CHANGED
