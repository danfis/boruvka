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

#ifndef __FER_OBB_H__
#define __FER_OBB_H__

#include <stdio.h>
#include <fermat/core.h>
#include <fermat/vec3.h>
#include <fermat/mat3.h>
#include <fermat/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Primitive types */
#define FER_OBB_PRI_TRI 1
#define FER_OBB_PRI_TRIMESH 2

/**
 * Geometry primitive base class
 */
struct _fer_obb_pri_t {
    char type;
};
typedef struct _fer_obb_pri_t fer_obb_pri_t;

/**
 * Triangle primitive
 */
struct _fer_obb_tri_t {
    fer_obb_pri_t pri;        /*!< Base class */
    const fer_vec3_t *p0, *p1, *p2; /*!< Triangle end points */
};
typedef struct _fer_obb_tri_t fer_obb_tri_t;

struct _fer_obb_trimesh_t {
    fer_obb_pri_t pri;        /*!< Base class */
    fer_vec3_t *pts;
    unsigned int *ids;
    size_t len;
};
typedef struct _fer_obb_trimesh_t fer_obb_trimesh_t;


/**
 * Creates new triangle
 */
fer_obb_tri_t *ferOBBTriNew(const fer_vec3_t *p1, const fer_vec3_t *p2,
                            const fer_vec3_t *p3);

/**
 * Deletes triangle
 */
void ferOBBTriDel(fer_obb_tri_t *tri);


/**
 * Creates new trimesh
 */
fer_obb_trimesh_t *ferOBBTriMeshNew(const fer_vec3_t *pts,
                                    const unsigned int *ids, size_t len);

/**
 * Deletes trimesh
 */
void ferOBBTriMeshDel(fer_obb_trimesh_t *t);


/**
 * OBB - Oriented Bounding Box
 * ============================
 */
struct _fer_obb_t {
    /* R = { x | x = center + r[0] * axis[0] + r[1] * axis[1] + r[2] * axis[2] },
     *   where |r[i]| <= half_extents[i] */
    fer_vec3_t center;       /*!< Center of bounding box */
    fer_vec3_t axis[3];      /*!< Axis - local coordinate system */
    fer_vec3_t half_extents; /*!< Half extents along box's axis */

    fer_obb_pri_t *pri; /*!< Reference to primitive that is holded by bounding box */
    fer_list_t obbs;    /*!< List of children obbs */
    fer_list_t list;    /*!< Reference to parent's list of obbs */
} fer_aligned(16) fer_packed;
typedef struct _fer_obb_t fer_obb_t;


/**
 * Creates new oriented bounding box.
 */
fer_obb_t *ferOBBNew(const fer_vec3_t *c, const fer_vec3_t *a1,
                     const fer_vec3_t *a2, const fer_vec3_t *a3,
                     const fer_vec3_t *half_extents,
                     fer_obb_pri_t *pri);

/**
 * Creates new bounding box for given primitive.
 */
//fer_obb_t *ferOBBNewPri(fer_obb_pri_t *pri);

/**
 * Creates new bounding box from triangle.
 */
fer_obb_t *ferOBBNewTri(const fer_vec3_t *p1, const fer_vec3_t *p2,
                        const fer_vec3_t *p3);

fer_obb_t *ferOBBNewTriMesh(const fer_vec3_t *pts,
                            const unsigned *ids, size_t len);

/**
 * Frees allocated memory.
 */
void ferOBBDel(fer_obb_t *obb);

/**
 * Returns true if given obbs are disjoint, relative rotation and
 * translation are also given.
 */
int ferOBBDisjoint(const fer_obb_t *obb1,
                   const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                   const fer_obb_t *obb2,
                   const fer_mat3_t *rot2, const fer_vec3_t *tr2);


void ferOBBDumpSVT(const fer_obb_t *obb, FILE *out, const char *name);
void ferOBBTriDumpSVT(const fer_obb_tri_t *tri, FILE *out, const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_OBB_H__ */

