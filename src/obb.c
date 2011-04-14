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
#include <fermat/chull3.h>
#include <fermat/quat.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** Compute convex hull of whole obb subtree */
static void ferOBBCHull(fer_obb_t *obb, fer_chull3_t *hull);
/** Finds min/max values to given axis */
static void ferOBBMergeBestMinMax(fer_chull3_t *hull, const fer_vec3_t *axis,
                                  fer_real_t *min, fer_real_t *max);
/** Finds out best fitting axis */
static void ferOBBMergeBest(fer_obb_t *obb, fer_chull3_t *hull,
                            fer_real_t *min_h, fer_real_t *max_h);
/** Merge two given OBBs */
static fer_obb_t *ferOBBMerge(fer_obb_t *obb1, fer_obb_t *obb2);


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
    fer_obb_t *o;

    // remove all children from list
    while (!ferListEmpty(&obb->obbs)){
        item = ferListNext(&obb->obbs);
        o    = FER_LIST_ENTRY(item, fer_obb_t, list);
        ferListDel(item);

        ferOBBDel(o);
    }

    // remove itself from parent's list
    ferListDel(&obb->list);

    if (obb->pri){
        if (obb->pri->type == FER_OBB_PRI_TRI){
            ferOBBTriDel((fer_obb_tri_t *)obb->pri);
        }else if (obb->pri->type == FER_OBB_PRI_TRIMESH){
            ferOBBTriMeshDel((fer_obb_trimesh_t *)obb->pri);
        }
    }

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
    fprintf(out, "Faces:\n");
    fprintf(out, "0 1 2\n");

    fprintf(out, "----\n");
}




static void ferOBBCHull(fer_obb_t *obb, fer_chull3_t *hull)
{
    fer_list_t *item;
    fer_obb_t *o;
    fer_obb_tri_t *tri;

    if (ferListEmpty(&obb->obbs)){
        if (obb->pri->type == FER_OBB_PRI_TRI){
            tri = (fer_obb_tri_t *)obb->pri;

            ferCHull3Add(hull, tri->p0);
            ferCHull3Add(hull, tri->p1);
            ferCHull3Add(hull, tri->p2);
        }
    }else{
        FER_LIST_FOR_EACH(&obb->obbs, item){
            o = FER_LIST_ENTRY(item, fer_obb_t, list);
            ferOBBCHull(o, hull);
        }
    }
}

static void ferOBBMergeBestMinMax(fer_chull3_t *hull, const fer_vec3_t *axis,
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

static void ferOBBMergeBest(fer_obb_t *obb, fer_chull3_t *hull,
                            fer_real_t *min_h, fer_real_t *max_h)
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

            angle_step = M_PI / FER_OBB_NUM_ROTATIONS;

            ferQuatSetAngleAxis(&rot, angle_step, &axis[2]);
            for (i = 0; i < FER_OBB_NUM_ROTATIONS; i++){
                ferVec3Cross(&axis[0], &axis[1], &axis[2]);
                ferVec3Normalize(&axis[0]);

                ferOBBMergeBestMinMax(hull, axis, min, max);
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

            /*
            int ax_i;
            for (ax_i = 0; ax_i < 3; ax_i++){
                ferQuatSetAngleAxis(&rot, angle_step, &axis[ax_i]);
                ferVec3Copy(&ax[ax_i], &axis[ax_i]);
                ferVec3Copy(&ax[(ax_i + 1) % 3], &axis[(ax_i + 1) % 3]);
                for (i = 0; i < 100; i++){
                    ferVec3Cross(&ax[(ax_i + 2) % 3], &ax[ax_i], &ax[(ax_i + 1) % 3]);
                    ferVec3Normalize(&ax[(ax_i + 2) % 3]);

                    ferOBBMergeBestMinMax(hull, ax, min, max);
                    area  = max[0] - min[0];
                    area *= max[1] - min[1];
                    area *= max[2] - min[2];

                    //DBG("area: %f", area);

                    if (area < best_area){
                        //DBG2("");
                        best_area = area;
                        for (j = 0; j < 3; j++){
                            ferVec3Copy(&obb->axis[j], &ax[j]);
                            min_h[j] = min[j];
                            max_h[j] = max[j];
                        }
                    }

                    ferQuatRotVec(&ax[(ax_i + 1) % 3], &rot);
                }
            }
            */
        }
    }

    //DBG("best_area: %f", best_area);
}

static fer_obb_t *ferOBBMerge(fer_obb_t *obb1, fer_obb_t *obb2)
{
    fer_vec3_t v;
    fer_obb_t *obb;
    fer_real_t min[3], max[3];
    fer_chull3_t *hull;

    // create OBB
    obb = FER_ALLOC_ALIGN(fer_obb_t, 16);
    obb->pri = NULL;

    // initialize lists
    ferListInit(&obb->obbs);
    ferListInit(&obb->list);

    // add child OBBs into list
    ferListAppend(&obb->obbs, &obb1->list);
    ferListAppend(&obb->obbs, &obb2->list);

    // compute convex hull
    hull = ferCHull3New();
    ferOBBCHull(obb, hull);

    // find out best axis and min/max values
    min[0] = min[1] = min[2] = max[0] = max[1] = max[2] = 0;
    ferOBBMergeBest(obb, hull, min, max);

    // delete convex hull
    ferCHull3Del(hull);

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


