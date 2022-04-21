#ifndef AG_LINKED_LIST_ITERATOR_H
#define AG_LINKED_LIST_ITERATOR_H
#include "agLLNode.h"
#include "agLinkedList.h"
#include <stdlib.h>

typedef struct agLLIterator
{
    agLLNode *prev, *current, *next;
    agLinkedList *list;
} agLLIterator;

agLLIterator* agLLIInit(agLinkedList *list);
void agLLIFree(agLLIterator *list);
// has
int agLLIHasNext(const agLLIterator *it);
int agLLIHasPrev(const agLLIterator *it);
// next
void *agLLINext(agLLIterator *it);
int agLLINextInt(agLLIterator *it);
char* agLLINextStr(agLLIterator *it);
char agLLINextChar(agLLIterator *it);
// prev
void *agLLIPrev(agLLIterator *it);
int agLLIPrevInt(agLLIterator *it);
char* agLLIPrevStr(agLLIterator *it);
char agLLIPrevChar(agLLIterator *it);
// remove
void *agLLIRemove(agLLIterator *it);
void agLLIRemoveFree(agLLIterator *it);
void *agLLIInsert(agLLIterator *it, void *ptr, size_t size);
// util
void agLLIReset(agLLIterator *it);

#endif
