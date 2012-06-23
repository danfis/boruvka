/***
 * Boruvka
 * --------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __BOR_ALLOC_H__
#define __BOR_ALLOC_H__

#include <stdlib.h>
#include <boruvka/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Alloc - Memory Allocation
 * ==========================
 *
 * Functions and macros for memory allocation.
 */

/* Memory allocation: - internal macro */
#define _BOR_ALLOC_MEMORY(type, ptr_old, size) \
    (type *)borRealloc((void *)ptr_old, (size))

/**
 * Allocate memory for one element of given type.
 */
#define BOR_ALLOC(type) \
    _BOR_ALLOC_MEMORY(type, NULL, sizeof(type))

/**
 * Allocates aligned memory
 */
#define BOR_ALLOC_ALIGN(type, align) \
    (type *)borAllocAlign(sizeof(type), align)

/**
 * Allocates aligned array
 */
#define BOR_ALLOC_ALIGN_ARR(type, num_els, align) \
    (type *)borAllocAlign(sizeof(type) * (num_els), align)

/**
 * Allocate array of elements of given type.
 */
#define BOR_ALLOC_ARR(type, num_elements) \
    _BOR_ALLOC_MEMORY(type, NULL, sizeof(type) * (num_elements))

/**
 * Reallocates array.
 */
#define BOR_REALLOC_ARR(ptr, type, num_elements) \
    _BOR_ALLOC_MEMORY(type, ptr, sizeof(type) * (num_elements))

#ifndef BOR_MEMCHECK
# define BOR_FREE(ptr) free(ptr) /*!< Deallocates memory */
#else /* BOR_MEMCHECK */
# define BOR_FREE(ptr) borFreeCheck(ptr)
#endif /* BOR_MEMCHECK */

void *borRealloc(void *ptr, size_t size);
void *borAllocAlign(size_t size, size_t alignment);

#ifdef BOR_MEMCHECK
void borFreeCheck(void *ptr);
#endif /* BOR_MEMCHECK */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_ALLOC_H__ */
