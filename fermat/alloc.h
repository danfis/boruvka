/***
 * fermat
 * -------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
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
#include <fermat/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Functions and macros required for memory allocation.
 */

/* Memory allocation: */
#define _FER_ALLOC_MEMORY(type, ptr_old, size) \
    (type *)ferRealloc((void *)ptr_old, (size))

/** Allocate memory for one element of type.  */
#define FER_ALLOC(type) \
    _FER_ALLOC_MEMORY(type, NULL, sizeof(type))

/** Allocates aligned memory */
#define FER_ALLOC_ALIGN(type, align) \
    (type *)ferAllocAlign(sizeof(type), align)

/** Allocates aligned array */
#define FER_ALLOC_ALIGN_ARR(type, num_els, align) \
    (type *)ferAllocAlign(sizeof(type) * (num_els), align)

/** Allocate memory for array of elements of type type.  */
#define FER_ALLOC_ARR(type, num_elements) \
    _FER_ALLOC_MEMORY(type, NULL, sizeof(type) * (num_elements))

#define FER_REALLOC_ARR(ptr, type, num_elements) \
    _FER_ALLOC_MEMORY(type, ptr, sizeof(type) * (num_elements))

void *ferRealloc(void *ptr, size_t size);
void *ferAllocAlign(size_t size, size_t alignment);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_ALLOC_H__ */
