/***
 * Boruvka
 * --------
 * Copyright (c)2010-2011 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_PC_H__
#define __BOR_PC_H__

#include <boruvka/vec.h>
#include <boruvka/rand-mt.h>
#include <boruvka/pc-internal.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef BOR_PC_MIN_CHUNK_SIZE
# define BOR_PC_MIN_CHUNK_SIZE (1024 * 1024)
#endif /* BOR_PC_MIN_CHUNK_SIZE */

/**
 * Point Cloud
 * ============
 *
 * Struct representing point cloud.
 * Point cloud is internally allocated using several memory chunks to be
 * able to represent huge amount points.
 */
struct _bor_pc_t {
    size_t dim;      /*!< Dimension of points stored in pc */
    bor_list_t head; /*!< Head of list of memory chunks (bor_pc_mem_t) */
    size_t len;      /*! Overall number of points */

    size_t min_chunk_size; /*!< min number of points per chunk, by default
                                BOR_PC_MIN_CHUNK_SIZE */

    bor_rand_mt_t *rand;
};
typedef struct _bor_pc_t bor_pc_t;


/**
 * Functions
 * ----------
 */

/**
 * Creates point cloud.
 */
bor_pc_t *borPCNew(size_t dim);

/**
 * Creates point cloud.
 */
bor_pc_t *borPCNew2(size_t dim, size_t min_chunk_size);

/**
 * Delets point cloud.
 */
void borPCDel(bor_pc_t *);

/**
 * Adds point to point cloud.
 */
void borPCAdd(bor_pc_t *pc, const bor_vec_t *v);

/**
 * Returns number of points in point cloud.
 */
_bor_inline size_t borPCLen(const bor_pc_t *pc);

/**
 * Returns n'th point from point cloud.
 * Note that this function must iterate over memory chunks to find the
 * point. Use iterators if you don't need random access.
 */
bor_vec_t *borPCGet(bor_pc_t *pc, size_t n);

/**
 * Permutates points in point cloud.
 * Permutated pc can be used for random access to whole point clouds' pool.
 */
void borPCPermutate(bor_pc_t *pc);


/**
 * Adds points from given file.
 * File must have one point on each file.
 * Returns number of added points.
 */
size_t borPCAddFromFile(bor_pc_t *pc, const char *filename);

/**
 * Sets {aabb} array which must have at least 2 * dim items to axis aligned
 * bounding box of points in point cloud.
 */
void borPCAABB(const bor_pc_t *pc, bor_real_t *aabb);


/**
 * Point Cloud Iterator
 * ---------------------
 *
 * Iterator over point cloud.
 * See borPCIt*() functions.
 */
struct _bor_pc_it_t {
    bor_pc_t *pc;      /*!< Reference to point cloud */
    bor_pc_mem_t *mem; /*!< Current mem chunk */
    size_t pos;        /*!< Current position in mem chunk */
};
typedef struct _bor_pc_it_t bor_pc_it_t;


/**
 * Functions
 * ^^^^^^^^^^
 */

/**
 * Initializes point cloud iterator and set iterator to beggining.
 */
_bor_inline void borPCItInit(bor_pc_it_t *it, bor_pc_t *pc);

/**
 * Returns true if iterator points at (or after) end of point cloud.
 */
_bor_inline int borPCItEnd(const bor_pc_it_t *it);

/**
 * Returns point from current position of iterator.
 * No boundaries are checked!
 */
_bor_inline bor_vec_t *borPCItGet(bor_pc_it_t *it);

/**
 * Moves iterator to next (prev) position.
 * No boundaries are checked!
 */
_bor_inline void borPCItNext(bor_pc_it_t *it);
_bor_inline void borPCItPrev(bor_pc_it_t *it);


/**** INLINES ****/
_bor_inline size_t borPCLen(const bor_pc_t *pc)
{
    return pc->len;
}




_bor_inline void borPCItInit(bor_pc_it_t *it, bor_pc_t *pc)
{
    it->pc = pc;
    it->mem = BOR_LIST_ENTRY(borListNext(&pc->head), bor_pc_mem_t, list);
    it->pos = 0;
}

_bor_inline int borPCItEnd(const bor_pc_it_t *it)
{
    return borListNext(&it->mem->list) == &it->pc->head
                && it->pos >= it->mem->len;
}

_bor_inline bor_vec_t *borPCItGet(bor_pc_it_t *it)
{
    return borPCMemGet2(it->mem, it->pos, bor_vec_t, it->pc->dim * sizeof(bor_vec_t));
}

_bor_inline void borPCItNext(bor_pc_it_t *it)
{
    it->pos++;
    if (it->pos >= it->mem->len
            && borListNext(&it->mem->list) != &it->pc->head){
        it->mem = BOR_LIST_ENTRY(borListNext(&it->mem->list), bor_pc_mem_t, list);
        it->pos = 0;
    }
}

_bor_inline void borPCItPrev(bor_pc_it_t *it)
{
    if (it->pos == 0 && borListPrev(&it->mem->list) != &it->pc->head){
        it->mem = BOR_LIST_ENTRY(borListPrev(&it->mem->list), bor_pc_mem_t, list);
        it->pos = it->mem->len - 1;
    }else{
        it->pos--;
    }
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_PC_H__ */

