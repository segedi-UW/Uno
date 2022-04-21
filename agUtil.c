#include "agUtil.h"
#include <stdio.h>
#include <stdbool.h>

void writePtrF(char *buf, void *ptr, const char *fStr, char *str, const char format)
{
    switch (format)
    {
    case 'd':
        sprintf(buf, fStr, *((int *)ptr), str);
        break;
    case 's':
        sprintf(buf, fStr, *((char *)ptr), str);
        break;
    default:
        sprintf(buf, fStr, ptr, str);
        break;
    }
}
