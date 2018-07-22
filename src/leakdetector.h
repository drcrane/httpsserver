#ifndef __LEAKDETECTOR_H__
#define __LEAKDETECTOR_H__

#include <stddef.h>

#define  LEAKDETECTOR_MAX_FILE_NAME_LENGTH   	   256

#define  malloc(size) 	    	   	leakdetector_m_alloc (size, __FILE__, __LINE__)
#define  calloc(elements, size)  	leakdetector_calloc (elements, size, __FILE__, __LINE__)
#define  free(mem_ref) 		  	 	leakdetector_free(mem_ref)

struct _MEM_INFO
{
	void			*address;
	unsigned int	size;
	char			file_name[LEAKDETECTOR_MAX_FILE_NAME_LENGTH];
	unsigned int	line;
	int thread_id;
};
typedef struct _MEM_INFO MEM_INFO;

void leakdetector_initialise();

void * leakdetector_m_alloc(size_t size, const char * file, unsigned int line);
void * leakdetector_calloc(unsigned int elements, size_t size, const char * file, unsigned int line);
void leakdetector_free(void * mem_ref);

void leakdetector_writeallocations(void);

#endif // __LEAKDETECTOR_H__
