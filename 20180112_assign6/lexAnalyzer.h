/*--------------------------------------------------------------------*/
/* lexAnalyzer.h                                                      */
/* Author: Seungsu Kim, Youngmoo Kim                                  */
/*--------------------------------------------------------------------*/

#ifndef UNIXSHELL_LEXANALYZER_H
#define UNIXSHELL_LEXANALYZER_H

#include "dynarray.h"

typedef struct Token * Token_T;

enum TokenType { TOKEN_ORDINARY, TOKEN_SPECIAL };

/* A Token is either ORDINARY, including ordinary words and quoted
 * words, or SPECIAL, including special characters such as '|', '>',
 * '<'.
 */
struct Token {
    /* The type of the token. */
    enum TokenType eType;

    /* The string which is the token's value. */
    char *pcValue;
};

int lexLine(char* acLine, DynArray_T oTokens, char *pcPgmName);

void printToken(void *pvToken, void *pvExtra);
void freeToken(void *pvToken, void *pvExtra);

#endif //UNIXSHELL_LEXANALYZER_H
