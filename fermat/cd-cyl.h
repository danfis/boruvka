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

#ifndef __FER_CD_CYL_H__
#define __FER_CD_CYL_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Cylinder shape
 */
struct _fer_cd_cyl_t {
    fer_cd_shape_t shape;
    fer_real_t radius, half_height;
};
typedef struct _fer_cd_cyl_t fer_cd_cyl_t;

/**
 * Creates sphere shape
 */
fer_cd_cyl_t *ferCDCylNew(fer_real_t radius, fer_real_t height);

/**
 * Deletes sphere.
 */
void ferCDCylDel(fer_cd_cyl_t *c);

void ferCDCylSupport(const fer_cd_cyl_t *c, const fer_vec3_t *dir,
                     fer_vec3_t *p);

void ferCDCylFitOBB(const fer_cd_cyl_t *c,
                    fer_vec3_t *center,
                    fer_vec3_t *axis0,
                    fer_vec3_t *axis1,
                    fer_vec3_t *axis2,
                    fer_vec3_t *half_extents, int flags);

int ferCDCylUpdateCHull(const fer_cd_cyl_t *c, fer_chull3_t *chull,
                        const fer_mat3_t *rot, const fer_vec3_t *tr);

void ferCDCylUpdateMinMax(const fer_cd_cyl_t *c, const fer_vec3_t *axis,
                          const fer_mat3_t *rot, const fer_vec3_t *tr,
                          fer_real_t *min, fer_real_t *max);

void ferCDCylDumpSVT(const fer_cd_cyl_t *s,
                     FILE *out, const char *name,
                     const fer_mat3_t *rot, const fer_vec3_t *tr);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_CYL_H__ */
