/***
 * fermat
 * -------
 * Copyright (c)2010-2011 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __FER_PC4_H__
#define __FER_PC4_H__

#include <fermat/vec4.h>
#include <fermat/pc-internal.h>

#ifndef FER_PC4_MIN_CHUNK_SIZE
# define FER_PC4_MIN_CHUNK_SIZE 100000
#endif /* FER_PC4_MIN_CHUNK_SIZE */

/**
 * Point Cloud - 4-D
 * ======================
 *
 * Struct representing point cloud.
 * Point cloud is internally allocated using several memory chunks to be
 * able to represent huge amount points.
 */
struct _fer_pc4_t {
    fer_list_t head; /*!< Head of list of memory chunks (fer_pc_mem_t) */
    size_t len;      /*! Overall number of points */

    fer_real_t aabb[4 * 2]; /*!< Axis aligned bounding box encapsulating point cloud
                                     [x_min, x_max, y_min, y_max, z_min, z_max] */

    size_t min_chunk_size; /*!< min number of points per chunk, by default
                                FER_PC4_MIN_CHUNK_SIZE */
};
typedef struct _fer_pc4_t fer_pc4_t;


/**
 * Functions
 * ----------
 */

/**
 * Creates point cloud.
 */
fer_pc4_t *ferPC4New(void);

/**
 * Delets point cloud.
 */
void ferPC4Del(fer_pc4_t *);

/**
 * Changes minimal points per memory chunk.
 */
_fer_inline void ferPC4SetMinChunkSize(fer_pc4_t *pc, size_t size);

/**
 * Adds point to point cloud.
 */
void ferPC4Add(fer_pc4_t *pc, const fer_vec4_t *v);

/**
 * Returns number of points in point cloud.
 */
_fer_inline size_t ferPC4Len(const fer_pc4_t *pc);

/**
 * Returns n'th point from point cloud.
 * Note that this function must iterate over memory chunks to find the
 * point. Use iterators if you don't need random access.
 */
fer_vec4_t *ferPC4Get(fer_pc4_t *pc, size_t n);

/**
 * Permutates points in point cloud.
 * Permutated pc can be used for random access to whole point clouds' pool.
 */
void ferPC4Permutate(fer_pc4_t *pc);


/**
 * Adds points from given file.
 * File must have one point on each file.
 * Returns number of added points.
 */
size_t ferPC4AddFromFile(fer_pc4_t *pc, const char *filename);

/**
 * Returns axis-aligned bounding box of all points in point cloud.
 * [xmin, xmax, ymin, ymax, zmin, zmax]
 */
_fer_inline const fer_real_t *ferPC4AABB(const fer_pc4_t *pc);



/**
 * Point Cloud Iterator
 * ---------------------
 *
 * Iterator over point cloud.
 * See ferPC4It*() functions.
 */
struct _fer_pc4_it_t {
    fer_pc4_t *pc;     /*!< Reference to point cloud */
    fer_pc_mem_t *mem; /*!< Current mem chunk */
    size_t pos;        /*!< Current position in mem chunk */
};
typedef struct _fer_pc4_it_t fer_pc4_it_t;


/**
 * Functions
 * ^^^^^^^^^^
 */

/**
 * Initializes point cloud iterator and set iterator to beggining.
 */
_fer_inline void ferPC4ItInit(fer_pc4_it_t *it, fer_pc4_t *pc);

/**
 * Returns true if iterator points at (or after) end of point cloud.
 */
_fer_inline int ferPC4ItEnd(const fer_pc4_it_t *it);

/**
 * Returns point from current position of iterator.
 * No boundaries are checked!
 */
_fer_inline fer_vec4_t *ferPC4ItGet(fer_pc4_it_t *it);

/**
 * Moves iterator to next (prev) position.
 * No boundaries are checked!
 */
_fer_inline void ferPC4ItNext(fer_pc4_it_t *it);
_fer_inline void ferPC4ItPrev(fer_pc4_it_t *it);


/**** INLINES ****/
_fer_inline void ferPC4SetMinChunkSize(fer_pc4_t *pc, size_t size)
{
    pc->min_chunk_size = size;
}

_fer_inline size_t ferPC4Len(const fer_pc4_t *pc)
{
    return pc->len;
}

_fer_inline const fer_real_t *ferPC4AABB(const fer_pc4_t *pc)
{
    return pc->aabb;
}



_fer_inline void ferPC4ItInit(fer_pc4_it_t *it, fer_pc4_t *pc)
{
    it->pc = pc;
    it->mem = ferListEntry(ferListNext(&pc->head), fer_pc_mem_t, list);
    it->pos = 0;
}

_fer_inline int ferPC4ItEnd(const fer_pc4_it_t *it)
{
    return ferListNext(&it->mem->list) == &it->pc->head
                && it->pos >= it->mem->len;
}

_fer_inline fer_vec4_t *ferPC4ItGet(fer_pc4_it_t *it)
{
    return ferPCMemGet(it->mem, it->pos, fer_vec4_t);
}

_fer_inline void ferPC4ItNext(fer_pc4_it_t *it)
{
    it->pos++;
    if (it->pos >= it->mem->len
            && ferListNext(&it->mem->list) != &it->pc->head){
        it->mem = ferListEntry(ferListNext(&it->mem->list), fer_pc_mem_t, list);
        it->pos = 0;
    }
}

_fer_inline void ferPC4ItPrev(fer_pc4_it_t *it)
{
    if (it->pos == 0 && ferListPrev(&it->mem->list) != &it->pc->head){
        it->mem = ferListEntry(ferListPrev(&it->mem->list), fer_pc_mem_t, list);
        it->pos = it->mem->len - 1;
    }else{
        it->pos--;
    }
}
#endif /* __FER_PC4_H__ */
