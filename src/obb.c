/**2
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

#include <fermat/obb.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>


fer_obb_tri_t *ferOBBTriNew(const fer_vec3_t *p1, const fer_vec3_t *p2,
                            const fer_vec3_t *p3)
{
    fer_obb_tri_t *tri;

    tri = FER_ALLOC(fer_obb_tri_t);

    tri->pri.type = FER_OBB_PRI_TRI;
    tri->p0 = p1;
    tri->p1 = p2;
    tri->p2 = p3;

    return tri;
}

void ferOBBTriDel(fer_obb_tri_t *tri)
{
    free(tri);
}

fer_obb_trimesh_t *ferOBBTriMeshNew(const fer_vec3_t *pts,
                                    const unsigned int *ids, size_t len)
{
    fer_obb_trimesh_t *t;
    size_t i, ptslen;

    t = FER_ALLOC(fer_obb_trimesh_t);

    t->pri.type = FER_OBB_PRI_TRIMESH;

    ptslen = 0;
    for (i = 0; i < 3 * len; i++){
        if (ids[i] + 1 > ptslen)
            ptslen = ids[i] + 1;
    }

    t->pts = ferVec3ArrNew(ptslen);
    t->ids = FER_ALLOC_ARR(unsigned int, len * 3);
    t->len = len;

    for (i = 0; i < ptslen; i++){
        ferVec3Copy(t->pts + i, pts + i);
    }
    for (i = 0; i < 3 * len; i++){
        t->ids[i] = ids[i];
    }

    return t;
}

void ferOBBTriMeshDel(fer_obb_trimesh_t *t)
{
    free(t->pts);
    free(t->ids);
    free(t);
}



fer_obb_t *ferOBBNew(const fer_vec3_t *c, const fer_vec3_t *a1,
                     const fer_vec3_t *a2, const fer_vec3_t *a3,
                     const fer_vec3_t *half_extents,
                     fer_obb_pri_t *pri)
{
    fer_obb_t *obb;

    obb = FER_ALLOC_ALIGN(fer_obb_t, 16);

    ferVec3Copy(&obb->center, c);
    ferVec3Copy(&obb->axis[0], a1);
    ferVec3Copy(&obb->axis[1], a2);
    ferVec3Copy(&obb->axis[2], a3);
    ferVec3Copy(&obb->half_extents, half_extents);

    obb->pri = pri;

    ferListInit(&obb->obbs);
    ferListInit(&obb->list);

    return obb;
}

/*
fer_obb_t *ferOBBNewPri(fer_obb_pri_t *pri)
{
    if (pri->type == FER_OBB_PRI_TRI)
        return ferOBBNewTri((fer_obb_tri_t *)pri);
    return NULL;
}
*/

fer_obb_t *ferOBBNewTri(const fer_vec3_t *p1, const fer_vec3_t *p2,
                        const fer_vec3_t *p3)
{
    fer_obb_tri_t *tri;
    fer_obb_t *obb;
    fer_vec3_t e01, e02, e12; // triangle edges
    fer_vec3_t v;
    fer_real_t d01, d02, d12;
    fer_real_t min[3], max[3], m;
    int i;

    tri = ferOBBTriNew(p1, p2, p3);
    obb = FER_ALLOC_ALIGN(fer_obb_t, 16);

    // 1. compute triangle edges
    ferVec3Sub2(&e01, tri->p1, tri->p0);
    ferVec3Sub2(&e02, tri->p2, tri->p0);
    ferVec3Sub2(&e12, tri->p2, tri->p1);

    // 2. find longest edge and compute from that first normalized axis of
    //    bounding box
    d01 = ferVec3Len2(&e01);
    d02 = ferVec3Len2(&e02);
    d12 = ferVec3Len2(&e12);
    if (d01 > d02){
        if (d01 > d12){
            ferVec3Scale2(&obb->axis[0], &e01, ferRsqrt(d01));
        }else{
            ferVec3Scale2(&obb->axis[0], &e12, ferRsqrt(d12));
        }
    }else{
        if (d02 > d12){
            ferVec3Scale2(&obb->axis[0], &e02, ferRsqrt(d02));
        }else{
            ferVec3Scale2(&obb->axis[0], &e12, ferRsqrt(d12));
        }
    }

    // 3. compute third axis as vector perpendicular to triangle
    ferVec3Cross(&obb->axis[2], &e01, &e02);
    ferVec3Normalize(&obb->axis[2]);

    // 4. compute second axis
    //    note that normalization is not needed because a0 and a2 are
    //    already normalized
    ferVec3Cross(&obb->axis[1], &obb->axis[2], &obb->axis[0]);

    // 5. min and max values of projected points of triangle on bounding
    //    boxes' axes.
    for (i = 0; i < 3; i++){
        min[i] = max[i] = ferVec3Dot(tri->p0, &obb->axis[i]);

        m = ferVec3Dot(tri->p1, &obb->axis[i]);
        if (m < min[i]){
            min[i] = m;
        }else{
            max[i] = m;
        }

        m = ferVec3Dot(tri->p2, &obb->axis[i]);
        if (m < min[i]){
            min[i] = m;
        }else if (m > max[i]){
            max[i] = m;
        }
    }

    // 6. compute center from min/max values
    ferVec3Scale2(&obb->center, &obb->axis[0], (min[0] + max[0]) * FER_REAL(0.5));
    ferVec3Scale2(&v, &obb->axis[1], (min[1] + max[1]) * FER_REAL(0.5));
    ferVec3Add(&obb->center, &v);
    ferVec3Scale2(&v, &obb->axis[2], (min[2] + max[2]) * FER_REAL(0.5));
    ferVec3Add(&obb->center, &v);

    // 7. compute extents
    ferVec3Set(&obb->half_extents, (max[0] - min[0]) * FER_REAL(0.5),
                                   (max[1] - min[1]) * FER_REAL(0.5),
                                   (max[2] - min[2]) * FER_REAL(0.5));


    obb->pri = (fer_obb_pri_t *)tri;

    ferListInit(&obb->obbs);
    ferListInit(&obb->list);

    return obb;
}



static void __ferOBBMergeMean(fer_obb_t *obb, fer_vec3_t *mean, fer_real_t *num)
{
    fer_list_t *item;
    fer_obb_t *o;
    fer_obb_tri_t *tri;

    if (ferListEmpty(&obb->obbs)){
        if (obb->pri->type == FER_OBB_PRI_TRI){
            tri = (fer_obb_tri_t *)obb->pri;
            ferVec3Add(mean, tri->p0);
            ferVec3Add(mean, tri->p1);
            ferVec3Add(mean, tri->p2);
            *num += 1;
        }
    }else{
        FER_LIST_FOR_EACH(&obb->obbs, item){
            o = FER_LIST_ENTRY(item, fer_obb_t, list);
            __ferOBBMergeMean(o, mean, num);
        }
    }
}

static void ferOBBMergeMean(fer_obb_t *obb, fer_vec3_t *mean)
{
    fer_list_t *item;
    fer_obb_t *o;
    fer_real_t num;

    ferVec3Set(mean, FER_ZERO, FER_ZERO, FER_ZERO);
    num = FER_ZERO;

    FER_LIST_FOR_EACH(&obb->obbs, item){
        o = FER_LIST_ENTRY(item, fer_obb_t, list);
        __ferOBBMergeMean(o, mean, &num);
    }

    if (num > 0)
        ferVec3Scale(mean, ferRecp(num * FER_REAL(3.)));
}


static void __ferOBBMergeCov(fer_obb_t *obb, const fer_vec3_t *mean,
                             fer_real_t *num, fer_mat3_t *cov)
{
    fer_list_t *item;
    fer_obb_t *o;
    fer_obb_tri_t *tri;
    size_t i, j;
    fer_real_t val;

    if (ferListEmpty(&obb->obbs)){
        if (obb->pri->type == FER_OBB_PRI_TRI){
            tri = (fer_obb_tri_t *)obb->pri;

            for (i = 0; i < 3; i++){
                for (j = 0; j < 3; j++){
                    val  = ferMat3Get(cov, i, j);
                    val += (ferVec3Get(tri->p0, i) - ferVec3Get(mean, i))
                            * (ferVec3Get(tri->p0, j) - ferVec3Get(mean, j));
                    val += (ferVec3Get(tri->p1, i) - ferVec3Get(mean, i))
                            * (ferVec3Get(tri->p1, j) - ferVec3Get(mean, j));
                    val += (ferVec3Get(tri->p2, i) - ferVec3Get(mean, i))
                            * (ferVec3Get(tri->p2, j) - ferVec3Get(mean, j));
                    ferMat3Set1(cov, i, j, val);
                }
            }

            *num += 1;
        }
    }else{
        FER_LIST_FOR_EACH(&obb->obbs, item){
            o = FER_LIST_ENTRY(item, fer_obb_t, list);
            __ferOBBMergeCov(o, mean, num, cov);
        }
    }
}

static void ferOBBMergeCov(fer_obb_t *obb, const fer_vec3_t *mean,
                           fer_mat3_t *cov)
{
    fer_list_t *item;
    fer_obb_t *o;
    fer_real_t num;

    ferMat3SetZero(cov);
    num = FER_ZERO;

    FER_LIST_FOR_EACH(&obb->obbs, item){
        o = FER_LIST_ENTRY(item, fer_obb_t, list);
        __ferOBBMergeCov(o, mean, &num, cov);
    }


    if (num > 0.)
        ferMat3Scale(cov, ferRecp(num * FER_REAL(3.)));
}

static void __ferOBBMergeMinMax(fer_obb_t *par, fer_obb_t *obb, fer_real_t *min, fer_real_t *max)
{
    fer_obb_tri_t *tri;
    fer_list_t *item;
    fer_obb_t *o;
    fer_real_t m;
    size_t i;

    if (ferListEmpty(&obb->obbs)){
        if (obb->pri->type == FER_OBB_PRI_TRI){
            tri = (fer_obb_tri_t *)obb->pri;

            for (i = 0; i < 3; i++){
                m = ferVec3Dot(tri->p0, par->axis + i);
                if (m < min[i]){
                    min[i] = m;
                }else if (m > max[i]){
                    max[i] = m;
                }

                m = ferVec3Dot(tri->p1, par->axis + i);
                if (m < min[i]){
                    min[i] = m;
                }else if (m > max[i]){
                    max[i] = m;
                }

                m = ferVec3Dot(tri->p2, par->axis + i);
                if (m < min[i]){
                    min[i] = m;
                }else if (m > max[i]){
                    max[i] = m;
                }
            }
        }
    }else{
        FER_LIST_FOR_EACH(&obb->obbs, item){
            o = FER_LIST_ENTRY(item, fer_obb_t, list);
            __ferOBBMergeMinMax(par, o, min, max);
        }
    }
}

static void ferOBBMergeMinMax(fer_obb_t *obb, fer_real_t *min, fer_real_t *max)
{
    fer_list_t *item;
    fer_obb_t *o;

    min[0] = min[1] = min[2] = FER_REAL_MAX;
    max[0] = max[1] = max[2] = FER_REAL_MIN;

    FER_LIST_FOR_EACH(&obb->obbs, item){
        o = FER_LIST_ENTRY(item, fer_obb_t, list);
        __ferOBBMergeMinMax(obb, o, min, max);
    }
}

static fer_obb_t *ferOBBMerge(fer_obb_t *obb1, fer_obb_t *obb2)
{
    fer_vec3_t mean, v;
    fer_mat3_t cov, eigen;
    fer_obb_t *obb;
    fer_real_t min[3], max[3];

    // create OBB
    obb = FER_ALLOC_ALIGN(fer_obb_t, 16);
    obb->pri = NULL;

    // initialize lists
    ferListInit(&obb->obbs);
    ferListInit(&obb->list);

    // add child OBBs into list
    ferListAppend(&obb->obbs, &obb1->list);
    ferListAppend(&obb->obbs, &obb2->list);

    // find out mean and covariance matrix
    ferOBBMergeMean(obb, &mean);
    ferOBBMergeCov(obb, &mean, &cov);

    // compute eigen vectors from covariance matrix
    ferMat3Eigen(&cov, &eigen, NULL);

    // pick up eigen vectors and normalize them
    ferMat3CopyCol(&obb->axis[0], &eigen, 0);
    ferMat3CopyCol(&obb->axis[1], &eigen, 1);
    ferMat3CopyCol(&obb->axis[2], &eigen, 2);
    ferVec3Normalize(&obb->axis[0]);
    ferVec3Normalize(&obb->axis[1]);
    ferVec3Normalize(&obb->axis[2]);

    // find out min and max
    ferOBBMergeMinMax(obb, min, max);

    // set center
    ferVec3Scale2(&obb->center, &obb->axis[0], (min[0] + max[0]) * FER_REAL(0.5));
    ferVec3Scale2(&v, &obb->axis[1], (min[1] + max[1]) * FER_REAL(0.5));
    ferVec3Add(&obb->center, &v);
    ferVec3Scale2(&v, &obb->axis[2], (min[2] + max[2]) * FER_REAL(0.5));
    ferVec3Add(&obb->center, &v);

    // compute extents
    ferVec3Set(&obb->half_extents, (max[0] - min[0]) * FER_REAL(0.5),
                                   (max[1] - min[1]) * FER_REAL(0.5),
                                   (max[2] - min[2]) * FER_REAL(0.5));

    return obb;
}


fer_obb_t *ferOBBNewTriMesh(const fer_vec3_t *pts,
                            const unsigned *ids, size_t len)
{
    fer_obb_t *root;
    fer_obb_trimesh_t *trimesh;
    fer_obb_t **obbs;
    size_t i, num_obbs;
    int obb1, obb2;
    fer_real_t dist, mindist;

    // create trimesh and root obb
    trimesh = ferOBBTriMeshNew(pts, ids, len);

    // create obb for each triangle
    obbs = FER_ALLOC_ARR(fer_obb_t *, trimesh->len);
    for (i = 0; i < trimesh->len; i++){
        obbs[i] = ferOBBNewTri(&trimesh->pts[trimesh->ids[i * 3]],
                               &trimesh->pts[trimesh->ids[i * 3 + 1]],
                               &trimesh->pts[trimesh->ids[i * 3 + 2]]);
    }

    // merge obbs
    num_obbs = trimesh->len;
    root = NULL;
    mindist = FER_REAL_MAX;
    obb1 = obb2 = -1;
    for (i = 0; num_obbs != 1; i = (i + 1) % trimesh->len){
        if (obbs[i] == NULL)
            continue;

        if (obb1 < 0){
            obb1    = i;
            mindist = FER_REAL_MAX;
        }else if (obb1 == i){
            // merge with nearest obb
            obbs[obb1] = ferOBBMerge(obbs[obb1], obbs[obb2]);
            obbs[obb2] = NULL;
            num_obbs--;

            if (num_obbs == 1)
                root = obbs[obb1];

            obb1 = obb2 = -1;
        }else{
            dist = ferVec3Dist2(&obbs[obb1]->center, &obbs[i]->center);
            if (dist < mindist){
                obb2    = i;
                mindist = dist;
            }
        }
    }

    free(obbs);

    root->pri = (fer_obb_pri_t *)trimesh;

    return root;
}


void ferOBBDel(fer_obb_t *obb)
{
    fer_list_t *item;

    // remove all children from list
    while (!ferListEmpty(&obb->obbs)){
        item = ferListNext(&obb->obbs);
        ferListDel(item);
    }

    // remove itself from parent's list
    ferListDel(&obb->list);

    if (obb->pri)
        free(obb->pri);

    free(obb);
}


int ferOBBDisjoint(const fer_obb_t *obb1,
                   const fer_mat3_t *_rot1, const fer_vec3_t *tr1,
                   const fer_obb_t *obb2,
                   const fer_mat3_t *_rot2, const fer_vec3_t *tr2)
{
    // See 4.4 of "Orange" book

    fer_vec3_t tr;
    fer_mat3_t rot, abs_rot, rot1, rot2;
    fer_real_t tl, ra, rb;
    int i;

    // first compute translation from obb1 to obb2 in global coordinates
    ferVec3Add2(&tr, &obb2->center, tr2);
    ferVec3Sub(&tr, &obb1->center);
    ferVec3Sub(&tr, tr1);

    // and transform it to a's coordinate frame
    ferVec3Set(&tr, ferVec3Dot(&tr, &obb1->axis[0]),
                    ferVec3Dot(&tr, &obb1->axis[1]),
                    ferVec3Dot(&tr, &obb1->axis[2]));


    // compute absolute rotation of obb1 and obb2
    ferMat3MulColVecs2(&rot1, _rot1,
                       &obb1->axis[0], &obb1->axis[1], &obb1->axis[2]);
    ferMat3MulColVecs2(&rot2, _rot2,
                       &obb2->axis[0], &obb2->axis[1], &obb2->axis[2]);

    // compute rotation matrix expressing b in a's coordinate frame
    ferMat3Trans(&rot1);
    ferMat3Mul2(&rot, &rot1, &rot2);
    /*
    {
        DBG("rot[0,]: %f %f %f", rot.f[0], rot.f[1], rot.f[2]);
        DBG("rot[1,]: %f %f %f", rot.f[4], rot.f[5], rot.f[6]);
        DBG("rot[2,]: %f %f %f", rot.f[8], rot.f[9], rot.f[10]);
    }
    */

    // precompute absolute values of rot
    ferMat3Abs2(&abs_rot, &rot);

    // L = obb1->axis[0, 1, 2]
    for (i = 0; i < 3; i++){
        tl = FER_FABS(ferVec3Get(&tr, i));
        ra = ferVec3Get(&obb1->half_extents, i);
        rb = ferMat3DotRow(&abs_rot, i, &obb2->half_extents);
        //DBG("tl, ra, rb %f %f %f", tl, ra, rb);

        if (tl > ra + rb)
            return i + 1;
    }

    // L = obb2->axis[0, 1, 2]
    for (i = 0; i < 3; i++){
        tl = FER_FABS(ferMat3DotCol(&rot, i, &tr));
        ra = ferMat3DotCol(&abs_rot, i, &obb1->half_extents);
        rb = ferVec3Get(&obb2->half_extents, i);

        if (tl > ra + rb)
            return i + 4;
    }

    // L = obb1->axis[0] x obb2->axis[0]
    tl  = ferVec3Get(&tr, 2) * ferMat3Get(&rot, 1, 0);
    tl -= ferVec3Get(&tr, 1) * ferMat3Get(&rot, 2, 0);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 1) * ferMat3Get(&abs_rot, 2, 0);
    ra += ferVec3Get(&obb1->half_extents, 2) * ferMat3Get(&abs_rot, 1, 0);
    rb  = ferVec3Get(&obb2->half_extents, 1) * ferMat3Get(&abs_rot, 0, 2);
    rb += ferVec3Get(&obb2->half_extents, 2) * ferMat3Get(&abs_rot, 0, 1);
    if (tl > ra + rb)
        return 7;

    // L = obb1->axis[0] x obb2->axis[1]
    tl  = ferVec3Get(&tr, 2) * ferMat3Get(&rot, 1, 1);
    tl -= ferVec3Get(&tr, 1) * ferMat3Get(&rot, 2, 1);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 1) * ferMat3Get(&abs_rot, 2, 1);
    ra += ferVec3Get(&obb1->half_extents, 2) * ferMat3Get(&abs_rot, 1, 1);
    rb  = ferVec3Get(&obb2->half_extents, 0) * ferMat3Get(&abs_rot, 0, 2);
    rb += ferVec3Get(&obb2->half_extents, 2) * ferMat3Get(&abs_rot, 0, 0);
    if (tl > ra + rb)
        return 8;

    // L = obb1->axis[0] x obb2->axis[2]
    tl  = ferVec3Get(&tr, 2) * ferMat3Get(&rot, 1, 2);
    tl -= ferVec3Get(&tr, 1) * ferMat3Get(&rot, 2, 2);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 1) * ferMat3Get(&abs_rot, 2, 2);
    ra += ferVec3Get(&obb1->half_extents, 2) * ferMat3Get(&abs_rot, 1, 2);
    rb  = ferVec3Get(&obb2->half_extents, 0) * ferMat3Get(&abs_rot, 0, 1);
    rb += ferVec3Get(&obb2->half_extents, 1) * ferMat3Get(&abs_rot, 0, 0);
    if (tl > ra + rb)
        return 9;


    // L = obb1->axis[1] x obb2->axis[0]
    tl  = ferVec3Get(&tr, 0) * ferMat3Get(&rot, 2, 0);
    tl -= ferVec3Get(&tr, 2) * ferMat3Get(&rot, 0, 0);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 0) * ferMat3Get(&abs_rot, 2, 0);
    ra += ferVec3Get(&obb1->half_extents, 2) * ferMat3Get(&abs_rot, 0, 0);
    rb  = ferVec3Get(&obb2->half_extents, 1) * ferMat3Get(&abs_rot, 1, 2);
    rb += ferVec3Get(&obb2->half_extents, 2) * ferMat3Get(&abs_rot, 1, 1);
    if (tl > ra + rb)
        return 10;

    // L = obb1->axis[1] x obb2->axis[1]
    tl  = ferVec3Get(&tr, 0) * ferMat3Get(&rot, 2, 1);
    tl -= ferVec3Get(&tr, 2) * ferMat3Get(&rot, 0, 1);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 0) * ferMat3Get(&abs_rot, 2, 1);
    ra += ferVec3Get(&obb1->half_extents, 2) * ferMat3Get(&abs_rot, 0, 1);
    rb  = ferVec3Get(&obb2->half_extents, 0) * ferMat3Get(&abs_rot, 1, 2);
    rb += ferVec3Get(&obb2->half_extents, 2) * ferMat3Get(&abs_rot, 1, 0);
    if (tl > ra + rb)
        return 11;

    // L = obb1->axis[1] x obb2->axis[2]
    tl  = ferVec3Get(&tr, 0) * ferMat3Get(&rot, 2, 2);
    tl -= ferVec3Get(&tr, 2) * ferMat3Get(&rot, 0, 2);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 0) * ferMat3Get(&abs_rot, 2, 2);
    ra += ferVec3Get(&obb1->half_extents, 2) * ferMat3Get(&abs_rot, 0, 2);
    rb  = ferVec3Get(&obb2->half_extents, 0) * ferMat3Get(&abs_rot, 1, 1);
    rb += ferVec3Get(&obb2->half_extents, 1) * ferMat3Get(&abs_rot, 1, 0);
    if (tl > ra + rb)
        return 12;


    // L = obb1->axis[2] x obb2->axis[0]
    tl  = ferVec3Get(&tr, 1) * ferMat3Get(&rot, 0, 0);
    tl -= ferVec3Get(&tr, 0) * ferMat3Get(&rot, 1, 0);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 0) * ferMat3Get(&abs_rot, 1, 0);
    ra += ferVec3Get(&obb1->half_extents, 1) * ferMat3Get(&abs_rot, 0, 0);
    rb  = ferVec3Get(&obb2->half_extents, 1) * ferMat3Get(&abs_rot, 2, 2);
    rb += ferVec3Get(&obb2->half_extents, 2) * ferMat3Get(&abs_rot, 2, 1);
    if (tl > ra + rb)
        return 13;

    // L = obb1->axis[2] x obb2->axis[1]
    tl  = ferVec3Get(&tr, 1) * ferMat3Get(&rot, 0, 1);
    tl -= ferVec3Get(&tr, 0) * ferMat3Get(&rot, 1, 1);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 0) * ferMat3Get(&abs_rot, 1, 1);
    ra += ferVec3Get(&obb1->half_extents, 1) * ferMat3Get(&abs_rot, 0, 1);
    rb  = ferVec3Get(&obb2->half_extents, 0) * ferMat3Get(&abs_rot, 2, 2);
    rb += ferVec3Get(&obb2->half_extents, 2) * ferMat3Get(&abs_rot, 2, 0);
    if (tl > ra + rb)
        return 14;

    // L = obb1->axis[2] x obb2->axis[2]
    tl  = ferVec3Get(&tr, 1) * ferMat3Get(&rot, 0, 2);
    tl -= ferVec3Get(&tr, 0) * ferMat3Get(&rot, 1, 2);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 0) * ferMat3Get(&abs_rot, 1, 2);
    ra += ferVec3Get(&obb1->half_extents, 1) * ferMat3Get(&abs_rot, 0, 2);
    rb  = ferVec3Get(&obb2->half_extents, 0) * ferMat3Get(&abs_rot, 2, 1);
    rb += ferVec3Get(&obb2->half_extents, 1) * ferMat3Get(&abs_rot, 2, 0);
    if (tl > ra + rb)
        return 15;

    return 0;
}



void ferOBBDumpSVT(const fer_obb_t *obb, FILE *out, const char *name)
{
    fer_vec3_t v, w;

    fprintf(out, "----\n");

    if (name){
        fprintf(out, "Name: %s\n", name);
    }

    fprintf(out, "Point color: 0.8 0 0\n");
    fprintf(out, "Points:\n");
    ferVec3Print(&obb->center, out);
    fprintf(out, "\n----\n");

    fprintf(out, "Points:\n");
    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], -ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], -ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], -ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], -ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], -ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], -ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], -ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], -ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], -ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], -ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], -ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], -ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    fprintf(out, "Edges:\n");
    fprintf(out, "0 1 0 2 0 4\n");
    fprintf(out, "1 5 1 3\n");
    fprintf(out, "2 3 2 6 3 7\n");
    fprintf(out, "4 6 4 5 5 7 6 7\n");


    fprintf(out, "----\n");
}

void ferOBBTriDumpSVT(const fer_obb_tri_t *tri, FILE *out, const char *name)
{
    fprintf(out, "----\n");

    if (name){
        fprintf(out, "Name: %s\n", name);
    }

    fprintf(out, "Points:\n");
    ferVec3Print(tri->p0, out);
    fprintf(out, "\n");
    ferVec3Print(tri->p1, out);
    fprintf(out, "\n");
    ferVec3Print(tri->p2, out);
    fprintf(out, "\n");

    fprintf(out, "Edges:\n");
    fprintf(out, "0 1 1 2 2 0\n");

    fprintf(out, "----\n");
}

