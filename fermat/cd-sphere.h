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

#ifndef __FER_CD_SPHERE_H__
#define __FER_CD_SPHERE_H__

#include <fermat/cd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * Sphere shape
 */
struct _fer_cd_sphere_t {
    fer_cd_shape_t shape; /*!< Base class */
    fer_vec3_t *center;
    fer_real_t radius;
};
typedef struct _fer_cd_sphere_t fer_cd_sphere_t;


/**
 * Creates sphere shape
 */
fer_cd_sphere_t *ferCDSphereNew(fer_real_t radius, const fer_vec3_t *center);

/**
 * Deletes sphere.
 */
void ferCDSphereDel(fer_cd_sphere_t *s);


void ferCDSphereDumpSVT(const fer_cd_sphere_t *s, FILE *out, const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_SPHERE_H__ */



