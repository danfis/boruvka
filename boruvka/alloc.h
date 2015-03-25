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
#include <string.h>
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
#ifdef BOR_MEMCHECK
# define _BOR_ALLOC_MEMORY(type, ptr_old, size) \
    (type *)borRealloc((void *)ptr_old, (size), __FILE__, __LINE__, __func__)

# define _BOR_ALLOC_ALIGN_MEMORY(type, size, align) \
    (type *)borAllocAlign((size), (align), __FILE__, __LINE__, __func__)

# define _BOR_CALLOC_MEMORY(type, num_elements, size_of_el) \
    (type *)borCalloc((num_elements), (size_of_el), __FILE__, __LINE__, __func__)

# define _BOR_STRDUP(str) borStrdup((str), __FILE__, __LINE__, __func__)

# define BOR_FREE(ptr) borFreeCheck((ptr), __FILE__, __LINE__, __func__)

#else /* BOR_MEMCHECK */
# define _BOR_ALLOC_MEMORY(type, ptr_old, size) \
    (type *)borRealloc((void *)ptr_old, (size))

# define _BOR_ALLOC_ALIGN_MEMORY(type, size, align) \
    (type *)borAllocAlign((size), (align))

# define _BOR_CALLOC_MEMORY(type, num_elements, size_of_el) \
    (type *)borCalloc((num_elements), (size_of_el))

# define _BOR_STRDUP(str) borStrdup((str))

# define BOR_FREE(ptr) free(ptr) /*!< Deallocates memory */
#endif /* BOR_MEMCHECK */



/**
 * Allocate memory for one element of given type.
 */
#define BOR_ALLOC(type) \
    _BOR_ALLOC_MEMORY(type, NULL, sizeof(type))

/**
 * Allocates aligned memory
 */
#define BOR_ALLOC_ALIGN(type, align) \
    _BOR_ALLOC_ALIGN_MEMORY(type, sizeof(type), align)

/**
 * Allocates aligned array
 */
#define BOR_ALLOC_ALIGN_ARR(type, num_els, align) \
    _BOR_ALLOC_ALIGN_MEMORY(type, sizeof(type) * (num_els), align)

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

/**
 * Allocate array of elements of given type initialized to zero.
 */
#define BOR_CALLOC_ARR(type, num_elements) \
    _BOR_CALLOC_MEMORY(type, (num_elements), sizeof(type))

/**
 * Raw memory allocation.
 */
#define BOR_MALLOC(size) \
    _BOR_ALLOC_MEMORY(void, NULL, (size))

/**
 * Raw realloc
 */
#define BOR_REALLOC(ptr, size) \
    _BOR_ALLOC_MEMORY(void, (ptr), (size))

/**
 * Wrapped strdup() for consistency in memory allocation.
 */
#define BOR_STRDUP(str) \
    _BOR_STRDUP(str)


#ifdef BOR_MEMCHECK
void *borRealloc(void *ptr, size_t size,
                 const char *file, int line, const char *func);
void *borAllocAlign(size_t size, size_t alignment,
                    const char *file, int line, const char *func);
void *borCalloc(size_t nmemb, size_t size,
                const char *file, int line, const char *func);
char *borStrdup(const char *str, const char *file, int line, const char *func);
void borFreeCheck(void *ptr, const char *file, int line, const char *func);

#else /* BOR_MEMCHECK */
void *borRealloc(void *ptr, size_t size);
void *borAllocAlign(size_t size, size_t alignment);
void *borCalloc(size_t nmemb, size_t size);
char *borStrdup(const char *str);
#endif /* BOR_MEMCHECK */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_ALLOC_H__ */
