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

#ifndef __BOR_QHULL_H__
#define __BOR_QHULL_H__

#include <boruvka/core.h>
#include <boruvka/pc.h>
#include <boruvka/mesh3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Default path to qdelaunay binary */
#define BOR_QDELAUNAY_BIN_PATH "/usr/bin/qdelaunay"

/**
 * Qhull - Wrappers around qhull software tool
 * ============================================
 *
 */

/**
 * Mesh3 implementation based on bor_mesh3_t suitable for qhull algorithms.
 *
 * Use borQHullMesh3() function to get pointer to bor_mesh3_t you can work
 * with.
 * Call borQHullMesh3Del() on all meshes that are returned from QHull
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
void borQHullMesh3Del(bor_qhull_mesh3_t *m);

/**
 * Returns pointer to bor_mesh3_t struct.
 */
_bor_inline bor_mesh3_t *borQHullMesh3(bor_qhull_mesh3_t *m);



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
bor_qdelaunay_t *borQDelaunayNew(void);

/**
 * Deallocates and frees qdelaunay struct.
 */
void borQDelaunayDel(bor_qdelaunay_t *q);

/**
 * Returns path to qdelaunay binary.
 */
_bor_inline const char *borQDelaunayPath(const bor_qdelaunay_t *q);

/**
 * Sets path to qdelaunay binary.
 */
void borQDelaunaySetPath(bor_qdelaunay_t *q, const char *path);

/**
 * Performs 3D delaunay triangulation on given point cloud.
 * New Mesh3 instance is returned.
 */
bor_qhull_mesh3_t *borQDelaunayMesh3(bor_qdelaunay_t *q, const bor_pc_t *pc);

/**** INLINES ****/
_bor_inline bor_mesh3_t *borQHullMesh3(bor_qhull_mesh3_t *m)
{
    return m->mesh;
}

_bor_inline const char *borQDelaunayPath(const bor_qdelaunay_t *q)
{
    return q->bin_path;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_QHULL_H__ */

