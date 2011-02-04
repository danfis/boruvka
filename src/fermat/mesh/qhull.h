/***
 * fermat
 * -------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __FER_QHULL_H__
#define __FER_QHULL_H__

#include <fermat/core.h>
#include <fermat/point_cloud.h>
#include <fermat/mesh/mesh3.h>

/** Default path to qdelaunay binary */
#define FER_QDELAUNAY_BIN_PATH "/usr/bin/qdelaunay"

/**
 * Qhull - Wrappers around qhull software tool
 * ============================================
 *
 */

/**
 * Mesh3 implementation based on fer_mesh3_t suitable for qhull algorithms.
 *
 * Use ferQHullMesh3() function to get pointer to fer_mesh3_t you can work
 * with.
 * Call ferQHullMesh3Del() on all meshes that are returned from QHull
 * functions.
 */
struct _fer_qhull_mesh3_t {
    fer_mesh3_t *mesh; /*!< Pointer to Mesh3 */

    fer_vec3_t *vecs; /*!< Array of Vec3 vectors */
    size_t vecs_len;  /*!< Length of .vecs array */
};
typedef struct _fer_qhull_mesh3_t fer_qhull_mesh3_t;

/**
 * Deletes mesh3 returned from some qhull functions.
 */
void ferQHullMesh3Del(fer_qhull_mesh3_t *m);

/**
 * Returns pointer to fer_mesh3_t struct.
 */
_fer_inline fer_mesh3_t *ferQHullMesh3(fer_qhull_mesh3_t *m);



/**
 * QDelaunay - Delaunay triangulation
 * -----------------------------------
 */
struct _fer_qdelaunay_t {
    char *bin_path; /*!< Path to qdelaunay binary */
};
typedef struct _fer_qdelaunay_t fer_qdelaunay_t;

/**
 * Creates new qdelaunay instance.
 */
fer_qdelaunay_t *ferQDelaunayNew(void);

/**
 * Deallocates and frees qdelaunay struct.
 */
void ferQDelaunayDel(fer_qdelaunay_t *q);

/**
 * Returns path to qdelaunay binary.
 */
_fer_inline const char *ferQDelaunayPath(const fer_qdelaunay_t *q);

/**
 * Sets path to qdelaunay binary.
 */
void ferQDelaunaySetPath(fer_qdelaunay_t *q, const char *path);

/**
 * Performs 3D delaunay triangulation on given point cloud.
 * New Mesh3 instance is returned.
 */
fer_qhull_mesh3_t *ferQDelaunayMesh3(fer_qdelaunay_t *q, const fer_pc_t *pc);

/**** INLINES ****/
_fer_inline fer_mesh3_t *ferQHullMesh3(fer_qhull_mesh3_t *m)
{
    return m->mesh;
}

_fer_inline const char *ferQDelaunayPath(const fer_qdelaunay_t *q)
{
    return q->bin_path;
}

#endif /* __FER_QHULL_H__ */

