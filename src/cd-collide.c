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
#include <fermat/ccd.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

int ferCDCollideSphereSphere(fer_cd_t *cd,
                             const fer_cd_sphere_t *s1,
                             const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                             const fer_cd_sphere_t *s2,
                             const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    fer_real_t dist;
    dist = ferVec3Dist(tr1, tr2);
    return dist < (s1->radius + s2->radius);
}

int ferCDCollideSphereBox(struct _fer_cd_t *cd,
                          const fer_cd_sphere_t *s1,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_box_t *s2,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    fer_vec3_t p;
    fer_real_t l1, l2;

    // get point on OBB closest to sphere center
    __ferCDBoxClosestPoint(s2->half_extents, rot2, tr2, tr1, &p);

    ferVec3Sub(&p, tr1);

    l1 = ferVec3Len2(&p);
    l2 = FER_CUBE(s1->radius);
    return l1 < l2 || ferEq(l1, l2);
}

int ferCDCollideSphereCap(struct _fer_cd_t *cd,
                          const fer_cd_sphere_t *s1,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_cap_t *s2,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    fer_real_t d1, d2;
    fer_vec3_t a, b;

    ferVec3Set(&a, ferMat3Get(rot2, 0, 2) * s2->half_height,
                   ferMat3Get(rot2, 1, 2) * s2->half_height,
                   ferMat3Get(rot2, 2, 2) * s2->half_height);
    ferVec3Scale2(&b, &a, -FER_ONE);
    ferVec3Add(&a, tr2);
    ferVec3Add(&b, tr2);

    d1 = ferVec3PointSegmentDist2(tr1, &a, &b, NULL);
    d2 = FER_CUBE(s1->radius + s2->radius);

    return d1 < d2 || ferEq(d1, d2);
}

int ferCDCollideSphereTri(struct _fer_cd_t *cd,
                          const fer_cd_sphere_t *s,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_tri_t *t,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    fer_real_t l1, l2;
    fer_vec3_t a, b, c;

    ferMat3MulVec(&a, rot2, t->p[0]);
    ferMat3MulVec(&b, rot2, t->p[1]);
    ferMat3MulVec(&c, rot2, t->p[2]);
    ferVec3Add(&a, tr2);
    ferVec3Add(&b, tr2);
    ferVec3Add(&c, tr2);

    l1 = ferVec3PointTriDist2(tr1, &a, &b, &c, NULL);
    l2 = FER_CUBE(s->radius);

    return l1 < l2 || ferEq(l1, l2);
}

int ferCDCollideBoxBox(struct _fer_cd_t *cd,
                       const fer_cd_box_t *s1,
                       const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                       const fer_cd_box_t *s2,
                       const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    fer_vec3_t tr, trtmp;;
    fer_mat3_t rot;
    int ret;

    ferMat3Trans2(&rot, rot1);

    // compute translation in obb1's frame
    ferVec3Sub2(&trtmp, tr2, tr1);
    ferMat3MulVec(&tr, &rot, &trtmp);

    // compute rotation in obb1's frame
    ferMat3Mul(&rot, rot2);

    ret = __ferCDBoxDisjoint(s1->half_extents, s2->half_extents, &rot, &tr);
    return !ret;
}

int ferCDCollidePlaneSphere(struct _fer_cd_t *cd,
                            const fer_cd_plane_t *p,
                            const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                            const fer_cd_sphere_t *s,
                            const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    fer_vec3_t ps, axis;
    fer_real_t d1;

    ferVec3Sub2(&ps, tr2, tr1);
    ferMat3CopyCol(&axis, rot1, 2);

    d1 = ferVec3Dot(&ps, &axis);

    return d1 < s->radius || ferEq(d1, s->radius) || d1 < FER_ZERO;
}

int ferCDCollidePlaneBox(struct _fer_cd_t *cd,
                         const fer_cd_plane_t *p,
                         const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                         const fer_cd_box_t *b,
                         const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    fer_vec3_t vs[8], axis, vbox;
    fer_real_t m;
    int i;

    ferMat3CopyCol(&axis, rot1, 2);
    __ferCDBoxGetCorners(b, rot2, tr2, vs);

    for (i = 0; i < 8; i++){
        ferVec3Sub2(&vbox, &vs[i], tr1);
        m = ferVec3Dot(&axis, &vbox);
        if (m < FER_ZERO)
            return 1;
    }

    return 0;
}

int ferCDCollidePlaneCap(struct _fer_cd_t *cd,
                         const fer_cd_plane_t *p,
                         const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                         const fer_cd_cap_t *c,
                         const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    fer_vec3_t ca, cb;
    fer_real_t ma, mb;

    ferMat3CopyCol(&ca, rot2, 2);
    ferVec3Scale(&ca, c->half_height);
    ferVec3Scale2(&cb, &ca, -FER_ONE);
    ferVec3Add(&ca, tr2);
    ferVec3Add(&cb, tr2);

    ferVec3Sub(&ca, tr1);
    ferVec3Sub(&cb, tr1);

    ma = ferMat3DotCol(rot1, 2, &ca);
    mb = ferMat3DotCol(rot1, 2, &cb);

    if (ferSign(ma) != ferSign(mb))
        return 1;

    if (ma < mb){
        return ma < c->radius;
    }else{
        return mb < c->radius;
    }
}

int ferCDCollidePlaneCyl(struct _fer_cd_t *cd,
                         const fer_cd_plane_t *p,
                         const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                         const fer_cd_cyl_t *c,
                         const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    fer_real_t s, s2;
    fer_vec3_t cyldir, planenorm, v, v2;
    fer_vec3_t ca, cb;
    fer_real_t ma, mb;

    ferMat3CopyCol(&cyldir, rot2, 2);
    ferMat3CopyCol(&planenorm, rot1, 2);

    s = ferVec3Dot(&cyldir, &planenorm);
    if (ferEq(s, FER_ONE)){
        // plane and cylinder's discs are parallel
        ferVec3Sub2(&v, tr2, tr1);
        s = ferVec3Dot(&v, &planenorm);
        return s < c->half_height;
    }else{
        ferMat3CopyCol(&ca, rot2, 2);
        ferVec3Scale(&ca, c->half_height);
        ferVec3Scale2(&cb, &ca, -FER_ONE);
        ferVec3Add(&ca, tr2);
        ferVec3Add(&cb, tr2);

        ferVec3Sub(&ca, tr1);
        ferVec3Sub(&cb, tr1);

        ma = ferMat3DotCol(rot1, 2, &ca);
        mb = ferMat3DotCol(rot1, 2, &cb);

        if (ferSign(ma) != ferSign(mb))
            return 1;

        if (ma > mb){
            ma = mb;
        }


        ferVec3Cross(&v2, &planenorm, &cyldir);
        ferVec3Cross(&v, &v2, &cyldir);
        ferVec3Scale(&v, c->radius);
        s2 = ferVec3Dot(&v, &planenorm);
        s2 = FER_FABS(s2);

        return s2 > ma  || ferEq(s2, ma);
    }
}

int ferCDCollidePlaneTri(struct _fer_cd_t *cd,
                         const fer_cd_plane_t *p,
                         const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                         const fer_cd_tri_t *t,
                         const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    fer_vec3_t a, b, c, axis;

    ferMat3MulVec(&a, rot2, t->p[0]);
    ferMat3MulVec(&b, rot2, t->p[1]);
    ferMat3MulVec(&c, rot2, t->p[2]);
    ferVec3Add(&a, tr2);
    ferVec3Add(&b, tr2);
    ferVec3Add(&c, tr2);
    ferVec3Sub(&a, tr1);
    ferVec3Sub(&b, tr1);
    ferVec3Sub(&c, tr1);

    ferMat3CopyCol(&axis, rot1, 2);

    if (ferVec3Dot(&a, &axis) < FER_ZERO)
        return 1;
    if (ferVec3Dot(&b, &axis) < FER_ZERO)
        return 1;
    if (ferVec3Dot(&c, &axis) < FER_ZERO)
        return 1;
    return 0;
}

int ferCDCollideTriTri(struct _fer_cd_t *cd,
                       const fer_cd_tri_t *tri1,
                       const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                       const fer_cd_tri_t *tri2,
                       const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    fer_vec3_t p1, q1, r1, p2, q2, r2;

    ferMat3MulVec(&p1, rot1, tri1->p[0]);
    ferVec3Add(&p1, tr1);
    ferMat3MulVec(&q1, rot1, tri1->p[1]);
    ferVec3Add(&q1, tr1);
    ferMat3MulVec(&r1, rot1, tri1->p[2]);
    ferVec3Add(&r1, tr1);

    ferMat3MulVec(&p2, rot2, tri2->p[0]);
    ferVec3Add(&p2, tr2);
    ferMat3MulVec(&q2, rot2, tri2->p[1]);
    ferVec3Add(&q2, tr2);
    ferMat3MulVec(&r2, rot2, tri2->p[2]);
    ferVec3Add(&r2, tr2);

    /*
    printf("----\nPoints:\n");
    ferVec3Print(&p1, stdout); printf("\n");
    ferVec3Print(&q1, stdout); printf("\n");
    ferVec3Print(&r1, stdout); printf("\n");
    ferVec3Print(&p2, stdout); printf("\n");
    ferVec3Print(&q2, stdout); printf("\n");
    ferVec3Print(&r2, stdout); printf("\n");
    printf("Faces:\n0 1 2\n3 4 5\n--\n");
    */

    return ferVec3TriTriOverlap(&p1, &q1, &r1, &p2, &q2, &r2);
    /*
    if (ferVec3TriTriOverlap(&p1, &q1, &r1, &p2, &q2, &r2)){
        printf("----\nFace color: 1 0 0\nPoints:\n");
        ferVec3Print(&p1, stdout); printf("\n");
        ferVec3Print(&q1, stdout); printf("\n");
        ferVec3Print(&r1, stdout); printf("\n");
        ferVec3Print(&p2, stdout); printf("\n");
        ferVec3Print(&q2, stdout); printf("\n");
        ferVec3Print(&r2, stdout); printf("\n");
        printf("Faces:\n0 1 2\n3 4 5\n--\n");
        return 1;
    }
    return 0;
    */
}

int ferCDCollideOffOff(struct _fer_cd_t *cd,
                       const fer_cd_shape_off_t *s1,
                       const fer_mat3_t *_rot1, const fer_vec3_t *_tr1,
                       const fer_cd_shape_off_t *s2,
                       const fer_mat3_t *_rot2, const fer_vec3_t *_tr2)
{
    fer_mat3_t rot1, rot2;
    fer_vec3_t tr1, tr2;

    ferMat3Mul2(&rot1, _rot1, s1->rot);
    ferMat3MulVec(&tr1, _rot1, s1->tr);
    ferVec3Add(&tr1, _tr1);

    ferMat3Mul2(&rot2, _rot2, s2->rot);
    ferMat3MulVec(&tr2, _rot2, s2->tr);
    ferVec3Add(&tr2, _tr2);

    return __ferCDShapeCollide(cd, s1->shape, &rot1, &tr1,
                                   s2->shape, &rot2, &tr2);
}

int ferCDCollideOffAny(struct _fer_cd_t *cd,
                       const fer_cd_shape_off_t *s1,
                       const fer_mat3_t *_rot1, const fer_vec3_t *_tr1,
                       const fer_cd_shape_t *s2,
                       const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    fer_mat3_t rot1;
    fer_vec3_t tr1;

    ferMat3Mul2(&rot1, _rot1, s1->rot);
    ferMat3MulVec(&tr1, _rot1, s1->tr);
    ferVec3Add(&tr1, _tr1);

    return __ferCDShapeCollide(cd, s1->shape, &rot1, &tr1,
                                   s2, rot2, tr2);
}

