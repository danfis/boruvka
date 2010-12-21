/***
 * mg
 * ---
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of mg.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __MG_ALLOC_H__
#define __MG_ALLOC_H__

#include <stdlib.h>
#include <mg/core.h>

/**
 * Functions and macros required for memory allocation.
 */

/* Memory allocation: */
#define _MG_ALLOC_MEMORY(type, ptr_old, size) \
    (type *)mgRealloc((void *)ptr_old, (size))

/** Allocate memory for one element of type.  */
#define MG_ALLOC(type) \
    _MG_ALLOC_MEMORY(type, NULL, sizeof(type))

/** Allocates aligned memory */
#define MG_ALLOC_ALIGN(type, align) \
    (type *)mgAllocAlign(sizeof(type), align)

/** Allocates aligned array */
#define MG_ALLOC_ALIGN_ARR(type, num_els, align) \
    (type *)mgAllocAlign(sizeof(type) * (num_els), align)

/** Allocate memory for array of elements of type type.  */
#define MG_ALLOC_ARR(type, num_elements) \
    _MG_ALLOC_MEMORY(type, NULL, sizeof(type) * (num_elements))

#define MG_REALLOC_ARR(ptr, type, num_elements) \
    _MG_ALLOC_MEMORY(type, ptr, sizeof(type) * (num_elements))

void *mgRealloc(void *ptr, size_t size);
void *mgAllocAlign(size_t size, size_t alignment);

#endif /* __MG_ALLOC_H__ */
