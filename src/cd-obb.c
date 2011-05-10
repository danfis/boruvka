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

/** Build OBB tree by top-down method */
static fer_cd_obb_t *mergeTopDown(fer_list_t *obbs, int flags);
/** Build OBB tree by bottom-up method */
static fer_cd_obb_t *mergeBottomUp(fer_list_t *obbs, int flags);
/** Finds two nearest OBBs in list and returns new OBB that contains those
 *  OBBs, no other properties of OBB is set. Returns NULL if no two nearest
 *  OBBs are available */
static fer_cd_obb_t *mergeChooseNearest(fer_list_t *obbs);
/** Fit givev {obb} to shapes in {obbs} */
static void mergeFit(fer_cd_obb_t *obb, fer_list_t *obbs, int flags);
/** Fit OBB to its content using covariance matrix */
static void mergeFitCovariance(fer_cd_obb_t *obb, fer_list_t *obbs);
/** Fit OBB using "rotation calipers" algorithm.
 *  See "Finding Minimal Enclosing Boxes" from Joseph O'Rourke.
 *  This algorithm is much slower than mergeFitCovariance() but should fit
 *  boxes more tightly. */
static void mergeFitCalipers(fer_cd_obb_t *obb, fer_list_t *obbs, int num_rot);
/** Finds out axis to fit OBB along single edge - it is assumed given
 *  convex hull has only */
static void mergeFitSingleEdgeAxis(fer_cd_obb_t *obb, fer_chull3_t *hull);
/** Use "Polyhedral Mass Properties method" for fitting */
static void mergeFitPolyhedralMass(fer_cd_obb_t *obb, fer_list_t *obbs);
/** Use "naive" approach */
static void mergeFitNaive(fer_cd_obb_t *obb, fer_list_t *obbs, int num_rot);

/** Sets center and half_extents according to axis[] */
static void mergeFitSetCenterExtents(fer_cd_obb_t *obb, fer_list_t *obbs);

/** Compute convex hull of whole obb subtree.
 *  Returns 1 if convex hull contains all objects from OBBs, i.e., also
 *  min/max values can be taken from convex hull. */
static int updateCHull(fer_cd_obb_t *obb, fer_chull3_t *hull);
/** Updates hull with points from hull2 */
//static void updateCHull2(fer_chull3_t *hull, fer_chull3_t *hull2);

static void findOBBMinMax(fer_cd_obb_t *obb, fer_list_t *obbs,
                          fer_real_t *min, fer_real_t *max);
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

    obb->nearest = NULL;

    obb->sphere_radius = -1;

    return obb;
}


fer_cd_obb_t *ferCDOBBNewShape(fer_cd_shape_t *shape, int flags)
{
    fer_cd_obb_t *obb;

    if (shape->cl->type == FER_CD_SHAPE_TRIMESH)
        return ferCDOBBNewTriMesh((fer_cd_trimesh_t *)shape, flags);

    obb = FER_ALLOC_ALIGN(fer_cd_obb_t, 16);

    obb->shape = shape;
    if (obb->shape->cl->fit_obb){
        obb->shape->cl->fit_obb(obb->shape, &obb->center,
                                            &obb->axis[0],
                                            &obb->axis[1],
                                            &obb->axis[2],
                                            &obb->half_extents,
                                            flags);
    }

    ferListInit(&obb->obbs);
    ferListInit(&obb->list);

    obb->nearest = NULL;

    obb->sphere_radius = ferVec3Len(&obb->half_extents);

    return obb;
}


fer_cd_obb_t *ferCDOBBNewTriMesh(fer_cd_trimesh_t *trimesh, int mergeflags)
{
    fer_list_t obbs, *item;
    fer_cd_obb_t *obb;
    fer_cd_trimesh_tri_t *tri;
    size_t i;

    // create obb for each triangle
    ferListInit(&obbs);
    for (i = 0; i < trimesh->len; i++){
        tri = ferCDTriMeshTriNew(&trimesh->pts[trimesh->ids[i * 3]],
                                 &trimesh->pts[trimesh->ids[i * 3 + 1]],
                                 &trimesh->pts[trimesh->ids[i * 3 + 2]]);
        obb = ferCDOBBNewShape((fer_cd_shape_t *)tri, mergeflags);
        ferListAppend(&obbs, &obb->list);
    }

    ferCDOBBMerge(&obbs, mergeflags);

    item = ferListNext(&obbs);
    obb = fer_container_of(item, fer_cd_obb_t, list);
    obb->shape = (fer_cd_shape_t *)trimesh;

    obb->nearest = NULL;

    obb->sphere_radius = ferVec3Len(&obb->half_extents);

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

    // delete shape
    if (obb->shape && obb->shape->cl->del)
        obb->shape->cl->del(obb->shape);

    free(obb);
}


_fer_inline int __ferCDOBBDisjoint(const fer_cd_obb_t *obb1,
                                   const fer_cd_obb_t *obb2,
                                   const fer_mat3_t *rot,
                                   const fer_vec3_t *tr)
{
    fer_mat3_t abs_rot;
    fer_real_t tl, ra, rb;
    int i;

    // precompute absolute values of rot
    ferMat3Abs2(&abs_rot, rot);

    // L = obb1->axis[0, 1, 2]
    for (i = 0; i < 3; i++){
        tl = FER_FABS(ferVec3Get(tr, i));
        ra = ferVec3Get(&obb1->half_extents, i);
        rb = ferMat3DotRow(&abs_rot, i, &obb2->half_extents);
        //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);

        if (tl > ra + rb)
            return i + 1;
    }

    // L = obb2->axis[0, 1, 2]
    for (i = 0; i < 3; i++){
        tl = FER_FABS(ferMat3DotCol(rot, i, tr));
        ra = ferMat3DotCol(&abs_rot, i, &obb1->half_extents);
        rb = ferVec3Get(&obb2->half_extents, i);
        //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);

        if (tl > ra + rb)
            return i + 4;
    }

    return 0;

    // L = obb1->axis[0] x obb2->axis[0]
    tl  = ferVec3Get(tr, 2) * ferMat3Get(rot, 1, 0);
    tl -= ferVec3Get(tr, 1) * ferMat3Get(rot, 2, 0);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 1) * ferMat3Get(&abs_rot, 2, 0);
    ra += ferVec3Get(&obb1->half_extents, 2) * ferMat3Get(&abs_rot, 1, 0);
    rb  = ferVec3Get(&obb2->half_extents, 1) * ferMat3Get(&abs_rot, 0, 2);
    rb += ferVec3Get(&obb2->half_extents, 2) * ferMat3Get(&abs_rot, 0, 1);
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
    if (tl > ra + rb)
        return 7;

    // L = obb1->axis[0] x obb2->axis[1]
    tl  = ferVec3Get(tr, 2) * ferMat3Get(rot, 1, 1);
    tl -= ferVec3Get(tr, 1) * ferMat3Get(rot, 2, 1);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 1) * ferMat3Get(&abs_rot, 2, 1);
    ra += ferVec3Get(&obb1->half_extents, 2) * ferMat3Get(&abs_rot, 1, 1);
    rb  = ferVec3Get(&obb2->half_extents, 0) * ferMat3Get(&abs_rot, 0, 2);
    rb += ferVec3Get(&obb2->half_extents, 2) * ferMat3Get(&abs_rot, 0, 0);
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
    if (tl > ra + rb)
        return 8;

    // L = obb1->axis[0] x obb2->axis[2]
    tl  = ferVec3Get(tr, 2) * ferMat3Get(rot, 1, 2);
    tl -= ferVec3Get(tr, 1) * ferMat3Get(rot, 2, 2);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 1) * ferMat3Get(&abs_rot, 2, 2);
    ra += ferVec3Get(&obb1->half_extents, 2) * ferMat3Get(&abs_rot, 1, 2);
    rb  = ferVec3Get(&obb2->half_extents, 0) * ferMat3Get(&abs_rot, 0, 1);
    rb += ferVec3Get(&obb2->half_extents, 1) * ferMat3Get(&abs_rot, 0, 0);
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
    if (tl > ra + rb)
        return 9;


    // L = obb1->axis[1] x obb2->axis[0]
    tl  = ferVec3Get(tr, 0) * ferMat3Get(rot, 2, 0);
    tl -= ferVec3Get(tr, 2) * ferMat3Get(rot, 0, 0);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 0) * ferMat3Get(&abs_rot, 2, 0);
    ra += ferVec3Get(&obb1->half_extents, 2) * ferMat3Get(&abs_rot, 0, 0);
    rb  = ferVec3Get(&obb2->half_extents, 1) * ferMat3Get(&abs_rot, 1, 2);
    rb += ferVec3Get(&obb2->half_extents, 2) * ferMat3Get(&abs_rot, 1, 1);
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
    if (tl > ra + rb)
        return 10;

    // L = obb1->axis[1] x obb2->axis[1]
    tl  = ferVec3Get(tr, 0) * ferMat3Get(rot, 2, 1);
    tl -= ferVec3Get(tr, 2) * ferMat3Get(rot, 0, 1);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 0) * ferMat3Get(&abs_rot, 2, 1);
    ra += ferVec3Get(&obb1->half_extents, 2) * ferMat3Get(&abs_rot, 0, 1);
    rb  = ferVec3Get(&obb2->half_extents, 0) * ferMat3Get(&abs_rot, 1, 2);
    rb += ferVec3Get(&obb2->half_extents, 2) * ferMat3Get(&abs_rot, 1, 0);
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
    if (tl > ra + rb)
        return 11;

    // L = obb1->axis[1] x obb2->axis[2]
    tl  = ferVec3Get(tr, 0) * ferMat3Get(rot, 2, 2);
    tl -= ferVec3Get(tr, 2) * ferMat3Get(rot, 0, 2);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 0) * ferMat3Get(&abs_rot, 2, 2);
    ra += ferVec3Get(&obb1->half_extents, 2) * ferMat3Get(&abs_rot, 0, 2);
    rb  = ferVec3Get(&obb2->half_extents, 0) * ferMat3Get(&abs_rot, 1, 1);
    rb += ferVec3Get(&obb2->half_extents, 1) * ferMat3Get(&abs_rot, 1, 0);
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
    if (tl > ra + rb)
        return 12;


    // L = obb1->axis[2] x obb2->axis[0]
    tl  = ferVec3Get(tr, 1) * ferMat3Get(rot, 0, 0);
    tl -= ferVec3Get(tr, 0) * ferMat3Get(rot, 1, 0);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 0) * ferMat3Get(&abs_rot, 1, 0);
    ra += ferVec3Get(&obb1->half_extents, 1) * ferMat3Get(&abs_rot, 0, 0);
    rb  = ferVec3Get(&obb2->half_extents, 1) * ferMat3Get(&abs_rot, 2, 2);
    rb += ferVec3Get(&obb2->half_extents, 2) * ferMat3Get(&abs_rot, 2, 1);
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
    if (tl > ra + rb)
        return 13;

    // L = obb1->axis[2] x obb2->axis[1]
    tl  = ferVec3Get(tr, 1) * ferMat3Get(rot, 0, 1);
    tl -= ferVec3Get(tr, 0) * ferMat3Get(rot, 1, 1);
    tl  = FER_FABS(tl);
    ra  = ferVec3Get(&obb1->half_extents, 0) * ferMat3Get(&abs_rot, 1, 1);
    ra += ferVec3Get(&obb1->half_extents, 1) * ferMat3Get(&abs_rot, 0, 1);
    rb  = ferVec3Get(&obb2->half_extents, 0) * ferMat3Get(&abs_rot, 2, 2);
    rb += ferVec3Get(&obb2->half_extents, 2) * ferMat3Get(&abs_rot, 2, 0);
    //DBG("tl > ra + rb, %f > %f + %f (%f)", tl, ra, rb, ra + rb);
    if (tl > ra + rb)
        return 14;

    // L = obb1->axis[2] x obb2->axis[2]
    tl  = ferVec3Get(tr, 1) * ferMat3Get(rot, 0, 2);
    tl -= ferVec3Get(tr, 0) * ferMat3Get(rot, 1, 2);
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

int ferCDOBBDisjointRel(const fer_cd_obb_t *obb1, const fer_cd_obb_t *obb2,
                        const fer_mat3_t *R, const fer_vec3_t *T)
{
    fer_mat3_t rot, A1t, A1tR;
    fer_vec3_t tr, trtmp;
    fer_real_t len, len2;

    // tr = A1^t (R . c2 - c1 + T)
    ferMat3MulVec(&trtmp, R, &obb2->center);
    ferVec3Sub(&trtmp, &obb1->center);
    ferVec3Add(&trtmp, T);

    if (obb1->sphere_radius < FER_ZERO)
        ((fer_cd_obb_t *)obb1)->sphere_radius = ferVec3Len(&obb1->half_extents);
    if (obb2->sphere_radius < FER_ZERO)
        ((fer_cd_obb_t *)obb2)->sphere_radius = ferVec3Len(&obb2->half_extents);

    // perform bounding sphere test - for early quit
    len   = ferVec3Len2(&trtmp);
    len2  = obb1->sphere_radius + obb2->sphere_radius;
    len2 *= len2;
    if (len > len2)
        return 100;


    // rot = A1^t . R . A2
    //    where A is matrix of columns corresponding to each axis
    ferMat3SetRows(&A1t, &obb1->axis[0], &obb1->axis[1], &obb1->axis[2]);
    ferMat3Mul2(&A1tR, &A1t, R);
    ferMat3MulColVecs2(&rot, &A1tR,
                       &obb2->axis[0], &obb2->axis[1], &obb2->axis[2]);


    // finish computing translation
    ferMat3MulVec(&tr, &A1t, &trtmp);

    /*

    DBG("rot: %f %f %f",
            ferMat3Get(&rot, 0, 0),
            ferMat3Get(&rot, 0, 1),
            ferMat3Get(&rot, 0, 2));
    DBG("     %f %f %f",
            ferMat3Get(&rot, 1, 0),
            ferMat3Get(&rot, 1, 1),
            ferMat3Get(&rot, 1, 2));
    DBG("     %f %f %f",
            ferMat3Get(&rot, 2, 0),
            ferMat3Get(&rot, 2, 1),
            ferMat3Get(&rot, 2, 2));
    DBG_VEC3(&tr, "tr: ");
    */

    return __ferCDOBBDisjoint(obb1, obb2, &rot, &tr);
}

_fer_inline int ferCDOBBDisjointRelOBB1(const fer_cd_obb_t *o1, const fer_cd_obb_t *o2,
                                        const fer_mat3_t *rot, const fer_vec3_t *tr,
                                        const fer_mat3_t *R, const fer_vec3_t *T)
{
    /*
      It is expected that T and R are:
        ferVec3Sub2(&T, &tr, &o1->center);
        ferMat3MulLeftRowVecs2(&R, &rot, &o1->axis[0],
                                         &o1->axis[1],
                                         &o1->axis[2]);
    */

    fer_vec3_t trtmp, T2;
    fer_mat3_t R2;
    fer_real_t len, len2;

    ferMat3MulVec(&trtmp, rot, &o2->center);
    ferVec3Add(&trtmp, T);

    if (o1->sphere_radius < FER_ZERO)
        ((fer_cd_obb_t *)o1)->sphere_radius = ferVec3Len(&o1->half_extents);
    if (o2->sphere_radius < FER_ZERO)
        ((fer_cd_obb_t *)o2)->sphere_radius = ferVec3Len(&o2->half_extents);

    // perform bounding sphere test - for early quit
    len   = ferVec3Len2(&trtmp);
    len2  = o1->sphere_radius + o2->sphere_radius;
    len2 *= len2;
    if (len > len2){
        return 100;
    }

    ferMat3MulColVecs2(&R2, R, &o2->axis[0], &o2->axis[1], &o2->axis[2]);

    ferVec3Set(&T2, ferVec3Dot(&o1->axis[0], &trtmp),
                    ferVec3Dot(&o1->axis[1], &trtmp),
                    ferVec3Dot(&o1->axis[2], &trtmp));

    return __ferCDOBBDisjoint(o1, o2, &R2, &T2);
}

_fer_inline int ferCDOBBDisjointRelOBB2(const fer_cd_obb_t *o1, const fer_cd_obb_t *o2,
                                        const fer_mat3_t *rot, const fer_vec3_t *tr,
                                        const fer_mat3_t *R, const fer_vec3_t *T)
{
    /*
      It is expected that T and R are:
        ferMat3MulVec(&T, rot, &o2->center);
        ferVec3Add(&T, tr);

        ferMat3MulColVecs2(&R, rot, &o2->axis[0], &o2->axis[1], &o2->axis[2]);

    */
    fer_mat3_t R2;
    fer_vec3_t T2, trtmp;
    fer_real_t len, len2;

    // tr = A1^t (R . c2 - c1 + T)
    ferVec3Sub2(&trtmp, T, &o1->center);

    if (o1->sphere_radius < FER_ZERO)
        ((fer_cd_obb_t *)o1)->sphere_radius = ferVec3Len(&o1->half_extents);
    if (o2->sphere_radius < FER_ZERO)
        ((fer_cd_obb_t *)o2)->sphere_radius = ferVec3Len(&o2->half_extents);

    // perform bounding sphere test - for early quit
    len   = ferVec3Len2(&trtmp);
    len2  = o1->sphere_radius + o2->sphere_radius;
    len2 *= len2;
    if (len > len2)
        return 100;


    // rot = A1^t . R . A2
    //    where A is matrix of columns corresponding to each axis
    ferMat3MulLeftRowVecs2(&R2, R, &o1->axis[0], &o1->axis[1], &o1->axis[2]);


    // finish computing translation
    ferVec3Set(&T2, ferVec3Dot(&o1->axis[0], &trtmp),
                    ferVec3Dot(&o1->axis[1], &trtmp),
                    ferVec3Dot(&o1->axis[2], &trtmp));

    return __ferCDOBBDisjoint(o1, o2, &R2, &T2);
}

int ferCDOBBDisjoint(const fer_cd_obb_t *obb1,
                     const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                     const fer_cd_obb_t *obb2,
                     const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
#if 0
    fer_mat3_t rot, Rt;
    fer_vec3_t tr, trtmp;

    ferMat3Trans2(&Rt, rot1);
    ferMat3Mul2(&rot, &Rt, rot2);
    ferVec3Sub2(&trtmp, tr2, tr1);
    ferMat3MulVec(&tr, &Rt, &trtmp);

    return ferCDOBBDisjointRel(obb1, obb2, &rot, &tr);
#endif

    // See 4.4 of "Orange" book

    fer_vec3_t tr, trtmp;;
    fer_mat3_t A1t, rot;

    // compute translation in obb1's frame
    ferMat3MulVec(&trtmp, rot2, &obb2->center);
    ferMat3MulVec(&tr, rot1, &obb1->center);
    ferVec3Add(&trtmp, tr2);
    ferVec3Sub(&trtmp, &tr);
    ferVec3Sub(&trtmp, tr1);

    if (obb1->sphere_radius < FER_ZERO)
        ((fer_cd_obb_t *)obb1)->sphere_radius = ferVec3Len(&obb1->half_extents);
    if (obb2->sphere_radius < FER_ZERO)
        ((fer_cd_obb_t *)obb2)->sphere_radius = ferVec3Len(&obb2->half_extents);

    // perform bounding sphere test - for early quit
    fer_real_t len = ferVec3Len2(&trtmp);
    if (len > FER_CUBE(obb1->sphere_radius + obb2->sphere_radius))
        return 100;

    // compute rotation in obb1's frame
    ferMat3MulColVecs2(&A1t, rot1,
                       &obb1->axis[0], &obb1->axis[1], &obb1->axis[2]);
    ferVec3Set(&tr, ferMat3DotCol(&A1t, 0, &trtmp),
                    ferMat3DotCol(&A1t, 1, &trtmp),
                    ferMat3DotCol(&A1t, 2, &trtmp));
    ferMat3MulColVecs2(&rot, rot2,
                       &obb2->axis[0], &obb2->axis[1], &obb2->axis[2]);

    // finish translation computation
    ferMat3MulLeftTrans(&rot, &A1t);

    return __ferCDOBBDisjoint(obb1, obb2, &rot, &tr);
}



_fer_inline void __addPair(const fer_cd_obb_t *o1, const fer_cd_obb_t *o2,
                           fer_list_t *list)
{
    fer_cd_obb_pair_t *p;

    p = FER_ALLOC(fer_cd_obb_pair_t);
    p->obb1 = (fer_cd_obb_t *)o1;
    p->obb2 = (fer_cd_obb_t *)o2;
    ferListPrepend(list, &p->list);
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
    fer_mat3_t rot, Rt, R;
    fer_vec3_t tr, trtmp, T;

    ferMat3Trans2(&Rt, rot1);
    ferMat3Mul2(&rot, &Rt, rot2);
    ferVec3Sub2(&trtmp, tr2, tr1);
    ferMat3MulVec(&tr, &Rt, &trtmp);

    ferListInit(&fifo);

    __addPair(obb1, obb2, &fifo);

    while (!ferListEmpty(&fifo)){
        item = ferListNext(&fifo);
        ferListDel(item);
        pair = FER_LIST_ENTRY(item, fer_cd_obb_pair_t, list);

        if (ferListEmpty(&pair->obb1->obbs)
                && ferListEmpty(&pair->obb2->obbs)){
            // we have both leaf OBBs - call callback
            if (cb(pair->obb1, pair->obb2, data) == -1){
                free(pair);
                break;
            }
        }else if (ferListEmpty(&pair->obb1->obbs)){
            // obb1 is leaf OBB, obb2 is not

            ferVec3Sub2(&T, &tr, &pair->obb1->center);
            ferMat3MulLeftRowVecs2(&R, &rot, &pair->obb1->axis[0],
                                             &pair->obb1->axis[1],
                                             &pair->obb1->axis[2]);

            FER_LIST_FOR_EACH(&pair->obb2->obbs, item2){
                o2 = fer_container_of(item2, fer_cd_obb_t, list);

                if (!ferCDOBBDisjointRelOBB1(pair->obb1, o2, &rot, &tr, &R, &T)){
                    __addPair(pair->obb1, o2, &fifo);
                }
            }
        }else if (ferListEmpty(&pair->obb2->obbs)){
            // obb2 is leaf OBB, obb1 is not

            ferMat3MulVec(&T, &rot, &pair->obb2->center);
            ferVec3Add(&T, &tr);

            ferMat3MulColVecs2(&R, &rot, &pair->obb2->axis[0],
                                         &pair->obb2->axis[1],
                                         &pair->obb2->axis[2]);

            FER_LIST_FOR_EACH(&pair->obb1->obbs, item1){
                o1 = fer_container_of(item1, fer_cd_obb_t, list);

                if (!ferCDOBBDisjointRelOBB2(o1, pair->obb2, &rot, &tr, &R, &T)){
                    __addPair(o1, pair->obb2, &fifo);
                }
            }
        }else{
            // obb1 nor obb2 are leaf CDOBBs
            FER_LIST_FOR_EACH(&pair->obb1->obbs, item1){
                o1 = fer_container_of(item1, fer_cd_obb_t, list);

                ferVec3Sub2(&T, &tr, &o1->center);
                ferMat3MulLeftRowVecs2(&R, &rot, &o1->axis[0], &o1->axis[1], &o1->axis[2]);

                FER_LIST_FOR_EACH(&pair->obb2->obbs, item2){
                    o2 = fer_container_of(item2, fer_cd_obb_t, list);

                    if (!ferCDOBBDisjointRelOBB1(o1, o2, &rot, &tr, &R, &T)){
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
    int method;
    fer_cd_obb_t *obb;

    method = flags & 0x1;

    if (method == FER_CD_TOP_DOWN){
        obb = mergeTopDown(obbs, flags);
    }else if (method == FER_CD_BOTTOM_UP){
        obb = mergeBottomUp(obbs, flags);
    }

    ferListInit(obbs);
    ferListAppend(obbs, &obb->list);
}


void ferCDOBBDumpSVT(const fer_cd_obb_t *obb,
                     FILE *out, const char *name,
                     const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_vec3_t v, w;

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

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

static void __ferCDOBBDumpTreeSVT(const fer_cd_obb_t *obb,
                                  FILE *out, const char *name,
                                  const fer_mat3_t *rot, const fer_vec3_t *tr,
                                  int level)
{
    fer_list_t *item;
    fer_cd_obb_t *o;
    char name2[300];

    if (name){
        sprintf(name2, "%s - %02d - OBB", name, level);
    }else{
        sprintf(name2, "%02d - OBB", level);
    }
    ferCDOBBDumpSVT(obb, out, name2, rot, tr);

    if (obb->shape){
        if (obb->shape->cl->dump_svt){
            if (name){
                sprintf(name2, "%s - %02d - shape", name, level);
            }else{
                sprintf(name2, "%02d - shape", level);
            }
            obb->shape->cl->dump_svt(obb->shape, out, name2, rot, tr);
        }
    }else{
        FER_LIST_FOR_EACH(&obb->obbs, item){
            o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            __ferCDOBBDumpTreeSVT(o, out, name, rot, tr, level + 1);
        }
    }
}

void ferCDOBBDumpTreeSVT(const fer_cd_obb_t *obb,
                         FILE *out, const char *name,
                         const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    __ferCDOBBDumpTreeSVT(obb, out, name, rot, tr, 0);
}






static void _mergeTopDownSplit(fer_cd_obb_t *obb, fer_list_t *obbs,
                               fer_list_t *obb_list1, fer_list_t *obb_list2)
{
    const fer_vec3_t *axis;
    fer_vec3_t center;
    fer_list_t *item;
    fer_cd_obb_t *o;
    fer_real_t avg, m;
    int num;

    ferListInit(obb_list1);
    ferListInit(obb_list2);

    if (ferVec3X(&obb->half_extents) > ferVec3Y(&obb->half_extents)){
        if (ferVec3X(&obb->half_extents) > ferVec3Z(&obb->half_extents)){
            axis = &obb->axis[0];
        }else{
            axis = &obb->axis[2];
        }
    }else{
        if (ferVec3Y(&obb->half_extents) > ferVec3Z(&obb->half_extents)){
            axis = &obb->axis[1];
        }else{
            axis = &obb->axis[2];
        }
    }

    avg = FER_ZERO;
    num = 0;
    FER_LIST_FOR_EACH(obbs, item){
        o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        if (o->shape){
            o->shape->cl->center(o->shape, NULL, NULL, &center);
            m = ferVec3Dot(axis, &center);
        }else{
            m = ferVec3Dot(axis, &o->center);
        }

        avg += m;
        num += 1;
    }

    avg = avg / (fer_real_t)num;
    //DBG("avg: %f", avg);

    while (!ferListEmpty(obbs)){
        item = ferListNext(obbs);
        ferListDel(item);
        o    = FER_LIST_ENTRY(item, fer_cd_obb_t, list);

        if (o->shape){
            o->shape->cl->center(o->shape, NULL, NULL, &center);
            m = ferVec3Dot(axis, &center);
        }else{
            m = ferVec3Dot(axis, &o->center);
        }

        if (m < avg){
            ferListAppend(obb_list1, &o->list);
        }else{
            ferListAppend(obb_list2, &o->list);
        }
    }
}

static fer_cd_obb_t *mergeTopDown(fer_list_t *obbs, int flags)
{
    fer_cd_obb_t *obb, *obb2;
    fer_list_t *item, obb_list[2];

    // list have only one OBB, so return it
    if (ferListNext(obbs) == ferListPrev(obbs)){
        item = ferListNext(obbs);
        obb  = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        return obb;
    }

    // create new OBB
    obb = ferCDOBBNew();
    //DBG("obb: %lx", (long)obb);

    // fit OBB to underlying OBBs
    mergeFit(obb, obbs, flags);
    //DBG_VEC3(&obb->axis[0], "axis[0]: ");
    //DBG_VEC3(&obb->axis[1], "axis[1]: ");
    //DBG_VEC3(&obb->axis[2], "axis[2]: ");

    _mergeTopDownSplit(obb, obbs, obb_list + 0, obb_list + 1);
    if (ferListEmpty(obb_list + 0) || ferListEmpty(obb_list + 1)){
        while (!ferListEmpty(obb_list + 0)){
            item = ferListNext(obb_list + 0);
            ferListDel(item);
            obb2 = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            ferListAppend(&obb->obbs, &obb2->list);
        }
        while (!ferListEmpty(obb_list + 1)){
            item = ferListNext(obb_list + 1);
            ferListDel(item);
            obb2 = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            ferListAppend(&obb->obbs, &obb2->list);
        }
    }else{
        if (ferListNext(obb_list + 0) == ferListPrev(obb_list + 0)){
            item = ferListNext(obb_list + 0);
            obb2 = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            ferListAppend(&obb->obbs, &obb2->list);
        }else{
            obb2 = mergeTopDown(&obb_list[0], flags);
            ferListAppend(&obb->obbs, &obb2->list);
        }

        if (ferListNext(obb_list + 1) == ferListPrev(obb_list + 1)){
            item = ferListNext(obb_list + 1);
            obb2 = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            ferListAppend(&obb->obbs, &obb2->list);
        }else{
            obb2 = mergeTopDown(&obb_list[1], flags);
            ferListAppend(&obb->obbs, &obb2->list);
        }
    }

    return obb;
}


static fer_cd_obb_t *mergeChooseNearest(fer_list_t *obbs)
{
    fer_cd_obb_t *newobb;
    fer_list_t *item1, *item2;
    fer_cd_obb_t *obb[2];
    fer_real_t dist, dist_nearest;

    // only one OBB in list (or list is empty)
    if (ferListNext(obbs) == ferListPrev(obbs))
        return NULL;

    item1 = ferListNext(obbs);
    obb[0] = FER_LIST_ENTRY(item1, fer_cd_obb_t, list);
    if (!obb[0]->nearest){
        FER_LIST_FOR_EACH(obbs, item1){
            obb[0] = FER_LIST_ENTRY(item1, fer_cd_obb_t, list);

            if (obb[0]->nearest)
                continue;

            dist_nearest = FER_REAL_MAX;
            for (item2 = ferListNext(item1);
                    item2 != obbs;
                    item2 = ferListNext(item2)){
                obb[1] = FER_LIST_ENTRY(item2, fer_cd_obb_t, list);

                if (obb[1]->nearest)
                    continue;

                dist = ferVec3Dist2(&obb[0]->center, &obb[1]->center);
                if (dist < dist_nearest){
                    if (obb[0]->nearest)
                        obb[0]->nearest->nearest = NULL;

                    dist_nearest = dist;
                    obb[0]->nearest = obb[1];
                    obb[1]->nearest = obb[0];
                }
            }
        }

        item1 = ferListNext(obbs);
        obb[0] = FER_LIST_ENTRY(item1, fer_cd_obb_t, list);
    }
    obb[1] = obb[0]->nearest;

    ferListDel(&obb[0]->list);
    ferListDel(&obb[1]->list);
    obb[0]->nearest = NULL;
    obb[1]->nearest = NULL;

    newobb = ferCDOBBNew();
    ferListAppend(&newobb->obbs, &obb[0]->list);
    ferListAppend(&newobb->obbs, &obb[1]->list);
    ferListAppend(obbs, &newobb->list);
    return newobb;
}

static fer_cd_obb_t *mergeBottomUp(fer_list_t *obbs, int flags)
{
    fer_cd_obb_t *obb;
    fer_list_t *item;

    while ((obb = mergeChooseNearest(obbs)) != NULL){
        mergeFit(obb, &obb->obbs, flags);
    }

    item = ferListNext(obbs);
    obb  = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
    return obb;
}


static void mergeFit(fer_cd_obb_t *obb, fer_list_t *obbs, int flags)
{
    int num_rot;

    num_rot = __FER_CD_NUM_ROT(flags);

    if (__FER_CD_FIT_COVARIANCE(flags)){
        mergeFitCovariance(obb, obbs);
    }else if (__FER_CD_FIT_CALIPERS(flags)){
        mergeFitCalipers(obb, obbs, num_rot);
    }else if (__FER_CD_FIT_POLYHEDRAL_MASS(flags)){
        mergeFitPolyhedralMass(obb, obbs);
    }else if (__FER_CD_FIT_NAIVE(flags)){
        mergeFitNaive(obb, obbs, num_rot);
    }else{
        fprintf(stderr, "CD Error: Unkown fitting method (flags: %x)\n", flags);
    }
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

static void mergeFitCovariance(fer_cd_obb_t *obb, fer_list_t *obbs)
{
    fer_chull3_t *chull;
    fer_list_t *item;
    fer_cd_obb_t *o;
    fer_vec3_t mean;
    fer_mat3_t cov, eigen;

    chull = ferCHull3New();

    FER_LIST_FOR_EACH(obbs, item){
        o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        updateCHull(o, chull);
    }

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

    mergeFitSetCenterExtents(obb, obbs);

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

static void mergeFitCalipers(fer_cd_obb_t *obb, fer_list_t *obbs, int num_rot)
{
    fer_list_t *item;
    fer_cd_obb_t *o;
    fer_chull3_t *chull;
    fer_real_t min[3], max[3];

    if (num_rot == 0)
        num_rot = 5;

    chull = ferCHull3New();

    FER_LIST_FOR_EACH(obbs, item){
        o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        updateCHull(o, chull);
    }

    if (ferMesh3VerticesLen(ferCHull3Mesh(chull)) == 2){
        mergeFitSingleEdgeAxis(obb, chull);
    }else{
        min[0] = min[1] = min[2] = max[0] = max[1] = max[2] = FER_ZERO;
        __mergeFitCalipersBestAxis(obb, chull, min, max, num_rot);
    }

    mergeFitSetCenterExtents(obb, obbs);

    ferCHull3Del(chull);
}

static void mergeFitSingleEdgeAxis(fer_cd_obb_t *obb, fer_chull3_t *hull)
{
    fer_list_t *list, *item;
    fer_mesh3_vertex_t *mv[2];

    if (ferMesh3VerticesLen(ferCHull3Mesh(hull)) <= 1){
        ferVec3Set(&obb->axis[0], FER_ONE,  FER_ZERO, FER_ZERO);
        ferVec3Set(&obb->axis[1], FER_ZERO, FER_ONE,  FER_ZERO);
        ferVec3Set(&obb->axis[2], FER_ZERO, FER_ZERO, FER_ONE);
        return;
    }

    list = ferMesh3Vertices(ferCHull3Mesh(hull));
    item = ferListNext(list);
    mv[0] = FER_LIST_ENTRY(item, fer_mesh3_vertex_t, list);
    item = ferListNext(item);
    mv[1] = FER_LIST_ENTRY(item, fer_mesh3_vertex_t, list);

    if (ferVec3Eq(mv[0]->v, mv[1]->v)){
        ferVec3Set(&obb->axis[0], FER_ONE,  FER_ZERO, FER_ZERO);
        ferVec3Set(&obb->axis[1], FER_ZERO, FER_ONE,  FER_ZERO);
        ferVec3Set(&obb->axis[2], FER_ZERO, FER_ZERO, FER_ONE);
        return;
    }

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



_fer_inline void __polyhedralMassSubexpression(fer_real_t w0,
                                               fer_real_t w1,
                                               fer_real_t w2,
                                               fer_real_t *f1,
                                               fer_real_t *f2,
                                               fer_real_t *f3,
                                               fer_real_t *g0,
                                               fer_real_t *g1,
                                               fer_real_t *g2)
{
    fer_real_t temp0, temp1, temp2;

    temp0 = w0 + w1;
    temp1 = w0 * w0;
    temp2 = temp1 + (w1 * temp0);
    *f1 = temp0 + w2;
    *f2 = temp2 + (w2 * *f1);
    *f3 = (w0 * temp1) + (w1 * temp2) + (w2 * *f2);
    *g0 = *f2 + (w0 * (*f1 + w0));
    *g1 = *f2 + (w1 * (*f1 + w1));
    *g2 = *f2 + (w2 * (*f1 + w2));
}

static void mergeFitPolyhedralMass(fer_cd_obb_t *obb, fer_list_t *obbs)
{

    static const fer_real_t mult[10] = {FER_ONE / FER_REAL(6.),
                                        FER_ONE / FER_REAL(24.),
                                        FER_ONE / FER_REAL(24.),
                                        FER_ONE / FER_REAL(24.),
                                        FER_ONE / FER_REAL(60.),
                                        FER_ONE / FER_REAL(60.),
                                        FER_ONE / FER_REAL(60.),
                                        FER_ONE / FER_REAL(120.),
                                        FER_ONE / FER_REAL(120.),
                                        FER_ONE / FER_REAL(120.) };
    fer_real_t intg[10]; // order: 1, x, y, z, x^2, y^2, z^2, xy, yz, zx
    fer_real_t mass;
    fer_chull3_t *chull;
    fer_list_t *list, *item;
    fer_mesh3_face_t *face;
    fer_mesh3_vertex_t *v[3];
    fer_cd_obb_t *o;
    fer_vec3_t v01, v02, cross;
    fer_real_t f1x, f2x, f3x, g0x, g1x, g2x;
    fer_real_t f1y, f2y, f3y, g0y, g1y, g2y;
    fer_real_t f1z, f2z, f3z, g0z, g1z, g2z;
    fer_real_t xx, yy, zz, xy, yz, xz;
    fer_vec3_t center;
    fer_mat3_t inertia, eigen;
    int i;

    for (i = 0; i < 10; i++)
        intg[i] = FER_ZERO;

    chull = ferCHull3New();
    FER_LIST_FOR_EACH(obbs, item){
        o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        updateCHull(o, chull);
    }

    list = ferMesh3Faces(ferCHull3Mesh(chull));
    FER_LIST_FOR_EACH(list, item){
        face = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);

        ferMesh3FaceVertices(face, v);

        // compute normal of triangle
        ferVec3Sub2(&v01, v[1]->v, v[0]->v);
        ferVec3Sub2(&v02, v[2]->v, v[0]->v);
        ferVec3Cross(&cross, &v01, &v02);

        // compute integral terms
        __polyhedralMassSubexpression(ferVec3X(v[0]->v), ferVec3X(v[1]->v),
                                      ferVec3X(v[2]->v),
                                      &f1x, &f2x, &f3x, &g0x, &g1x, &g2x);
        __polyhedralMassSubexpression(ferVec3Y(v[0]->v), ferVec3Y(v[1]->v),
                                      ferVec3Y(v[2]->v),
                                      &f1y, &f2y, &f3y, &g0y, &g1y, &g2y);
        __polyhedralMassSubexpression(ferVec3Z(v[0]->v), ferVec3Z(v[1]->v),
                                      ferVec3Z(v[2]->v),
                                      &f1z, &f2z, &f3z, &g0z, &g1z, &g2z);

        // update integrals
        intg[0] += ferVec3X(&cross) * f1x;
        intg[1] += ferVec3X(&cross) * f2x;
        intg[2] += ferVec3Y(&cross) * f2y;
        intg[3] += ferVec3Z(&cross) * f2z;
        intg[4] += ferVec3X(&cross) * f3x;
        intg[5] += ferVec3Y(&cross) * f3y;
        intg[6] += ferVec3Z(&cross) * f3z;
        intg[7] += ferVec3X(&cross) * ((ferVec3Y(v[0]->v) * g0x) +
                                       (ferVec3Y(v[1]->v) * g1x) +
                                       (ferVec3Y(v[2]->v) * g2x));
        intg[8] += ferVec3Y(&cross) * ((ferVec3Z(v[0]->v) * g0y) +
                                       (ferVec3Z(v[1]->v) * g1y) +
                                       (ferVec3Z(v[2]->v) * g2y));
        intg[9] += ferVec3Z(&cross) * ((ferVec3X(v[0]->v) * g0z) +
                                       (ferVec3X(v[1]->v) * g1z) +
                                       (ferVec3X(v[2]->v) * g2z));
    }

    for (i = 0; i < 10; i++)
        intg[i] *= mult[i];
    mass = intg[0];

    // center of mass
    ferVec3Set(&center, intg[1] * ferRecp(mass),
                        intg[2] * ferRecp(mass),
                        intg[3] * ferRecp(mass));

    // inertia tensor relative to center of mass
    xx = FER_CUBE(ferVec3Y(&center)) + FER_CUBE(ferVec3Z(&center));
    xx = xx * mass;
    xx = intg[5] + intg[6] - xx;
    yy = FER_CUBE(ferVec3Z(&center)) + FER_CUBE(ferVec3X(&center));
    yy = yy * mass;
    yy = intg[4] + intg[6] - yy;
    zz = FER_CUBE(ferVec3X(&center)) + FER_CUBE(ferVec3Y(&center));
    zz = zz * mass;
    zz = intg[4] + intg[5] - zz;

    xy = -(intg[7] - (mass * ferVec3X(&center) * ferVec3Y(&center)));
    yz = -(intg[8] - (mass * ferVec3Y(&center) * ferVec3Z(&center)));
    xz = -(intg[9] - (mass * ferVec3Z(&center) * ferVec3X(&center)));

    // set axis[0,1,2]
    ferMat3Set(&inertia, xx, xy, xz,
                         xy, yy, yz,
                         xz, yz, zz);

    // compute eigen vectors from inertia matrix
    ferMat3Eigen(&inertia, &eigen, NULL);

    // pick up eigen vectors and normalize them
    ferMat3CopyCol(&obb->axis[0], &eigen, 0);
    ferMat3CopyCol(&obb->axis[1], &eigen, 1);
    ferMat3CopyCol(&obb->axis[2], &eigen, 2);
    ferVec3Normalize(&obb->axis[0]);
    ferVec3Normalize(&obb->axis[1]);
    ferVec3Normalize(&obb->axis[2]);

    mergeFitSetCenterExtents(obb, obbs);

    ferCHull3Del(chull);
}


static void mergeFitNaive(fer_cd_obb_t *obb, fer_list_t *obbs, int num_rot)
{
    fer_real_t volume, best_volume;
    fer_vec3_t axis[3], best_axis[3];
    fer_mat3_t rot;
    fer_real_t angle_x, angle_y, angle_z, angle_step;
    fer_real_t min[3], max[3];
    int i, j, k;

    if (num_rot == 0)
        num_rot = 3;

    ferVec3Set(&obb->axis[0], FER_ONE,  FER_ZERO, FER_ZERO);
    ferVec3Set(&obb->axis[1], FER_ZERO, FER_ONE,  FER_ZERO);
    ferVec3Set(&obb->axis[2], FER_ZERO, FER_ZERO, FER_ONE);
    mergeFitSetCenterExtents(obb, obbs);
    //mergeFitCovariance(obb, obbs);

    ferVec3Copy(&axis[0], &obb->axis[0]);
    ferVec3Copy(&axis[1], &obb->axis[1]);
    ferVec3Copy(&axis[2], &obb->axis[2]);

    ferVec3Copy(&best_axis[0], &obb->axis[0]);
    ferVec3Copy(&best_axis[1], &obb->axis[1]);
    ferVec3Copy(&best_axis[2], &obb->axis[2]);
    best_volume  = ferVec3X(&obb->half_extents) * FER_REAL(2.);
    best_volume *= ferVec3Y(&obb->half_extents) * FER_REAL(2.);
    best_volume *= ferVec3Z(&obb->half_extents) * FER_REAL(2.);

    angle_step = M_PI_2 / num_rot;

    angle_x = -M_PI_2;
    for (i = 0; i < 2 * num_rot + 1; i++){
        angle_y = -M_PI_2;
        for (j = 0; j < 2 * num_rot + 1; j++){
            angle_z = -M_PI_2;
            for (k = 0; k < 2 * num_rot + 1; k++){
                ferMat3SetRot3D(&rot, angle_x, angle_y, angle_z);

                ferMat3MulVec(&obb->axis[0], &rot, &axis[0]);
                ferMat3MulVec(&obb->axis[1], &rot, &axis[1]);
                ferMat3MulVec(&obb->axis[2], &rot, &axis[2]);

                findOBBMinMax(obb, obbs, min, max);
                volume  = max[0] - min[0];
                volume *= max[1] - min[1];
                volume *= max[2] - min[2];
                if (volume < best_volume){
                    best_volume = volume;
                    ferVec3Copy(&best_axis[0], &obb->axis[0]);
                    ferVec3Copy(&best_axis[1], &obb->axis[1]);
                    ferVec3Copy(&best_axis[2], &obb->axis[2]);
                }

                angle_z += angle_step;
            }

            angle_y += angle_step;
        }

        angle_x += angle_step;
    }

    ferVec3Copy(&obb->axis[0], &best_axis[0]);
    ferVec3Copy(&obb->axis[1], &best_axis[1]);
    ferVec3Copy(&obb->axis[2], &best_axis[2]);

    mergeFitSetCenterExtents(obb, obbs);
}



static void mergeFitSetCenterExtents(fer_cd_obb_t *obb, fer_list_t *obbs)
{
    fer_vec3_t v;
    fer_real_t min[3], max[3];

    // find out min and max
    findOBBMinMax(obb, obbs, min, max);

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
    obb->sphere_radius = ferVec3Len(&obb->half_extents);
}

static int updateCHull(fer_cd_obb_t *obb, fer_chull3_t *hull)
{
    fer_list_t *item;
    fer_cd_obb_t *o;
    int ret = 1;

    if (obb->shape){
        if (obb->shape->cl->update_chull){
            if (!obb->shape->cl->update_chull(obb->shape, hull, NULL, NULL))
                ret = 0;
        }
    }else{
        FER_LIST_FOR_EACH(&obb->obbs, item){
            o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            if (!updateCHull(o, hull))
                ret = 0;
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
    size_t i;

    if (obb->shape){
        if (obb->shape->cl->update_minmax){
            for (i = 0; i < 3; i++){
                obb->shape->cl->update_minmax(obb->shape, frame->axis + i,
                                              NULL, NULL,
                                              min + i, max + i);
            }
        }
    }else{
        FER_LIST_FOR_EACH(&obb->obbs, item){
            o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            _findOBBMinMax(frame, o, min, max);
        }
    }
}
static void findOBBMinMax(fer_cd_obb_t *obb, fer_list_t *obbs,
                          fer_real_t *min, fer_real_t *max)
{
    fer_list_t *item;
    fer_cd_obb_t *o;

    min[0] = min[1] = min[2] = FER_REAL_MAX;
    max[0] = max[1] = max[2] = -FER_REAL_MAX;

    FER_LIST_FOR_EACH(obbs, item){
        o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        _findOBBMinMax(obb, o, min, max);
    }
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

        //DBG_VEC3(v->v, "v->v: ");
        for (i = 0; i < 3; i++){
            m = ferVec3Dot(v->v, axis + i);
            //DBG("    m: %f", m);
            if (m < min[i])
                min[i] = m;
            if (m > max[i])
                max[i] = m;
        }
    }
}
