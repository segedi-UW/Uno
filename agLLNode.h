#ifndef AG_LINKEDLIST_INTERNAL_H
#define AG_LINKEDLIST_INTERNAL_H
#include <stdlib.h>

typedef struct agLLNode
{
    void *ptr, *next, *prev;
} agLLNode;

agLLNode *agNodeInit(void *ptr, size_t size);
void agNodeConnect(agLLNode *n1, agLLNode *n2);

#endif
