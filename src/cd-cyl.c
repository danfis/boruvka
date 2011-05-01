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
#include <fermat/dbg.h>

static fer_cd_shape_class_t shape = {
    .type          = FER_CD_SHAPE_CYL,
    .support       = (fer_cd_shape_support)ferCDCylSupport,
    .fit_obb       = (fer_cd_shape_fit_obb)ferCDCylFitOBB,
    .update_chull  = (fer_cd_shape_update_chull)ferCDCylUpdateCHull,
    .update_minmax = (fer_cd_shape_update_minmax)ferCDCylUpdateMinMax,
    .dump_svt      = (fer_cd_shape_dump_svt)ferCDCylDumpSVT
};

fer_cd_cyl_t *ferCDCylNew(fer_real_t radius, fer_real_t height)
{
    fer_cd_cyl_t *c;

    c = FER_ALLOC(fer_cd_cyl_t);
    c->shape.cl = &shape;
    c->radius = radius;
    c->half_height = height * FER_REAL(0.5);

    return c;
}

void ferCDCylDel(fer_cd_cyl_t *c)
{
    free(c);
}

void ferCDCylSupport(const fer_cd_cyl_t *c, const fer_vec3_t *dir,
                     fer_vec3_t *p)
{
    fer_real_t d;

    d = ferVec3X(dir) * ferVec3X(dir) + ferVec3Y(dir) * ferVec3Y(dir);
    d = FER_SQRT(d);

    if (ferIsZero(d)){
        ferVec3Set(p, FER_ZERO, FER_ZERO,
                      ferSign(ferVec3Z(dir)) * c->half_height);
    }else{
        d = ferRecp(d) * c->radius;

        ferVec3Set(p, d * ferVec3X(dir),
                      d * ferVec3Y(dir),
                      ferSign(ferVec3Z(dir)) * c->half_height);
    }
}

void ferCDCylFitOBB(const fer_cd_cyl_t *c,
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
    ferVec3Set(half_extents, c->radius, c->radius, c->half_height);
}

int ferCDCylUpdateCHull(const fer_cd_cyl_t *c, fer_chull3_t *chull,
                        const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_vec3_t a, v;

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

    // get z axis
    ferMat3CopyCol(&a, rot, 2);
    ferVec3Scale(&a, c->half_height); // it is assumend len(&a) == 1

    ferVec3Add2(&v, tr, &a);
    ferCHull3Add(chull, &v);

    ferVec3Sub2(&v, tr, &a);
    ferCHull3Add(chull, &v);

    return 0;
}

void ferCDCylUpdateMinMax(const fer_cd_cyl_t *c, const fer_vec3_t *_axis,
                          const fer_mat3_t *rot, const fer_vec3_t *tr,
                          fer_real_t *min, fer_real_t *max)
{
    fer_vec3_t axis, p, q;
    fer_real_t m;

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

    ferMat3MulVecTrans(&axis, rot, _axis);
    ferCDCylSupport(c, &axis, &p);
    ferMat3MulVec(&q, rot, &p);
    ferVec3Add(&q, tr);

    m = ferVec3Dot(&q, _axis);
    if (m < *min)
        *min = m;
    if (m > *max)
        *max = m;

    ferVec3Scale(&axis, -FER_ONE);
    ferCDCylSupport(c, &axis, &p);
    ferMat3MulVec(&q, rot, &p);
    ferVec3Add(&q, tr);

    m = ferVec3Dot(&q, _axis);
    if (m < *min)
        *min = m;
    if (m > *max)
        *max = m;
}


void ferCDCylDumpSVT(const fer_cd_cyl_t *c,
                     FILE *out, const char *name,
                     const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_vec3_t v, axis;

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

    ferVec3Scale2(&v, fer_vec3_axis[2], c->half_height);
    ferMat3MulVec(&axis, rot, &v);

    fprintf(out, "----\n");

    if (name){
        fprintf(out, "Name: %s\n", name);
    }

    fprintf(out, "Points:\n");
    ferVec3Add2(&v, tr, &axis);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Sub2(&v, tr, &axis);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    fprintf(out, "Edges:\n");
    fprintf(out, "0 1\n");

    fprintf(out, "----\n");
}
