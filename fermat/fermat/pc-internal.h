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

#include <fermat/vec3.h>
#include <fermat/list.h>

/**
 * One memory chunk used for storing points.
 */
struct _fer_pc_mem_t {
    fer_vec3_t *data;
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
fer_pc_mem_t *ferPCMemNew(size_t min_size);
void ferPCMemDel(fer_pc_mem_t *m);

/**
 * Returns true if chunk is full.
 */
_fer_inline int ferPCMemFull(const fer_pc_mem_t *m);

/**
 * Returns point stored on i'th position. No boundaries are checked!
 */
_fer_inline fer_vec3_t *ferPCMemGet(fer_pc_mem_t *m, size_t i);

/**
 * Adds point to chunk. No boundaries are checked!
 */
_fer_inline void ferPCMemAdd(fer_pc_mem_t *m, const fer_vec3_t *v);
_fer_inline void ferPCMemAddCoords(fer_pc_mem_t *m,
                                 fer_real_t x, fer_real_t y, fer_real_t z);



/**** INLINES ****/
_fer_inline int ferPCMemFull(const fer_pc_mem_t *m)
{
    return m->len >= m->size;
}

_fer_inline fer_vec3_t *ferPCMemGet(fer_pc_mem_t *m, size_t i)
{
    return m->data + i;
}

_fer_inline void ferPCMemAdd(fer_pc_mem_t *m, const fer_vec3_t *v)
{
    ferVec3Copy(m->data + m->len, v);
    m->len++;
}

_fer_inline void ferPCMemAddCoords(fer_pc_mem_t *m,
                                 fer_real_t x, fer_real_t y, fer_real_t z)
{
    ferVec3Set(m->data + m->len, x, y, z);
    m->len++;
}

#endif /* __FER_PC_INTERNAL_H__ */

