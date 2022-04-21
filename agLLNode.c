#include "agLLNode.h"
#include <stdio.h>
#include <string.h>

void agNodeConnect(agLLNode *n1, agLLNode *n2)
{
    if (n1 == NULL || n2 == NULL)
    {
        fprintf(stderr, "ERROR: Cannot connect a null node (agLinkedList)\n");
        return;
    }
    n1->next = n2;
    n2->prev = n1;
}

agLLNode *agNodeInit(void *ptr, size_t size)
{
	void *p = malloc(size);
    agLLNode *n = malloc(sizeof(agLLNode));

	if (p == NULL || n == NULL) {
		perror("agLLNode malloc()");
		exit(EXIT_FAILURE);
	}

	memcpy(p, ptr, size);

    n->ptr = p;
    n->prev = NULL;
    n->next = NULL;

    return n;
}

