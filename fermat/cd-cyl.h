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
    fer_cd_shape_t shape; /*!< Base class */
    fer_vec3_t *center;
    fer_mat3_t *axis;
    fer_real_t radius, half_height;
};
typedef struct _fer_cd_cyl_t fer_cd_cyl_t;


/**
 * Creates sphere shape
 */
fer_cd_cyl_t *ferCDCylNew(fer_real_t radius, fer_real_t height,
                          const fer_vec3_t *center, const fer_mat3_t *rot);

/**
 * Deletes sphere.
 */
void ferCDCylDel(fer_cd_cyl_t *c);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_CYL_H__ */
