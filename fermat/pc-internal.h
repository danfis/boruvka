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

#ifndef __FER_PC_INTERNAL_H__
#define __FER_PC_INTERNAL_H__

#include <fermat/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * One memory chunk used for storing points.
 */
struct _fer_pc_mem_t {
    void *data;
    size_t len;  /*! actual number of points in data array */
    size_t size; /*! number of points that can be stored in data array */
    fer_list_t list;
};
typedef struct _fer_pc_mem_t fer_pc_mem_t;

/**
 * Creates new memory chunk.
 * The min_size argument is minimal number of points that memory chunk
 * should be able to hold. It is also base number from which will be
 * estimated amount of allocated memory.
 */
fer_pc_mem_t *ferPCMemNew(size_t min_size, size_t elsize, int align);
void ferPCMemDel(fer_pc_mem_t *m);

/**
 * Returns true if chunk is full.
 */
_fer_inline int ferPCMemFull(const fer_pc_mem_t *m);

/**
 * Returns element (of given type) stored on pos'th position.
 * No boundaries are checked!
 */
#define ferPCMemGet(mem, pos, TYPE) \
    (TYPE *)((char *)(mem)->data + ((pos) * sizeof(TYPE)))

/**
 * Adds element (of given type) to chunk.
 * No boundaries are checked!
 */
#define ferPCMemAdd(mem, el, TYPE) \
    do { \
        *((TYPE *)((char *)(mem)->data + ((mem)->len * sizeof(TYPE)))) = *(el); \
        (mem)->len++; \
    } while (0)



/**** INLINES ****/
_fer_inline int ferPCMemFull(const fer_pc_mem_t *m)
{
    return m->len >= m->size;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_PC_INTERNAL_H__ */

