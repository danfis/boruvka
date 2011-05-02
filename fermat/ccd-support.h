/***
 * fermat
 * -------
 * Copyright (c)2010,2011 Daniel Fiser <danfis@danfis.cz>
 *
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

#ifndef __FER_CCD_SUPPORT_H__
#define __FER_CCD_SUPPORT_H__

#include <fermat/vec3.h>
#include <fermat/ccd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_ccd_support_t {
    fer_vec3_t v;  /*!< Support point in minkowski sum */
    fer_vec3_t v1; /*!< Support point in obj1 */
    fer_vec3_t v2; /*!< Support point in obj2 */
};
typedef struct _fer_ccd_support_t fer_ccd_support_t;

_fer_inline void ferCCDSupportCopy(fer_ccd_support_t *, const fer_ccd_support_t *s);

/**
 * Computes support point of obj1 and obj2 in direction dir.
 * Support point is returned via supp.
 */
void __ferCCDSupport(const void *obj1, const void *obj2,
                     const fer_vec3_t *dir, const fer_ccd_t *ccd,
                     fer_ccd_support_t *supp);


/**** INLINES ****/
_fer_inline void ferCCDSupportCopy(fer_ccd_support_t *d, const fer_ccd_support_t *s)
{
    *d = *s;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_CCD_SUPPORT_H__ */
