/*--------------------------------------------------------------------*/
/* synAalyzer.h                                                       */
/* Author: Seungsu Kim, Youngmoo Kim                                  */
/*--------------------------------------------------------------------*/
#ifndef UNIXSHELL_SYNANALYZER_H
#define UNIXSHELL_SYNANALYZER_H

#include "dynarray.h"

typedef struct Command * Command_T;

struct Command {
    DynArray_T oArgs;
    char *input;
    char *output;
};

void printCommand(void *pvCommand, void *pvExtra);
void freeCommand(void *pvCommand, void *pvExtra);
int synTokens(DynArray_T oTokens, char *pcPgmName);
int genCommands(DynArray_T oTokens, DynArray_T oCommands, char *pcPgmName);

#endif //UNIXSHELL_SYNANALYZER_H
