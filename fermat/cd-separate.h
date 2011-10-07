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

    size_t size;
};
typedef struct _fer_cd_contacts_t fer_cd_contacts_t;

/**
 * Returns new contacts struct
 */
fer_cd_contacts_t *ferCDContactsNew(size_t size);

/**
 * Deletes fer_cd_contacts_t struct.
 */
void ferCDContactsDel(fer_cd_contacts_t *contacts);

/**
 * Returns number of contact points found.
 */
typedef int (*fer_cd_separate_fn)(struct _fer_cd_t *cd,
                                  const fer_cd_shape_t *s1,
                                  const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                                  const fer_cd_shape_t *s2,
                                  const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                                  fer_cd_contacts_t *con);


/**
 * Sphere-Sphere separator
 */
int ferCDSeparateSphereSphere(struct _fer_cd_t *cd,
                              const fer_cd_sphere_t *s1,
                              const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                              const fer_cd_sphere_t *s2,
                              const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                              fer_cd_contacts_t *con);

/**
 * Sphere-Cap separator
 */
int ferCDSeparateSphereCap(struct _fer_cd_t *cd,
                           const fer_cd_sphere_t *s1,
                           const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                           const fer_cd_cap_t *c2,
                           const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                           fer_cd_contacts_t *con);

/**
 * Sphere-Tri separator
 */
int ferCDSeparateSphereTri(struct _fer_cd_t *cd,
                           const fer_cd_sphere_t *s1,
                           const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                           const fer_cd_tri_t *t2,
                           const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                           fer_cd_contacts_t *con);

/**
 * Cap-Cap separator
 */
int ferCDSeparateCapCap(struct _fer_cd_t *cd,
                        const fer_cd_cap_t *c1,
                        const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                        const fer_cd_cap_t *c2,
                        const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                        fer_cd_contacts_t *con);

/**
 * Plane-Sphere separator
 */
int ferCDSeparatePlaneSphere(struct _fer_cd_t *cd,
                             const fer_cd_plane_t *s1,
                             const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                             const fer_cd_sphere_t *s2,
                             const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                             fer_cd_contacts_t *con);

/**
 * Plane-Box separator
 */
int ferCDSeparatePlaneBox(struct _fer_cd_t *cd,
                          const fer_cd_plane_t *s1,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_box_t *s2,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                          fer_cd_contacts_t *con);

/**
 * Plane-Cap separator
 */
int ferCDSeparatePlaneCap(struct _fer_cd_t *cd,
                          const fer_cd_plane_t *s1,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_cap_t *s2,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                          fer_cd_contacts_t *con);

/**
 * Plane-any convex shape separator
 */
int ferCDSeparatePlaneConvex(struct _fer_cd_t *cd,
                             const fer_cd_plane_t *s1,
                             const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                             const fer_cd_shape_t *s2,
                             const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                             fer_cd_contacts_t *con);

/**
 * Plane-Tri separator
 */
int ferCDSeparatePlaneTri(struct _fer_cd_t *cd,
                          const fer_cd_plane_t *p,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_tri_t *t,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                          fer_cd_contacts_t *con);


/**
 * ShapeOffset-ShapeOffset separator
 */
int ferCDSeparateOffOff(struct _fer_cd_t *cd,
                        const fer_cd_shape_off_t *s1,
                        const fer_mat3_t *_rot1, const fer_vec3_t *_tr1,
                        const fer_cd_shape_off_t *s2,
                        const fer_mat3_t *_rot2, const fer_vec3_t *_tr2,
                        fer_cd_contacts_t *con);

/**
 * ShapeOffset-AnyShape separator.
 */
int ferCDSeparateOffAny(struct _fer_cd_t *cd,
                        const fer_cd_shape_off_t *s1,
                        const fer_mat3_t *_rot1, const fer_vec3_t *_tr1,
                        const fer_cd_shape_t *s2,
                        const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                        fer_cd_contacts_t *con);

/**
 * Tri-Tri separator
 */
int ferCDSeparateTriTri(struct _fer_cd_t *cd,
                        const fer_cd_tri_t *t1,
                        const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                        const fer_cd_tri_t *t2,
                        const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                        fer_cd_contacts_t *con);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_SEPARATE_H__ */


