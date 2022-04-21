/*
 * Simple doubly linked LinkedList iterator implementation.
 * The corresponding LinkedList is listed as
 * agLinkedList.
 *
 * author: Anthony Segedi
 *
 */
#include "agLLIterator.h"
#include <stdlib.h>
#include <stdio.h>

agLLIterator *agLLIInit(agLinkedList *list)
{
    agLLIterator *it = malloc(sizeof(agLLIterator));
	if (it == NULL) {
		perror("agLLIterator malloc()");
		exit(EXIT_FAILURE);
	}
    it->list = list;
    it->current = NULL;
    it->prev = list->tail;
    it->next = list->head;
    return it;
}

void agLLIFree(agLLIterator *list) {
	free(list);
}

int agLLIHasNext(const agLLIterator *it)
{
    return it->next != NULL;
}

int agLLIHasPrev(const agLLIterator *it)
{
    return it->prev != NULL;
}

void setAll(agLLIterator *it, agLLNode *n)
{
    if (n == NULL)
    {
        it->current = NULL;
        it->next = NULL;
        it->prev = NULL;
        return;
    }
    it->current = n;
    it->next = n->next;
    it->prev = n->prev;
}

void *agLLINext(agLLIterator *it)
{
    if (it->next == NULL)
    {
        fprintf(stderr, "ERROR: no more elements\n");
        return NULL;
    }
    setAll(it, it->next);
    return it->current->ptr;
}

int agLLINextInt(agLLIterator *it)
{
    return *(int *)agLLINext(it);
}

char* agLLINextStr(agLLIterator *it)
{
    return (char*)agLLINext(it);
}

char agLLINextChar(agLLIterator *it)
{
    return *(char *)agLLINext(it);
}

void *agLLIPrev(agLLIterator *it)
{
    if (it->prev == NULL)
    {
        fprintf(stderr, "ERROR: no more elements\n");
        return NULL;
    }
    setAll(it, it->prev);
    return it->current->ptr;
}

int agLLIPrevInt(agLLIterator *it)
{
    return *(int *)agLLIPrev(it);
}

char* agLLIPrevStr(agLLIterator *it)
{
    return (char*)agLLIPrev(it);
}

char agLLIPrevChar(agLLIterator *it)
{
    return *(char *)agLLIPrev(it);
}

void *agLLIRemove(agLLIterator *it)
{
    if (it->current == NULL)
    {
        fprintf(stderr, "ERROR: cannot remove NULL element\n");
        return NULL;
    }
    agLLNode *c = it->current;
    agLLNode *n = c->next;
    agLLNode *p = c->prev;

    if (p == NULL) it->list->head = n;
    else p->next = n;
    if (n == NULL) it->list->tail = p;
    else n->prev = p;

    it->current = NULL;
	it->list->size--;
    return c->ptr;
}

void agLLIRemoveFree(agLLIterator *it)
{
	void *p = agLLIRemove(it);
	free(p);
}


void *agLLIInsert(agLLIterator *it, void *ptr, size_t size)
{
	if (it->current == NULL) {
		fprintf(stderr, "Cannot insert at a NULL node\n");
		return NULL;
	}
	agLLNode *i = agNodeInit(ptr, size);
    agLLNode *p = it->prev;
    agLLNode *n = it->current;

	if (p == NULL) {
		agNodeConnect(i, it->list->head);
		it->list->head = i;
	} else {
		agNodeConnect(p, i);
		agNodeConnect(i, n);
	}
	it->list->size++;
	return i->ptr;
}

void agLLIReset(agLLIterator *it) 
{
    it->current = NULL;
    it->prev = it->list->tail;
    it->next = it->list->head;
}
