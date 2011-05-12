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

int ferCDCollideTriTri(struct _fer_cd_t *cd,
                       const fer_cd_tri_t *tri1,
                       const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                       const fer_cd_tri_t *tri2,
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
