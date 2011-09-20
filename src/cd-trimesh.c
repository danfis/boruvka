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

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <fermat/cd.h>
#include <fermat/parse.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

static fer_cd_shape_class_t shape_tri = {
    .type          = FER_CD_SHAPE_TRI,
    .del           = (fer_cd_shape_del_fn)ferCDTriDel,
    .support       = (fer_cd_shape_support_fn)ferCDTriSupport,
    .center        = (fer_cd_shape_center_fn)ferCDTriCenter,
    .fit_obb       = (fer_cd_shape_fit_obb_fn)ferCDTriFitOBB,
    .update_chull  = (fer_cd_shape_update_chull_fn)ferCDTriUpdateCHull,
    .update_minmax = (fer_cd_shape_update_minmax_fn)ferCDTriUpdateMinMax,
    .update_cov    = (fer_cd_shape_update_cov_fn)ferCDTriUpdateCov,
    .dump_svt      = (fer_cd_shape_dump_svt_fn)ferCDTriDumpSVT
};

static fer_cd_shape_class_t shape_trimesh_tri = {
    .type          = FER_CD_SHAPE_TRIMESH_TRI,
    .del           = (fer_cd_shape_del_fn)ferCDTriMeshTriDel,
    .support       = (fer_cd_shape_support_fn)ferCDTriSupport,
    .center        = (fer_cd_shape_center_fn)ferCDTriCenter,
    .fit_obb       = (fer_cd_shape_fit_obb_fn)ferCDTriFitOBB,
    .update_chull  = (fer_cd_shape_update_chull_fn)ferCDTriUpdateCHull,
    .update_minmax = (fer_cd_shape_update_minmax_fn)ferCDTriUpdateMinMax,
    .update_cov    = (fer_cd_shape_update_cov_fn)ferCDTriUpdateCov,
    .dump_svt      = (fer_cd_shape_dump_svt_fn)ferCDTriDumpSVT
};

static fer_cd_shape_class_t shape = {
    .type          = FER_CD_SHAPE_TRIMESH,
    .del           = (fer_cd_shape_del_fn)ferCDTriMeshDel,
    .support       = (fer_cd_shape_support_fn)ferCDTriMeshSupport,
    .center        = (fer_cd_shape_center_fn)ferCDTriMeshCenter,
    .fit_obb       = (fer_cd_shape_fit_obb_fn)ferCDTriMeshFitOBB,
    .update_chull  = (fer_cd_shape_update_chull_fn)ferCDTriMeshUpdateCHull,
    .update_minmax = (fer_cd_shape_update_minmax_fn)ferCDTriMeshUpdateMinMax,
    .update_cov    = (fer_cd_shape_update_cov_fn)ferCDTriMeshUpdateCov,
    .dump_svt      = (fer_cd_shape_dump_svt_fn)ferCDTriMeshDumpSVT
};

fer_cd_trimesh_tri_t *ferCDTriNew(const fer_vec3_t *p1,
                                  const fer_vec3_t *p2,
                                  const fer_vec3_t *p3)
{
    fer_cd_tri_t *t;

    t = FER_ALLOC(fer_cd_tri_t);
    t->shape.cl = &shape_tri;
    t->p[0] = ferVec3Clone(p1);
    t->p[1] = ferVec3Clone(p2);
    t->p[2] = ferVec3Clone(p3);

    return t;
}

void ferCDTriDel(fer_cd_trimesh_tri_t *t)
{
    ferVec3Del(t->p[0]);
    ferVec3Del(t->p[1]);
    ferVec3Del(t->p[2]);
    FER_FREE(t);
}

fer_cd_trimesh_tri_t *ferCDTriMeshTriNew(const fer_vec3_t *p1,
                                         const fer_vec3_t *p2,
                                         const fer_vec3_t *p3)
{
    fer_cd_trimesh_tri_t *tri;

    tri = FER_ALLOC(fer_cd_trimesh_tri_t);

    tri->shape.cl = &shape_trimesh_tri;
    tri->p[0] = (fer_vec3_t *)p1;
    tri->p[1] = (fer_vec3_t *)p2;
    tri->p[2] = (fer_vec3_t *)p3;

    return tri;
}

void ferCDTriMeshTriDel(fer_cd_trimesh_tri_t *tri)
{
    FER_FREE(tri);
}




fer_cd_trimesh_t *ferCDTriMeshNew(const fer_vec3_t *pts,
                                  const unsigned int *ids, size_t len,
                                  const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_cd_trimesh_t *t;
    size_t i, ptslen;

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

    t = FER_ALLOC(fer_cd_trimesh_t);

    t->shape.cl = &shape;

    ptslen = 0;
    for (i = 0; i < 3 * len; i++){
        if (ids[i] + 1 > ptslen)
            ptslen = ids[i] + 1;
    }

    t->pts = ferVec3ArrNew(ptslen);
    t->ptslen = ptslen;
    t->ids = FER_ALLOC_ARR(unsigned int, len * 3);
    t->len = len;

    for (i = 0; i < ptslen; i++){
        ferMat3MulVec(t->pts + i, rot, pts + i);
        ferVec3Add(t->pts + i, tr);
    }
    for (i = 0; i < 3 * len; i++){
        t->ids[i] = ids[i];
    }

    return t;
}


void ferCDTriMeshDel(fer_cd_trimesh_t *t)
{
    FER_FREE(t->pts);
    FER_FREE(t->ids);
    FER_FREE(t);
}

void ferCDTriSupport(const fer_cd_trimesh_tri_t *t, const fer_vec3_t *dir,
                     fer_vec3_t *p)
{
    fer_real_t d1, d2, d3;

    d1 = ferVec3Dot(t->p[0], dir);
    d2 = ferVec3Dot(t->p[1], dir);
    d3 = ferVec3Dot(t->p[2], dir);

    if (d1 > d2){
        if (d1 > d3){
            ferVec3Copy(p, t->p[0]);
        }else{ // d1 <= d3
            ferVec3Copy(p, t->p[2]);
        }
    }else{ // d1 <= d2
        if (d2 > d3){
            ferVec3Copy(p, t->p[1]);
        }else{ // d2 <= d3
            ferVec3Copy(p, t->p[2]);
        }
    }
}

void ferCDTriMeshSupport(const fer_cd_trimesh_t *t,
                         const fer_vec3_t *dir,
                         fer_vec3_t *p)
{
    fer_real_t d, dtmp;
    size_t i;

    d = FER_REAL_MAX;
    for (i = 0; i < t->ptslen; i++){
        dtmp = ferVec3Dot(dir, &t->pts[i]);
        if (dtmp < d){
            d = dtmp;
            ferVec3Copy(p, &t->pts[i]);
        }
    }
}

void ferCDTriCenter(const fer_cd_trimesh_tri_t *t,
                    const fer_mat3_t *rot, const fer_vec3_t *tr,
                    fer_vec3_t *center)
{
    fer_vec3_t v;

    ferVec3Add2(center, t->p[0], t->p[1]);
    ferVec3Add(center, t->p[2]);
    ferVec3Scale(center, ferRecp(FER_REAL(3.)));

    if (rot){
        ferVec3Copy(&v, center);
        ferMat3MulVec(center, rot, &v);
    }
    if (tr)
        ferVec3Add(center, tr);
}

void ferCDTriMeshCenter(const fer_cd_trimesh_t *t,
                        const fer_mat3_t *rot,
                        const fer_vec3_t *tr,
                        fer_vec3_t *center)
{
    fer_vec3_t v;
    size_t i;

    ferVec3Set(center, FER_ZERO, FER_ZERO, FER_ZERO);
    for (i = 0; i < t->ptslen; i++){
        ferVec3Add(center, &t->pts[i]);
    }
    ferVec3Scale(center, ferRecp((fer_real_t)t->ptslen));

    if (rot){
        ferVec3Copy(&v, center);
        ferMat3MulVec(center, rot, &v);
    }
    if (tr)
        ferVec3Add(center, tr);
}

void ferCDTriFitOBB(const fer_cd_trimesh_tri_t *tri,
                    fer_vec3_t *center,
                    fer_vec3_t *_axis0,
                    fer_vec3_t *_axis1,
                    fer_vec3_t *_axis2,
                    fer_vec3_t *half_extents, int flags)
{
    fer_vec3_t e01, e02, e12; // triangle edges
    fer_vec3_t *axis[3];
    fer_vec3_t v;
    fer_real_t d01, d02, d12;
    fer_real_t min[3], max[3], m;
    int i;

    axis[0] = _axis0;
    axis[1] = _axis1;
    axis[2] = _axis2;

    // 1. compute triangle edges
    ferVec3Sub2(&e01, tri->p[1], tri->p[0]);
    ferVec3Sub2(&e02, tri->p[2], tri->p[0]);
    ferVec3Sub2(&e12, tri->p[2], tri->p[1]);

    // 2. find longest edge and compute from that first normalized axis of
    //    bounding box
    d01 = ferVec3Len2(&e01);
    d02 = ferVec3Len2(&e02);
    d12 = ferVec3Len2(&e12);
    if (d01 > d02){
        if (d01 > d12){
            ferVec3Scale2(axis[0], &e01, ferRsqrt(d01));
        }else{
            ferVec3Scale2(axis[0], &e12, ferRsqrt(d12));
        }
    }else{
        if (d02 > d12){
            ferVec3Scale2(axis[0], &e02, ferRsqrt(d02));
        }else{
            ferVec3Scale2(axis[0], &e12, ferRsqrt(d12));
        }
    }

    // 3. compute third axis as vector perpendicular to triangle
    ferVec3Cross(axis[2], &e01, &e02);
    ferVec3Normalize(axis[2]);

    // 4. compute second axis
    //    note that normalization is not needed because a0 and a2 are
    //    already normalized
    ferVec3Cross(axis[1], axis[2], axis[0]);

    // 5. min and max values of projected points of triangle on bounding
    //    boxes' axes.
    for (i = 0; i < 3; i++){
        min[i] = max[i] = ferVec3Dot(tri->p[0], axis[i]);

        m = ferVec3Dot(tri->p[1], axis[i]);
        if (m < min[i]){
            min[i] = m;
        }else{
            max[i] = m;
        }

        m = ferVec3Dot(tri->p[2], axis[i]);
        if (m < min[i]){
            min[i] = m;
        }else if (m > max[i]){
            max[i] = m;
        }
    }

    // 6. compute center from min/max values
    ferVec3Scale2(center, axis[0], (min[0] + max[0]) * FER_REAL(0.5));
    ferVec3Scale2(&v, axis[1], (min[1] + max[1]) * FER_REAL(0.5));
    ferVec3Add(center, &v);
    ferVec3Scale2(&v, axis[2], (min[2] + max[2]) * FER_REAL(0.5));
    ferVec3Add(center, &v);

    // 7. compute extents
    ferVec3Set(half_extents, (max[0] - min[0]) * FER_REAL(0.5),
                             (max[1] - min[1]) * FER_REAL(0.5),
                             (max[2] - min[2]) * FER_REAL(0.5));
}

void ferCDTriMeshFitOBB(const fer_cd_trimesh_t *t,
                        fer_vec3_t *center,
                        fer_vec3_t *_axis0,
                        fer_vec3_t *_axis1,
                        fer_vec3_t *_axis2,
                        fer_vec3_t *half_extents, int flags)
{
    // TODO
    fprintf(stderr, "ferCDTriMeshFitOBB: Not working\n");
}

int ferCDTriUpdateCHull(const fer_cd_trimesh_tri_t *tri, fer_chull3_t *chull,
                        const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_vec3_t v;

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

    ferMat3MulVec(&v, rot, tri->p[0]);
    ferVec3Add(&v, tr);
    ferCHull3Add(chull, &v);

    ferMat3MulVec(&v, rot, tri->p[1]);
    ferVec3Add(&v, tr);
    ferCHull3Add(chull, &v);

    ferMat3MulVec(&v, rot, tri->p[2]);
    ferVec3Add(&v, tr);
    ferCHull3Add(chull, &v);

    return 1;
}

int ferCDTriMeshUpdateCHull(const fer_cd_trimesh_t *t, fer_chull3_t *chull,
                            const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_vec3_t v;
    size_t i;

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

    for (i = 0; i < t->ptslen; i++){
        ferMat3MulVec(&v, rot, &t->pts[i]);
        ferVec3Add(&v, tr);
        ferCHull3Add(chull, &v);
    }

    return 1;
}

void ferCDTriUpdateMinMax(const fer_cd_trimesh_tri_t *tri,
                          const fer_vec3_t *axis,
                          const fer_mat3_t *rot, const fer_vec3_t *tr,
                          fer_real_t *min, fer_real_t *max)
{
    fer_vec3_t p;
    fer_real_t m;

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

    /*
    DBG_VEC3(tri->p0, "tri->p0: ");
    DBG_VEC3(tri->p1, "tri->p1: ");
    DBG_VEC3(tri->p2, "tri->p2: ");
    */
    ferMat3MulVec(&p, rot, tri->p[0]);
    ferVec3Add(&p, tr);
    m = ferVec3Dot(&p, axis);
    if (m < *min)
        *min = m;
    if (m > *max)
        *max = m;

    ferMat3MulVec(&p, rot, tri->p[1]);
    ferVec3Add(&p, tr);
    m = ferVec3Dot(&p, axis);
    if (m < *min)
        *min = m;
    if (m > *max)
        *max = m;

    ferMat3MulVec(&p, rot, tri->p[2]);
    ferVec3Add(&p, tr);
    m = ferVec3Dot(&p, axis);
    if (m < *min)
        *min = m;
    if (m > *max)
        *max = m;
}

void ferCDTriMeshUpdateMinMax(const fer_cd_trimesh_t *t,
                              const fer_vec3_t *axis,
                              const fer_mat3_t *rot, const fer_vec3_t *tr,
                              fer_real_t *min, fer_real_t *max)
{
    fer_vec3_t p;
    fer_real_t m;
    size_t i;

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

    for (i = 0; i < t->ptslen; i++){
        ferMat3MulVec(&p, rot, &t->pts[i]);
        ferVec3Add(&p, tr);
        m = ferVec3Dot(&p, axis);
        if (m < *min)
            *min = m;
        if (m > *max)
            *max = m;
    }
}


void ferCDTriUpdateCov(const fer_cd_tri_t *t,
                       const fer_mat3_t *rot, const fer_vec3_t *tr,
                       fer_vec3_t *wcenter, fer_mat3_t *cov,
                       fer_real_t *area, int *num)
{
    ferCDShapeUpdateCovTri(t->p[0], t->p[1], t->p[2], rot, tr, wcenter, cov, area, num);
}

void ferCDTriMeshUpdateCov(const fer_cd_trimesh_t *t,
                           const fer_mat3_t *rot, const fer_vec3_t *tr,
                           fer_vec3_t *wcenter, fer_mat3_t *cov,
                           fer_real_t *area, int *num)
{
    size_t i;

    for (i = 0; i < t->len; i++){
        ferCDShapeUpdateCovTri(&t->pts[t->ids[3 * i + 0]],
                               &t->pts[t->ids[3 * i + 1]],
                               &t->pts[t->ids[3 * i + 2]],
                               rot, tr, wcenter, cov, area, num);
    }
}

void ferCDTriDumpSVT(const fer_cd_trimesh_tri_t *tri,
                     FILE *out, const char *name,
                     const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_vec3_t v;

    fprintf(out, "----\n");

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

    if (name){
        fprintf(out, "Name: %s\n", name);
    }

    fprintf(out, "Points:\n");
    ferMat3MulVec(&v, rot, tri->p[0]);
    ferVec3Add(&v, tr);
    ferVec3Print(&v, out);
    fprintf(out, "\n");
    ferMat3MulVec(&v, rot, tri->p[1]);
    ferVec3Add(&v, tr);
    ferVec3Print(&v, out);
    fprintf(out, "\n");
    ferMat3MulVec(&v, rot, tri->p[2]);
    ferVec3Add(&v, tr);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    fprintf(out, "Edges:\n");
    fprintf(out, "0 1 1 2 2 0\n");
    fprintf(out, "Faces:\n");
    fprintf(out, "0 1 2\n");

    fprintf(out, "----\n");
}

void ferCDTriMeshDumpSVT(const fer_cd_trimesh_t *t,
                         FILE *out, const char *name,
                         const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    size_t i, numpts;
    fer_vec3_t v;

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

    fprintf(out, "----\n");

    if (name){
        fprintf(out, "Name: %s\n", name);
    }

    numpts = 0;
    for (i = 0; i < 3 * t->len; i++){
        if (t->ids[i] + 1 > numpts)
            numpts = t->ids[i] + 1;
    }

    fprintf(out, "Points:\n");
    for (i = 0; i < numpts; i++){
        ferMat3MulVec(&v, rot, &t->pts[i]);
        ferVec3Add(&v, tr);
        ferVec3Print(&v, out);
        fprintf(out, "\n");
    }

    fprintf(out, "Edges:\n");
    for (i = 0; i < t->len; i++){
        fprintf(out, "%u %u\n", t->ids[3 * i], t->ids[3 * i + 1]);
        fprintf(out, "%u %u\n", t->ids[3 * i + 1], t->ids[3 * i + 2]);
        fprintf(out, "%u %u\n", t->ids[3 * i], t->ids[3 * i + 2]);
    }

    fprintf(out, "Faces:\n");
    for (i = 0; i < t->len; i++){
        fprintf(out, "%u %u %u\n", t->ids[3 * i], t->ids[3 * i + 1], t->ids[3 * i + 2]);
    }

    fprintf(out, "----\n");
}
