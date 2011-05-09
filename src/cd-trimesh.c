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
    .type          = FER_CD_SHAPE_TRIMESH_TRI,
    .del           = (fer_cd_shape_del_fn)ferCDTriMeshTriDel,
    .support       = (fer_cd_shape_support_fn)ferCDTriMeshTriSupport,
    .center        = (fer_cd_shape_center_fn)ferCDTriMeshTriCenter,
    .fit_obb       = (fer_cd_shape_fit_obb_fn)ferCDTriMeshTriFitOBB,
    .update_chull  = (fer_cd_shape_update_chull_fn)ferCDTriMeshTriUpdateCHull,
    .update_minmax = (fer_cd_shape_update_minmax_fn)ferCDTriMeshTriUpdateMinMax,
    .dump_svt      = (fer_cd_shape_dump_svt_fn)ferCDTriMeshTriDumpSVT
};

static fer_cd_shape_class_t shape = {
    .type          = FER_CD_SHAPE_TRIMESH,
    .del           = (fer_cd_shape_del_fn)ferCDTriMeshDel,
    .support       = (fer_cd_shape_support_fn)ferCDTriMeshSupport,
    .center        = (fer_cd_shape_center_fn)ferCDTriMeshCenter,
    .fit_obb       = (fer_cd_shape_fit_obb_fn)ferCDTriMeshFitOBB,
    .update_chull  = (fer_cd_shape_update_chull_fn)ferCDTriMeshUpdateCHull,
    .update_minmax = (fer_cd_shape_update_minmax_fn)ferCDTriMeshUpdateMinMax,
    .dump_svt      = (fer_cd_shape_dump_svt_fn)ferCDTriMeshDumpSVT
};

fer_cd_trimesh_tri_t *ferCDTriMeshTriNew(const fer_vec3_t *p1,
                                         const fer_vec3_t *p2,
                                         const fer_vec3_t *p3)
{
    fer_cd_trimesh_tri_t *tri;

    tri = FER_ALLOC(fer_cd_trimesh_tri_t);

    tri->shape.cl = &shape_tri;
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

    printf("----\nPoints:\n");
    ferVec3Print(&p1, stdout); printf("\n");
    ferVec3Print(&q1, stdout); printf("\n");
    ferVec3Print(&r1, stdout); printf("\n");
    ferVec3Print(&p2, stdout); printf("\n");
    ferVec3Print(&q2, stdout); printf("\n");
    ferVec3Print(&r2, stdout); printf("\n");
    printf("Faces:\n0 1 2\n3 4 5\n--\n");

    return ferVec3TriTriOverlap(&p1, &q1, &r1, &p2, &q2, &r2);
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

static void ferCDTriMeshFromRawAdd(fer_cd_trimesh_t *t,
                                   size_t *ptsalloced,
                                   size_t *idsalloced,
                                   fer_real_t *r)
{
    if (!t->pts){
        *ptsalloced = 3 * 100;
        t->pts = ferVec3ArrNew(*ptsalloced);
    }
    if (!t->ids){
        *idsalloced = 3 * 100;
        t->ids = FER_ALLOC_ARR(unsigned int, *idsalloced);
    }

    if (t->ptslen + 3 > *ptsalloced){
        *ptsalloced *= 2;
        t->pts = FER_REALLOC_ARR(t->pts, fer_vec3_t, *ptsalloced);
    }
    if (3 * (t->len + 1) > *idsalloced){
        *idsalloced *= 2;
        t->ids = FER_REALLOC_ARR(t->ids, unsigned int, *idsalloced);
    }

    ferVec3Set(t->pts + t->ptslen, r[0], r[1], r[2]);
    ferVec3Set(t->pts + t->ptslen + 1, r[3], r[4], r[5]);
    ferVec3Set(t->pts + t->ptslen + 2, r[6], r[7], r[8]);

    t->ids[3 * t->len] = t->ptslen;
    t->ids[3 * t->len + 1] = t->ptslen + 1;
    t->ids[3 * t->len + 2] = t->ptslen + 2;

    t->ptslen += 3;
    t->len++;
}

fer_cd_trimesh_t *ferCDTriMeshFromRaw(const char *filename)
{
    int fd;
    size_t size;
    struct stat st;
    void *file;
    char *fstr, *fend, *fnext;
    fer_real_t r[9];
    int ri;
    size_t ptsalloced, idsalloced;
    fer_cd_trimesh_t *t;

    // open file
    if ((fd = open(filename, O_RDONLY)) == -1){
        fprintf(stderr, "CD Error: Can't open file `%s'\n", filename);
        return NULL;
    }

    // get stats (mainly size of file)
    if (fstat(fd, &st) == -1){
        close(fd);
        fprintf(stderr, "CD Error: Can't get file info of `%s'\n", filename);
        return NULL;
    }

    // pick up size of file
    size = st.st_size;

    // mmap whole file into memory, we need only read from it and don't need
    // to share anything
    file = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file == MAP_FAILED){
        close(fd);
        fprintf(stderr, "CD Error: Can't map file `%s' into memory: %s\n", filename, strerror(errno));

        // Fall to stdio method if it's not possible to map whole file into
        // memory TODO
        return NULL;
    }

    t = FER_ALLOC(fer_cd_trimesh_t);
    t->shape.cl = &shape;
    t->pts = NULL;
    t->ptslen = 0;
    ptsalloced = 0;
    t->ids = NULL;
    t->len = 0;
    idsalloced = 0;

    fstr = (char *)file;
    fend = (char *)file + size;
    while (1){
        for (ri = 0; ri < 9; ri++){
            if (ferParseReal(fstr, fend, r + ri, &fnext) != 0)
                break;
            fstr = fnext;
        }
        if (ri != 9)
            break;

        ferCDTriMeshFromRawAdd(t, &ptsalloced, &idsalloced, r);
    }

    // unmap mapped memory
    munmap(file, size);

    // close file
    close(fd);

    return t;
}

void ferCDTriMeshDel(fer_cd_trimesh_t *t)
{
    free(t->pts);
    free(t->ids);
    free(t);
}

void ferCDTriMeshTriSupport(const fer_cd_trimesh_tri_t *t,
                            const fer_vec3_t *dir,
                            fer_vec3_t *p)
{
    fer_real_t d1, d2, d3;

    d1 = ferVec3Dot(t->p0, dir);
    d2 = ferVec3Dot(t->p1, dir);
    d3 = ferVec3Dot(t->p2, dir);

    if (d1 > d2){
        if (d1 > d3){
            ferVec3Copy(p, t->p0);
        }else{ // d1 <= d3
            ferVec3Copy(p, t->p2);
        }
    }else{ // d1 <= d2
        if (d2 > d3){
            ferVec3Copy(p, t->p1);
        }else{ // d2 <= d3
            ferVec3Copy(p, t->p2);
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

void ferCDTriMeshTriCenter(const fer_cd_trimesh_tri_t *t,
                           const fer_mat3_t *rot,
                           const fer_vec3_t *tr,
                           fer_vec3_t *center)
{
    fer_vec3_t v;

    ferVec3Add2(center, t->p0, t->p1);
    ferVec3Add(center, t->p2);
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

void ferCDTriMeshTriFitOBB(const fer_cd_trimesh_tri_t *tri,
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
        min[i] = max[i] = ferVec3Dot(tri->p0, axis[i]);

        m = ferVec3Dot(tri->p1, axis[i]);
        if (m < min[i]){
            min[i] = m;
        }else{
            max[i] = m;
        }

        m = ferVec3Dot(tri->p2, axis[i]);
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

int ferCDTriMeshTriUpdateCHull(const fer_cd_trimesh_tri_t *tri, fer_chull3_t *chull,
                               const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_vec3_t v;

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

    ferMat3MulVec(&v, rot, tri->p0);
    ferVec3Add(&v, tr);
    ferCHull3Add(chull, &v);

    ferMat3MulVec(&v, rot, tri->p1);
    ferVec3Add(&v, tr);
    ferCHull3Add(chull, &v);

    ferMat3MulVec(&v, rot, tri->p2);
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

void ferCDTriMeshTriUpdateMinMax(const fer_cd_trimesh_tri_t *tri,
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
    ferMat3MulVec(&p, rot, tri->p0);
    ferVec3Add(&p, tr);
    m = ferVec3Dot(&p, axis);
    if (m < *min)
        *min = m;
    if (m > *max)
        *max = m;

    ferMat3MulVec(&p, rot, tri->p1);
    ferVec3Add(&p, tr);
    m = ferVec3Dot(&p, axis);
    if (m < *min)
        *min = m;
    if (m > *max)
        *max = m;

    ferMat3MulVec(&p, rot, tri->p2);
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

void ferCDTriMeshTriDumpSVT(const fer_cd_trimesh_tri_t *tri,
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
    ferMat3MulVec(&v, rot, tri->p0);
    ferVec3Add(&v, tr);
    ferVec3Print(&v, out);
    fprintf(out, "\n");
    ferMat3MulVec(&v, rot, tri->p1);
    ferVec3Add(&v, tr);
    ferVec3Print(&v, out);
    fprintf(out, "\n");
    ferMat3MulVec(&v, rot, tri->p2);
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
