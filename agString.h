#ifndef AG_STRING_H
#include <stdlib.h>

#define AG_STRING_H

/*
 * This is a null terminating version of strncpy.
 * It returns a pointer to the destination string.
 * If the size of the string is smaller
 * than the specified n, then the rest of the destination
 * is filled with null characters.
 */
char *agStrntcpy(char *dest, const char *src, size_t n);

/*
 * This is a null terminating version of stpncpy.
 * It returns a pointer to the null terminating byte
 * of the string. If the size of the string is smaller
 * than the specified n, then the rest of the destination
 * is filled with null characters.
 */
char *agStpntcpy(char *dest, const char *src, size_t n);


#endif
