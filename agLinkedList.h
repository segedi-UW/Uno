#ifndef AG_LINKED_LIST_H
#define AG_LINKED_LIST_H
#include <stdlib.h>
#include "agLLNode.h"

typedef struct agLinkedList
{
    agLLNode *head, *tail;
    int size;
} agLinkedList;

agLinkedList *agLLInit(void);
// deconstructor
void agLLFree(agLinkedList *list);

// stack methods
int agLLPush(agLinkedList *list, void *ptr, size_t size);
void *agLLPeek(agLinkedList *list);
void *agLLPop(agLinkedList *list);
void agLLPopFree(agLinkedList *list);

// list methods
int agLLAppend(agLinkedList *list, void *ptr, size_t size);
int agLLInsert(agLinkedList *list, int index, void *ptr, size_t size);
void *agLLGet(agLinkedList *list, int index);
void *agLLRemove(agLinkedList *list, int index);
void agLLRemoveFree(agLinkedList *list, int index);
int agLLIsEmpty(agLinkedList *list);
void agLLClear(agLinkedList *list);
int agLLContains(agLinkedList *list, void *ptr, size_t size);
void *agLLGetp(agLinkedList *list, void *ptr, size_t size);
void *agLLRemovep(agLinkedList *list, void *ptr, size_t size);
int agLLRemovepFree(agLinkedList *list, void *ptr, size_t size);

// util
char *agLLToString(agLinkedList *list, const char format);
char *agLLToDebugString(agLinkedList *list);

#endif
