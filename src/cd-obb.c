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
#include <fermat/chull3.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** Creates new bounding box from trimesh's triangle.  */
static fer_cd_obb_t *ferCDOBBNewTriMeshTri(const fer_vec3_t *p1,
                                           const fer_vec3_t *p2,
                                           const fer_vec3_t *p3);

/** Finds two nearest OBBs in list and returns new OBB that contains those
 *  OBBs, no other properties of OBB is set. Returns NULL if no two nearest
 *  OBBs are available */
static fer_cd_obb_t *mergeChooseNearest(fer_list_t *obbs);
/** Fit OBB to its content using covariance matrix */
static void mergeFitCovariance(fer_cd_obb_t *obb);
/** Fit OBB using "rotation calipers" algorithm.
 *  See "Finding Minimal Enclosing Boxes" from Joseph O'Rourke.
 *  This algorithm is much slower than mergeFitCovariance() but should fit
 *  boxes more tightly. */
static void mergeFitCalipers(fer_cd_obb_t *obb, int num_rot);
/** Finds out axis to fit OBB along single edge - it is assumed given
 *  convex hull has only */
static void mergeFitSingleEdgeAxis(fer_cd_obb_t *obb, fer_chull3_t *hull);

/** Compute convex hull of whole obb subtree.
 *  Returns 0 if convex hull contains all objects from OBBs, i.e., also
 *  min/max values can be taken from convex hull. */
static int updateCHull(fer_cd_obb_t *obb, fer_chull3_t *hull);
/** Updates hull with points from hull2 */
//static void updateCHull2(fer_chull3_t *hull, fer_chull3_t *hull2);

static void findOBBMinMax(fer_cd_obb_t *obb, fer_real_t *min, fer_real_t *max);
/** Finds min/max values to given axis */
static void findCHullMinMax(fer_chull3_t *hull, const fer_vec3_t *axis,
                            fer_real_t *min, fer_real_t *max);

_fer_inline void __addPair(const fer_cd_obb_t *o1, const fer_cd_obb_t *o2,
                           fer_list_t *list);



fer_cd_obb_t *ferCDOBBNew(void)
{
    fer_cd_obb_t *obb;

    obb = FER_ALLOC_ALIGN(fer_cd_obb_t, 16);

    ferVec3Set(&obb->center, FER_ZERO, FER_ZERO, FER_ZERO);
    ferVec3Set(&obb->axis[0],  FER_ZERO, FER_ZERO, FER_ZERO);
    ferVec3Set(&obb->axis[1],  FER_ZERO, FER_ZERO, FER_ZERO);
    ferVec3Set(&obb->axis[2],  FER_ZERO, FER_ZERO, FER_ZERO);
    ferVec3Set(&obb->half_extents,  FER_ZERO, FER_ZERO, FER_ZERO);

    obb->shape = NULL;

    ferListInit(&obb->obbs);
    ferListInit(&obb->list);

    return obb;
}

fer_cd_obb_t *ferCDOBBNewSphere(const fer_vec3_t *center, fer_real_t radius)
{
    fer_cd_obb_t *obb;

    obb = FER_ALLOC_ALIGN(fer_cd_obb_t, 16);
    ferVec3Copy(&obb->center, center);
    ferVec3Set(&obb->axis[0], FER_ONE,  FER_ZERO, FER_ZERO);
    ferVec3Set(&obb->axis[1], FER_ZERO, FER_ONE,  FER_ZERO);
    ferVec3Set(&obb->axis[2], FER_ZERO, FER_ZERO, FER_ONE);
    ferVec3Set(&obb->half_extents, radius, radius, radius);

    obb->shape = (fer_cd_shape_t *)ferCDSphereNew(center, radius);

    ferListInit(&obb->obbs);

    return obb;
}


static fer_cd_obb_t *ferCDOBBNewTriMeshTri(const fer_vec3_t *p1,
                                           const fer_vec3_t *p2,
                                           const fer_vec3_t *p3)
{
    fer_cd_trimesh_tri_t *tri;
    fer_cd_obb_t *obb;
    fer_vec3_t e01, e02, e12; // triangle edges
    fer_vec3_t v;
    fer_real_t d01, d02, d12;
    fer_real_t min[3], max[3], m;
    int i;

    tri = ferCDTriMeshTriNew(p1, p2, p3);
    obb = FER_ALLOC_ALIGN(fer_cd_obb_t, 16);

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


    obb->shape = (fer_cd_shape_t *)tri;

    ferListInit(&obb->obbs);
    ferListInit(&obb->list);

    return obb;
}

fer_cd_obb_t *ferCDOBBNewTriMesh(const fer_vec3_t *pts,
                                 const unsigned *ids, size_t len,
                                 int flags)
{
    fer_list_t obbs, *item;
    fer_cd_obb_t *obb;
    fer_cd_trimesh_t *trimesh;
    size_t i;

    // create trimesh and root obb
    trimesh = ferCDTriMeshNew(pts, ids, len);

    // create obb for each triangle
    ferListInit(&obbs);
    for (i = 0; i < trimesh->len; i++){
        obb = ferCDOBBNewTriMeshTri(&trimesh->pts[trimesh->ids[i * 3]],
                                    &trimesh->pts[trimesh->ids[i * 3 + 1]],
                                    &trimesh->pts[trimesh->ids[i * 3 + 2]]);
        ferListAppend(&obbs, &obb->list);
    }

    ferCDOBBMerge(&obbs, flags);

    item = ferListNext(&obbs);
    obb = fer_container_of(item, fer_cd_obb_t, list);
    obb->shape = (fer_cd_shape_t *)trimesh;
    return obb;
}


void ferCDOBBDel(fer_cd_obb_t *obb)
{
    fer_list_t *item;
    fer_cd_obb_t *o;

    // remove all children from list
    while (!ferListEmpty(&obb->obbs)){
        item = ferListNext(&obb->obbs);
        o    = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        ferListDel(item);

        ferCDOBBDel(o);
    }

    if (obb->shape){
        if (obb->shape->type == FER_CD_SHAPE_SPHERE){
            ferCDSphereDel((fer_cd_sphere_t *)obb->shape);

        }else if (obb->shape->type == FER_CD_SHAPE_BOX){
            // TODO

        }else if (obb->shape->type == FER_CD_SHAPE_CYL){
            // TODO

        }else if (obb->shape->type == FER_CD_SHAPE_CAP){
            // TODO

        }else if (obb->shape->type == FER_CD_SHAPE_TRIMESH_TRI){
            ferCDTriMeshTriDel((fer_cd_trimesh_tri_t *)obb->shape);

        }else if (obb->shape->type == FER_CD_SHAPE_TRIMESH){
            ferCDTriMeshDel((fer_cd_trimesh_t *)obb->shape);
        }
    }

    free(obb);
}


int ferCDOBBDisjoint(const fer_cd_obb_t *obb1,
                     const fer_mat3_t *_rot1, const fer_vec3_t *tr1,
                     const fer_cd_obb_t *obb2,
                     const fer_mat3_t *_rot2, const fer_vec3_t *tr2)
{
    // See 4.4 of "Orange" book

    fer_vec3_t tr, trtmp;;
    fer_mat3_t A1t, rot;
    fer_mat3_t abs_rot;
    fer_real_t tl, ra, rb;
    int i;

    // compute rotation in obb1's frame
    ferMat3MulColVecs2(&A1t, _rot1,
                       &obb1->axis[0], &obb1->axis[1], &obb1->axis[2]);
    ferMat3MulColVecs2(&rot, _rot2,
                       &obb2->axis[0], &obb2->axis[1], &obb2->axis[2]);
    ferMat3MulLeftTrans(&rot, &A1t);

    // compute translation in obb1's frame
    ferMat3MulVec(&trtmp, _rot2, &obb2->center);
    ferMat3MulVec(&tr, _rot1, &obb1->center);
    ferVec3Add(&trtmp, tr2);
    ferVec3Sub(&trtmp, &tr);
    ferVec3Sub(&trtmp, tr1);
    ferVec3Set(&tr, ferMat3DotCol(&A1t, 0, &trtmp),
                    ferMat3DotCol(&A1t, 1, &trtmp),
                    ferMat3DotCol(&A1t, 2, &trtmp));


    // precompute absolute values of rot
    ferMat3Abs2(&abs_rot, &rot);

    // L = obb1->axis[0, 1, 2]
    for (i = 0; i < 3; i++){
        tl = FER_FABS(ferVec3Get(&tr, i));
        ra = ferVec3Get(&obb1->half_extents, i);
        rb = ferMat3DotRow(&abs_rot, i, &obb2->half_extents);
        //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);

        if (tl > ra + rb)
            return i + 1;
    }

    // L = obb2->axis[0, 1, 2]
    for (i = 0; i < 3; i++){
        tl = FER_FABS(ferMat3DotCol(&rot, i, &tr));
        ra = ferMat3DotCol(&abs_rot, i, &obb1->half_extents);
        rb = ferVec3Get(&obb2->half_extents, i);
        //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);

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
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
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
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
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
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
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
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
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
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
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
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
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
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
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
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
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
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
    if (tl > ra + rb)
        return 15;

    //DBG2("0");
    return 0;
}



_fer_inline void __addPair(const fer_cd_obb_t *o1, const fer_cd_obb_t *o2,
                           fer_list_t *list)
{
    fer_cd_obb_pair_t *p;

    p = FER_ALLOC(fer_cd_obb_pair_t);
    p->obb1 = (fer_cd_obb_t *)o1;
    p->obb2 = (fer_cd_obb_t *)o2;
    ferListAppend(list, &p->list);
}


struct __overlap_pairs_t {
    fer_list_t *list;
    int len;
};

static int __ferCBOBBOverlapPairs(const fer_cd_obb_t *obb1,
                                  const fer_cd_obb_t *obb2,
                                  void *data)
{
    struct __overlap_pairs_t *p = (struct __overlap_pairs_t *)data;
    __addPair(obb1, obb2, p->list);
    p->len++;
    return 0;
}

int ferCDOBBOverlapPairs(const fer_cd_obb_t *obb1,
                         const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                         const fer_cd_obb_t *obb2,
                         const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                         fer_list_t *pairs)
{
    struct __overlap_pairs_t p;

    p.list = pairs;
    p.len = 0;
    ferCDOBBOverlapPairsCB(obb1, rot1, tr1, obb2, rot2, tr2,
                           __ferCBOBBOverlapPairs, (void *)&p);
    return p.len;
}

void ferCDOBBOverlapPairsCB(const fer_cd_obb_t *obb1,
                            const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                            const fer_cd_obb_t *obb2,
                            const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                            fer_cd_obb_overlap cb, void *data)
{
    fer_list_t fifo;
    fer_cd_obb_pair_t *pair;
    fer_list_t *item1, *item2, *item;
    fer_cd_obb_t *o1, *o2;

    ferListInit(&fifo);

    __addPair(obb1, obb2, &fifo);

    while (!ferListEmpty(&fifo)){
        item = ferListNext(&fifo);
        ferListDel(item);
        pair = FER_LIST_ENTRY(item, fer_cd_obb_pair_t, list);

        if (ferListEmpty(&pair->obb1->obbs)
                && ferListEmpty(&pair->obb2->obbs)){
            // we have both leaf OBBs - call callback
            if (!ferCDOBBDisjoint(pair->obb1, rot1, tr1,
                                  pair->obb2, rot2, tr2)){
                if (cb(pair->obb1, pair->obb2, data) == -1){
                    free(pair);
                    break;
                }
            }
        }else if (ferListEmpty(&pair->obb1->obbs)){
            // obb1 is leaf OBB, obb2 is not
            FER_LIST_FOR_EACH(&pair->obb2->obbs, item2){
                o2 = fer_container_of(item2, fer_cd_obb_t, list);

                if (!ferCDOBBDisjoint(pair->obb1, rot1, tr1, o2, rot2, tr2)){
                    __addPair(pair->obb1, o2, &fifo);
                }
            }
        }else if (ferListEmpty(&pair->obb2->obbs)){
            // obb2 is leaf OBB, obb1 is not
            FER_LIST_FOR_EACH(&pair->obb1->obbs, item1){
                o1 = fer_container_of(item1, fer_cd_obb_t, list);

                if (!ferCDOBBDisjoint(o1, rot1, tr1, pair->obb2, rot2, tr2)){
                    __addPair(o1, pair->obb2, &fifo);
                }
            }
        }else{
            // obb1 nor obb2 are leaf CDOBBs
            FER_LIST_FOR_EACH(&pair->obb1->obbs, item1){
                o1 = fer_container_of(item1, fer_cd_obb_t, list);

                FER_LIST_FOR_EACH(&pair->obb2->obbs, item2){
                    o2 = fer_container_of(item2, fer_cd_obb_t, list);

                    if (!ferCDOBBDisjoint(o1, rot1, tr1, o2, rot2, tr2)){
                        __addPair(o1, o2, &fifo);
                    }
                }
            }
        }

        free(pair);
    }

    // free fifo
    ferCDOBBFreePairs(&fifo);
}


void ferCDOBBFreePairs(fer_list_t *pairs)
{
    fer_list_t *item;
    fer_cd_obb_pair_t *pair;

    while (!ferListEmpty(pairs)){
        item = ferListNext(pairs);
        ferListDel(item);
        pair = FER_LIST_ENTRY(item, fer_cd_obb_pair_t, list);
        free(pair);
    }
}

void ferCDOBBMerge(fer_list_t *obbs, int flags)
{
    fer_cd_obb_t *obb;
    int fit, num_rot;

    fit = flags & 0x1;
    num_rot = (flags >> 8) & 0xFF;
    if (num_rot == 0)
        num_rot = 20;

    while ((obb = mergeChooseNearest(obbs)) != NULL){
        if (fit == FER_CDOBB_MERGE_FIT_COVARIANCE){
            mergeFitCovariance(obb);
        }else{ // FER_CDOBB_MERGE_FIT_CALIPERS
            mergeFitCalipers(obb, num_rot);
        }
    }
}


void ferCDOBBDumpSVT(const fer_cd_obb_t *obb, FILE *out, const char *name)
{
    fer_mat3_t rot;
    fer_vec3_t tr;

    ferMat3SetRot3D(&rot, 0., 0., 0.);
    ferVec3Set(&tr, 0., 0., 0.);
    ferCDOBBDumpSVT2(obb, &rot, &tr, out, name);
}

void ferCDOBBDumpSVT2(const fer_cd_obb_t *obb,
                      const fer_mat3_t *rot, const fer_vec3_t *tr,
                      FILE *out, const char *name)
{
    fer_vec3_t v, w;

    fprintf(out, "----\n");

    if (name){
        fprintf(out, "Name: %s\n", name);
    }

    //fprintf(out, "Point color: 0.8 0 0\n");
    //fprintf(out, "Points:\n");
    //ferVec3Print(&obb->center, out);
    //fprintf(out, "\n----\n");

    fprintf(out, "Edge color: 0.7 0.5 0.5\n");
    fprintf(out, "Points:\n");
    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferMat3MulVec(&w, rot, &v);
    ferVec3Add2(&v, &w, tr);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], -ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferMat3MulVec(&w, rot, &v);
    ferVec3Add2(&v, &w, tr);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], -ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferMat3MulVec(&w, rot, &v);
    ferVec3Add2(&v, &w, tr);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], -ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], -ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferMat3MulVec(&w, rot, &v);
    ferVec3Add2(&v, &w, tr);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], -ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferMat3MulVec(&w, rot, &v);
    ferVec3Add2(&v, &w, tr);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], -ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], -ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferMat3MulVec(&w, rot, &v);
    ferVec3Add2(&v, &w, tr);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], -ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], -ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferMat3MulVec(&w, rot, &v);
    ferVec3Add2(&v, &w, tr);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, &obb->center);
    ferVec3Scale2(&w, &obb->axis[0], -ferVec3X(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[1], -ferVec3Y(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &obb->axis[2], -ferVec3Z(&obb->half_extents));
    ferVec3Add(&v, &w);
    ferMat3MulVec(&w, rot, &v);
    ferVec3Add2(&v, &w, tr);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    fprintf(out, "Edges:\n");
    fprintf(out, "0 1 0 2 0 4\n");
    fprintf(out, "1 5 1 3\n");
    fprintf(out, "2 3 2 6 3 7\n");
    fprintf(out, "4 6 4 5 5 7 6 7\n");


    fprintf(out, "----\n");
}




static fer_cd_obb_t *mergeChooseNearest(fer_list_t *obbs)
{
    fer_cd_obb_t *newobb;
    fer_list_t *item1, *item2;
    fer_cd_obb_t *obb[2];
    fer_cd_obb_t *obb_nearest[2];
    fer_real_t dist, dist_nearest;

    // only one OBB in list (or list is empty)
    if (ferListNext(obbs) == ferListPrev(obbs))
        return NULL;

    obb_nearest[0] = obb_nearest[1] = NULL;
    dist_nearest = FER_REAL_MAX;

    FER_LIST_FOR_EACH(obbs, item1){
        obb[0] = FER_LIST_ENTRY(item1, fer_cd_obb_t, list);
        for (item2 = ferListNext(item1);
                item2 != obbs;
                item2 = ferListNext(item2)){
            obb[1] = FER_LIST_ENTRY(item2, fer_cd_obb_t, list);

            dist = ferVec3Dist2(&obb[0]->center, &obb[1]->center);
            if (dist < dist_nearest){
                dist_nearest = dist;
                obb_nearest[0] = obb[0];
                obb_nearest[1] = obb[1];
            }
        }
    }

    ferListDel(&obb_nearest[0]->list);
    ferListDel(&obb_nearest[1]->list);

    newobb = ferCDOBBNew();
    ferListAppend(&newobb->obbs, &obb_nearest[0]->list);
    ferListAppend(&newobb->obbs, &obb_nearest[1]->list);
    ferListAppend(obbs, &newobb->list);
    return newobb;
}


static void __mergeFitCovarianceMean(fer_chull3_t *hull, fer_vec3_t *mean)
{
    fer_list_t *list, *item;
    fer_mesh3_vertex_t *v;
    fer_real_t num;

    ferVec3Set(mean, FER_ZERO, FER_ZERO, FER_ZERO);
    num = 0;

    list = ferMesh3Vertices(ferCHull3Mesh(hull));
    FER_LIST_FOR_EACH(list, item){
        v = FER_LIST_ENTRY(item, fer_mesh3_vertex_t, list);
        ferVec3Add(mean, v->v);
        num += FER_ONE;
    }

    ferVec3Scale(mean, ferRecp(num));
}

static void __mergeFitCovarianceCov(fer_chull3_t *hull, const fer_vec3_t *mean,
                                     fer_mat3_t *cov)
{
    fer_list_t *list, *item;
    fer_mesh3_face_t *f;
    fer_mesh3_vertex_t *v[3];
    fer_real_t num, val;
    int i, j;

    ferMat3SetZero(cov);
    num = 0;

    list = ferMesh3Faces(ferCHull3Mesh(hull));
    FER_LIST_FOR_EACH(list, item){
        f = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
        ferMesh3FaceVertices(f, v);

        for (i = 0; i < 3; i++){
            for (j = 0; j < 3; j++){
                val  = ferMat3Get(cov, i, j);
                val += (ferVec3Get(v[0]->v, i) - ferVec3Get(mean, i))
                            * (ferVec3Get(v[0]->v, j) - ferVec3Get(mean, j));
                val += (ferVec3Get(v[1]->v, i) - ferVec3Get(mean, i))
                            * (ferVec3Get(v[1]->v, j) - ferVec3Get(mean, j));
                val += (ferVec3Get(v[2]->v, i) - ferVec3Get(mean, i))
                            * (ferVec3Get(v[2]->v, j) - ferVec3Get(mean, j));
                ferMat3Set1(cov, i, j, val);
            }
        }

        num += FER_ONE;
    }

    ferMat3Scale(cov, ferRecp(num * FER_REAL(3.)));
}

static void mergeFitCovariance(fer_cd_obb_t *obb)
{
    fer_chull3_t *chull;
    fer_vec3_t mean, v;
    fer_mat3_t cov, eigen;
    fer_real_t min[3], max[3];
    int obb_in_chull;

    chull = ferCHull3New();
    obb_in_chull = updateCHull(obb, chull);

    if (ferMesh3VerticesLen(ferCHull3Mesh(chull)) == 2){
        mergeFitSingleEdgeAxis(obb, chull);
    }else{
        // compute mean value
        __mergeFitCovarianceMean(chull, &mean);
        // compute covariance matrix
        __mergeFitCovarianceCov(chull, &mean, &cov);

        // compute eigen vectors from covariance matrix
        ferMat3Eigen(&cov, &eigen, NULL);

        // pick up eigen vectors and normalize them
        ferMat3CopyCol(&obb->axis[0], &eigen, 0);
        ferMat3CopyCol(&obb->axis[1], &eigen, 1);
        ferMat3CopyCol(&obb->axis[2], &eigen, 2);
        ferVec3Normalize(&obb->axis[0]);
        ferVec3Normalize(&obb->axis[1]);
        ferVec3Normalize(&obb->axis[2]);
    }

    // find out min and max
    if (obb_in_chull == 0){
        findCHullMinMax(chull, obb->axis, min, max);
    }else{
        findOBBMinMax(obb, min, max);
    }

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

    ferCHull3Del(chull);
}


static void __mergeFitCalipersBestAxis(fer_cd_obb_t *obb, fer_chull3_t *hull,
                                       fer_real_t *min_h, fer_real_t *max_h,
                                       int num_rot)
{
    fer_list_t *list, *item1, *item2;
    fer_mesh3_edge_t *e1, *e2;
    fer_mesh3_vertex_t *v1[2], *v2[2];
    fer_vec3_t axis[3];
    fer_vec3_t tmp;
    fer_real_t min[3], max[3];
    fer_real_t angle_step, area, best_area;
    fer_quat_t rot;
    int j, i;

    best_area = FER_REAL_MAX;

    list = ferMesh3Edges(ferCHull3Mesh(hull));
    FER_LIST_FOR_EACH(list, item1){
        for (item2 = ferListNext(item1);
                item2 != list;
                item2 = ferListNext(item2)){

            e1 = FER_LIST_ENTRY(item1, fer_mesh3_edge_t, list);
            e2 = FER_LIST_ENTRY(item2, fer_mesh3_edge_t, list);
            v1[0] = ferMesh3EdgeVertex(e1, 0);
            v1[1] = ferMesh3EdgeVertex(e1, 1);
            v2[0] = ferMesh3EdgeVertex(e2, 0);
            v2[1] = ferMesh3EdgeVertex(e2, 1);

            ferVec3Sub2(&axis[0], v1[0]->v, v1[1]->v);
            ferVec3Normalize(&axis[0]);

            ferVec3Sub2(&tmp, v2[0]->v, v2[1]->v);
            ferVec3Cross(&axis[2], &tmp, &axis[0]);
            ferVec3Normalize(&axis[2]);

            ferVec3Cross(&axis[1], &axis[0], &axis[2]);
            ferVec3Normalize(&axis[1]);

            angle_step = M_PI / num_rot;

            ferQuatSetAngleAxis(&rot, angle_step, &axis[2]);
            for (i = 0; i < num_rot; i++){
                ferVec3Cross(&axis[0], &axis[1], &axis[2]);
                ferVec3Normalize(&axis[0]);

                findCHullMinMax(hull, axis, min, max);
                area  = max[0] - min[0];
                area *= max[1] - min[1];
                area *= max[2] - min[2];

                //DBG("area: %f", area);

                if (area < best_area){
                    best_area = area;
                    for (j = 0; j < 3; j++){
                        ferVec3Copy(&obb->axis[j], &axis[j]);
                        min_h[j] = min[j];
                        max_h[j] = max[j];
                    }
                }

                ferQuatRotVec(&axis[1], &rot);
            }
        }
    }

    //DBG("best_area: %f", best_area);
}

static void mergeFitCalipers(fer_cd_obb_t *obb, int num_rot)
{
    fer_chull3_t *chull;
    fer_vec3_t v;
    fer_real_t min[3], max[3];
    int obb_in_chull;

    chull = ferCHull3New();
    obb_in_chull = updateCHull(obb, chull);

    if (ferMesh3VerticesLen(ferCHull3Mesh(chull)) == 2){
        mergeFitSingleEdgeAxis(obb, chull);

        if (obb_in_chull){
            findCHullMinMax(chull, obb->axis, min, max);
        }else{
            findOBBMinMax(obb, min, max);
        }
    }else{
        min[0] = min[1] = min[2] = max[0] = max[1] = max[2] = FER_ZERO;
        __mergeFitCalipersBestAxis(obb, chull, min, max, num_rot);
    }

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

    ferCHull3Del(chull);
}

static void mergeFitSingleEdgeAxis(fer_cd_obb_t *obb, fer_chull3_t *hull)
{
    fer_list_t *list, *item;
    fer_mesh3_vertex_t *mv[2];

    list = ferMesh3Vertices(ferCHull3Mesh(hull));
    item = ferListNext(list);
    mv[0] = FER_LIST_ENTRY(item, fer_mesh3_vertex_t, list);
    item = ferListNext(item);
    mv[1] = FER_LIST_ENTRY(item, fer_mesh3_vertex_t, list);

    ferVec3Sub2(&obb->axis[0], mv[0]->v, mv[1]->v);
    ferVec3Normalize(&obb->axis[0]);
    ferVec3Set(&obb->axis[2], ferVec3Z(&obb->axis[0]) * -FER_ONE,
                              ferVec3X(&obb->axis[0]),
                              ferVec3Y(&obb->axis[0]));
    ferVec3Normalize(&obb->axis[2]);
    ferVec3Cross(&obb->axis[1], &obb->axis[0], &obb->axis[2]);
    ferVec3Normalize(&obb->axis[1]);
    ferVec3Cross(&obb->axis[2], &obb->axis[0], &obb->axis[1]);
    ferVec3Normalize(&obb->axis[2]);
}


static int updateCHull(fer_cd_obb_t *obb, fer_chull3_t *hull)
{
    fer_list_t *item;
    fer_cd_obb_t *o;
    fer_cd_sphere_t *sphere;
    fer_cd_trimesh_tri_t *tri;
    int ret = 0;

    if (ferListEmpty(&obb->obbs)){
        if (obb->shape->type == FER_CD_SHAPE_SPHERE){
            sphere = (fer_cd_sphere_t *)obb->shape;

            ferCHull3Add(hull, sphere->center);
            ret = -1;

        }else if (obb->shape->type == FER_CD_SHAPE_BOX){
            // TODO

        }else if (obb->shape->type == FER_CD_SHAPE_CYL){
            ret = -1;
            // TODO

        }else if (obb->shape->type == FER_CD_SHAPE_CAP){
            ret = -1;
            // TODO

        }else if (obb->shape->type == FER_CD_SHAPE_TRIMESH_TRI){
            tri = (fer_cd_trimesh_tri_t *)obb->shape;

            ferCHull3Add(hull, tri->p0);
            ferCHull3Add(hull, tri->p1);
            ferCHull3Add(hull, tri->p2);
        }
    }else{
        FER_LIST_FOR_EACH(&obb->obbs, item){
            o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            if (updateCHull(o, hull) != 0)
                ret = -1;
        }
    }

    return ret;
}

/*
static void updateCHull2(fer_chull3_t *hull, fer_chull3_t *hull2)
{
    fer_list_t *list, *item;
    fer_mesh3_vertex_t *v;

    list = ferMesh3Vertices(ferCHull3Mesh(hull2));
    FER_LIST_FOR_EACH(list, item){
        v = FER_LIST_ENTRY(item, fer_mesh3_vertex_t, list);
        ferCHull3Add(hull, v->v);
    }

}
*/

static void _findOBBMinMax(fer_cd_obb_t *frame,
                           fer_cd_obb_t *obb, fer_real_t *min, fer_real_t *max)
{
    fer_list_t *item;
    fer_cd_obb_t *o;
    fer_cd_sphere_t *sphere;
    fer_cd_trimesh_tri_t *trimesh_tri;
    fer_cd_trimesh_t *trimesh;
    fer_real_t m;
    size_t i, j;

    if (obb->shape){
        if (obb->shape->type == FER_CD_SHAPE_SPHERE){
            sphere = (fer_cd_sphere_t *)obb->shape;

            for (i = 0; i < 3; i++){
                m = ferVec3Dot(sphere->center, frame->axis + i);

                m += sphere->radius;
                if (m < min[i])
                    min[i] = m;
                if (m > max[i])
                    max[i] = m;

                m -= FER_REAL(2.) * sphere->radius;
                if (m < min[i])
                    min[i] = m;
                if (m > max[i])
                    max[i] = m;
            }

        }else if (obb->shape->type == FER_CD_SHAPE_BOX){
            // TODO

        }else if (obb->shape->type == FER_CD_SHAPE_CYL){
            // TODO

        }else if (obb->shape->type == FER_CD_SHAPE_CAP){
            // TODO

        }else if (obb->shape->type == FER_CD_SHAPE_TRIMESH_TRI){
            trimesh_tri = (fer_cd_trimesh_tri_t *)obb->shape;

            for (i = 0; i < 3; i++){
                m = ferVec3Dot(trimesh_tri->p0, frame->axis + i);
                if (m < min[i])
                    min[i] = m;
                if (m > max[i])
                    max[i] = m;

                m = ferVec3Dot(trimesh_tri->p1, frame->axis + i);
                if (m < min[i])
                    min[i] = m;
                if (m > max[i])
                    max[i] = m;

                m = ferVec3Dot(trimesh_tri->p2, frame->axis + i);
                if (m < min[i])
                    min[i] = m;
                if (m > max[i])
                    max[i] = m;
            }

        }else if (obb->shape->type == FER_CD_SHAPE_TRIMESH){
            trimesh = (fer_cd_trimesh_t *)obb->shape;

            for (j = 0; j < trimesh->ptslen; j++){
                for (i = 0; i < 3; i++){
                    m = ferVec3Dot(trimesh->pts + j, frame->axis + i);
                    if (m < min[i])
                        min[i] = m;
                    if (m > max[i])
                        max[i] = m;
                }
            }
        }
    }else{
        FER_LIST_FOR_EACH(&obb->obbs, item){
            o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            _findOBBMinMax(frame, o, min, max);
        }
    }
}
static void findOBBMinMax(fer_cd_obb_t *obb, fer_real_t *min, fer_real_t *max)
{
    min[0] = min[1] = min[2] = FER_REAL_MAX;
    max[0] = max[1] = max[2] = -FER_REAL_MAX;

    _findOBBMinMax(obb, obb, min, max);
}

static void findCHullMinMax(fer_chull3_t *hull, const fer_vec3_t *axis,
                            fer_real_t *min, fer_real_t *max)
{
    fer_list_t *list, *item;
    fer_mesh3_vertex_t *v;
    fer_real_t m;
    int i;

    min[0] = min[1] = min[2] = FER_REAL_MAX;
    max[0] = max[1] = max[2] = -FER_REAL_MAX;

    list = ferMesh3Vertices(ferCHull3Mesh(hull));
    FER_LIST_FOR_EACH(list, item){
        v = FER_LIST_ENTRY(item, fer_mesh3_vertex_t, list);

        for (i = 0; i < 3; i++){
            m = ferVec3Dot(v->v, axis + i);
            if (m < min[i])
                min[i] = m;
            if (m > max[i])
                max[i] = m;
        }
    }
}
