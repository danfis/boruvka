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

#ifndef __FER_NEAREST_VEC_POOL_H__
#define __FER_NEAREST_VEC_POOL_H__

#include <fermat/vec.h>
#ifdef FER_OPENCL
# include <fermat/opencl.h>
#endif /* FER_OPENCL */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Nearest Neighbors In Vector Pool
 * =================================
 */
struct _fer_nn_vec_pool_t {
    int len;
    int dim;
    fer_vec_t *vecs;
    fer_real_t *dists;

#ifdef FER_OPENCL
    fer_cl_t *cl;         /*!< OpenCL program */
    fer_vec_t *vecs_cl;   /*!< Array of vectors for OpenCL device */
    fer_real_t *dists_cl; /*!< Array for distances for OpenCL device */
#endif
};
typedef struct _fer_nn_vec_pool_t fer_nn_vec_pool_t;

/**
 * Creates new vector pool. Parameters are maximal length and dimension of
 * vector.
 */
fer_nn_vec_pool_t *ferNNVecPoolNew(size_t len, size_t dim);

/**
 * Deletes vector pool.
 */
void ferNNVecPoolDel(fer_nn_vec_pool_t *vp);

/**
 * Sets i'th vector in pool.
 * Returns 0 on success.
 */
int ferNNVecPoolSetVec(fer_nn_vec_pool_t *vp, size_t i, fer_vec_t *v);

/**
 * Finds {k} nearest neighbors to vector {p} from first {len} vectors from
 * vector pool.
 * Linear searching is used.
 *
 * Returns 0 on success.
 */
int ferNNVecPoolNearest(fer_nn_vec_pool_t *vp, size_t len, size_t k,
                        const fer_vec_t *p, fer_vec_t **vecs);

/**
 * Same as ferNNVecPoolNearest() function but OpenCL device is used for
 * computation.
 */
int ferNNVecPoolNearestCL(fer_nn_vec_pool_t *vp, size_t len, size_t k,
                          const fer_vec_t *p, fer_vec_t **vecs);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_NEAREST_VEC_POOL_H__ */


