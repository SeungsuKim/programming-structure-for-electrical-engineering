/*--------------------------------------------------------------------*/
/* lexAnalyzer.c                                                      */
/* Author: Seungsu Kim, Youngmoo Kim                                  */
/*--------------------------------------------------------------------*/

#include "dynarray.h"
#include "lexAnalyzer.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

enum { MAX_LINE_SIZE = 1024 };
enum { FALSE, TRUE };
enum LexState { STATE_START, STATE_ORDINARY, STATE_SPECIAL,
    STATE_SPACE, STATE_QUOTED, STATE_QUOTED_IN, STATE_QUOTED_OUT };

const char specialChars[] = "|<>";

void printToken(void *pvToken, void *pvExtra) {
    assert(pvToken != NULL);

    Token_T pToken = (Token_T)pvToken;
    if (pToken->eType == TOKEN_SPECIAL)
        printf("%s(special)\n", pToken->pcValue);
    else
        printf("%s(ordianry)\n", pToken->pcValue);
}

void freeToken(void *pvToken, void *pvExtra) {
    assert(pvToken != NULL);

    Token_T pToken = (Token_T)pvToken;
    if (pToken->pcValue != NULL) free(pToken->pcValue);
    free(pvToken);
}

int isSpecial(int c) {
    for (int i = 0; specialChars[i] != '\0'; i++) {
        if ( c == specialChars[i] ) return TRUE;
    }
    return FALSE;
}

int isQuote(int c) {
    return c == '"';
}

int isOrdianry(int c) {
    return !(isSpecial(c) || isQuote(c) || isspace(c));
}

int lexLine(char* acLine, DynArray_T oTokens, char *pcPgmName) {
    int c;
    char *pcLine, *pcStart;
    Token_T pToken;

    assert(acLine != NULL);
    assert(oTokens != NULL);

    enum LexState state = STATE_START;

    for (pcLine = acLine, pcStart = acLine; *pcLine != '\n'; pcLine++) {
        c = (int)*pcLine;
        switch (state) {
            case STATE_START:
                if (isspace(c)) state = STATE_SPACE;
                else if (isSpecial(c)) {
                    state = STATE_SPECIAL;
                    pcStart = pcLine;
                }
                else if (isQuote((c))) state = STATE_QUOTED_IN;
                else if (isOrdianry(c)) {
                    state = STATE_ORDINARY;
                    pcStart = pcLine;
                }
                break;

            case STATE_ORDINARY:
                if (!isOrdianry(c)) {
                    pToken = (Token_T)malloc(sizeof(struct Token));
                    if (pToken == NULL) {
                        fprintf(stderr, "%s: Can't allocate a memory for Token_T.\n", pcPgmName);
                        return FALSE;
                    }
                    pToken->eType = TOKEN_ORDINARY;
                    pToken->pcValue = (char *)calloc((size_t)1, (size_t)(pcLine-pcStart+1));
                    if (pToken->pcValue == NULL) {
                        fprintf(stderr, "%s: Can't allocate a memory for the Token's pcValue.\n", pcPgmName);
                        free(pToken);
                        return FALSE;
                    }
                    strncpy(pToken->pcValue, pcStart, (size_t)(pcLine-pcStart));
                    DynArray_add(oTokens, pToken);

                    if (isspace(c)) state = STATE_SPACE;
                    else if (isSpecial(c)) {
                        state = STATE_SPECIAL;
                        pcStart = pcLine;
                    }
                    else if (isQuote(c)) state = STATE_QUOTED_IN;
                } else {
                    state = STATE_ORDINARY;
                }
                break;

            case STATE_SPECIAL:
                pToken = (Token_T)malloc(sizeof(struct Token));
                if (pToken == NULL) {
                    fprintf(stderr, "%s: Can't allocate a memory for Token_T.\n", pcPgmName);
                    return FALSE;
                }
                pToken->eType = TOKEN_SPECIAL;
                pToken->pcValue = (char *)calloc((size_t)1, (size_t)(pcLine-pcStart+1));
                if (pToken->pcValue == NULL) {
                    fprintf(stderr, "%s: Can't allocate a memory for the Token's pcValue.\n", pcPgmName);
                    free(pToken);
                    return FALSE;
                }
                strncpy(pToken->pcValue, pcStart, (size_t)(pcLine-pcStart));
                DynArray_add(oTokens, pToken);

                if (isspace(c)) state = STATE_SPACE;
                else if (isSpecial(c)) {
                    state = STATE_SPECIAL;
                    pcStart = pcLine;
                }
                else if (isQuote(c)) state = STATE_QUOTED_IN;
                else if (isOrdianry(c)) {
                    state = STATE_ORDINARY;
                    pcStart = pcLine;
                }
                break;

            case STATE_SPACE:
                if (isspace(c)) state = STATE_SPACE;
                else if (isSpecial(c)) {
                    state = STATE_SPECIAL;
                    pcStart = pcLine;
                }
                else if (isQuote(c)) state = STATE_QUOTED_IN;
                else if (isOrdianry(c)) {
                    state = STATE_ORDINARY;
                    pcStart = pcLine;
                }
                break;

            case STATE_QUOTED:
                if (isQuote(c)) {
                    pToken = (Token_T)malloc(sizeof(struct Token));
                    if (pToken == NULL) {
                        fprintf(stderr, "%s: Can't allocate a memory for Token_T.\n", pcPgmName);
                        return FALSE;
                    }
                    pToken->eType = TOKEN_ORDINARY;
                    pToken->pcValue = (char *)calloc((size_t)1, (size_t)(pcLine-pcStart+1));
                    if (pToken->pcValue == NULL) {
                        fprintf(stderr, "%s: Can't allocate a memory for the Token's pcValue.\n", pcPgmName);
                        free(pToken);
                        return FALSE;
                    }
                    strncpy(pToken->pcValue, pcStart, (size_t)(pcLine-pcStart));
                    DynArray_add(oTokens, pToken);

                    state = STATE_QUOTED_OUT;
                }
                else state = STATE_QUOTED;
                break;

            case STATE_QUOTED_IN:
                if (isQuote(c)) state = STATE_QUOTED_OUT;
                else {
                    state = STATE_QUOTED;
                    pcStart = pcLine;
                }
                break;

            case STATE_QUOTED_OUT:
                if (isspace(c)) state = STATE_SPACE;
                else if (isSpecial(c)) {
                    state = STATE_SPECIAL;
                    pcStart = pcLine;
                }
                else if (isQuote(c)) state = STATE_QUOTED_IN;
                else if (isOrdianry(c)) {
                    state = STATE_ORDINARY;
                    pcStart = pcLine;
                }
                break;
        }
    }

    if (state == STATE_QUOTED_IN || state == STATE_QUOTED) {
        fprintf(stderr, "%s: Could not find quote pair.\n", pcPgmName);
        // free
        return FALSE;
    }

    if (state == STATE_ORDINARY || state == STATE_SPECIAL) {
        pToken = (Token_T)malloc(sizeof(struct Token));
        if (pToken == NULL) {
            fprintf(stderr, "%s: Can't allocate a memory for Token_T.\n", pcPgmName);
            return FALSE;
        }
        if (state == STATE_ORDINARY)
            pToken->eType = TOKEN_ORDINARY;
        else
            pToken->eType = TOKEN_SPECIAL;
        pToken->pcValue = (char *)calloc((size_t)1, (size_t)(pcLine-pcStart+1));
        if (pToken->pcValue == NULL) {
            fprintf(stderr, "%s: Can't allocate a memory for the Token's pcValue.\n", pcPgmName);
            free(pToken);
            return FALSE;
        }
        strncpy(pToken->pcValue, pcStart, (size_t)(pcLine-pcStart));
        DynArray_add(oTokens, pToken);
    }

    return TRUE;
}