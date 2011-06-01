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

#ifndef __FER_CD_SEPARATE_H__
#define __FER_CD_SEPARATE_H__

#include <fermat/cd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_cd_t;

struct _fer_cd_contacts_t {
    size_t num;        /*!< Number of contacts (length of .num, .dir and
                            .depth arrays */
    fer_vec3_t *pos;   /*!< Positions where force should take place */
    fer_vec3_t *dir;   /*!< Direction of separation vector (unit vector) */
    fer_real_t *depth; /*!< Depth of penetration (length of separation vector) */
};
typedef struct _fer_cd_contacts_t fer_cd_contacts_t;

/**
 * Returns new contacts struct
 */
fer_cd_contacts_t *ferCDContactsNew(size_t num);

/**
 * Deletes fer_cd_contacts_t struct.
 */
void ferCDContactsDel(fer_cd_contacts_t *contacts);

/**
 * TODO
 */
typedef fer_cd_contacts_t *(*fer_cd_separate_fn)(struct _fer_cd_t *cd,
                                                 const fer_cd_shape_t *s1,
                                                 const fer_mat3_t *rot1,
                                                 const fer_vec3_t *tr1,
                                                 const fer_cd_shape_t *s2,
                                                 const fer_mat3_t *rot2,
                                                 const fer_vec3_t *tr2);

/**
 * Sphere-Sphere separater
 */
fer_cd_contacts_t *ferCDSeparateSphereSphere(struct _fer_cd_t *cd,
                                             const fer_cd_sphere_t *s1,
                                             const fer_mat3_t *rot1,
                                             const fer_vec3_t *tr1,
                                             const fer_cd_sphere_t *s2,
                                             const fer_mat3_t *rot2,
                                             const fer_vec3_t *tr2);

/**
 * ShapeOffset-ShapeOffset separator
 */
fer_cd_contacts_t *ferCDSeparateOffOff(struct _fer_cd_t *cd,
                                       const fer_cd_shape_off_t *s1,
                                       const fer_mat3_t *rot1,
                                       const fer_vec3_t *tr1,
                                       const fer_cd_shape_off_t *s2,
                                       const fer_mat3_t *rot2,
                                       const fer_vec3_t *tr2);
/**
 * ShapeOffset-AnyShape separator.
 */
fer_cd_contacts_t *ferCDSeparateOffAny(struct _fer_cd_t *cd,
                                       const fer_cd_shape_off_t *s1,
                                       const fer_mat3_t *rot1,
                                       const fer_vec3_t *tr1,
                                       const fer_cd_shape_t *s2,
                                       const fer_mat3_t *rot2,
                                       const fer_vec3_t *tr2);
/**
 * Sphere-Box separater
 */
int ferCDSeparateSphereBox(struct _fer_cd_t *cd,
                          const fer_cd_sphere_t *s1,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_box_t *s2,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Sphere-Capsule separater
 */
int ferCDSeparateSphereCap(struct _fer_cd_t *cd,
                          const fer_cd_sphere_t *s1,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_cap_t *s2,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Sphere-Tri separater.
 */
int ferCDSeparateSphereTri(struct _fer_cd_t *cd,
                          const fer_cd_sphere_t *s,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_tri_t *t,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Box-Box separater
 */
int ferCDSeparateBoxBox(struct _fer_cd_t *cd,
                       const fer_cd_box_t *s1,
                       const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                       const fer_cd_box_t *s2,
                       const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Plane-Sphere separater
 */
int ferCDSeparatePlaneSphere(struct _fer_cd_t *cd,
                            const fer_cd_plane_t *p,
                            const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                            const fer_cd_sphere_t *s,
                            const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Plane-Box separater
 */
int ferCDSeparatePlaneBox(struct _fer_cd_t *cd,
                         const fer_cd_plane_t *p,
                         const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                         const fer_cd_box_t *b,
                         const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Plane-Capsule separater
 */
int ferCDSeparatePlaneCap(struct _fer_cd_t *cd,
                         const fer_cd_plane_t *p,
                         const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                         const fer_cd_cap_t *c,
                         const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Plane-Cylinder separater
 */
int ferCDSeparatePlaneCyl(struct _fer_cd_t *cd,
                         const fer_cd_plane_t *p,
                         const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                         const fer_cd_cyl_t *c,
                         const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Plane-Tri separater
 */
int ferCDSeparatePlaneTri(struct _fer_cd_t *cd,
                         const fer_cd_plane_t *p,
                         const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                         const fer_cd_tri_t *t,
                         const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * TriMeshTri-TriMeshTri separater.
 */
int ferCDSeparateTriTri(struct _fer_cd_t *cd,
                       const fer_cd_tri_t *t1,
                       const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                       const fer_cd_tri_t *t2,
                       const fer_mat3_t *rot2, const fer_vec3_t *tr2);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_SEPARATE_H__ */


