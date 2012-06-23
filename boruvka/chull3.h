/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_CHULL3_H__
#define __BOR_CHULL3_H__

#include <boruvka/mesh3.h>
#include <boruvka/list.h>
/* #include <boruvka/predicates.h> */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * CHull - Convex Hull
 * ====================
 */
struct _bor_chull3_t {
    bor_mesh3_t *mesh; /*!< Mesh representing convex hull */
    int coplanar;

    /* bor_pred_t pred; */
};
typedef struct _bor_chull3_t bor_chull3_t;

/**
 * Creates new convex hull
 */
bor_chull3_t *ferCHull3New(void);

/**
 * Deletes convex hull
 */
void ferCHull3Del(bor_chull3_t *h);

/**
 * Returns Mesh3 with convex hull.
 */
_fer_inline bor_mesh3_t *ferCHull3Mesh(bor_chull3_t *h);

/**
 * Adds point to convex hull.
 */
void ferCHull3Add(bor_chull3_t *h, const bor_vec3_t *point);

/**
 * Returns number of points on hull.
 */
_fer_inline size_t ferCHull3NumPoints(const bor_chull3_t *h);

/**
 * Dump mesh in SVT format
 */
void ferCHull3DumpSVT(bor_chull3_t *h, FILE *out, const char *name);

/**** INLINES ****/
_fer_inline bor_mesh3_t *ferCHull3Mesh(bor_chull3_t *h)
{
    return h->mesh;
}

_fer_inline size_t ferCHull3NumPoints(const bor_chull3_t *h)
{
    return ferMesh3VerticesLen(h->mesh);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_CHULL_H__ */



