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
    .del           = (fer_cd_shape_del_fn)ferCDSphereDel,
    .support       = (fer_cd_shape_support_fn)ferCDSphereSupport,
    .center        = (fer_cd_shape_center_fn)ferCDSphereCenter,
    .fit_obb       = (fer_cd_shape_fit_obb_fn)ferCDSphereFitOBB,
    .update_chull  = (fer_cd_shape_update_chull_fn)ferCDSphereUpdateCHull,
    .update_minmax = (fer_cd_shape_update_minmax_fn)ferCDSphereUpdateMinMax,
    .dump_svt      = (fer_cd_shape_dump_svt_fn)ferCDSphereDumpSVT
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

void ferCDSphereCenter(const fer_cd_sphere_t *s,
                       const fer_mat3_t *rot, const fer_vec3_t *tr,
                       fer_vec3_t *center)
{
    if (!tr)
        tr = fer_vec3_origin;
    ferVec3Copy(center, tr);
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

void ferCDSphereUpdateMinMax(const fer_cd_sphere_t *s, const fer_vec3_t *axis,
                             const fer_mat3_t *rot, const fer_vec3_t *tr,
                             fer_real_t *min, fer_real_t *max)
{
    fer_real_t m;

    m = FER_ZERO;
    if (tr){
        m = ferVec3Dot(tr, axis);
    }

    m += s->radius;
    if (m < *min)
        *min = m;
    if (m > *max)
        *max = m;

    m -= FER_REAL(2.) * s->radius;
    if (m < *min)
        *min = m;
    if (m > *max)
        *max = m;
}


void ferCDSphereDumpSVT(const fer_cd_sphere_t *s,
                        FILE *out, const char *name,
                        const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    if (!tr)
        tr = fer_vec3_origin;

    fprintf(out, "-----\n");
    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Spheres:\n");
    fprintf(out, "%f ", s->radius);
    ferVec3Print(tr, out);
    fprintf(out, "\n");
    fprintf(out, "-----\n");
}
