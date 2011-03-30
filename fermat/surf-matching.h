/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __FER_SURF_MATCHING_H__
#define __FER_SURF_MATCHING_H__

#include <fermat/vec.h>
#include <fermat/opencl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_surf_match_t {
    fer_vec_t *vecs1;
    fer_vec_t *vecs2;

    fer_cl_t *cl;

    fer_real_t *dist2; /*!< Stores aquare of distances to nearest surfs */
    int *nearest;      /*!< Array of indicies of nearest surfs */
};
typedef struct _fer_surf_match_t fer_surf_match_t;

/**
 * Creates struct for surf matching, maximal lengths of first and second
 * array of vectors must be provided.
 */
fer_surf_match_t *ferSurfMatchNew(size_t maxlen1, size_t maxlen2);

/**
 * Destructor.
 */
void ferSurfMatchDel(fer_surf_match_t *sm);

/**
 * Sets i'th vector in first vector array.
 */
void ferSurfMatchSet1(fer_surf_match_t *sm, size_t i, const fer_vec_t *v);

/**
 * Sets i'th vector in second vector array.
 */
void ferSurfMatchSet2(fer_surf_match_t *sm, size_t i, const fer_vec_t *v);

/**
 * Returns i'th vector from first array.
 */
const fer_vec_t *ferSurfMatchGet1(fer_surf_match_t *sm, size_t i);

/**
 * Returns i'th vector from second array.
 */
const fer_vec_t *ferSurfMatchGet2(fer_surf_match_t *sm, size_t i);

/**
 * Matches first len1 vectors from frist vector array agains first len2
 * vectors from second vector array. For each vector from first array it
 * finds two nearest vectors from second array.
 * Parameter max_threads is maximal number of threads per block, reasonable
 * value could be, say, 256.
 * Use function ferSurfMatchNearest() to get results.
 */
void ferSurfMatch(fer_surf_match_t *sm, size_t len1, size_t len2);

/**
 * Same as ferSurfMatch() but uses linear search instead of OpenCL.
 */
void ferSurfMatchLinear(fer_surf_match_t *sm, size_t len1, size_t len2);


/**
 * Returns two nearest vectors to i'th vector from first vector array.
 * Via nearest1 and nearest2 are returned indices of vectors from second
 * vector array and via dist1 and dist2 are returned squared distances.
 */
void ferSurfMatchNearest(const fer_surf_match_t *sm, size_t i,
                         int *nearest1, int *nearest2,
                         fer_real_t *dist1, fer_real_t *dist2);
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_SURF_MATCHING_H__ */


