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

#ifndef __MG_PC_H__
#define __MG_PC_H__

#include <mg/vec3.h>
#include <mg/point_cloud_internal.h>

#ifndef MG_PC_MIN_CHUNK_SIZE
# define MG_PC_MIN_CHUNK_SIZE 100000
#endif /* MG_PC_MIN_CHUNK_SIZE */

/**
 * Struct representing point cloud.
 * Point cloud is internally allocated using several memory chunks to be
 * able to represent huge amount points.
 */
struct _mg_pc_t {
    mg_list_t head; /*!< Head of list of memory chunks (mg_pc_mem_t) */
    size_t len;     /*! Overall number of points */

    mg_real_t aabb[6]; /*!< Axis aligned bounding box encapsulating point cloud
                            [x_min, x_max, y_min, y_max, z_min, z_max] */

    size_t min_chunk_size; /*!< min number of points per chunk, by default
                                MG_PC_MIN_CHUNK_SIZE */
};
typedef struct _mg_pc_t mg_pc_t;


/**
 * Iterator over point cloud.
 * See mgPCIt*() functions.
 */
struct _mg_pc_it_t {
    mg_pc_t *pc; /*!< Reference point cloud */
    mg_pc_mem_t *mem; /*!< Current mem chunk */
    size_t pos;       /*!< Current position in mem chunk */
};
typedef struct _mg_pc_it_t mg_pc_it_t;


/**
 * Creates and deletes point cloud.
 */
mg_pc_t *mgPCNew(void);
void mgPCDel(mg_pc_t *);

/**
 * Changes minimal points per memory chunk.
 */
_mg_inline void mgPCSetMinChunkSize(mg_pc_t *pc, size_t size);

/**
 * Adds point to point cloud.
 */
_mg_inline void mgPCAdd(mg_pc_t *pc, const mg_vec3_t *v);
void mgPCAddCoords(mg_pc_t *pc, mg_real_t x, mg_real_t y, mg_real_t z);

/**
 * Returns number of points in point cloud.
 */
_mg_inline size_t mgPCLen(const mg_pc_t *pc);

/**
 * Returns n'th point from point cloud.
 * Note that this function must iterate over memory chunks to find the
 * point. Use iterators if you don't need random access.
 */
mg_vec3_t *mgPCGet(mg_pc_t *pc, size_t n);

/**
 * Permutates points in point cloud.
 * Permutated pc can be used for random access to whole point clouds' pool.
 */
void mgPCPermutate(mg_pc_t *pc);




/**
 * Initializes point cloud iterator and set iterator to beggining.
 */
_mg_inline void mgPCItInit(mg_pc_it_t *it, mg_pc_t *pc);

/**
 * Returns true if iterator points at (or after) end of point cloud.
 */
_mg_inline int mgPCItEnd(const mg_pc_it_t *it);

/**
 * Returns point from current position of iterator.
 * No boundaries are checked!
 */
_mg_inline mg_vec3_t *mgPCItGet(mg_pc_it_t *it);

/**
 * Moves iterator to next (prev) position.
 * No boundaries are checked!
 */
_mg_inline void mgPCItNext(mg_pc_it_t *it);
_mg_inline void mgPCItPrev(mg_pc_it_t *it);


/**** INLINES ****/
_mg_inline void mgPCSetMinChunkSize(mg_pc_t *pc, size_t size)
{
    pc->min_chunk_size = size;
}

_mg_inline void mgPCAdd(mg_pc_t *pc, const mg_vec3_t *v)
{
    mgPCAddCoords(pc, mgVec3X(v), mgVec3Y(v), mgVec3Z(v));
}

_mg_inline size_t mgPCLen(const mg_pc_t *pc)
{
    return pc->len;
}




_mg_inline void mgPCItInit(mg_pc_it_t *it, mg_pc_t *pc)
{
    it->pc = pc;
    it->mem = mgListEntry(mgListNext(&pc->head), mg_pc_mem_t, list);
    it->pos = 0;
}

_mg_inline int mgPCItEnd(const mg_pc_it_t *it)
{
    return mgListNext(&it->mem->list) == &it->pc->head
                && it->pos >= it->mem->len;
}

_mg_inline mg_vec3_t *mgPCItGet(mg_pc_it_t *it)
{
    return mgPCMemGet(it->mem, it->pos);
}

_mg_inline void mgPCItNext(mg_pc_it_t *it)
{
    it->pos++;
    if (it->pos >= it->mem->len
            && mgListNext(&it->mem->list) != &it->pc->head){
        it->mem = mgListEntry(mgListNext(&it->mem->list), mg_pc_mem_t, list);
        it->pos = 0;
    }
}

_mg_inline void mgPCItPrev(mg_pc_it_t *it)
{
    if (it->pos == 0 && mgListPrev(&it->mem->list) != &it->pc->head){
        it->mem = mgListEntry(mgListPrev(&it->mem->list), mg_pc_mem_t, list);
        it->pos = it->mem->len - 1;
    }else{
        it->pos--;
    }
}
#endif /* __MG_PC_H__ */
