#ifndef __MG_ALLOC_H__
#define __MG_ALLOC_H__

#include <stdlib.h>

/**
 * Functions and macros required for memory allocation.
 */

/* Memory allocation: */
#define _MG_ALLOC_MEMORY(type, ptr_old, size) \
    (type *)mgRealloc((void *)ptr_old, (size))

/** Allocate memory for one element of type.  */
#define MG_ALLOC(type) \
    _MG_ALLOC_MEMORY(type, NULL, sizeof(type))

/** Allocate memory for array of elements of type type.  */
#define MG_ALLOC_ARR(type, num_elements) \
    _MG_ALLOC_MEMORY(type, NULL, sizeof(type) * (num_elements))

#define MG_REALLOC_ARR(ptr, type, num_elements) \
    _MG_ALLOC_MEMORY(type, ptr, sizeof(type) * (num_elements))

void *mgRealloc(void *ptr, size_t size);

#endif /* __MG_ALLOC_H__ */
