#pragma once
#include "lexer.h"
#include "background.h"
#include <stdbool.h>

// returns 1 if tokens->items[0] is internal (cd/exit), else 0 
int is_internal_command(tokenlist *tokens);

/* run internal; return:
 *  1 = success (e.g., cd worked)
 *  0 = handled but failed (e.g., cd error)
 * -1 = exit shell
 */
int run_internal_command(tokenlist *tokens, backgroundProcs *procs);

// record one valid command (<=200 chars) for exit history 
void ic_history_record(const char *line);
