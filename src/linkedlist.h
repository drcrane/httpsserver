#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

#include <stdint.h>

typedef struct s_linkedlist {
	struct s_linkedlist * prev;
	struct s_linkedlist * next;
	uint32_t elementType;
	void * ptr;
} linkedlist_t;

linkedlist_t * linkedlist_shift(linkedlist_t ** linkedlist);
linkedlist_t * linkedlist_pop(linkedlist_t ** linkedlist);
int linkedlist_append(linkedlist_t ** linkedlist, linkedlist_t * element);
int linkedlist_append_ptr(linkedlist_t ** linkedlist, void * ptr, uint32_t elementType);
int linkedlist_prepend(linkedlist_t ** linkedlist, linkedlist_t * element);
void linkedlist_remove(linkedlist_t ** linkedlist, linkedlist_t * element);

#endif /* LINKEDLIST_H_ */
