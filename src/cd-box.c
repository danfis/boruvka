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

/** Fills cs[8] with corners of given box */
static void getCorners(const fer_cd_box_t *b,
                       const fer_mat3_t *rot, const fer_vec3_t *tr,
                       fer_vec3_t *cs);


static fer_cd_shape_class_t shape = {
    .type          = FER_CD_SHAPE_BOX,
    .del           = (fer_cd_shape_del_fn)ferCDBoxDel,
    .support       = (fer_cd_shape_support_fn)ferCDBoxSupport,
    .fit_obb       = (fer_cd_shape_fit_obb_fn)ferCDBoxFitOBB,
    .update_chull  = (fer_cd_shape_update_chull_fn)ferCDBoxUpdateCHull,
    .update_minmax = (fer_cd_shape_update_minmax_fn)ferCDBoxUpdateMinMax,
    .dump_svt      = (fer_cd_shape_dump_svt_fn)ferCDBoxDumpSVT
};

fer_cd_box_t *ferCDBoxNew(fer_real_t lx, fer_real_t ly, fer_real_t lz)
{
    fer_cd_box_t *b;

    b = FER_ALLOC(fer_cd_box_t);
    b->shape.cl = &shape;
    b->half_extents = ferVec3New(lx, ly, lz);
    ferVec3Scale(b->half_extents, FER_REAL(0.5));

    return b;
}

void ferCDBoxDel(fer_cd_box_t *b)
{
    ferVec3Del(b->half_extents);
    free(b);
}

void ferCDBoxSupport(const fer_cd_box_t *b, const fer_vec3_t *dir,
                     fer_vec3_t *p)
{
    ferVec3Set(p, ferSign(ferVec3X(dir)) * ferVec3X(b->half_extents),
                  ferSign(ferVec3Y(dir)) * ferVec3Y(b->half_extents),
                  ferSign(ferVec3Z(dir)) * ferVec3Z(b->half_extents));
}

void ferCDBoxFitOBB(const fer_cd_box_t *b,
                    fer_vec3_t *center,
                    fer_vec3_t *axis0,
                    fer_vec3_t *axis1,
                    fer_vec3_t *axis2,
                    fer_vec3_t *half_extents, int flags)
{
    ferVec3Set(center, FER_ZERO, FER_ZERO, FER_ZERO);
    ferVec3Set(axis0, FER_ONE,  FER_ZERO, FER_ZERO);
    ferVec3Set(axis1, FER_ZERO, FER_ONE,  FER_ZERO);
    ferVec3Set(axis2, FER_ZERO, FER_ZERO, FER_ONE);
    ferVec3Copy(half_extents, b->half_extents);
}

int ferCDBoxUpdateCHull(const fer_cd_box_t *b, fer_chull3_t *chull,
                        const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_vec3_t cs[8];
    int i;

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

    getCorners(b, rot, tr, cs);

    for (i = 0; i < 8; i++){
        ferCHull3Add(chull, &cs[i]);
    }

    return 1;
}

void ferCDBoxUpdateMinMax(const fer_cd_box_t *b, const fer_vec3_t *axis,
                          const fer_mat3_t *rot, const fer_vec3_t *tr,
                          fer_real_t *min, fer_real_t *max)
{
    fer_vec3_t c[8];
    fer_real_t m;
    size_t i;

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

    getCorners(b, rot, tr, c);

    for (i = 0; i < 8; i++){
        m = ferVec3Dot(&c[i], axis);

        if (m < *min)
            *min = m;
        if (m > *max)
            *max = m;
    }
}

void ferCDBoxDumpSVT(const fer_cd_box_t *b,
                     FILE *out, const char *name,
                     const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_vec3_t v, w;
    fer_vec3_t axis[3];
    int i;

    if (!rot)
        rot = fer_mat3_identity;
    if (!tr)
        tr = fer_vec3_origin;

    for (i = 0; i < 3; i++){
        ferMat3MulVec(&axis[i], rot, fer_vec3_axis[i]);
    }

    fprintf(out, "----==\n");

    if (name){
        fprintf(out, "Name: %s\n", name);
    }

    //fprintf(out, "Point color: 0.8 0 0\n");
    //fprintf(out, "Points:\n");
    //ferVec3Print(tr, out);
    //fprintf(out, "\n----\n");

    fprintf(out, "Edge color: 0.7 0.5 1.\n");
    fprintf(out, "Points:\n");
    ferVec3Copy(&v, tr);
    ferVec3Scale2(&w, &axis[0], ferVec3X(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[1], ferVec3Y(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[2], ferVec3Z(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, tr);
    ferVec3Scale2(&w, &axis[0], ferVec3X(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[1], ferVec3Y(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[2], -ferVec3Z(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, tr);
    ferVec3Scale2(&w, &axis[0], -ferVec3X(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[1], ferVec3Y(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[2], ferVec3Z(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, tr);
    ferVec3Scale2(&w, &axis[0], -ferVec3X(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[1], ferVec3Y(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[2], -ferVec3Z(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, tr);
    ferVec3Scale2(&w, &axis[0], ferVec3X(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[1], -ferVec3Y(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[2], ferVec3Z(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, tr);
    ferVec3Scale2(&w, &axis[0], ferVec3X(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[1], -ferVec3Y(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[2], -ferVec3Z(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, tr);
    ferVec3Scale2(&w, &axis[0], -ferVec3X(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[1], -ferVec3Y(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[2], ferVec3Z(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Print(&v, out);
    fprintf(out, "\n");

    ferVec3Copy(&v, tr);
    ferVec3Scale2(&w, &axis[0], -ferVec3X(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[1], -ferVec3Y(b->half_extents));
    ferVec3Add(&v, &w);
    ferVec3Scale2(&w, &axis[2], -ferVec3Z(b->half_extents));
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


static void getCorners(const fer_cd_box_t *b,
                       const fer_mat3_t *rot, const fer_vec3_t *tr,
                       fer_vec3_t *cs)
{
    fer_vec3_t axis[3];
    int i;

    ferMat3CopyCol(&axis[0], rot, 0);
    ferMat3CopyCol(&axis[1], rot, 1);
    ferMat3CopyCol(&axis[2], rot, 2);
    ferVec3Scale(&axis[0], ferVec3X(b->half_extents));
    ferVec3Scale(&axis[1], ferVec3Y(b->half_extents));
    ferVec3Scale(&axis[2], ferVec3Z(b->half_extents));

    ferVec3Add2(&cs[0], &axis[0], &axis[1]);
    ferVec3Add(&cs[0], &axis[2]);

    ferVec3Add2(&cs[1], &axis[0], &axis[1]);
    ferVec3Sub(&cs[1], &axis[2]);

    ferVec3Sub2(&cs[2], &axis[0], &axis[1]);
    ferVec3Add(&cs[2], &axis[2]);

    ferVec3Sub2(&cs[3], &axis[0], &axis[1]);
    ferVec3Sub(&cs[3], &axis[2]);

    ferVec3Sub2(&cs[4], &axis[1], &axis[0]);
    ferVec3Add(&cs[4], &axis[2]);

    ferVec3Sub2(&cs[5], &axis[1], &axis[0]);
    ferVec3Sub(&cs[5], &axis[2]);

    ferVec3Scale2(&cs[6], &axis[0], -FER_ONE);
    ferVec3Sub(&cs[6], &axis[1]);
    ferVec3Add(&cs[6], &axis[2]);

    ferVec3Scale2(&cs[7], &axis[0], -FER_ONE);
    ferVec3Sub(&cs[7], &axis[1]);
    ferVec3Sub(&cs[7], &axis[2]);

    for (i = 0; i < 8; i++){
        ferVec3Add(&cs[i], tr);
    }
}
