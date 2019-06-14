// KimSeungsu Assignment2
// str.h
#include <stddef.h>
#include <assert.h>

// The StrGetLenght() function returns the number of characters, excluding the terminating null byte, in the string src.
size_t StrGetLength(const char *src);

// The StrCopy() function copies the string pointed by src, including the terminating null byte, to the buffer pointed by dest.
// The StrCopy() function returns a pointer to the destination string dest.
// The destination string dest must be large enough to receive the string src.
char *StrCopy(char *dest, const char *src);

// The StrCompare() function compares two strings str1 and str2.
// If two strings are the same, the function returns 0.
// If str1 is greater than str2, the function retuns an positive integer.
// If str1 is less than str2, the function returns an negative integer.
int StrCompare(const char *str1, const char *str2);

// The StrSearch() function finds the first occurrence of the substring needle in the string haystack.
// The StrSearch() functino returns a pointer to the begining of the substring in the string haystack.
// If the substring needle is not found in the string haystack, the function returns NULL;
char *StrSearch(const char *haystack, const char *needle);

// The StrConcat() function appends the src string to the dest string, overwriting the terminal null byte of the dest, and adds a terminating null byte at the end.
// The StrConcat() function returns a pointer to the resulting string dest.
// The destinatino string dest must be large enough for the result.
char *StrConcat(char *dest, const char *src);