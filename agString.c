#include "agString.h"
#include <string.h>

char *agStrntcpy(char *dest, const char *src, size_t n) {
	strncpy(dest, src, n);
	dest[n-1] = '\0';
	return dest;
}

char *agStpntcpy(char *dest, const char *src, size_t n) {
	char *p = stpncpy(dest, src, n);
	dest[n-1] = '\0';
	char *nd = dest + n - 1;
	return nd < p ? nd : p;
}
