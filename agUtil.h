#ifndef AG_UTIL_H
#define AG_UTIL_H

#define arrlen(array, type) sizeof(array) / sizeof(type)

void writePtrF(char *buf, void *ptr, const char *fStr, char *str, const char format);

#endif
