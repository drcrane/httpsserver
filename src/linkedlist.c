#include "linkedlist.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#ifndef NULL
#define NULL (void *)0
#endif

int linkedlist_append(linkedlist_t ** linkedlist, linkedlist_t * element) {
	linkedlist_t * ll = *linkedlist;
	if (ll == NULL) {
		*linkedlist = element;
		ll = *linkedlist;
		ll->prev = NULL;
		ll->next = NULL;
		return 0;
	}
	while (ll->next != NULL) {
		ll = ll->next;
	}
	element->next = NULL;
	ll->next = element;
	element->prev = ll;
	return 0;
}

linkedlist_t * linkedlist_shift(linkedlist_t ** linkedlist) {
	linkedlist_t * ll = *linkedlist;
	linkedlist_t * element = ll;
	if (ll == NULL) {
		return NULL;
	}
	if (ll->next != NULL) {
		ll = ll->next;
		ll->prev = NULL;
	} else {
		ll = NULL;
	}
	*linkedlist = ll;
	return element;
}

void linkedlist_remove(linkedlist_t ** linkedlist, linkedlist_t * element) {
	if (*linkedlist == NULL) {
		return;
	}
	if (element->prev != NULL) {
		element->prev->next = element->next;
	}
	if (element->next != NULL) {
		element->next->prev = element->prev;
	}
	if (element == *linkedlist) {
		*linkedlist = element->next;
	}
	element->next = NULL;
	element->prev = NULL;
}

void linkedlist_insertafter(linkedlist_t * ll_element, linkedlist_t * element) {

}
