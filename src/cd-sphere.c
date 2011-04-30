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

static fer_cd_shape_class_t shape = {
    .type = FER_CD_SHAPE_SPHERE,
    .support = (fer_cd_shape_support)ferCDSphereSupport,
    .fit_obb = (fer_cd_shape_fit_obb)ferCDSphereFitOBB,
    .update_chull = (fer_cd_shape_update_chull)ferCDSphereUpdateCHull,
    .dump_svt = (fer_cd_shape_dump_svt)ferCDSphereDumpSVT
};

fer_cd_sphere_t *ferCDSphereNew(fer_real_t radius)
{
    fer_cd_sphere_t *s;

    s = FER_ALLOC(fer_cd_sphere_t);
    s->shape.cl = &shape;
    s->radius = radius;

    return s;
}

void ferCDSphereDel(fer_cd_sphere_t *s)
{
    free(s);
}

void ferCDSphereSupport(const fer_cd_sphere_t *s, const fer_vec3_t *dir,
                        fer_vec3_t *p)
{
    ferVec3Scale2(p, dir, s->radius);
}

void ferCDSphereFitOBB(const fer_cd_sphere_t *s,
                       fer_vec3_t *center,
                       fer_vec3_t *axis0,
                       fer_vec3_t *axis1,
                       fer_vec3_t *axis2,
                       fer_vec3_t *half_extents, int flags)
{
    ferVec3Set(center, FER_ZERO, FER_ZERO, FER_ZERO);
    ferVec3Set(axis0, FER_ONE,  FER_ZERO, FER_ZERO);
    ferVec3Set(axis1, FER_ZERO, FER_ONE,  FER_ZERO);
    ferVec3Set(axis2, FER_ZERO, FER_ZERO, FER_ONE);
    ferVec3Set(half_extents, s->radius, s->radius, s->radius);
}

int ferCDSphereUpdateCHull(const fer_cd_sphere_t *s, fer_chull3_t *chull,
                           const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    if (!tr)
        tr = fer_vec3_origin;

    ferCHull3Add(chull, tr);
    return 0;
}

void ferCDSphereDumpSVT(const fer_cd_sphere_t *s,
                        FILE *out, const char *name,
                        const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_vec3_t v;
    size_t i;

    if (!tr)
        tr = fer_vec3_origin;

    fprintf(out, "-----\n");
    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Points:\n");
    for (i = 0; i < fer_points_on_sphere_len; i++){
        ferVec3Scale2(&v, fer_points_on_sphere + i, s->radius);
        ferVec3Add(&v, tr);

        ferVec3Print(&v, out);
        fprintf(out, "\n");
    }
    fprintf(out, "-----\n");
}
