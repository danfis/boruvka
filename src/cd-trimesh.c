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

fer_cd_trimesh_tri_t *ferCDTriMeshTriNew(const fer_vec3_t *p1,
                                 const fer_vec3_t *p2,
                                 const fer_vec3_t *p3)
{
    fer_cd_trimesh_tri_t *tri;

    tri = FER_ALLOC(fer_cd_trimesh_tri_t);

    tri->shape.type = FER_CD_SHAPE_TRIMESH_TRI;
    tri->p0 = p1;
    tri->p1 = p2;
    tri->p2 = p3;

    return tri;
}

void ferCDTriMeshTriDel(fer_cd_trimesh_tri_t *tri)
{
    free(tri);
}


int ferCDTriMeshTriCollide(const fer_cd_trimesh_tri_t *tri1,
                           const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                           const fer_cd_trimesh_tri_t *tri2,
                           const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    fer_vec3_t p1, q1, r1, p2, q2, r2;

    ferMat3MulVec(&p1, rot1, tri1->p0);
    ferVec3Add(&p1, tr1);
    ferMat3MulVec(&q1, rot1, tri1->p1);
    ferVec3Add(&q1, tr1);
    ferMat3MulVec(&r1, rot1, tri1->p2);
    ferVec3Add(&r1, tr1);

    ferMat3MulVec(&p2, rot2, tri2->p0);
    ferVec3Add(&p2, tr2);
    ferMat3MulVec(&q2, rot2, tri2->p1);
    ferVec3Add(&q2, tr2);
    ferMat3MulVec(&r2, rot2, tri2->p2);
    ferVec3Add(&r2, tr2);

    return ferVec3TriTriOverlap(&p1, &q1, &r1, &p2, &q2, &r2);
}




fer_cd_trimesh_t *ferCDTriMeshNew(const fer_vec3_t *pts,
                                    const unsigned int *ids, size_t len)
{
    fer_cd_trimesh_t *t;
    size_t i, ptslen;

    t = FER_ALLOC(fer_cd_trimesh_t);

    t->shape.type = FER_CD_SHAPE_TRIMESH;

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

void ferCDTriMeshDel(fer_cd_trimesh_t *t)
{
    free(t->pts);
    free(t->ids);
    free(t);
}



void ferCDTriMeshTriDumpSVT(const fer_cd_trimesh_tri_t *tri,
                            FILE *out, const char *name)
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

void ferCDTriMeshDumpSVT(const fer_cd_trimesh_t *t, FILE *out,
                         const char *name, int edges)
{
    size_t i, numpts;

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
        ferVec3Print(&t->pts[i], out);
        fprintf(out, "\n");
    }

    if (edges){
        fprintf(out, "Edges:\n");
        for (i = 0; i < t->len; i++){
            fprintf(out, "%u %u\n", t->ids[3 * i], t->ids[3 * i + 1]);
            fprintf(out, "%u %u\n", t->ids[3 * i + 1], t->ids[3 * i + 2]);
            fprintf(out, "%u %u\n", t->ids[3 * i], t->ids[3 * i + 2]);
        }
    }

    fprintf(out, "Faces:\n");
    for (i = 0; i < t->len; i++){
        fprintf(out, "%u %u %u\n", t->ids[3 * i], t->ids[3 * i + 1], t->ids[3 * i + 2]);
    }

    fprintf(out, "----\n");
}

void ferCDTriMeshDumpSVT2(const fer_cd_trimesh_t *t,
                          const fer_mat3_t *rot, const fer_vec3_t *tr,
                          FILE *out, const char *name, int edges)
{
    size_t i, numpts;
    fer_vec3_t v;

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

    if (edges){
        fprintf(out, "Edges:\n");
        for (i = 0; i < t->len; i++){
            fprintf(out, "%u %u\n", t->ids[3 * i], t->ids[3 * i + 1]);
            fprintf(out, "%u %u\n", t->ids[3 * i + 1], t->ids[3 * i + 2]);
            fprintf(out, "%u %u\n", t->ids[3 * i], t->ids[3 * i + 2]);
        }
    }

    fprintf(out, "Faces:\n");
    for (i = 0; i < t->len; i++){
        fprintf(out, "%u %u %u\n", t->ids[3 * i], t->ids[3 * i + 1], t->ids[3 * i + 2]);
    }

    fprintf(out, "----\n");
}
