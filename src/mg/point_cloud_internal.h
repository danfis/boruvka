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

#ifndef __MG_PC_INTERNAL_H__
#define __MG_PC_INTERNAL_H__

#include <mg/vec3.h>
#include <mg/list.h>

/**
 * One memory chunk used for storing points.
 */
struct _mg_pc_mem_t {
    mg_vec3_t *data;
    size_t len;  /*! actual number of points in data array */
    size_t size; /*! number of points that can be stored in data array */
    mg_list_t list;
};
typedef struct _mg_pc_mem_t mg_pc_mem_t;

/**
 * Creates new memory chunk.
 * The min_size argument is minimal number of points that memory chunk
 * should be able to hold. It is also base number from which will be
 * estimated amount of allocated memory.
 */
mg_pc_mem_t *mgPCMemNew(size_t min_size);
void mgPCMemDel(mg_pc_mem_t *m);

/**
 * Returns true if chunk is full.
 */
_mg_inline int mgPCMemFull(const mg_pc_mem_t *m);

/**
 * Returns point stored on i'th position. No boundaries are checked!
 */
_mg_inline mg_vec3_t *mgPCMemGet(mg_pc_mem_t *m, size_t i);

/**
 * Adds point to chunk. No boundaries are checked!
 */
_mg_inline void mgPCMemAdd(mg_pc_mem_t *m, const mg_vec3_t *v);
_mg_inline void mgPCMemAddCoords(mg_pc_mem_t *m,
                                 mg_real_t x, mg_real_t y, mg_real_t z);



/**** INLINES ****/
_mg_inline int mgPCMemFull(const mg_pc_mem_t *m)
{
    return m->len >= m->size;
}

_mg_inline mg_vec3_t *mgPCMemGet(mg_pc_mem_t *m, size_t i)
{
    return m->data + i;
}

_mg_inline void mgPCMemAdd(mg_pc_mem_t *m, const mg_vec3_t *v)
{
    mgVec3Copy(m->data + m->len, v);
    m->len++;
}

_mg_inline void mgPCMemAddCoords(mg_pc_mem_t *m,
                                 mg_real_t x, mg_real_t y, mg_real_t z)
{
    mgVec3Set(m->data + m->len, x, y, z);
    m->len++;
}

#endif /* __MG_PC_INTERNAL_H__ */

