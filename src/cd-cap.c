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
    .type          = FER_CD_SHAPE_CAP,
    .del           = (fer_cd_shape_del_fn)ferCDCapDel,
    .support       = (fer_cd_shape_support_fn)ferCDCapSupport,
    .center        = (fer_cd_shape_center_fn)ferCDCapCenter,
    .fit_obb       = (fer_cd_shape_fit_obb_fn)ferCDCapFitOBB,
    .update_chull  = (fer_cd_shape_update_chull_fn)ferCDCapUpdateCHull,
    .update_minmax = (fer_cd_shape_update_minmax_fn)ferCDCapUpdateMinMax,
    .update_cov    = (fer_cd_shape_update_cov_fn)ferCDCapUpdateCov,
    .dump_svt      = (fer_cd_shape_dump_svt_fn)ferCDCapDumpSVT
};

fer_cd_cap_t *ferCDCapNew(fer_real_t radius, fer_real_t height)
{
    fer_cd_cap_t *c;

    c = FER_ALLOC(fer_cd_cap_t);
    c->shape.cl = &shape;
    c->radius = radius;
    c->half_height = height * FER_REAL(0.5);

    return c;
}

void ferCDCapDel(fer_cd_cap_t *c)
{
    free(c);
}

void ferCDCapSupport(const fer_cd_cap_t *c, const fer_vec3_t *dir,
                     fer_vec3_t *p)
{
    ferVec3Scale2(p, dir, c->radius);

    if (ferVec3Z(p) > -ferVec3Z(p)){
        ferVec3SetZ(p, ferVec3Z(p) + c->half_height);
    }else{
        ferVec3SetZ(p, ferVec3Z(p) - c->half_height);
    }
}

void ferCDCapCenter(const fer_cd_cap_t *s,
                    const fer_mat3_t *rot, const fer_vec3_t *tr,
                    fer_vec3_t *center)
{
    if (!tr)
        tr = fer_vec3_origin;
    ferVec3Copy(center, tr);
}

void ferCDCapFitOBB(const fer_cd_cap_t *c,
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
    ferVec3Set(half_extents, c->radius, c->radius, c->half_height + c->radius);
}

int ferCDCapUpdateCHull(const fer_cd_cap_t *c, fer_chull3_t *chull,
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

void ferCDCapUpdateMinMax(const fer_cd_cap_t *c, const fer_vec3_t *_axis,
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
    ferCDCapSupport(c, &axis, &p);
    ferMat3MulVec(&q, rot, &p);
    ferVec3Add(&q, tr);

    m = ferVec3Dot(&q, _axis);
    if (m < *min)
        *min = m;
    if (m > *max)
        *max = m;

    ferVec3Scale(&axis, -FER_ONE);
    ferCDCapSupport(c, &axis, &p);
    ferMat3MulVec(&q, rot, &p);
    ferVec3Add(&q, tr);

    m = ferVec3Dot(&q, _axis);
    if (m < *min)
        *min = m;
    if (m > *max)
        *max = m;
}

void ferCDCapUpdateCov(const fer_cd_cap_t *s,
                       const fer_mat3_t *rot, const fer_vec3_t *tr,
                       fer_vec3_t *wcenter, fer_mat3_t *cov,
                       fer_real_t *area, int *num)
{
    fer_real_t A, val, val2;
    fer_vec3_t center;
    int i, j;

    if (!tr)
        tr = fer_vec3_origin;

    A = s->radius * s->radius * M_PI * FER_REAL(2.) * s->half_height;

    ferVec3Scale2(&center, tr, A);

    // update covariance matrix
    for (i = 0; i < 3; i++){
        for (j = 0; j < 3; j++){
            val  = A * ferVec3Get(&center, i) * ferVec3Get(&center, j);
            val2 = ferMat3Get(cov, i, j);
            ferMat3Set1(cov, i, j, val + val2);
        }
    }

    ferVec3Add(wcenter, &center);
    *area += A;
    *num += 1;
}

void ferCDCapDumpSVT(const fer_cd_cap_t *c,
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

    fprintf(out, "Spheres:\n");
    ferVec3Add2(&v, tr, &axis);
    fprintf(out, "%f ", (float)c->radius);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Sub2(&v, tr, &axis);
    fprintf(out, "%f ", (float)c->radius);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    fprintf(out, "----\n");
}
