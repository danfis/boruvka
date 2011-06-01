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
