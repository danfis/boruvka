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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_COLLIDE_H__ */

