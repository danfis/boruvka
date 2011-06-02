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

void ferCDContactsDel(fer_cd_contacts_t *contacts)
{
    if (contacts->num > 0){
        free(contacts->pos);
        free(contacts->dir);
        free(contacts->depth);
    }
    free(contacts);
}

fer_cd_contacts_t *ferCDContactsNew(size_t num)
{
    fer_cd_contacts_t *c;

    c = FER_ALLOC(fer_cd_contacts_t);
    c->num   = num;
    c->pos   = ferVec3ArrNew(num);
    c->dir   = ferVec3ArrNew(num);
    c->depth = FER_ALLOC_ARR(fer_real_t, num);

    return c;
}


fer_cd_contacts_t *ferCDSeparateSphereSphere(struct _fer_cd_t *cd,
                                             const fer_cd_sphere_t *s1,
                                             const fer_mat3_t *rot1,
                                             const fer_vec3_t *tr1,
                                             const fer_cd_sphere_t *s2,
                                             const fer_mat3_t *rot2,
                                             const fer_vec3_t *tr2)
{
    fer_cd_contacts_t *con = NULL;
    fer_vec3_t sep;
    fer_real_t len, len2;

    ferVec3Sub2(&sep, tr2, tr1);
    len = ferVec3Len(&sep);
    len2 = s1->radius + s2->radius;

    if (len < len2){
        con = ferCDContactsNew(1);
        con->depth[0] = len2 - len;

        if (!ferIsZero(len)){
            ferVec3Scale2(&con->dir[0], &sep, ferRecp(len));
            ferVec3Scale2(&con->pos[0], &con->dir[0],
                          s1->radius - ((len2 - len) / FER_REAL(2.)));
            ferVec3Add(&con->pos[0], tr1);
        }else{
            ferVec3Set(&con->dir[0], FER_ONE, FER_ZERO, FER_ZERO);
            ferVec3Copy(&con->pos[0], tr1);
        }
    }

    return con;
}


fer_cd_contacts_t *ferCDSeparatePlaneSphere(struct _fer_cd_t *cd,
                                            const fer_cd_plane_t *s1,
                                            const fer_mat3_t *rot1,
                                            const fer_vec3_t *tr1,
                                            const fer_cd_sphere_t *s2,
                                            const fer_mat3_t *rot2,
                                            const fer_vec3_t *tr2)
{
    fer_cd_contacts_t *con = NULL;
    fer_vec3_t sep, v;
    fer_real_t len;

    ferMat3CopyCol(&sep, rot1, 2);
    ferVec3Sub2(&v, tr2, tr1);
    len = ferVec3Dot(&sep, &v);
    if (len < s2->radius){
        con = ferCDContactsNew(1);
        con->depth[0] = s2->radius - len;

        ferVec3Copy(&con->dir[0], &sep);
        ferVec3Normalize(&con->dir[0]);

        ferVec3Scale2(&con->pos[0], &con->dir[0],
                      -FER_ONE * (s2->radius - con->depth[0] * FER_REAL(0.5)));
        ferVec3Add(&con->pos[0], tr2);
    }

    return con;
}

static int planeBoxPlaneSegment(const fer_vec3_t *orig, const fer_vec3_t *dir,
                                const fer_vec3_t *n, fer_real_t d,
                                fer_real_t *t)
{
    fer_real_t vd;

    vd = ferVec3Dot(n, dir);
    if (ferIsZero(vd))
        return -1;

    *t  = -ferVec3Dot(orig, n) - d;
    *t *= ferRecp(vd);

    //DBG("*t: %f (%f %f %f)", *t, ferVec3Dot(orig, n), d, vd);
    if (*t >= FER_ZERO && *t <= FER_ONE)
        return 0;
    return -1;
}

int planeBoxIntersection(const fer_cd_plane_t *s1,
                         const fer_mat3_t *rot1,
                         const fer_vec3_t *tr1,
                         const fer_cd_box_t *s2,
                         const fer_mat3_t *rot2,
                         const fer_vec3_t *tr2,
                         fer_vec3_t *cs)
{
    fer_vec3_t pn; // plane's normal
    fer_real_t pd;
    fer_vec3_t ba[3]; // box's axis
    fer_vec3_t bp;
    int i, cslen = 0;
    fer_real_t t;

    // get planes' parameters
    ferMat3CopyCol(&pn, rot1, 2);
    pd = -ferVec3Dot(&pn, tr1);

    // get boxes' parameters
    ferMat3CopyCol(&ba[0], rot2, 0);
    ferMat3CopyCol(&ba[1], rot2, 1);
    ferMat3CopyCol(&ba[2], rot2, 2);

    ferVec3Scale(&ba[0], ferVec3X(s2->half_extents) * FER_REAL(2.));
    ferVec3Scale(&ba[1], ferVec3Y(s2->half_extents) * FER_REAL(2.));
    ferVec3Scale(&ba[2], ferVec3Z(s2->half_extents) * FER_REAL(2.));

    ferVec3Sub2(&bp, tr2, &ba[0]);
    ferVec3Sub(&bp, &ba[1]);
    ferVec3Sub(&bp, &ba[2]);
    ferVec3Scale(&bp, FER_REAL(0.5));

    for (i = 0; i < 3; i++){
        if (planeBoxPlaneSegment(&bp, &ba[i], &pn, pd, &t) == 0){
            ferVec3Scale2(&cs[cslen], &ba[i], t);
            ferVec3Add(&cs[cslen], &bp);
            cslen++;
        }
    }


    ferVec3Add(&bp, &ba[0]);
    ferVec3Add(&bp, &ba[1]);
    ferVec3Scale(&ba[0], -FER_ONE);
    ferVec3Scale(&ba[1], -FER_ONE);
    for (i = 0; i < 3; i++){
        if (planeBoxPlaneSegment(&bp, &ba[i], &pn, pd, &t) == 0){
            ferVec3Scale2(&cs[cslen], &ba[i], t);
            ferVec3Add(&cs[cslen], &bp);
            cslen++;
        }
    }


    ferVec3Add(&bp, &ba[0]);
    ferVec3Add(&bp, &ba[2]);
    ferVec3Scale(&ba[0], -FER_ONE);
    ferVec3Scale(&ba[2], -FER_ONE);
    for (i = 0; i < 3; i++){
        if (planeBoxPlaneSegment(&bp, &ba[i], &pn, pd, &t) == 0){
            ferVec3Scale2(&cs[cslen], &ba[i], t);
            ferVec3Add(&cs[cslen], &bp);
            cslen++;
        }
    }


    ferVec3Add(&bp, &ba[0]);
    ferVec3Add(&bp, &ba[1]);
    ferVec3Scale(&ba[0], -FER_ONE);
    ferVec3Scale(&ba[1], -FER_ONE);
    for (i = 0; i < 3; i++){
        if (planeBoxPlaneSegment(&bp, &ba[i], &pn, pd, &t) == 0){
            ferVec3Scale2(&cs[cslen], &ba[i], t);
            ferVec3Add(&cs[cslen], &bp);
            cslen++;
        }
    }

    //DBG("cslen: %d", cslen);
    return cslen;
}

fer_cd_contacts_t *ferCDSeparatePlaneBox(struct _fer_cd_t *cd,
                                         const fer_cd_plane_t *s1,
                                         const fer_mat3_t *rot1,
                                         const fer_vec3_t *tr1,
                                         const fer_cd_box_t *s2,
                                         const fer_mat3_t *rot2,
                                         const fer_vec3_t *tr2)
{
    fer_cd_contacts_t *con = NULL;
    fer_vec3_t cs[14];
    fer_vec3_t norm, v;
    fer_real_t depth, d;
    int i, num, num2;

    __ferCDBoxGetCorners(s2, rot2, tr2, cs);
    ferMat3CopyCol(&norm, rot1, 2);

    depth = FER_REAL_MAX;
    num = 0;
    for (i = 0; i < 8; i++){
        ferVec3Sub2(&v, &cs[i], tr1);
        d = ferVec3Dot(&norm, &v);
        //DBG("d: %f", d);

        if (d < FER_ZERO){
            // keep all corners under plane in cs[] array and keep the
            // deepest in cs[0]
            if (d < depth){
                depth = d;

                if (num != i){
                    ferVec3Copy(&cs[num], &cs[0]);
                    ferVec3Copy(&cs[0], &cs[i]);
                }else if (num != 0){
                    ferVec3Copy(&v, &cs[0]);
                    ferVec3Copy(&cs[0], &cs[i]);
                    ferVec3Copy(&cs[num], &v);
                }
            }else{
                if (num != i)
                    ferVec3Copy(&cs[num], &cs[i]);
            }

            num++;
        }
    }

    if (depth < FER_ZERO){
        //DBG("depth: %f, num: %d", depth, num);
        depth *= -FER_ONE;

        num2 = planeBoxIntersection(s1, rot1, tr1, s2, rot2, tr2, cs + num);
        //DBG("depth: %f, num: %d, num2: %d", depth, num, num2);
        if (cd->max_contacts >= num2){
            con = ferCDContactsNew(num2);
            for (i = 0; i < num2; i++){
                con->depth[i] = depth;
                ferVec3Copy(&con->dir[i], &norm);
                ferVec3Copy(&con->pos[i], &cs[num + i]);
            }
        }else{
            con = ferCDContactsNew(1);
            con->depth[0] = depth;
            ferVec3Copy(&con->dir[0], &norm);

            ferVec3Copy(&con->pos[i], &cs[0]);
            num += num2;
            for (i = 1; i < num; i++){
                ferVec3Add(&con->pos[i], &cs[i]);
            }
            ferVec3Scale(&con->pos[i], ferRecp(num));
        }
    }

    return con;
}


fer_cd_contacts_t *ferCDSeparatePlaneCap(struct _fer_cd_t *cd,
                                         const fer_cd_plane_t *s1,
                                         const fer_mat3_t *rot1,
                                         const fer_vec3_t *tr1,
                                         const fer_cd_cap_t *s2,
                                         const fer_mat3_t *rot2,
                                         const fer_vec3_t *tr2)
{
    fer_cd_contacts_t *con = NULL;
    fer_vec3_t pn, cn, cp, cp2;
    fer_real_t sign, depth, depth2;

    ferMat3CopyCol(&pn, rot1, 2);
    ferMat3CopyCol(&cn, rot2, 2);

    if (ferVec3Dot(&pn, &cn) > 0){
        sign = -FER_ONE;
    }else{
        sign = FER_ONE;
    }

    ferVec3Scale2(&cp, &cn, s2->half_height * sign);
    ferVec3Add(&cp, tr2);

    depth  = -ferVec3Dot(&pn, tr1);
    depth -= ferVec3Dot(&pn, &cp);
    depth += s2->radius;
    if (depth > FER_ZERO){
        depth2 = -FER_ONE;
        ferVec3Set(&cp2, FER_ZERO, FER_ZERO, FER_ZERO);

        if (cd->max_contacts > 1){
            ferVec3Scale2(&cp2, &cn, -FER_ONE * s2->half_height * sign);
            ferVec3Add(&cp2, tr2);

            depth2  = -ferVec3Dot(&pn, tr1);
            depth2 -= ferVec3Dot(&pn, &cp2);
            depth2 += s2->radius;
            con = ferCDContactsNew(2);
        }

        if (depth2 > FER_ZERO){
            con = ferCDContactsNew(2);
        }else{
            con = ferCDContactsNew(1);
        }

        con->depth[0] = depth;
        ferVec3Copy(&con->dir[0], &pn);
        ferVec3Scale2(&con->pos[0], &pn, -s2->radius);
        ferVec3Add(&con->pos[0], &cp);

        if (depth2 > FER_ZERO){
            con->depth[1] = depth2;
            ferVec3Copy(&con->dir[1], &pn);
            ferVec3Scale2(&con->pos[1], &pn, -s2->radius);
            ferVec3Add(&con->pos[1], &cp2);
        }
    }

    return con;
}


fer_cd_contacts_t *ferCDSeparatePlaneConvex(struct _fer_cd_t *cd,
                                            const fer_cd_plane_t *s1,
                                            const fer_mat3_t *rot1,
                                            const fer_vec3_t *tr1,
                                            const fer_cd_shape_t *s2,
                                            const fer_mat3_t *rot2,
                                            const fer_vec3_t *tr2)
{
    fer_cd_contacts_t *con = NULL;
    fer_vec3_t pn, p, p2;
    fer_real_t depth;

    // get reverse normal of plane
    ferMat3CopyCol(&pn, rot1, 2);
    ferVec3Scale(&pn, -FER_ONE);

    // get support vector in {pn}'s direction
    if (__ferCDSupport(s2, rot2, tr2, &pn, &p) != 0){
        fprintf(stderr, "CD Error: Shape (%d) doesn't have support function\n", s2->cl->type);
        return NULL;
    }

    // compute depth
    ferVec3Sub2(&p2, &p, tr1);
    depth = ferVec3Dot(&pn, &p2);

    if (depth > FER_ZERO){
        con = ferCDContactsNew(1);
        con->depth[0] = depth;
        ferVec3Scale2(&con->dir[0], &pn, -FER_ONE);
        ferVec3Copy(&con->pos[0], &p);
    }

    return con;
}

fer_cd_contacts_t *ferCDSeparateOffOff(struct _fer_cd_t *cd,
                                       const fer_cd_shape_off_t *s1,
                                       const fer_mat3_t *_rot1,
                                       const fer_vec3_t *_tr1,
                                       const fer_cd_shape_off_t *s2,
                                       const fer_mat3_t *_rot2,
                                       const fer_vec3_t *_tr2)
{
    fer_mat3_t rot1, rot2;
    fer_vec3_t tr1, tr2;

    ferMat3Mul2(&rot1, _rot1, s1->rot);
    ferMat3MulVec(&tr1, _rot1, s1->tr);
    ferVec3Add(&tr1, _tr1);

    ferMat3Mul2(&rot2, _rot2, s2->rot);
    ferMat3MulVec(&tr2, _rot2, s2->tr);
    ferVec3Add(&tr2, _tr2);

    return __ferCDShapeSeparate(cd, s1->shape, &rot1, &tr1,
                                    s2->shape, &rot2, &tr2);
}

fer_cd_contacts_t *ferCDSeparateOffAny(struct _fer_cd_t *cd,
                                       const fer_cd_shape_off_t *s1,
                                       const fer_mat3_t *_rot1,
                                       const fer_vec3_t *_tr1,
                                       const fer_cd_shape_t *s2,
                                       const fer_mat3_t *rot2,
                                       const fer_vec3_t *tr2)
{
    fer_mat3_t rot1;
    fer_vec3_t tr1;

    ferMat3Mul2(&rot1, _rot1, s1->rot);
    ferMat3MulVec(&tr1, _rot1, s1->tr);
    ferVec3Add(&tr1, _tr1);

    return __ferCDShapeSeparate(cd, s1->shape, &rot1, &tr1,
                                    s2, rot2, tr2);
}
