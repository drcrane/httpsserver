#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "linkedlist.h"
#include "leakdetector.h"

#undef		malloc
#undef		calloc
#undef 		free

//#include <windows.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat="

//static HANDLE malloc_mutex;

static volatile linkedlist_t * allocatedlist;

void leakdetector_initialise() {
//	malloc_mutex = CreateMutex(NULL, FALSE, NULL);
	allocatedlist = NULL;
}

/*
 * replacement of malloc
 */
void * leakdetector_m_alloc (size_t size, const char * file, unsigned int line) {
//	WaitForSingleObject(malloc_mutex, INFINITE);

	void * ptr = malloc(size);

	if (ptr != NULL) {
		MEM_INFO * mem_info = NULL;
		linkedlist_t * element = malloc(sizeof(linkedlist_t) + sizeof(MEM_INFO));
		linkedlist_t * ll;

		mem_info = (MEM_INFO *)(element + 1);
		mem_info->address = ptr;
		mem_info->size = size;
		strncpy(mem_info->file_name, file, LEAKDETECTOR_MAX_FILE_NAME_LENGTH);
		mem_info->line = line;
//		mem_info->thread_id = (int)GetCurrentThreadId();

		element->elementType = 0;
		element->next = NULL;
		element->prev = NULL;
		element->ptr = mem_info;
		ll = (linkedlist_t *)allocatedlist;
		linkedlist_append(&ll, element);
		allocatedlist = ll;
	}

//	ReleaseMutex(malloc_mutex);

	return ptr;
}

/*
 * replacement of calloc
 */
void * leakdetector_calloc (unsigned int elements, size_t size, const char * file, unsigned int line)
{
	void * ptr = calloc(elements, size);
	return ptr;
}


/*
 * replacement of free
 */
void leakdetector_free(void * mem_ref)
{
//	WaitForSingleObject(malloc_mutex, INFINITE);

	linkedlist_t * element;
	linkedlist_t * ll;
	MEM_INFO * mi_element;

	element = (linkedlist_t *)allocatedlist;
	ll = (linkedlist_t *)allocatedlist;
	while (element != NULL) {
		mi_element = element->ptr;
		if (mi_element->address == mem_ref) {
			linkedlist_remove(&ll, element);
			allocatedlist = ll;
			free(element);
			break;
		}
		element = element->next;
	}

	free(mem_ref);

//	ReleaseMutex(malloc_mutex);
}

void leakdetector_writeallocations(void) {
	linkedlist_t * element;
	MEM_INFO * mi_element;

	FILE * fp_write = fopen ("LEAKDETECTOR.txt", "wt");

	element = (linkedlist_t *)allocatedlist;
	while (element != NULL) {
		mi_element = element->ptr;
		fprintf(fp_write, "Block: 0x%llx %d @ %s:%d\n", mi_element->address, mi_element->size, mi_element->file_name, mi_element->line);
		element = element->next;
	}

	fclose(fp_write);
}

#pragma GCC diagnostic pop
