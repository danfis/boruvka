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

#ifndef __FER_CD_PLANE_H__
#define __FER_CD_PLANE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Planeinder shape
 */
struct _fer_cd_plane_t {
    fer_cd_shape_t shape;
};
typedef struct _fer_cd_plane_t fer_cd_plane_t;

/**
 * Creates plane shape.
 * Plane is has equation: z = 0
 */
fer_cd_plane_t *ferCDPlaneNew(void);

/**
 * Deletes plane.
 */
void ferCDPlaneDel(fer_cd_plane_t *c);

void ferCDPlaneSupport(const fer_cd_plane_t *c, const fer_vec3_t *dir,
                       fer_vec3_t *p);

void ferCDPlaneCenter(const fer_cd_plane_t *s,
                      const fer_mat3_t *rot, const fer_vec3_t *tr,
                      fer_vec3_t *center);

void ferCDPlaneFitOBB(const fer_cd_plane_t *c,
                      fer_vec3_t *center,
                      fer_vec3_t *axis0,
                      fer_vec3_t *axis1,
                      fer_vec3_t *axis2,
                      fer_vec3_t *half_extents, int flags);

int ferCDPlaneUpdateCHull(const fer_cd_plane_t *c, fer_chull3_t *chull,
                          const fer_mat3_t *rot, const fer_vec3_t *tr);

void ferCDPlaneUpdateCov(const fer_cd_plane_t *s,
                         const fer_mat3_t *rot, const fer_vec3_t *tr,
                         fer_vec3_t *wcenter, fer_mat3_t *cov,
                         fer_real_t *area, int *num);

void ferCDPlaneUpdateMinMax(const fer_cd_plane_t *c, const fer_vec3_t *axis,
                            const fer_mat3_t *rot, const fer_vec3_t *tr,
                            fer_real_t *min, fer_real_t *max);

void ferCDPlaneDumpSVT(const fer_cd_plane_t *s,
                       FILE *out, const char *name,
                       const fer_mat3_t *rot, const fer_vec3_t *tr);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_PLANE_H__ */
