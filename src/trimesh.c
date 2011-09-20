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

#include <fermat/trimesh.h>
#include <fermat/alloc.h>
#include <RAPID.H>

fer_trimesh_t *ferTriMeshNew(void)
{
    fer_trimesh_t *tm;

    tm = FER_ALLOC(fer_trimesh_t);
    ferVec3Set(&tm->pos, 0, 0, 0);
    ferQuatSet(&tm->rot, 0, 0, 0, 1);
    tm->data = NULL;

    return tm;
}

void ferTriMeshDel(fer_trimesh_t *tm)
{
    if (tm->data)
        delete (RAPID_model *)tm->data;

    FER_FREE(tm);
}

int ferTriMeshLoad(fer_trimesh_t *tm, const char *fn)
{
    float x, y, z;
    double *ps;
    int a, b, c;
    int i, num_pts;
    FILE *fin;
    RAPID_model *m;


    fin = fopen(fn, "r");
    if (!fin)
        return -1;

    m = (RAPID_model *)tm->data;
    if (m)
        delete m;
    tm->data = m = new RAPID_model;

    // first read number of points
    fscanf(fin, "%d", &num_pts);

    ps = FER_ALLOC_ARR(double, num_pts * 3);

    // read all points
    for (i = 0; i < num_pts; i++){
        fscanf(fin, "%f %f %f", &x, &y, &z);
        ps[3 * i + 0] = x;
        ps[3 * i + 1] = y;
        ps[3 * i + 2] = z;
    }

    // then read indices of triangles and fill RAPID's model
    m->BeginModel();
    while (fscanf(fin, "%d %d %d", &a, &b, &c) == 3){
        m->AddTri(&ps[3 * a], &ps[3 * b], &ps[3 * c], 0);
    }
    m->EndModel();

    FER_FREE(ps);

    fclose(fin);

    return 0;
}

int ferTriMeshCollide(fer_trimesh_t *tm1, fer_trimesh_t *tm2)
{
    fer_mat3_t r;
    double R1[3][3], R2[3][3], T1[3], T2[3];
    int result;

    T1[0] = ferVec3X(&tm1->pos);
    T1[1] = ferVec3Y(&tm1->pos);
    T1[2] = ferVec3Z(&tm1->pos);
    ferQuatToMat3(&tm1->rot, &r);
    R1[0][0] = ferMat3Get(&r, 0, 0);
    R1[1][0] = ferMat3Get(&r, 1, 0);
    R1[2][0] = ferMat3Get(&r, 2, 0);
    R1[0][1] = ferMat3Get(&r, 0, 1);
    R1[1][1] = ferMat3Get(&r, 1, 1);
    R1[2][1] = ferMat3Get(&r, 2, 1);
    R1[0][2] = ferMat3Get(&r, 0, 2);
    R1[1][2] = ferMat3Get(&r, 1, 2);
    R1[2][2] = ferMat3Get(&r, 2, 2);


    T2[0] = ferVec3X(&tm2->pos);
    T2[1] = ferVec3Y(&tm2->pos);
    T2[2] = ferVec3Z(&tm2->pos);
    ferQuatToMat3(&tm2->rot, &r);
    R2[0][0] = ferMat3Get(&r, 0, 0);
    R2[1][0] = ferMat3Get(&r, 1, 0);
    R2[2][0] = ferMat3Get(&r, 2, 0);
    R2[0][1] = ferMat3Get(&r, 0, 1);
    R2[1][1] = ferMat3Get(&r, 1, 1);
    R2[2][1] = ferMat3Get(&r, 2, 1);
    R2[0][2] = ferMat3Get(&r, 0, 2);
    R2[1][2] = ferMat3Get(&r, 1, 2);
    R2[2][2] = ferMat3Get(&r, 2, 2);

    result = RAPID_Collide(R1, T1, (RAPID_model *)tm1->data,
                           R2, T2, (RAPID_model *)tm2->data,
                           RAPID_ALL_CONTACTS);//RAPID_FIRST_CONTACT);
    if (result == RAPID_OK){
        if (RAPID_num_contacts == 0)
            return 0;
        return 1;
    }
    return -1;
}

struct tri
{
  int id;
  double p1[3], p2[3], p3[3];
};


int ferTriMeshDumpSVT(const fer_trimesh_t *tm, FILE *out, const char *name)
{
    RAPID_model *m;
    int len, i;
    fer_vec3_t v;

    if (!tm->data)
        return -1;

    m   = (RAPID_model *)tm->data;
    len = m->num_tris;

    fprintf(out, "----\n");
    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Points:\n");
    for (i = 0; i < len; i++){
        ferVec3Set(&v, m->tris[i].p1[0], m->tris[i].p1[1], m->tris[i].p1[2]);
        ferQuatRotVec(&v, &tm->rot);
        ferVec3Add(&v, &tm->pos);
        ferVec3Print(&v, out);
        fprintf(out, "\n");

        ferVec3Set(&v, m->tris[i].p2[0], m->tris[i].p2[1], m->tris[i].p2[2]);
        ferQuatRotVec(&v, &tm->rot);
        ferVec3Add(&v, &tm->pos);
        ferVec3Print(&v, out);
        fprintf(out, "\n");

        ferVec3Set(&v, m->tris[i].p3[0], m->tris[i].p3[1], m->tris[i].p3[2]);
        ferQuatRotVec(&v, &tm->rot);
        ferVec3Add(&v, &tm->pos);
        ferVec3Print(&v, out);
        fprintf(out, "\n");
    }

    fprintf(out, "Faces:\n");
    for (i = 0; i < len; i++){
        fprintf(out, "%d %d %d\n", 3 * i, 3 * i + 1, 3 * i + 2);
    }

    fprintf(out, "----\n");

    return 0;
}
