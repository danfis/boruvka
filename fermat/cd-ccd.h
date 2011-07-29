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

#ifndef __FER_CD_CCD_H__
#define __FER_CD_CCD_H__

#include <fermat/cd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_cd_t;


int ferCDCollideCCD(struct _fer_cd_t *cd,
                    const fer_cd_shape_t *_s1,
                    const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                    const fer_cd_shape_t *_s2,
                    const fer_mat3_t *rot2, const fer_vec3_t *tr2);

int ferCDSeparateCCD(struct _fer_cd_t *cd,
                     const fer_cd_shape_t *s1,
                     const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                     const fer_cd_shape_t *s2,
                     const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                     fer_cd_contacts_t *con);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_CCD_H__ */



