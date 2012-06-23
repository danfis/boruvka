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

#ifndef __FER_ALLOC_H__
#define __FER_ALLOC_H__

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
#define _FER_ALLOC_MEMORY(type, ptr_old, size) \
    (type *)ferRealloc((void *)ptr_old, (size))

/**
 * Allocate memory for one element of given type.
 */
#define FER_ALLOC(type) \
    _FER_ALLOC_MEMORY(type, NULL, sizeof(type))

/**
 * Allocates aligned memory
 */
#define FER_ALLOC_ALIGN(type, align) \
    (type *)ferAllocAlign(sizeof(type), align)

/**
 * Allocates aligned array
 */
#define FER_ALLOC_ALIGN_ARR(type, num_els, align) \
    (type *)ferAllocAlign(sizeof(type) * (num_els), align)

/**
 * Allocate array of elements of given type.
 */
#define FER_ALLOC_ARR(type, num_elements) \
    _FER_ALLOC_MEMORY(type, NULL, sizeof(type) * (num_elements))

/**
 * Reallocates array.
 */
#define FER_REALLOC_ARR(ptr, type, num_elements) \
    _FER_ALLOC_MEMORY(type, ptr, sizeof(type) * (num_elements))

#ifndef FER_MEMCHECK
# define FER_FREE(ptr) free(ptr) /*!< Deallocates memory */
#else /* FER_MEMCHECK */
# define FER_FREE(ptr) ferFreeCheck(ptr)
#endif /* FER_MEMCHECK */

void *ferRealloc(void *ptr, size_t size);
void *ferAllocAlign(size_t size, size_t alignment);

#ifdef FER_MEMCHECK
void ferFreeCheck(void *ptr);
#endif /* FER_MEMCHECK */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_ALLOC_H__ */
