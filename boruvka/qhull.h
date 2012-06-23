/***
 * Boruvka
 * --------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __FER_QHULL_H__
#define __FER_QHULL_H__

#include <boruvka/core.h>
#include <boruvka/pc.h>
#include <boruvka/mesh3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Default path to qdelaunay binary */
#define FER_QDELAUNAY_BIN_PATH "/usr/bin/qdelaunay"

/**
 * Qhull - Wrappers around qhull software tool
 * ============================================
 *
 */

/**
 * Mesh3 implementation based on bor_mesh3_t suitable for qhull algorithms.
 *
 * Use ferQHullMesh3() function to get pointer to bor_mesh3_t you can work
 * with.
 * Call ferQHullMesh3Del() on all meshes that are returned from QHull
 * functions.
 */
struct _bor_qhull_mesh3_t {
    bor_mesh3_t *mesh; /*!< Pointer to Mesh3 */

    bor_vec3_t *vecs; /*!< Array of Vec3 vectors */
    size_t vecs_len;  /*!< Length of .vecs array */
};
typedef struct _bor_qhull_mesh3_t bor_qhull_mesh3_t;

/**
 * Deletes mesh3 returned from some qhull functions.
 */
void ferQHullMesh3Del(bor_qhull_mesh3_t *m);

/**
 * Returns pointer to bor_mesh3_t struct.
 */
_fer_inline bor_mesh3_t *ferQHullMesh3(bor_qhull_mesh3_t *m);



/**
 * QDelaunay - Delaunay triangulation
 * -----------------------------------
 */
struct _bor_qdelaunay_t {
    char *bin_path; /*!< Path to qdelaunay binary */
};
typedef struct _bor_qdelaunay_t bor_qdelaunay_t;

/**
 * Creates new qdelaunay instance.
 */
bor_qdelaunay_t *ferQDelaunayNew(void);

/**
 * Deallocates and frees qdelaunay struct.
 */
void ferQDelaunayDel(bor_qdelaunay_t *q);

/**
 * Returns path to qdelaunay binary.
 */
_fer_inline const char *ferQDelaunayPath(const bor_qdelaunay_t *q);

/**
 * Sets path to qdelaunay binary.
 */
void ferQDelaunaySetPath(bor_qdelaunay_t *q, const char *path);

/**
 * Performs 3D delaunay triangulation on given point cloud.
 * New Mesh3 instance is returned.
 */
bor_qhull_mesh3_t *ferQDelaunayMesh3(bor_qdelaunay_t *q, const bor_pc_t *pc);

/**** INLINES ****/
_fer_inline bor_mesh3_t *ferQHullMesh3(bor_qhull_mesh3_t *m)
{
    return m->mesh;
}

_fer_inline const char *ferQDelaunayPath(const bor_qdelaunay_t *q)
{
    return q->bin_path;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_QHULL_H__ */

