#include "agString.h"
#include <string.h>

#if defined(_WIN32) || defined(__CYGWIN__)

char *stpncpy(char *dest, const char *src, size_t n) 
{
	char *ptr = NULL;
	int srcn = strlen(src);
	int count;
	for (count = 0; count < n; count++, dest++, src++)
	{
		if (count < srcn)
		{
			*dest = *src;
			continue;
		}

		*dest = '\0';
		if (ptr == NULL)
			ptr = dest;
	}
	return ptr == NULL ? dest : ptr;
}

#endif

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
