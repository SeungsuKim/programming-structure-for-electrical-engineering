/*--------------------------------------------------------------------*/
/* synAnalyzer.c                                                      */
/* Author: Seungsu Kim, Youngmoo Kim                                  */
/*--------------------------------------------------------------------*/

#include "synAnalyzer.h"
#include "lexAnalyzer.h"
#include "dynarray.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

void printCommand(void *pvCommand, void *pvExtra) {
    assert(pvCommand != NULL);

    Command_T pCommand = (Command_T)pvCommand;
    int i;
    int len = DynArray_getLength(pCommand->oArgs);
    for (i = 0; i < len; i++) {
        printf("arg:%s ", (char *)DynArray_get(pCommand->oArgs, i));
    }
    if (pCommand->input) printf(", input: %s ", pCommand->input);
    if (pCommand->output) printf(", output: %s", pCommand->output);
    printf("\n");
}

void freeArg(void *pvArg, void *pvExtra) {
    assert(pvArg != NULL);
    char *pcArg = (char *)pvArg;
    if (pcArg != NULL) free(pcArg);
}


void freeCommand(void *pvCommand, void *pvExtra) {
    assert(pvCommand != NULL);

    Command_T pCommand = (Command_T)pvCommand;

    if (pCommand->input) free(pCommand->input);
    if (pCommand->output) free(pCommand->output);

    if (pCommand->oArgs != NULL) {
        int numArgs = DynArray_getLength(pCommand->oArgs);
        DynArray_removeAt(pCommand->oArgs, numArgs-1);
        DynArray_map(pCommand->oArgs, freeArg, NULL);
    }
    DynArray_free(pCommand->oArgs);

    free(pCommand);
}


enum { FALSE, TRUE };
enum SynState { STATE_START, STATE_CMD, STATE_ARG, STATE_IN,
    STATE_IN_ARG, STATE_OUT, STATE_OUT_ARG, STATE_PIPE };

int isInToken(Token_T pToken) {
    assert(pToken != NULL);

    return (*(pToken->pcValue)=='<' && pToken->eType==TOKEN_SPECIAL);
}

int isOutToken(Token_T pToken) {
    assert(pToken != NULL);

    return (*(pToken->pcValue)=='>' && pToken->eType==TOKEN_SPECIAL);
}

int isPipe(Token_T pToken) {
    assert(pToken != NULL);

    return (*(pToken->pcValue)=='|' && pToken->eType==TOKEN_SPECIAL);
}

int isSpecialToken(Token_T pToken) {
    assert(pToken != NULL);

    return pToken->eType == TOKEN_SPECIAL;
}

int isBuiltInCmd(Token_T pToken) {
    assert(pToken != NULL);

    return (strcmp(pToken->pcValue, "cd")==0) ||
           (strcmp(pToken->pcValue, "setenv")==0) ||
           (strcmp(pToken->pcValue, "unsetenv")==0) ||
           (strcmp(pToken->pcValue, "exit")==0);

}

int synTokens(DynArray_T oTokens, char *pcPgmName) {
    int i, len, in, out, builtIn, piped;
    Token_T pToken;

    assert(oTokens != NULL);

    len = DynArray_getLength(oTokens);

    enum SynState state = STATE_START;

    for (i=0, in=FALSE, out=FALSE, builtIn=FALSE, piped=FALSE; 
    	i < len; i++) {
        pToken = (Token_T)DynArray_get(oTokens, i);
        switch (state) {
            case STATE_START:
                if (isSpecialToken(pToken)) {
                    fprintf(stderr, 
                    	"%s: Pipe or redirection without specified source.\n", 
                    	pcPgmName);
                    return FALSE;
                } else {
                    if (isBuiltInCmd(pToken)) builtIn = TRUE;
                    state = STATE_CMD;
                }
                break;
            case STATE_CMD:
                if (isInToken(pToken)) state = STATE_IN;
                else if (isOutToken(pToken)) state = STATE_OUT;
                else if (isPipe(pToken)) state = STATE_PIPE;
                else state = STATE_ARG;
                break;
            case STATE_ARG:
                if (isInToken(pToken)) state = STATE_IN;
                else if (isOutToken(pToken)) state = STATE_OUT;
                else if (isPipe(pToken)) state = STATE_PIPE;
                else state = STATE_ARG;
                break;
            case STATE_IN:
                if (in) {
                    fprintf(stderr, 
                    	"%s: Multiple redirection of standard input.\n", 
                    	pcPgmName);
                    return FALSE;
                }
                in = TRUE;
                if (isSpecialToken(pToken)) {
                    fprintf(stderr, 
                    	"%s: Standard input redirection without specified destination.\n", 
                    	pcPgmName);
                    return FALSE;
                }
                else state = STATE_IN_ARG;
                break;
            case STATE_IN_ARG:
                if (isInToken(pToken)) {
                    fprintf(stderr, 
                    	"%s: Multiple redirection of standard input.\n", 
                    	pcPgmName);
                    return FALSE;
                }
                else if (isOutToken(pToken)) state = STATE_OUT;
                else if (isPipe(pToken)) state = STATE_PIPE;
                else state = STATE_ARG;
                break;
            case STATE_OUT:
                if (out) {
                    fprintf(stderr, 
                    	"%s: Multiple redirection of standard output.\n", 
                    	pcPgmName);
                    return FALSE;
                }
                out = TRUE;
                if (isSpecialToken(pToken)) {
                    fprintf(stderr, 
                    	"%s: Standard output redirection without specified destination.\n", 
                    	pcPgmName);
                    return FALSE;
                }
                else state = STATE_OUT_ARG;
                break;
            case STATE_OUT_ARG:
                if (isOutToken(pToken)) {
                    fprintf(stderr, 
                    	"%s: Multiple redirection of standard output.\n", 
                    	pcPgmName);
                    return FALSE;
                }
                else if (isOutToken(pToken)) state = STATE_OUT;
                else if (isPipe(pToken)) state = STATE_PIPE;
                else state = STATE_ARG;
                break;
            case STATE_PIPE:
                if (builtIn) {
                    fprintf(stderr, 
                    	"%s: Pipe including built-in commands.\n", 
                    	pcPgmName);
                    return FALSE;
                }

                if (out) {
                    fprintf(stderr, 
                    	"%s: Multiple redirection of standard output.\n", 
                    	pcPgmName);
                    return FALSE;
                }

                in = TRUE;
                out = FALSE;
                piped = TRUE;

                if (isSpecialToken(pToken)) {
                    fprintf(stderr, 
                    	"%s: Pipe without specified destination.\n", 
                    	pcPgmName);
                    return FALSE;
                }
                else {
                    if (isBuiltInCmd(pToken)) builtIn = TRUE;
                    state = STATE_CMD;
                }

                break;
        }
    }

    if (state == STATE_IN || state == STATE_OUT || state == STATE_PIPE) {
        fprintf(stderr, 
        	"%s: Pipe or redirection without specified destination.\n", 
        	pcPgmName);
        return FALSE;
    }

    if (piped && builtIn) {
        fprintf(stderr, 
        	"%s: Pipe including built-in commands.\n", 
        	pcPgmName);
        return FALSE;
    }

    return TRUE;
}

int genCommands(DynArray_T oTokens, 
	DynArray_T oCommands, 
	char *pcPgmName) {

    int i, len;
    char *arg;
    Token_T pToken;
    Command_T pCommand;

    assert(oTokens != NULL);
    assert(oCommands != NULL);

    len = DynArray_getLength(oTokens);

    pCommand = (Command_T)malloc(sizeof(struct Command));
    if (pCommand == NULL) {
        fprintf(stderr, 
        	"%s: Can't allocate a memory for Command_T.\n", 
        	pcPgmName);
        return FALSE;
    }
    pCommand->oArgs = DynArray_new(0);
    if (pCommand->oArgs == NULL) {
        fprintf(stderr, 
        	"%s: Can't allocate a memory for pCommand->oArgs.\n", 
        	pcPgmName);
        return FALSE;
    }
    pCommand->input = NULL;
    pCommand->output = NULL;

    enum SynState state = STATE_START;

    for (i = 0; i < len; i++) {
        pToken = (Token_T)DynArray_get(oTokens, i);
        switch (state) {
            case STATE_START:
                arg = strdup(pToken->pcValue);
                if (arg == NULL) {
                    fprintf(stderr, 
                    	"%s: Can't allocate a memory for argument name.\n", 
                    	pcPgmName);
                }
                DynArray_add(pCommand->oArgs, arg);
                state = STATE_CMD;
                break;
            case STATE_CMD:
                if (isInToken(pToken)) state = STATE_IN;
                else if (isOutToken(pToken)) state = STATE_OUT;
                else if (isPipe(pToken)) state = STATE_PIPE;
                else {
                    arg = strdup(pToken->pcValue);
                    if (arg == NULL) {
                        fprintf(stderr, 
                        	"%s: Can't allocate a memory for argument name.\n", 
                        	pcPgmName);
                    }
                    DynArray_add(pCommand->oArgs, arg);
                    state = STATE_ARG;
                }
                break;
            case STATE_ARG:
                if (isInToken(pToken)) state = STATE_IN;
                else if (isOutToken(pToken)) state = STATE_OUT;
                else if (isPipe(pToken)) state = STATE_PIPE;
                else {
                    arg = strdup(pToken->pcValue);
                    if (arg == NULL) {
                        fprintf(stderr, 
                        	"%s: Can't allocate a memory for argument name.\n", 
                        	pcPgmName);
                    }
                    DynArray_add(pCommand->oArgs, arg);
                    state = STATE_ARG;
                }
                break;
            case STATE_IN:
                pCommand->input = strdup(pToken->pcValue);
                if (pCommand->input == NULL) {
                    fprintf(stderr, 
                    	"%s: Can't allocate a memory for input file name.\n", 
                    	pcPgmName);
                }
                state = STATE_IN_ARG;
                break;
            case STATE_IN_ARG:
                if (isOutToken(pToken)) state = STATE_OUT;
                else if (isPipe(pToken)) state = STATE_PIPE;
                else state = STATE_ARG;
                break;
            case STATE_OUT:
                pCommand->output = strdup(pToken->pcValue);
                if (pCommand->output == NULL) {
                    fprintf(stderr, 
                    	"%s: Can't allocate a memory for output file name.\n", 
                    	pcPgmName);
                }
                state = STATE_OUT_ARG;
                break;
            case STATE_OUT_ARG:
                if (isOutToken(pToken)) state = STATE_OUT;
                else if (isPipe(pToken)) state = STATE_PIPE;
                else state = STATE_ARG;
                break;
            case STATE_PIPE:
                DynArray_add(pCommand->oArgs, '\0');
                DynArray_add(oCommands, pCommand);

                pCommand = (Command_T)malloc(sizeof(struct Command));
                if (pCommand == NULL) {
                    fprintf(stderr, 
                    	"%s: Can't allocate a memory for Command_T.\n", 
                    	pcPgmName);
                    return FALSE;
                }
                pCommand->oArgs = DynArray_new(0);
                if (pCommand->oArgs == NULL) {
                    fprintf(stderr, 
                    	"%s: Can't allocate a memory for pCommand->oArgs.\n", 
                    	pcPgmName);
                    return FALSE;
                }
                pCommand->input = NULL;
                pCommand->output = NULL;

                arg = strdup(pToken->pcValue);
                if (arg == NULL) {
                    fprintf(stderr, 
                    	"%s: Can't allocate a memory for argument name.\n", 
                    	pcPgmName);
                }
                DynArray_add(pCommand->oArgs, arg);
                state = STATE_CMD;

                break;
        }
    }

    if (state == STATE_CMD || state == STATE_ARG || 
    	state == STATE_IN_ARG || state == STATE_OUT_ARG) {
        DynArray_add(pCommand->oArgs, '\0');
        DynArray_add(oCommands, pCommand);
    }

    return TRUE;
}