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

_fer_inline void setRotTr(const fer_cd_shape_off_t *s,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                          fer_mat3_t *tmprot, fer_vec3_t *tmptr,
                          fer_mat3_t **rot, fer_vec3_t **tr);

static fer_cd_shape_class_t shape_off = {
    .type          = FER_CD_SHAPE_OFF,
    .del           = (fer_cd_shape_del_fn)ferCDShapeOffDel,
    .support       = (fer_cd_shape_support_fn)ferCDShapeOffSupport,
    .fit_obb       = (fer_cd_shape_fit_obb_fn)ferCDShapeOffFitOBB,
    .update_chull  = (fer_cd_shape_update_chull_fn)ferCDShapeOffUpdateCHull,
    .update_minmax = (fer_cd_shape_update_minmax_fn)ferCDShapeOffUpdateMinMax,
    .dump_svt      = (fer_cd_shape_dump_svt_fn)ferCDShapeOffDumpSVT
};

fer_cd_shape_off_t *ferCDShapeOffNew(fer_cd_shape_t *shape,
                                     const fer_mat3_t *rot,
                                     const fer_vec3_t *tr)
{
    fer_cd_shape_off_t *s;

    s = FER_ALLOC(fer_cd_shape_off_t);
    s->cl    = &shape_off;
    s->shape = shape;
    s->rot   = ferMat3Clone(rot);
    s->tr    = ferVec3Clone(tr);

    return s;
}

void ferCDShapeOffDel(fer_cd_shape_off_t *s)
{
    if (s->shape->cl->del)
        s->shape->cl->del(s->shape);
    ferMat3Del(s->rot);
    ferVec3Del(s->tr);
    free(s);
}

void ferCDShapeOffSupport(const fer_cd_shape_off_t *s, const fer_vec3_t *_dir,
                          fer_vec3_t *p)
{
    fer_vec3_t dir, q;

    if (!s->shape->cl->support)
        return;

    ferMat3MulVecTrans(&dir, s->rot, _dir);
    s->shape->cl->support(s->shape, &dir, p);
    ferMat3MulVec(&q, s->rot, p);
    ferVec3Add2(p, &q, s->tr);
}

void ferCDShapeOffFitOBB(const fer_cd_shape_off_t *s,
                         fer_vec3_t *center,
                         fer_vec3_t *axis0,
                         fer_vec3_t *axis1,
                         fer_vec3_t *axis2,
                         fer_vec3_t *half_extents, int flags)
{
    fer_vec3_t a0, a1, a2;

    if (!s->shape->cl->fit_obb)
        return;

    s->shape->cl->fit_obb(s->shape, center, &a0, &a1, &a2,
                          half_extents, flags);
    ferMat3MulVec(axis0, s->rot, &a0);
    ferMat3MulVec(axis1, s->rot, &a1);
    ferMat3MulVec(axis2, s->rot, &a2);
    ferVec3Add(center, s->tr);
}

int ferCDShapeOffUpdateCHull(const fer_cd_shape_off_t *s, fer_chull3_t *chull,
                             const fer_mat3_t *_rot, const fer_vec3_t *_tr)
{
    fer_mat3_t *rot, rot2;
    fer_vec3_t *tr, tr2;

    if (!s->shape->cl->update_chull)
        return 0;

    setRotTr(s, _rot, _tr, &rot2, &tr2, &rot, &tr);

    return s->shape->cl->update_chull(s->shape, chull, rot, tr);
}

void ferCDShapeOffUpdateMinMax(const fer_cd_shape_off_t *s, const fer_vec3_t *axis,
                               const fer_mat3_t *_rot, const fer_vec3_t *_tr,
                               fer_real_t *min, fer_real_t *max)
{
    fer_mat3_t *rot, rot2;
    fer_vec3_t *tr, tr2;

    if (!s->shape->cl->update_minmax)
        return;

    setRotTr(s, _rot, _tr, &rot2, &tr2, &rot, &tr);

    s->shape->cl->update_minmax(s->shape, axis, rot, tr, min, max);
}


void ferCDShapeOffDumpSVT(const fer_cd_shape_off_t *s,
                          FILE *out, const char *name,
                          const fer_mat3_t *_rot, const fer_vec3_t *_tr)
{
    fer_mat3_t *rot, rot2;
    fer_vec3_t *tr, tr2;

    if (!s->shape->cl->dump_svt)
        return;

    setRotTr(s, _rot, _tr, &rot2, &tr2, &rot, &tr);

    s->shape->cl->dump_svt(s->shape, out, name, rot, tr);
}

_fer_inline void setRotTr(const fer_cd_shape_off_t *s,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                          fer_mat3_t *tmprot, fer_vec3_t *tmptr,
                          fer_mat3_t **rot, fer_vec3_t **tr)
{
    *rot = s->rot;
    *tr  = s->tr;

    if (rot2){
        ferMat3Mul2(tmprot, rot2, s->rot);
        *rot = tmprot;

        ferMat3MulVec(tmptr, rot2, s->tr);
        *tr = tmptr;

        if (tr2)
            ferVec3Add(*tr, tr2);
    }else if (tr2){
        ferVec3Add2(tmptr, s->tr, tr2);
        *tr = tmptr;
    }
}
