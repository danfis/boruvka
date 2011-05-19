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

#ifndef __FER_CD_TRIMESH_H__
#define __FER_CD_TRIMESH_H__

#include <fermat/cd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Triangle shape.
 */
struct _fer_cd_tri_t {
    fer_cd_shape_t shape;     /*!< Base class */
    fer_vec3_t *p0, *p1, *p2; /*!< Triangle end points */
};
typedef struct _fer_cd_tri_t fer_cd_tri_t;

/**
 * Triangle shape as reference to trimesh.
 */
typedef fer_cd_tri_t fer_cd_trimesh_tri_t;


/**
 * Trimesh shape
 */
struct _fer_cd_trimesh_t {
    fer_cd_shape_t shape; /*!< Base class */
    fer_vec3_t *pts;
    size_t ptslen;
    unsigned int *ids;
    size_t len;
};
typedef struct _fer_cd_trimesh_t fer_cd_trimesh_t;


/**
 * Creates new triangle
 */
fer_cd_trimesh_tri_t *ferCDTriNew(const fer_vec3_t *p1,
                                  const fer_vec3_t *p2,
                                  const fer_vec3_t *p3);

/**
 * Deletes trimesh triangle
 */
void ferCDTriDel(fer_cd_trimesh_tri_t *tri);

/**
 * Creates new triangle as reference to trimesh.
 */
fer_cd_trimesh_tri_t *ferCDTriMeshTriNew(const fer_vec3_t *p1,
                                         const fer_vec3_t *p2,
                                         const fer_vec3_t *p3);

/**
 * Deletes trimesh triangle
 */
void ferCDTriMeshTriDel(fer_cd_trimesh_tri_t *tri);



/**
 * Creates new trimesh
 */
fer_cd_trimesh_t *ferCDTriMeshNew(const fer_vec3_t *pts,
                                  const unsigned int *ids, size_t len,
                                  const fer_mat3_t *rot, const fer_vec3_t *tr);

/**
 * Deletes trimesh
 */
void ferCDTriMeshDel(fer_cd_trimesh_t *t);

void ferCDTriSupport(const fer_cd_trimesh_tri_t *t,
                     const fer_vec3_t *dir, fer_vec3_t *p);
void ferCDTriMeshSupport(const fer_cd_trimesh_t *t,
                         const fer_vec3_t *dir,
                         fer_vec3_t *p);

void ferCDTriCenter(const fer_cd_trimesh_tri_t *t,
                    const fer_mat3_t *rot, const fer_vec3_t *tr,
                    fer_vec3_t *center);
void ferCDTriMeshCenter(const fer_cd_trimesh_t *t,
                        const fer_mat3_t *rot,
                        const fer_vec3_t *tr,
                        fer_vec3_t *center);

void ferCDTriFitOBB(const fer_cd_trimesh_tri_t *tri,
                    fer_vec3_t *center,
                    fer_vec3_t *axis0,
                    fer_vec3_t *axis1,
                    fer_vec3_t *axis2,
                    fer_vec3_t *half_extents, int flags);
void ferCDTriMeshFitOBB(const fer_cd_trimesh_t *s,
                        fer_vec3_t *center,
                        fer_vec3_t *axis0,
                        fer_vec3_t *axis1,
                        fer_vec3_t *axis2,
                        fer_vec3_t *half_extents, int flags);

int ferCDTriUpdateCHull(const fer_cd_trimesh_tri_t *t, fer_chull3_t *chull,
                               const fer_mat3_t *rot, const fer_vec3_t *tr);
int ferCDTriMeshUpdateCHull(const fer_cd_trimesh_t *t, fer_chull3_t *chull,
                            const fer_mat3_t *rot, const fer_vec3_t *tr);

void ferCDTriUpdateMinMax(const fer_cd_trimesh_tri_t *t,
                          const fer_vec3_t *axis,
                          const fer_mat3_t *rot, const fer_vec3_t *tr,
                          fer_real_t *min, fer_real_t *max);
void ferCDTriMeshUpdateMinMax(const fer_cd_trimesh_t *t,
                              const fer_vec3_t *axis,
                              const fer_mat3_t *rot, const fer_vec3_t *tr,
                              fer_real_t *min, fer_real_t *max);

void ferCDTriUpdateCov(const fer_cd_tri_t *t,
                       const fer_mat3_t *rot, const fer_vec3_t *tr,
                       fer_vec3_t *wcenter, fer_mat3_t *cov,
                       fer_real_t *area, int *num);
void ferCDTriMeshUpdateCov(const fer_cd_trimesh_t *t,
                           const fer_mat3_t *rot, const fer_vec3_t *tr,
                           fer_vec3_t *wcenter, fer_mat3_t *cov,
                           fer_real_t *area, int *num);

void ferCDTriDumpSVT(const fer_cd_trimesh_tri_t *tri,
                     FILE *out, const char *name,
                     const fer_mat3_t *rot, const fer_vec3_t *tr);
void ferCDTriMeshDumpSVT(const fer_cd_trimesh_t *t,
                         FILE *out, const char *name,
                         const fer_mat3_t *rot, const fer_vec3_t *tr);

/** Returns (via {q}) closest point to point p */
void __ferCDTriClosestPoint(const fer_cd_tri_t *t, const fer_vec3_t *p,
                            fer_vec3_t *q);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_TRIMESH_H__ */


