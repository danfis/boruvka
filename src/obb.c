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

#include <fermat/obb.h>
#include <fermat/alloc.h>


fer_obb_tri_t *ferOBBTriNew(const fer_vec3_t *p1, const fer_vec3_t *p2,
                            const fer_vec3_t *p3)
{
    fer_obb_tri_t *tri;

    tri = FER_ALLOC(fer_obb_tri_t);

    tri->pri.type = FER_OBB_PRI_TRI;

    tri->p = ferVec3ArrNew(3);
    ferVec3Copy(tri->p + 0, p1);
    ferVec3Copy(tri->p + 1, p2);
    ferVec3Copy(tri->p + 2, p3);

    return tri;
}

void ferOBBTriDel(fer_obb_tri_t *tri)
{
    free(tri->p);
    free(tri);
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

fer_obb_t *ferOBBNewPri(fer_obb_pri_t *pri)
{
    if (pri->type == FER_OBB_PRI_TRI)
        return ferOBBNewTri((fer_obb_tri_t *)pri);
    return NULL;
}

fer_obb_t *ferOBBNewTri(fer_obb_tri_t *tri)
{
    fer_obb_t *obb;
    fer_vec3_t e01, e02, e12; // triangle edges
    fer_vec3_t v;
    fer_real_t d01, d02, d12;
    fer_real_t min[3], max[3], m;
    int i;

    obb = FER_ALLOC_ALIGN(fer_obb_t, 16);

    // 1. compute triangle edges
    ferVec3Sub2(&e01, tri->p + 1, tri->p + 0);
    ferVec3Sub2(&e02, tri->p + 2, tri->p + 0);
    ferVec3Sub2(&e12, tri->p + 2, tri->p + 1);

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
        min[i] = max[i] = ferVec3Dot(tri->p + 0, &obb->axis[i]);

        m = ferVec3Dot(tri->p + 1, &obb->axis[i]);
        if (m < min[i]){
            min[i] = m;
        }else{
            max[i] = m;
        }

        m = ferVec3Dot(tri->p + 2, &obb->axis[i]);
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

    free(obb);
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
    ferVec3Print(tri->p, out);
    fprintf(out, "\n");
    ferVec3Print(tri->p + 1, out);
    fprintf(out, "\n");
    ferVec3Print(tri->p + 2, out);
    fprintf(out, "\n");

    fprintf(out, "Edges:\n");
    fprintf(out, "0 1 1 2 2 0\n");

    fprintf(out, "----\n");
}

