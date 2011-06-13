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
    if (contacts->size > 0){
        free(contacts->pos);
        free(contacts->dir);
        free(contacts->depth);
    }
    free(contacts);
}

fer_cd_contacts_t *ferCDContactsNew(size_t size)
{
    fer_cd_contacts_t *c;

    c = FER_ALLOC(fer_cd_contacts_t);
    c->num   = 0;
    c->pos   = ferVec3ArrNew(size);
    c->dir   = ferVec3ArrNew(size);
    c->depth = FER_ALLOC_ARR(fer_real_t, size);
    c->size  = size;

    return c;
}


int ferCDSeparateSphereSphere(struct _fer_cd_t *cd,
                              const fer_cd_sphere_t *s1,
                              const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                              const fer_cd_sphere_t *s2,
                              const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                              fer_cd_contacts_t *con)
{
    fer_vec3_t sep;
    fer_real_t len, len2;
    int num = 0;

    ferVec3Sub2(&sep, tr2, tr1);
    len = ferVec3Len(&sep);
    len2 = s1->radius + s2->radius;

    if (len < len2 && con->size > con->num){
        con->depth[con->num] = len2 - len;

        if (!ferIsZero(len)){
            ferVec3Scale2(&con->dir[con->num], &sep, ferRecp(len));
            ferVec3Scale2(&con->pos[con->num], &con->dir[con->num],
                          s1->radius - ((len2 - len) / FER_REAL(2.)));
            ferVec3Add(&con->pos[con->num], tr1);
        }else{
            ferVec3Set(&con->dir[con->num], FER_ONE, FER_ZERO, FER_ZERO);
            ferVec3Copy(&con->pos[con->num], tr1);
        }

        con->num++;
        num = 1;
    }

    return num;
}


int ferCDSeparatePlaneSphere(struct _fer_cd_t *cd,
                             const fer_cd_plane_t *s1,
                             const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                             const fer_cd_sphere_t *s2,
                             const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                             fer_cd_contacts_t *con)
{
    fer_vec3_t sep, v;
    fer_real_t len;
    int num = 0;

    ferMat3CopyCol(&sep, rot1, 2);
    ferVec3Sub2(&v, tr2, tr1);
    len = ferVec3Dot(&sep, &v);
    if (len < s2->radius && con->size > con->num){
        con->depth[con->num] = s2->radius - len;

        ferVec3Copy(&con->dir[con->num], &sep);
        ferVec3Normalize(&con->dir[con->num]);

        ferVec3Scale2(&con->pos[con->num], &con->dir[con->num],
                      -FER_ONE * (s2->radius - con->depth[con->num] * FER_REAL(0.5)));
        ferVec3Add(&con->pos[con->num], tr2);

        con->num++;
        num = 1;
    }

    return num;
}

int ferCDSeparatePlaneBox(struct _fer_cd_t *cd,
                          const fer_cd_plane_t *s1,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_box_t *s2,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                          fer_cd_contacts_t *con)
{
    fer_vec3_t cs[8];
    fer_vec3_t norm, v;
    fer_real_t depth, d;
    int i, num, cnum = 0;

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

    if (depth < FER_ZERO && num < 8 && con->size > con->num){
        //DBG("depth: %f, num: %d", depth, num);
        depth *= -FER_ONE;

        if (con->size - con->num >= num){
            for (i = 0; i < num; i++){
                con->depth[con->num + i] = depth;
                ferVec3Copy(&con->dir[con->num + i], &norm);
                ferVec3Copy(&con->pos[con->num + i], &cs[i]);
            }
            con->num += num;
            cnum = num;
        }else{
            con->depth[con->num] = depth;
            ferVec3Copy(&con->dir[con->num], &norm);

            ferVec3Copy(&con->pos[i], &cs[0]);
            for (i = 1; i < num; i++){
                ferVec3Add(&con->pos[con->num], &cs[i]);
            }
            ferVec3Scale(&con->pos[con->num], ferRecp(num));

            con->num++;
            cnum = 1;
        }
    }

    return cnum;
}


int ferCDSeparatePlaneCap(struct _fer_cd_t *cd,
                          const fer_cd_plane_t *s1,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_cap_t *s2,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                          fer_cd_contacts_t *con)
{
    fer_vec3_t pn, cn, cp, cp2;
    fer_real_t sign, depth, depth2;
    int num = 0;

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
    if (depth > FER_ZERO && con->size > con->num){
        con->depth[con->num] = depth;
        ferVec3Copy(&con->dir[con->num], &pn);
        ferVec3Scale2(&con->pos[con->num], &pn, -s2->radius);
        ferVec3Add(&con->pos[con->num], &cp);
        con->num++;
        num = 1;


        if (con->size > con->num){
            ferVec3Scale2(&cp2, &cn, -FER_ONE * s2->half_height * sign);
            ferVec3Add(&cp2, tr2);

            depth2  = -ferVec3Dot(&pn, tr1);
            depth2 -= ferVec3Dot(&pn, &cp2);
            depth2 += s2->radius;

            if (depth2 > FER_ZERO){
                con->depth[con->num] = depth2;
                ferVec3Copy(&con->dir[con->num], &pn);
                ferVec3Scale2(&con->pos[con->num], &pn, -s2->radius);
                ferVec3Add(&con->pos[con->num], &cp2);
                con->num++;
                num++;
            }
        }
    }

    return num;
}


int ferCDSeparatePlaneConvex(struct _fer_cd_t *cd,
                             const fer_cd_plane_t *s1,
                             const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                             const fer_cd_shape_t *s2,
                             const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                             fer_cd_contacts_t *con)
{
    fer_vec3_t pn, p, p2;
    fer_real_t depth;
    int num = 0;

    // get reverse normal of plane
    ferMat3CopyCol(&pn, rot1, 2);
    ferVec3Scale(&pn, -FER_ONE);

    // get support vector in {pn}'s direction
    if (__ferCDSupport(s2, rot2, tr2, &pn, &p) != 0){
        fprintf(stderr, "CD Error: Shape (%d) doesn't have support function\n", s2->cl->type);
        return 0;
    }

    // compute depth
    ferVec3Sub2(&p2, &p, tr1);
    depth = ferVec3Dot(&pn, &p2);

    if (depth > FER_ZERO && con->size > con->num){
        con->depth[con->num] = depth;
        ferVec3Scale2(&con->dir[con->num], &pn, -FER_ONE);
        ferVec3Copy(&con->pos[con->num], &p);
        con->num++;
        num = 1;
    }

    return num;
}

int ferCDSeparatePlaneTri(struct _fer_cd_t *cd,
                          const fer_cd_plane_t *_,
                          const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                          const fer_cd_tri_t *t,
                          const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                          fer_cd_contacts_t *con)
{
    fer_vec3_t pn, p, pos;
    fer_real_t depth, d;
    int i, under, num = 0;

    depth = -FER_ONE;
    ferMat3CopyCol(&pn, rot1, 2);

    under = 0;
    depth = -FER_ONE;
    ferVec3Set(&pos, FER_ZERO, FER_ZERO, FER_ZERO);
    for (i = 0; i < 3; i++){
        ferMat3MulVec(&p, rot2, t->p[i]);
        ferVec3Add(&p, tr2);
        ferVec3Sub(&p, tr1);

        d = ferVec3Dot(&p, &pn);
        if (d < FER_ZERO){
            d = -d;
            if (d > depth)
                depth = d;
            ferVec3Add(&pos, &p);
            ferVec3Sub(&pos, tr1);
            under++;
        }
    }

    if (under > 0 && under < 3 && con->size > con->num){
        ferVec3Scale(&pos, ferRecp(under));
        con->depth[con->num] = depth;
        ferVec3Copy(&con->pos[con->num], &pos);
        ferVec3Copy(&con->dir[con->num], &pn);
        con->num++;
        num = 1;
    }

    return num;
}


int ferCDSeparateTriTri(struct _fer_cd_t *cd,
                        const fer_cd_tri_t *t1,
                        const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                        const fer_cd_tri_t *t2,
                        const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                        fer_cd_contacts_t *con)
{
    fer_vec3_t p1[3], p2[3], s, t;
    int i, ret, num = 0;

    for (i = 0; i < 3; i++){
        ferMat3MulVec(&p1[i], rot1, t1->p[i]);
        ferVec3Add(&p1[i], tr1);
        ferMat3MulVec(&p2[i], rot2, t2->p[i]);
        ferVec3Add(&p2[i], tr2);
    }

    ret = ferVec3TriTriIntersect(p1 + 0, p1 + 1, p1 + 2,
                                 p2 + 0, p2 + 1, p2 + 2,
                                 &s, &t);
    if (ret == 1 && con->size > con->num){
        if (con->size - con->num >= 2){
            con->depth[con->num] = con->depth[con->num + 1] = -1;
            ferVec3Set(&con->dir[con->num], FER_ZERO, FER_ZERO, FER_ZERO);
            ferVec3Set(&con->dir[con->num + 1], FER_ZERO, FER_ZERO, FER_ZERO);
            ferVec3Copy(&con->pos[con->num], &s);
            ferVec3Copy(&con->pos[con->num + 1], &t);
            con->num += 2;
            num = 2;
        }else{
            con->depth[con->num] = -1;
            ferVec3Add2(&con->pos[con->num], &s, &t);
            ferVec3Scale(&con->pos[con->num], FER_REAL(0.5));
            ferVec3Set(&con->dir[con->num], FER_ZERO, FER_ZERO, FER_ZERO);
            con->num++;
            num = 1;
        }
    }else if (ret == 2){
    }

    return num;
}



int ferCDSeparateOffOff(struct _fer_cd_t *cd,
                        const fer_cd_shape_off_t *s1,
                        const fer_mat3_t *_rot1, const fer_vec3_t *_tr1,
                        const fer_cd_shape_off_t *s2,
                        const fer_mat3_t *_rot2, const fer_vec3_t *_tr2,
                        fer_cd_contacts_t *con)
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
                                    s2->shape, &rot2, &tr2, con);
}

int ferCDSeparateOffAny(struct _fer_cd_t *cd,
                        const fer_cd_shape_off_t *s1,
                        const fer_mat3_t *_rot1, const fer_vec3_t *_tr1,
                        const fer_cd_shape_t *s2,
                        const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                        fer_cd_contacts_t *con)
{
    fer_mat3_t rot1;
    fer_vec3_t tr1;

    ferMat3Mul2(&rot1, _rot1, s1->rot);
    ferMat3MulVec(&tr1, _rot1, s1->tr);
    ferVec3Add(&tr1, _tr1);

    return __ferCDShapeSeparate(cd, s1->shape, &rot1, &tr1,
                                    s2, rot2, tr2, con);
}

