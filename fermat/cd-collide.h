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

#ifndef __FER_CD_COLLIDE_H__
#define __FER_CD_COLLIDE_H__

#include <fermat/cd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_cd_t;

/**
 * Function that should return true if {s1} and {s2} do collide.
 */
typedef int (*fer_cd_collide_fn)(struct _fer_cd_t *cd,
                                 const fer_cd_shape_t *s1,
                                 const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                                 const fer_cd_shape_t *s2,
                                 const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Sphere-Sphere collider
 */
int ferCDCollideSphereSphere(struct _fer_cd_t *cd,
                             const fer_cd_sphere_t *s1,
                             const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                             const fer_cd_sphere_t *s2,
                             const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Sphere-Box collider
 */
int ferCDCollideSphereBox(struct _fer_cd_t *cd,
                          const fer_cd_sphere_t *s1,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_box_t *s2,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Sphere-Capsule collider
 */
int ferCDCollideSphereCap(struct _fer_cd_t *cd,
                          const fer_cd_sphere_t *s1,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_cap_t *s2,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Sphere-Tri collider.
 */
int ferCDCollideSphereTri(struct _fer_cd_t *cd,
                          const fer_cd_sphere_t *s,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_tri_t *t,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Box-Box collider
 */
int ferCDCollideBoxBox(struct _fer_cd_t *cd,
                       const fer_cd_box_t *s1,
                       const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                       const fer_cd_box_t *s2,
                       const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Plane-Sphere collider
 */
int ferCDCollidePlaneSphere(struct _fer_cd_t *cd,
                            const fer_cd_plane_t *p,
                            const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                            const fer_cd_sphere_t *s,
                            const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Plane-Box collider
 */
int ferCDCollidePlaneBox(struct _fer_cd_t *cd,
                         const fer_cd_plane_t *p,
                         const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                         const fer_cd_box_t *b,
                         const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Plane-Tri collider
 */
int ferCDCollidePlaneTri(struct _fer_cd_t *cd,
                         const fer_cd_plane_t *p,
                         const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                         const fer_cd_tri_t *t,
                         const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * TriMeshTri-TriMeshTri collider.
 */
int ferCDCollideTriTri(struct _fer_cd_t *cd,
                       const fer_cd_tri_t *t1,
                       const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                       const fer_cd_tri_t *t2,
                       const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * ShapeOffset-ShapeOffset collider
 */
int ferCDCollideOffOff(struct _fer_cd_t *cd,
                       const fer_cd_shape_off_t *s1,
                       const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                       const fer_cd_shape_off_t *s2,
                       const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * ShapeOffset-AnyShape collider
 */
int ferCDCollideOffAny(struct _fer_cd_t *cd,
                       const fer_cd_shape_off_t *s1,
                       const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                       const fer_cd_shape_t *s2,
                       const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * CCD collider - can be used to any pair of shapes
 */
int ferCDCollideCCD(struct _fer_cd_t *cd,
                    const fer_cd_shape_t *s1,
                    const fer_mat3_t *_rot1, const fer_vec3_t *_tr1,
                    const fer_cd_shape_t *s2,
                    const fer_mat3_t *rot2, const fer_vec3_t *tr2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_COLLIDE_H__ */

