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
/* #include <fermat/predicates.h> */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * CHull - Convex Hull
 * ====================
 */
struct _fer_chull3_t {
    fer_mesh3_t *mesh; /*!< Mesh representing convex hull */
    int coplanar;

    /* fer_pred_t pred; */
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
 * Returns Mesh3 with convex hull.
 */
_fer_inline fer_mesh3_t *ferCHull3Mesh(fer_chull3_t *h);

/**
 * Adds point to convex hull.
 */
void ferCHull3Add(fer_chull3_t *h, const fer_vec3_t *point);

/**
 * Returns number of points on hull.
 */
_fer_inline size_t ferCHull3NumPoints(const fer_chull3_t *h);

/**
 * Dump mesh in SVT format
 */
void ferCHull3DumpSVT(fer_chull3_t *h, FILE *out, const char *name);

/**** INLINES ****/
_fer_inline fer_mesh3_t *ferCHull3Mesh(fer_chull3_t *h)
{
    return h->mesh;
}

_fer_inline size_t ferCHull3NumPoints(const fer_chull3_t *h)
{
    return ferMesh3VerticesLen(h->mesh);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_CHULL_H__ */



