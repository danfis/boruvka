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

fer_cd_sphere_t *ferCDSphereNew(const fer_vec3_t *center, fer_real_t radius)
{
    fer_cd_sphere_t *s;

    s = FER_ALLOC(fer_cd_sphere_t);
    s->shape.type = FER_CD_SHAPE_SPHERE;
    s->center = ferVec3Clone(center);
    s->radius = radius;

    return s;
}

void ferCDSphereDel(fer_cd_sphere_t *s)
{
    ferVec3Del(s->center);
    free(s);
}


void ferCDSphereDumpSVT(const fer_cd_sphere_t *s, FILE *out, const char *name)
{
    fer_vec3_t v;
    size_t i;

    fprintf(out, "-----\n");
    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Points:\n");
    for (i = 0; i < fer_points_on_sphere_len; i++){
        ferVec3Copy(&v, fer_points_on_sphere + i);
        ferVec3Scale(&v, s->radius);
        ferVec3Add(&v, s->center);

        ferVec3Print(&v, out);
        fprintf(out, "\n");
    }
    fprintf(out, "-----\n");
}
