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

#include <fermat/cd.h>
#include <fermat/alloc.h>


fer_cd_cyl_t *ferCDCylNew(fer_real_t radius, fer_real_t height,
                          const fer_vec3_t *center, const fer_mat3_t *rot)
{
    fer_cd_cyl_t *c;

    c = FER_ALLOC(fer_cd_cyl_t);
    c->shape.type = FER_CD_SHAPE_CYL;
    c->radius = radius;
    c->half_height = height * FER_REAL(0.5);
    c->center = ferVec3Clone(center);
    c->axis   = ferMat3Clone(rot);

    return c;
}

void ferCDCylDel(fer_cd_cyl_t *c)
{
    ferVec3Del(c->center);
    ferMat3Del(c->axis);
}
