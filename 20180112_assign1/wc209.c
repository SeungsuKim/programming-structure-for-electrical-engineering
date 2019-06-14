// KimSeungsu Assignment1 wc209.c

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

enum DFAState {START, OUT, OUTTEMP, WORDS, WORDSTEMP, COMMENT, COMMENTTEMP};

// This function reads a string from standard input stream.
// This function counts the number of words, lines, characters of the string, prints the result to the standard output stream, and return EXIT_SUCCESS.
// This function also checkes whether the comments of string is unterminated or not.
// If there is a undeterminated comment, prints the error message to the standard error stream, and return EXIT_FAILURE.
int main(void)
{
	int c;
	
	int nWords = 0;     // counts the number of words.
	int nLines = 0;     // counts the number of lines.
	int nChars = 0;     // counts the number of characters.
	int curLine = 1;    // saves the current line of the character c, without considering comments.

	enum DFAState state = START;

	while ((c = getchar()) != EOF) {
		switch (state) {
            // Program is in the START state when the program starts.
			case START:
				if (isspace(c)) {
					state = OUT;
					if (c == '\n') {
						nLines++;
					}
					nChars++;
				} else {
					if (c == '/') {
						state = OUTTEMP;
					} else {
						state = WORDS;
					}
					nWords++;
					nChars++;
				}
				nLines++;
				break;
            // Program is in the OUT state when the character c is not in the word or in the comment.
			case OUT:
				if (isspace(c)) {
					if (c == '\n') {
						nLines++;
					}
					nChars++;
				} else {
					if (c == '/') {
						state = OUTTEMP;
					} else {
						state = WORDS;
					}
					nWords++;
					nChars++;
				}
				break;
            // Program is in the OUTTEMP state when the character c is '/' and the before state is OUT.
            // This state is to check wheter the '/'is starting the comment or not.
			case OUTTEMP:
				if (isspace(c)) {
					state = OUT;
					if (c == '\n') {
						nLines++;
					}
					nChars++;
				} else {
					if (c == '*') {
						state = COMMENT;
						nWords--;
						nChars--;
						curLine = nLines;
					} else {
						if (c != '/') {
							state = WORDS;
						}
						nChars++;
					}
				}
				break;
            // Program is in the WORDS state when the character c is in the word.
			case WORDS:
				if (isspace(c)) {
					state = OUT;
					if (c == '\n') {
						nLines++;
					}
					nChars++;
				} else {
					if (c == '/') {
						state = WORDSTEMP;
					}
					nChars++;
				}
				break;
            // Program is in the WORDSTEMP state when the character c is '/' and the before state is WORDS.
            // This state is to check wheter the '/'is starting the comment or not.
			case WORDSTEMP:
				if (isspace(c)) {
					state = OUT;
					if (c == '\n') {
						nLines++;
					}
					nChars++;
				} else {
					if (c == '*') {
						state = COMMENT;
						nChars--;
						curLine = nLines;
					} else {
						if (c != '/') {
							state = WORDS;
						}
						nChars++;
					}
				}
				break;
            // Program is in the WORDS state when the character c is in the comment.
			case COMMENT:
				if (c == '\n') {
					nLines++;
					nChars++;
				} else if (c == '*') {
					state = COMMENTTEMP;
				}
				break;
            // Program is in the COMMENTTEMP state when the character c is '/' and the before state is COMMENT.
            // This state is to check wheter the '/'is starting the comment or not.
			case COMMENTTEMP:
				if (c == '/') {
					state = OUT;
					nChars++;
				} else {
					if (c != '*') {
						state = COMMENT;
						if (c == '\n') {
							nLines++;
							nChars++;
						}
					}
				}
				break;
			default:
				break;
		}
	}

	if (state == COMMENT || state == COMMENTTEMP) {
		fprintf(stderr, "Error: line %d: unterminated comment\n", curLine);
        return EXIT_FAILURE;
	} else {
        printf("%d %d %d\n", nLines, nWords, nChars);
        return EXIT_SUCCESS;
	}
}
