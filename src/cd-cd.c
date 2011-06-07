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

static void updateDirtyGeoms(fer_cd_t *cd);

void ferCDParamsInit(fer_cd_params_t *params)
{
    params->build_flags = FER_CD_TOP_DOWN
                            | FER_CD_FIT_COVARIANCE;

    params->use_sap = 1;
    params->sap_size = 1023;

    params->max_contacts = 20;
}

fer_cd_t *ferCDNew(const fer_cd_params_t *params)
{
    fer_cd_t *cd;
    size_t i, j;
    fer_cd_params_t __params;

    if (!params){
        ferCDParamsInit(&__params);
        params = &__params;
    }

    cd = FER_ALLOC(fer_cd_t);
    cd->build_flags = params->build_flags;

    // Set up collide callbacks
    for (i = 0; i < FER_CD_SHAPE_LEN; i++){
        for (j = 0; j < FER_CD_SHAPE_LEN; j++){
            cd->collide[i][j] = NULL;
        }
    }

    ferCDSetCollideFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_SPHERE,
                      (fer_cd_collide_fn)ferCDCollideSphereSphere);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_BOX,
                      (fer_cd_collide_fn)ferCDCollideSphereBox);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_CAP,
                      (fer_cd_collide_fn)ferCDCollideSphereCap);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_TRI,
                      (fer_cd_collide_fn)ferCDCollideSphereTri);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_TRIMESH_TRI,
                      (fer_cd_collide_fn)ferCDCollideSphereTri);

    ferCDSetCollideFn(cd, FER_CD_SHAPE_BOX, FER_CD_SHAPE_BOX,
                      (fer_cd_collide_fn)ferCDCollideBoxBox);

    ferCDSetCollideFn(cd, FER_CD_SHAPE_PLANE, FER_CD_SHAPE_SPHERE,
                      (fer_cd_collide_fn)ferCDCollidePlaneSphere);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_PLANE, FER_CD_SHAPE_BOX,
                      (fer_cd_collide_fn)ferCDCollidePlaneBox);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_PLANE, FER_CD_SHAPE_CAP,
                      (fer_cd_collide_fn)ferCDCollidePlaneCap);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_PLANE, FER_CD_SHAPE_CYL,
                      (fer_cd_collide_fn)ferCDCollidePlaneCyl);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_PLANE, FER_CD_SHAPE_TRI,
                      (fer_cd_collide_fn)ferCDCollidePlaneTri);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_PLANE, FER_CD_SHAPE_TRIMESH_TRI,
                      (fer_cd_collide_fn)ferCDCollidePlaneTri);

    ferCDSetCollideFn(cd, FER_CD_SHAPE_TRI, FER_CD_SHAPE_TRI,
                      (fer_cd_collide_fn)ferCDCollideTriTri);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_TRIMESH_TRI, FER_CD_SHAPE_TRIMESH_TRI,
                      (fer_cd_collide_fn)ferCDCollideTriTri);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_TRIMESH_TRI, FER_CD_SHAPE_TRI,
                      (fer_cd_collide_fn)ferCDCollideTriTri);

    ferCDSetCollideFn(cd, FER_CD_SHAPE_OFF, FER_CD_SHAPE_OFF,
                      (fer_cd_collide_fn)ferCDCollideOffOff);
    for (i = 0; i < FER_CD_SHAPE_LEN; i++){
        if (i != FER_CD_SHAPE_OFF)
            ferCDSetCollideFn(cd, FER_CD_SHAPE_OFF, i,
                              (fer_cd_collide_fn)ferCDCollideOffAny);
    }

    // for the rest set CCD collider
    for (i = 0; i < FER_CD_SHAPE_LEN; i++){
        for (j = i; j < FER_CD_SHAPE_LEN; j++){
            if (!cd->collide[i][j] && !cd->collide[j][i]){
                ferCDSetCollideFn(cd, i, j, (fer_cd_collide_fn)ferCDCollideCCD);
            }
        }
    }


    cd->max_contacts = params->max_contacts;

    // Set up separate callbacks
    for (i = 0; i < FER_CD_SHAPE_LEN; i++){
        for (j = 0; j < FER_CD_SHAPE_LEN; j++){
            cd->separate[i][j] = NULL;
        }
    }

    ferCDSetSeparateFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_SPHERE,
                       (fer_cd_separate_fn)ferCDSeparateSphereSphere);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_PLANE, FER_CD_SHAPE_SPHERE,
                       (fer_cd_separate_fn)ferCDSeparatePlaneSphere);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_PLANE, FER_CD_SHAPE_BOX,
                       (fer_cd_separate_fn)ferCDSeparatePlaneBox);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_PLANE, FER_CD_SHAPE_CAP,
                       (fer_cd_separate_fn)ferCDSeparatePlaneCap);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_PLANE, FER_CD_SHAPE_CYL,
                       (fer_cd_separate_fn)ferCDSeparatePlaneConvex);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_PLANE, FER_CD_SHAPE_TRI,
                       (fer_cd_separate_fn)ferCDSeparatePlaneTri);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_PLANE, FER_CD_SHAPE_TRIMESH_TRI,
                       (fer_cd_separate_fn)ferCDSeparatePlaneTri);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_TRI, FER_CD_SHAPE_TRI,
                       (fer_cd_separate_fn)ferCDSeparateTriTri);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_TRI, FER_CD_SHAPE_TRIMESH_TRI,
                       (fer_cd_separate_fn)ferCDSeparateTriTri);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_TRIMESH_TRI, FER_CD_SHAPE_TRIMESH_TRI,
                       (fer_cd_separate_fn)ferCDSeparateTriTri);


    ferCDSetSeparateFn(cd, FER_CD_SHAPE_OFF, FER_CD_SHAPE_OFF,
                       (fer_cd_separate_fn)ferCDSeparateOffOff);
    for (i = 0; i < FER_CD_SHAPE_LEN; i++){
        if (i != FER_CD_SHAPE_OFF)
            ferCDSetSeparateFn(cd, FER_CD_SHAPE_OFF, i,
                               (fer_cd_separate_fn)ferCDSeparateOffAny);
    }

    // for the rest set CCD separator
    for (i = 0; i < FER_CD_SHAPE_LEN; i++){
        for (j = i; j < FER_CD_SHAPE_LEN; j++){
            if (!cd->separate[i][j] && !cd->separate[j][i]){
                ferCDSetSeparateFn(cd, i, j, ferCDSeparateCCD);
            }
        }
    }


    ferListInit(&cd->geoms);
    ferListInit(&cd->geoms_dirty);

    cd->sap = NULL;
    if (params->use_sap && params->sap_size > 0){
        cd->sap = ferCDSAPNew(params->sap_size);
    }

    return cd;
}

void ferCDDel(fer_cd_t *cd)
{
    fer_list_t *item;
    fer_cd_geom_t *g;

    while (!ferListEmpty(&cd->geoms)){
        item = ferListNext(&cd->geoms);
        g    = FER_LIST_ENTRY(item, fer_cd_geom_t, list);
        ferCDGeomDel(cd, g);
    }

    if (cd->sap)
        ferCDSAPDel(cd->sap);

    free(cd);
}

void ferCDSetCollideFn(fer_cd_t *cd, int shape1, int shape2,
                       fer_cd_collide_fn collider)
{
    cd->collide[shape1][shape2] = collider;
}

void ferCDSetSeparateFn(fer_cd_t *cd, int shape1, int shape2,
                        fer_cd_separate_fn sep)
{
    cd->separate[shape1][shape2] = sep;
}


static int ferCDCollideBruteForce(fer_cd_t *cd, fer_cd_collide_cb cb, void *data)
{
    fer_list_t *item1, *item2;
    fer_cd_geom_t *g1, *g2;
    int ret = 0;

    FER_LIST_FOR_EACH(&cd->geoms, item1){
        g1 = FER_LIST_ENTRY(item1, fer_cd_geom_t, list);

        for (item2 = ferListNext(item1);
                item2 != &cd->geoms;
                item2 = ferListNext(item2)){
            g2 = FER_LIST_ENTRY(item2, fer_cd_geom_t, list);

            if (ferCDGeomCollide(cd, g1, g2)){
                ret = 1;

                if (cb){
                    if (cb(cd, g1, g2, data) == -1)
                        return ret;
                }else{
                    return 1;
                }
            }
        }
    }

    return ret;
}

int ferCDCollide(fer_cd_t *cd, fer_cd_collide_cb cb, void *data)
{
    const fer_list_t *pairs, *item;
    fer_cd_sap_pair_t *pair;
    int ret = 0;

    // if not using SAP fallback to brute force method
    if (!cd->sap)
        return ferCDCollideBruteForce(cd, cb, data);

    // first of all update all dirty geoms
    updateDirtyGeoms(cd);

    // try all pairs
    pairs = ferCDSAPCollidePairs(cd->sap);
    FER_LIST_FOR_EACH(pairs, item){
        pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, list);

        if (ferCDGeomCollide(cd, pair->g[0], pair->g[1])){
            ret = 1;

            if (cb){
                if (cb(cd, pair->g[0], pair->g[1], data) == -1)
                    return ret;
            }else{
                return 1;
            }
        }
    }

    return ret;
}

static void ferCDSeparateBruteForce(fer_cd_t *cd,
                                   fer_cd_separate_cb cb, void *data)
{
    fer_list_t *item1, *item2;
    fer_cd_geom_t *g1, *g2;

    FER_LIST_FOR_EACH(&cd->geoms, item1){
        g1 = FER_LIST_ENTRY(item1, fer_cd_geom_t, list);

        for (item2 = ferListNext(item1);
                item2 != &cd->geoms;
                item2 = ferListNext(item2)){
            g2 = FER_LIST_ENTRY(item2, fer_cd_geom_t, list);

            ferCDGeomSeparate(cd, g1, g2, cb, data);
        }
    }
}

void ferCDSeparate(fer_cd_t *cd, fer_cd_separate_cb cb, void *data)
{
    const fer_list_t *pairs, *item;
    fer_cd_sap_pair_t *pair;

    if (!cd->sap)
        ferCDSeparateBruteForce(cd, cb, data);

    // first of all update all dirty geoms
    updateDirtyGeoms(cd);

    // try all pairs
    pairs = ferCDSAPCollidePairs(cd->sap);
    FER_LIST_FOR_EACH(pairs, item){
        pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, list);
        ferCDGeomSeparate(cd, pair->g[0], pair->g[1], cb, data);
    }
}



void ferCDDumpSVT(const fer_cd_t *cd, FILE *out, const char *name)
{
    fer_list_t *item;
    fer_cd_geom_t *g;

    FER_LIST_FOR_EACH(&cd->geoms, item){
        g = FER_LIST_ENTRY(item, fer_cd_geom_t, list);
        ferCDGeomDumpSVT(g, out, name);
    }
}

int __ferCDShapeCollide(fer_cd_t *cd,
                        const fer_cd_shape_t *s1,
                        const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                        const fer_cd_shape_t *s2,
                        const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    int type1, type2, ret;

    type1 = s1->cl->type;
    type2 = s2->cl->type;

    ret = 0;
    if (cd->collide[type1][type2]){
        ret = cd->collide[type1][type2](cd, s1, rot1, tr1, s2, rot2, tr2);
    }else if (cd->collide[type2][type1]){
        ret = cd->collide[type2][type1](cd, s2, rot2, tr2, s1, rot1, tr1);
    }else{
        fprintf(stderr, "Error: No collider for %d-%d\n", type1, type2);
    }

    return ret;
}

fer_cd_contacts_t *__ferCDShapeSeparate(struct _fer_cd_t *cd,
                                        const fer_cd_shape_t *s1,
                                        const fer_mat3_t *rot1,
                                        const fer_vec3_t *tr1,
                                        const fer_cd_shape_t *s2,
                                        const fer_mat3_t *rot2,
                                        const fer_vec3_t *tr2)
{
    int type1, type2;
    fer_cd_contacts_t *con;
    size_t i;

    type1 = s1->cl->type;
    type2 = s2->cl->type;

    con = NULL;
    if (cd->separate[type1][type2]){
        con = cd->separate[type1][type2](cd, s1, rot1, tr1, s2, rot2, tr2);
    }else if (cd->separate[type2][type1]){
        con = cd->separate[type2][type1](cd, s2, rot2, tr2, s1, rot1, tr1);
        if (con){
            for (i = 0; i < con->num; i++){
                ferVec3Scale(&con->dir[i], -FER_ONE);
            }
        }
    }else{
        fprintf(stderr, "Error: No separator for %d-%d\n", type1, type2);
    }

    return con;
}

static void updateDirtyGeoms(fer_cd_t *cd)
{
    fer_list_t *item;
    fer_cd_geom_t *g;

    while (!ferListEmpty(&cd->geoms_dirty)){
        item = ferListNext(&cd->geoms_dirty);
        g    = FER_LIST_ENTRY(item, fer_cd_geom_t, list_dirty);
        ferCDSAPUpdate(cd->sap, g);
        __ferCDGeomResetDirty(cd, g);
    }
}
