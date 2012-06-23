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

#ifndef __BOR_PC_INTERNAL_H__
#define __BOR_PC_INTERNAL_H__

#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * One memory chunk used for storing points.
 */
struct _bor_pc_mem_t {
    void *data;
    size_t len;  /*! actual number of points in data array */
    size_t size; /*! number of points that can be stored in data array */
    bor_list_t list;
};
typedef struct _bor_pc_mem_t bor_pc_mem_t;

/**
 * Creates new memory chunk.
 * The min_size argument is minimal number of points that memory chunk
 * should be able to hold. It is also base number from which will be
 * estimated amount of allocated memory.
 */
bor_pc_mem_t *borPCMemNew(size_t min_size, size_t elsize, int align);
void borPCMemDel(bor_pc_mem_t *m);

/**
 * Returns true if chunk is full.
 */
_bor_inline int borPCMemFull(const bor_pc_mem_t *m);

/**
 * Returns element (of given type) stored on pos'th position.
 * No boundaries are checked!
 */
#define borPCMemGet(mem, pos, TYPE) \
    borPCMemGet2(mem, pos, TYPE, sizeof(TYPE))

#define borPCMemGet2(mem, pos, TYPE, SIZE) \
    (TYPE *)((char *)(mem)->data + ((pos) * SIZE))

/**
 * Adds element (of given type) to chunk.
 * No boundaries are checked!
 */
#define borPCMemAdd(mem, el, TYPE) \
    borPCMemAdd2(mem, el, TYPE, sizeof(TYPE))

#define borPCMemAdd2(mem, el, TYPE, SIZE) \
    do { \
        *((TYPE *)((char *)(mem)->data + ((mem)->len * SIZE))) = *(el); \
        (mem)->len++; \
    } while (0)

#define borPCMemAdd2Memcpy(mem, el, TYPE, SIZE) \
    do { \
        memcpy((void *)((char *)(mem)->data + ((mem)->len * SIZE)), el, SIZE); \
        (mem)->len++; \
    } while (0)



/**** INLINES ****/
_bor_inline int borPCMemFull(const bor_pc_mem_t *m)
{
    return m->len >= m->size;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_PC_INTERNAL_H__ */

