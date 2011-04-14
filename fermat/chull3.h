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

#ifndef __FER_CHULL3_H__
#define __FER_CHULL3_H__

#include <fermat/mesh3.h>
#include <fermat/list.h>
#include <fermat/rand.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef FER_SINGLE
# define FER_CHULL3_EPS 1E-6
#else /* FER_SINGLE */
# define FER_CHULL3_EPS 1E-9
#endif /* FER_SINGLE */

/**
 * CHull - Convex Hull
 * ====================
 */
struct _fer_chull3_t {
    fer_mesh3_t *mesh; /*!< Mesh representing convex hull */
    fer_real_t eps;    /*!< Epsilon, by default FER_CHULL3_EPS.
                            Note, that this should be higher than FER_EPS
                            because FER_EPS is too low to correct errors in
                            computation with floating point numbers */

    fer_rand_t rand;
};
typedef struct _fer_chull3_t fer_chull3_t;

/**
 * Creates new convex hull
 */
fer_chull3_t *ferCHull3New(void);

/**
 * Deletes convex hull
 */
void ferCHull3Del(fer_chull3_t *h);

/**
 * Sets CHull's epsilon parameter.
 */
_fer_inline void ferCHull3SetEPS(fer_chull3_t *h, fer_real_t eps);

/**
 * Returns Mesh3 with convex hull.
 */
_fer_inline fer_mesh3_t *ferCHull3Mesh(fer_chull3_t *h);

/**
 * Adds point to convex hull.
 */
void ferCHull3Add(fer_chull3_t *h, const fer_vec3_t *point);

/**
 * Dump mesh in SVT format
 */
void ferCHull3DumpSVT(fer_chull3_t *h, FILE *out, const char *name);


/**** INLINES ****/
_fer_inline void ferCHull3SetEPS(fer_chull3_t *h, fer_real_t eps)
{
    h->eps = eps;
}

_fer_inline fer_mesh3_t *ferCHull3Mesh(fer_chull3_t *h)
{
    return h->mesh;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_CHULL_H__ */



