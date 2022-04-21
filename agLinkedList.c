/*
 * Simple doubly linked LinkedList implementation.
 * The corresponding iterator is listed as
 * agLLIterator.
 *
 * author: Anthony Segedi
 *
 */

#include "agLinkedList.h"
#include "agLLNode.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "agUtil.h"

 /*
 * Clears the list and frees the list meta data
 * Equivalent to calling agLLClear(list); free(list);
 */
void agLLFree(agLinkedList *list) 
{
	agLLClear(list);
	free(list);
}

agLinkedList *agLLInit(void)
{
	agLinkedList *list = malloc(sizeof(agLinkedList));
	if (list == NULL) 
	{
		perror("agLinkedList malloc()");
		exit(EXIT_FAILURE);
	}
	list->size = 0;
	list->head = NULL;
	list->tail = NULL;
	return list;
}

int agLLAppend(agLinkedList *list, void *ptr, size_t size)
{
	if (list == NULL || ptr == NULL)
	{
		fprintf(stderr, "ERROR: Cannot add to null list or with null ptr\n");
		return 1;
	}
	agLLNode *n = agNodeInit(ptr, size);
	if (list->head == NULL)
	{
		list->head = n;
		list->tail = n;
	}
	else
	{
		agLLNode *pn = list->tail;
		agNodeConnect(pn, n);
		list->tail = n;
	}
	list->size++;
	return 0;
}

void static *removeNode(agLinkedList *list, agLLNode *c) {
	if (c == NULL) 
	{
		fprintf(stderr, "ERROR: Internal LinkedList traversal null error\n");
		return NULL;
	}

	agLLNode *n = c->next;
	agLLNode *p = c->prev;

	if (p == NULL) list->head = n;
	else p->next = n;
	if (n == NULL) list->tail = p;
	else n->prev = p;

	void *ptr = c->ptr;

	return ptr;
}

agLLNode static *getNode(agLinkedList *list, int index)
{
	if (index < 0 || index >= list->size)
	{
		fprintf(stderr, "ERROR: Index %d out of bounds [0-%d)\n", index, list->size);
		return NULL;
	}
	int isBeforeMid = index < list->size / 2;
	int ti = isBeforeMid ? index : list->size - 1 - index;
	agLLNode *n = isBeforeMid ? list->head : list->tail;
	for (int i = 0; n != NULL && i < ti; i++, n = isBeforeMid ? n->next : n->prev)
		;
	return n;
}

int agLLInsert(agLinkedList *list, int index, void *ptr, size_t size)
{
	if (list == NULL || ptr == NULL)
	{
		fprintf(stderr, "ERROR: Cannot add to null list or with null ptr\n");
		return 0;
	}
	if (index < 0 || index > list->size)
	{
		fprintf(stderr, "ERROR: Index %d out of bounds [0-%d]\n", index, list->size);
		return 0;
	}

	agLLNode *i = agNodeInit(ptr, size);

	if (list->head == NULL)
	{
		list->head = i;
		list->tail = i;
	}
	else
	{
		agLLNode *n;
		if (index == 0)
			n = list->head;
		else if (index == list->size)
			n = NULL;
		else
			n = getNode(list, index);
		if (n != NULL && n->prev == NULL)
		{
			list->head = i;
			agNodeConnect(i, n);
		}
		else if (n == NULL)
		{
			n = list->tail;
			list->tail = i;
			agNodeConnect(n, i);
		}
		else
		{
			agNodeConnect(n->prev, i);
			agNodeConnect(i, n);
		}
	}
	list->size++;
	return 1;
}


void *agLLRemove(agLinkedList *list, int index)
{
	if (index < 0 || index >= list->size)
	{
		fprintf(stderr, "ERROR: Index %d out of bounds [0-%d)\n", index, list->size);
		return NULL;
	}

	agLLNode *c = getNode(list, index);
	void *ptr = removeNode(list, c);
	list->size--;
	return ptr;
}

void agLLRemoveFree(agLinkedList *list, int index) {
	void *p = agLLRemove(list, index);
	if (p != NULL) free(p);
}

int agLLPush(agLinkedList *list, void *ptr, size_t size) {
	return agLLAppend(list, ptr, size);
}

void *agLLPeek(agLinkedList *list) {
	if (list->tail == NULL)
		fprintf(stderr, "Nothing to peek, empty list\n");

	return list->tail->ptr;
}

void *agLLPop(agLinkedList *list) {
	if (list->tail == NULL) {
		fprintf(stderr, "Nothing to peek, empty list\n");
		return NULL;
	}

	void *ptr = removeNode(list, list->tail);
	list->size--;
	return ptr;
}

void agLLPopFree(agLinkedList *list) {
	void *p = agLLPop(list);
	if (p != NULL) free(p);
}

void *agLLGet(agLinkedList *list, int index)
{
	agLLNode *n = getNode(list, index);
	return n != NULL ? n->ptr : NULL;
}

int agLLContains(agLinkedList *list, void *ptr, size_t size) {
	agLLNode *n = list->head;
	for (; n != NULL; n = n->next) {
		if (memcmp(n->ptr, ptr, size) == 0) 	
			return true;
	}
	return false;
}

void *agLLGetp(agLinkedList *list, void *ptr, size_t size) {
	agLLNode *n = list->head;
	for (; n != NULL; n = n->next) {
		if (memcmp(n->ptr, ptr, size) == 0) 	
			return n->ptr;
	}
	return NULL;
}

void *agLLRemovep(agLinkedList *list, void *ptr, size_t size) {
	agLLNode *n = list->head;
	for (; n != NULL; n = n->next) {
		if (memcmp(n->ptr, ptr, size) == 0) {
			void *p = removeNode(list, n);
			list->size--;
			return p;
		}
	}
	return NULL;
}

int agLLRemovepFree(agLinkedList *list, void *ptr, size_t size) {
	void *p = agLLRemovep(list, ptr, size);
	if (p != NULL) free(p);
	return p != NULL;
}

/*
 * Clears all internal nodes malloc allocated within this list, setting size to zero
 * and resetting the head and tail to NULL.
 * 
 */
void agLLClear(agLinkedList *list)
{
	agLLNode *c = list->head;
	agLLNode *n;
	while (c != NULL)
	{
		n = c->next;
		free(c->ptr);
		free(c);
		c = n;
	}
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
}

int agLLIsEmpty(agLinkedList *list) {
	return list->size == 0;
}

char *agLLToString(agLinkedList *list, const char format)
{
	if (format != 'd' && format != 's' && format != 'p')
		fprintf(stderr, "Unimplemented format (%c), default to ptr\n", format);
	int sn = list->size * 25;
	char buf[50]; // for
	char s[sn];
	sprintf(buf, "%%%c%%s", format);
	char *f = strdup(buf);
	s[0] = '\0';
	strcat(s, "[");
	agLLNode *n = list->head;
	char *fn;
	for (; n != NULL; n = n->next)
	{
		fn = n->next != NULL ? ", " : "";
		writePtrF(buf, n->ptr, f, fn, format);
		strcat(s, buf);
	}
	strcat(s, "]");
	if (list->head == NULL)
		strcat(s, " h: NULL");
	else
	{
		writePtrF(buf, list->head->ptr, f, "", format);
		strcat(s, " h: ");
		strcat(s, buf);
	}
	if (list->tail == NULL)
		strcat(s, " t: NULL");
	else
	{
		writePtrF(buf, list->tail->ptr, f, "", format);
		strcat(s, " t: ");
		strcat(s, buf);
	}

	free(f);
	return strdup(s);
}

char *agLLToDebugString(agLinkedList *list)
{
	int sn = list->size * 35;
	char buf[sn]; // for
	char catbuf[40];
	char s[sn];
	s[0] = '\0';
	strcat(s, "[");
	agLLNode *n = list->head;
	for (int i = 0; n != NULL; n = n->next, i++)
	{
		buf[0] = '\0';
		if (n->prev == NULL)
		{
			sprintf(catbuf, "NULL <-- %p", n->ptr);
			strcat(buf, catbuf);
		}
		else if (((agLLNode *)n->prev)->next != NULL && ((agLLNode *)n->prev)->next == n)
		{
			sprintf(catbuf, " <--> %p", n->ptr);
			strcat(buf, catbuf);
		}
		else
		{
			sprintf(catbuf, " <-- %p", n->ptr);
			strcat(buf, catbuf);
		}

		if (n->next == NULL)
		{
			strcat(buf, " --> NULL");
		}
		strcat(s, buf);
	}
	strcat(s, "]");
	if (list->head == NULL)
		strcat(s, " h: NULL");
	else
	{
		sprintf(catbuf, " h: %p", list->head);
		strcat(s, catbuf);
	}
	if (list->tail == NULL)
		strcat(s, " t: NULL");
	else
	{
		sprintf(catbuf, " t: %p", list->tail);
		strcat(s, catbuf);
	}

	return strdup(s);
}
