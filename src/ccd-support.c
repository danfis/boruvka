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

#include <fermat/ccd-support.h>

void __ferCCDSupport(const void *obj1, const void *obj2,
                     const fer_vec3_t *_dir, const fer_ccd_t *ccd,
                     fer_ccd_support_t *supp)
{
    fer_vec3_t dir;

    ferVec3Copy(&dir, _dir);

    ccd->support1(obj1, &dir, &supp->v1);

    ferVec3Scale(&dir, -FER_ONE);
    ccd->support2(obj2, &dir, &supp->v2);

    ferVec3Sub2(&supp->v, &supp->v1, &supp->v2);
}
