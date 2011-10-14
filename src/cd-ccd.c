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


struct _ccd_t {
    const fer_cd_shape_t *s;
    const fer_mat3_t *rot;
    const fer_vec3_t *tr;
};
typedef struct _ccd_t ccd_t;
static void ccdSupport(const void *obj, const fer_vec3_t *_dir,
                       fer_vec3_t *p)
{
    const ccd_t *s = (const ccd_t *)obj;

    __ferCDSupport(s->s, s->rot, s->tr, _dir, p);
}

static void ccdCenter(const void *obj, fer_vec3_t *c)
{
    const ccd_t *s = (const ccd_t *)obj;

    if (!s->s->cl->center){
        ferVec3Set(c, FER_ZERO, FER_ZERO, FER_ZERO);
        return;
    }

    s->s->cl->center(s->s, s->rot, s->tr, c);
}

static void ccdInit(struct _fer_cd_t *cd,
                    const fer_cd_shape_t *_s1,
                    const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                    const fer_cd_shape_t *_s2,
                    const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                    ccd_t *s1, ccd_t *s2)
{
    if (!rot1)
        rot1 = fer_mat3_identity;
    if (!tr1)
        tr1 = fer_vec3_origin;
    if (!rot2)
        rot2 = fer_mat3_identity;
    if (!tr2)
        tr2 = fer_vec3_origin;

    s1->s   = _s1;
    s1->rot = rot1;
    s1->tr  = tr1;
    s2->s   = _s2;
    s2->rot = rot2;
    s2->tr  = tr2;
}

int ferCDCollideCCD(struct _fer_cd_t *cd,
                    const fer_cd_shape_t *_s1,
                    const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                    const fer_cd_shape_t *_s2,
                    const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    ccd_t s1, s2;
    fer_ccd_t ccd;
    int ret;

    ferCCDInit(&ccd);
    ccd.support1 = ccd.support2 = ccdSupport;
    ccd.center1  = ccd.center2  = ccdCenter;
    ccd.max_iterations = cd->mpr_max_iterations;
    ccd.mpr_tolerance  = cd->mpr_tolerance;

    ccdInit(cd, _s1, rot1, tr1, _s2, rot2, tr2, &s1, &s2);

    ret = ferCCDMPRCollide(&ccd, &s1, &s2);
    return ret;
}

int ferCDSeparateCCD(struct _fer_cd_t *cd,
                     const fer_cd_shape_t *_s1,
                     const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                     const fer_cd_shape_t *_s2,
                     const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                     fer_cd_contacts_t *con)
{
    ccd_t s1, s2;
    fer_ccd_t ccd;
    int ret, num = 0;
    fer_vec3_t dir, pos;
    fer_real_t depth;

    ferCCDInit(&ccd);
    ccd.support1 = ccd.support2 = ccdSupport;
    ccd.center1  = ccd.center2  = ccdCenter;
    ccd.max_iterations = cd->mpr_max_iterations;
    ccd.mpr_tolerance  = cd->mpr_tolerance;

    ccdInit(cd, _s1, rot1, tr1, _s2, rot2, tr2, &s1, &s2);

    ret = ferCCDMPRPenetration(&ccd, &s1, &s2, &depth, &dir, &pos);
    if (ret == 0 && con->size > con->num){
        con->depth[con->num] = depth;
        ferVec3Copy(&con->dir[con->num], &dir);
        ferVec3Copy(&con->pos[con->num], &pos);
        con->num++;
        num = 1;
    }

    return num;
}
