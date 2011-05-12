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

#ifndef __FER_CD_BOX_H__
#define __FER_CD_BOX_H__

#include <fermat/cd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Box shape
 */
struct _fer_cd_box_t {
    fer_cd_shape_t shape; /*!< Base class */
    fer_vec3_t *half_extents;
};
typedef struct _fer_cd_box_t fer_cd_box_t;


/**
 * Creates box shape
 */
fer_cd_box_t *ferCDBoxNew(fer_real_t lx, fer_real_t ly, fer_real_t lz);

/**
 * Deletes box.
 */
void ferCDBoxDel(fer_cd_box_t *s);

void ferCDBoxSupport(const fer_cd_box_t *b, const fer_vec3_t *dir,
                     fer_vec3_t *p);

void ferCDBoxCenter(const fer_cd_box_t *s,
                    const fer_mat3_t *rot, const fer_vec3_t *tr,
                    fer_vec3_t *center);

void ferCDBoxFitOBB(const fer_cd_box_t *s,
                    fer_vec3_t *center,
                    fer_vec3_t *axis0,
                    fer_vec3_t *axis1,
                    fer_vec3_t *axis2,
                    fer_vec3_t *half_extents, int flags);

int ferCDBoxUpdateCHull(const fer_cd_box_t *b, fer_chull3_t *chull,
                        const fer_mat3_t *rot, const fer_vec3_t *tr);

void ferCDBoxUpdateMinMax(const fer_cd_box_t *b, const fer_vec3_t *axis,
                          const fer_mat3_t *rot, const fer_vec3_t *tr,
                          fer_real_t *min, fer_real_t *max);

void ferCDBoxUpdateCov(const fer_cd_box_t *s,
                       const fer_mat3_t *rot, const fer_vec3_t *tr,
                       fer_vec3_t *wcenter, fer_mat3_t *cov,
                       fer_real_t *area, int *num);

void ferCDBoxDumpSVT(const fer_cd_box_t *s,
                     FILE *out, const char *name,
                     const fer_mat3_t *rot, const fer_vec3_t *tr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_BOX_H__ */

