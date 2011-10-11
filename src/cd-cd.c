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
    params->sap_gpu = 0;
    params->sap_hashsize = 1023;

    params->max_contacts = 20;
    params->num_threads = 1;

    params->use_cp          = 1;
    params->cp_hashsize     = 0;
    params->cp_max_dist     = 1E-6;
}

fer_cd_t *ferCDNew(const fer_cd_params_t *params)
{
    fer_cd_t *cd;
    size_t i, j;
    size_t num_threads;
    fer_cd_params_t __params;
    uint64_t sapflags;

    if (!params){
        ferCDParamsInit(&__params);
        params = &__params;
    }

    num_threads = params->num_threads;
    if (num_threads == 0)
        num_threads = 1;

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
    ferCDSetCollideFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_CYL,
                      (fer_cd_collide_fn)ferCDCollideCCD);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_TRI,
                      (fer_cd_collide_fn)ferCDCollideSphereTri);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_TRIMESH_TRI,
                      (fer_cd_collide_fn)ferCDCollideSphereTri);

    ferCDSetCollideFn(cd, FER_CD_SHAPE_BOX, FER_CD_SHAPE_BOX,
                      (fer_cd_collide_fn)ferCDCollideBoxBox);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_BOX, FER_CD_SHAPE_CAP,
                      (fer_cd_collide_fn)ferCDCollideCCD);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_BOX, FER_CD_SHAPE_CYL,
                      (fer_cd_collide_fn)ferCDCollideCCD);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_BOX, FER_CD_SHAPE_TRI,
                      (fer_cd_collide_fn)ferCDCollideCCD);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_BOX, FER_CD_SHAPE_TRIMESH_TRI,
                      (fer_cd_collide_fn)ferCDCollideCCD);

    ferCDSetCollideFn(cd, FER_CD_SHAPE_CAP, FER_CD_SHAPE_CAP,
                      (fer_cd_collide_fn)ferCDCollideCapCap);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_CAP, FER_CD_SHAPE_CYL,
                      (fer_cd_collide_fn)ferCDCollideCCD);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_CAP, FER_CD_SHAPE_TRI,
                      (fer_cd_collide_fn)ferCDCollideCCD);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_CAP, FER_CD_SHAPE_TRIMESH_TRI,
                      (fer_cd_collide_fn)ferCDCollideCCD);

    ferCDSetCollideFn(cd, FER_CD_SHAPE_CYL, FER_CD_SHAPE_CYL,
                      (fer_cd_collide_fn)ferCDCollideCCD);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_CYL, FER_CD_SHAPE_TRI,
                      (fer_cd_collide_fn)ferCDCollideCCD);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_CYL, FER_CD_SHAPE_TRIMESH_TRI,
                      (fer_cd_collide_fn)ferCDCollideCCD);

    ferCDSetCollideFn(cd, FER_CD_SHAPE_TRI, FER_CD_SHAPE_TRI,
                      (fer_cd_collide_fn)ferCDCollideTriTri);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_TRIMESH_TRI, FER_CD_SHAPE_TRIMESH_TRI,
                      (fer_cd_collide_fn)ferCDCollideTriTri);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_TRIMESH_TRI, FER_CD_SHAPE_TRI,
                      (fer_cd_collide_fn)ferCDCollideTriTri);

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


    ferCDSetCollideFn(cd, FER_CD_SHAPE_OFF, FER_CD_SHAPE_OFF,
                      (fer_cd_collide_fn)ferCDCollideOffOff);
    for (i = 0; i < FER_CD_SHAPE_LEN; i++){
        if (i != FER_CD_SHAPE_OFF)
            ferCDSetCollideFn(cd, FER_CD_SHAPE_OFF, i,
                              (fer_cd_collide_fn)ferCDCollideOffAny);
    }


    cd->tasks = NULL;
    if (num_threads > 1){
        cd->tasks = ferTasksNew(num_threads);
        ferTasksRun(cd->tasks);
    }
    pthread_mutex_init(&cd->lock, NULL);

    cd->max_contacts = params->max_contacts;

    cd->contacts_len = num_threads;
    cd->contacts = FER_ALLOC_ARR(fer_cd_contacts_t *, num_threads);
    for (i = 0; i < num_threads; i++){
        cd->contacts[i] = ferCDContactsNew(cd->max_contacts);
    }

    // Set up separate callbacks
    for (i = 0; i < FER_CD_SHAPE_LEN; i++){
        for (j = 0; j < FER_CD_SHAPE_LEN; j++){
            cd->separate[i][j] = NULL;
        }
    }

    ferCDSetSeparateFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_SPHERE,
                       (fer_cd_separate_fn)ferCDSeparateSphereSphere);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_BOX,
                       (fer_cd_separate_fn)ferCDSeparateSphereBox);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_CYL,
                       (fer_cd_separate_fn)ferCDSeparateSphereCyl);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_CAP,
                       (fer_cd_separate_fn)ferCDSeparateSphereCap);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_TRI,
                       (fer_cd_separate_fn)ferCDSeparateSphereTri);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_TRIMESH_TRI,
                       (fer_cd_separate_fn)ferCDSeparateSphereTri);

    ferCDSetSeparateFn(cd, FER_CD_SHAPE_BOX, FER_CD_SHAPE_BOX,
                       (fer_cd_separate_fn)ferCDSeparateCCD);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_BOX, FER_CD_SHAPE_CYL,
                       (fer_cd_separate_fn)ferCDSeparateCCD);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_BOX, FER_CD_SHAPE_CAP,
                       (fer_cd_separate_fn)ferCDSeparateCCD);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_BOX, FER_CD_SHAPE_TRI,
                       (fer_cd_separate_fn)ferCDSeparateCCD);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_BOX, FER_CD_SHAPE_TRIMESH_TRI,
                       (fer_cd_separate_fn)ferCDSeparateCCD);

    ferCDSetSeparateFn(cd, FER_CD_SHAPE_CYL, FER_CD_SHAPE_CYL,
                       (fer_cd_separate_fn)ferCDSeparateCCD);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_CYL, FER_CD_SHAPE_CAP,
                       (fer_cd_separate_fn)ferCDSeparateCCD);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_CYL, FER_CD_SHAPE_TRI,
                       (fer_cd_separate_fn)ferCDSeparateCCD);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_CYL, FER_CD_SHAPE_TRIMESH_TRI,
                       (fer_cd_separate_fn)ferCDSeparateCCD);

    ferCDSetSeparateFn(cd, FER_CD_SHAPE_CAP, FER_CD_SHAPE_CAP,
                       (fer_cd_separate_fn)ferCDSeparateCapCap);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_CAP, FER_CD_SHAPE_TRI,
                       (fer_cd_separate_fn)ferCDSeparateCCD);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_CAP, FER_CD_SHAPE_TRIMESH_TRI,
                       (fer_cd_separate_fn)ferCDSeparateCCD);

    ferCDSetSeparateFn(cd, FER_CD_SHAPE_TRI, FER_CD_SHAPE_TRI,
                       (fer_cd_separate_fn)ferCDSeparateTriTri);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_TRI, FER_CD_SHAPE_TRIMESH_TRI,
                       (fer_cd_separate_fn)ferCDSeparateTriTri);
    ferCDSetSeparateFn(cd, FER_CD_SHAPE_TRIMESH_TRI, FER_CD_SHAPE_TRIMESH_TRI,
                       (fer_cd_separate_fn)ferCDSeparateTriTri);

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


    ferCDSetSeparateFn(cd, FER_CD_SHAPE_OFF, FER_CD_SHAPE_OFF,
                       (fer_cd_separate_fn)ferCDSeparateOffOff);
    for (i = 0; i < FER_CD_SHAPE_LEN; i++){
        if (i != FER_CD_SHAPE_OFF)
            ferCDSetSeparateFn(cd, FER_CD_SHAPE_OFF, i,
                               (fer_cd_separate_fn)ferCDSeparateOffAny);
    }


    ferListInit(&cd->geoms);
    cd->geoms_len = 0;
    ferListInit(&cd->geoms_dirty);
    cd->geoms_dirty_len = 0;

    cd->sap = NULL;
    if (params->use_sap && params->sap_hashsize > 0){
        sapflags = FER_CD_SAP_HASH_TABLE_SIZE(params->sap_hashsize);
        if (num_threads > 1)
            sapflags |= FER_CD_SAP_THREADS(num_threads);
        if (params->sap_gpu)
            sapflags |= FER_CD_SAP_GPU;

        cd->sap = ferCDSAPNew(cd, sapflags);
    }

    cd->cp = NULL;
    if (params->use_cp){
        if (params->cp_hashsize == 0){
            cd->cp = ferCDCPNew(params->sap_hashsize,
                                params->cp_max_dist);
        }else{
            cd->cp = ferCDCPNew(params->cp_hashsize,
                                params->cp_max_dist);
        }
    }

    return cd;
}

void ferCDDel(fer_cd_t *cd)
{
    fer_list_t *item;
    fer_cd_geom_t *g;
    size_t i;

    while (!ferListEmpty(&cd->geoms)){
        item = ferListNext(&cd->geoms);
        g    = FER_LIST_ENTRY(item, fer_cd_geom_t, list);
        ferCDGeomDel(cd, g);
    }

    if (cd->contacts){
        for (i = 0; i < cd->contacts_len; i++){
            ferCDContactsDel(cd->contacts[i]);
        }
        FER_FREE(cd->contacts);
    }

    if (cd->tasks)
        ferTasksDel(cd->tasks);
    pthread_mutex_destroy(&cd->lock);

    if (cd->sap)
        ferCDSAPDel(cd->sap);
    if (cd->cp)
        ferCDCPDel(cd->cp);

    FER_FREE(cd);
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
    size_t i, len;

    // if not using SAP fallback to brute force method
    if (!cd->sap)
        return ferCDCollideBruteForce(cd, cb, data);

    // first of all update all dirty geoms
    updateDirtyGeoms(cd);

    // try all pairs
    len = ferCDSAPCollidePairsBuckets(cd->sap);
    for (i = 0; i < len; i++){
        pairs = ferCDSAPCollidePairs(cd->sap, i);
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
    }

    return ret;
}

struct _sep_task_t {
    fer_cd_t *cd;
    fer_cd_separate_cb cb;
    void *data;
    int size;
};
typedef struct _sep_task_t sep_task_t;

static void __sepTask(int id, void *data,
                      const fer_tasks_thinfo_t *thinfo)
{
    sep_task_t *sep = (sep_task_t *)data;
    fer_cd_t *cd = sep->cd;
    fer_cd_contacts_t *con = cd->contacts[thinfo->id - 1];
    const fer_cd_contacts_t *con2;
    const fer_list_t *pairs, *item;
    fer_cd_sap_pair_t *pair;

    pairs = ferCDSAPCollidePairs(cd->sap, id);
    FER_LIST_FOR_EACH(pairs, item){
        pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, list);

        con->num = 0;
        ferCDGeomSeparate(cd, pair->g[0], pair->g[1], con);
        if (con->num > 0){
            con2 = ferCDCPUpdate(cd->cp, pair->g[0], pair->g[1], con);
            sep->cb(cd, pair->g[0], pair->g[1], con2, sep->data);
        }
    }
}

static void __sepTaskBruteForce(int id, void *data,
                                const fer_tasks_thinfo_t *thinfo)
{
    sep_task_t *sep = (sep_task_t *)data;
    fer_cd_t *cd = sep->cd;
    fer_cd_contacts_t *con = cd->contacts[id];
    const fer_cd_contacts_t *con2;
    fer_list_t *item1, *item2;
    fer_cd_geom_t *g1, *g2;
    size_t i;

    i = 0;
    FER_LIST_FOR_EACH(&cd->geoms, item1){
        g1 = FER_LIST_ENTRY(item1, fer_cd_geom_t, list);

        for (item2 = ferListNext(item1);
                i % sep->size == id && item2 != &cd->geoms;
                item2 = ferListNext(item2)){
            g2 = FER_LIST_ENTRY(item2, fer_cd_geom_t, list);

            con->num = 0;
            ferCDGeomSeparate(cd, g1, g2, con);
            if (con->num > 0){
                con2 = ferCDCPUpdate(cd->cp, g1, g2, con);
                sep->cb(cd, g1, g2, con2, sep->data);
            }
        }

        i++;
    }
}

static void ferCDSeparateBruteForce(fer_cd_t *cd,
                                    fer_cd_separate_cb cb, void *data)
{
    fer_list_t *item1, *item2;
    fer_cd_geom_t *g1, *g2;
    const fer_cd_contacts_t *con2;

    FER_LIST_FOR_EACH(&cd->geoms, item1){
        g1 = FER_LIST_ENTRY(item1, fer_cd_geom_t, list);

        for (item2 = ferListNext(item1);
                item2 != &cd->geoms;
                item2 = ferListNext(item2)){
            g2 = FER_LIST_ENTRY(item2, fer_cd_geom_t, list);

            cd->contacts[0]->num = 0;
            ferCDGeomSeparate(cd, g1, g2, cd->contacts[0]);
            if (cd->contacts[0]->num > 0){
                con2 = ferCDCPUpdate(cd->cp, g1, g2, cd->contacts[0]);
                cb(cd, g1, g2, con2, data);
            }
        }
    }

    if (cd->cp)
        ferCDCPMaintainance(cd->cp);
}

static void ferCDSeparateBruteForceThreads(fer_cd_t *cd,
                                           fer_cd_separate_cb cb, void *data)
{
    size_t i;
    sep_task_t s;

    s.cd = cd;
    s.cb = cb;
    s.data = data;
    s.size = ferTasksSize(cd->tasks);

    for (i = 0; i < s.size; i++){
        ferTasksAdd(cd->tasks, __sepTaskBruteForce, (int)i, &s);
    }

    ferTasksBarrier(cd->tasks);

    if (cd->cp)
        ferCDCPMaintainance(cd->cp);
}

static void ferCDSeparateSAP(fer_cd_t *cd,
                             fer_cd_separate_cb cb, void *data)
{
    const fer_list_t *pairs, *item;
    fer_cd_sap_pair_t *pair;
    fer_cd_contacts_t *con;
    const fer_cd_contacts_t *con2;
    size_t i, len;
    int num;

    // first of all update all dirty geoms
    updateDirtyGeoms(cd);

    // use first in contacts list
    con = cd->contacts[0];

    // try all pairs
    len = ferCDSAPCollidePairsBuckets(cd->sap);
    for (i = 0; i < len; i++){
        pairs = ferCDSAPCollidePairs(cd->sap, i);
        FER_LIST_FOR_EACH(pairs, item){
            pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, list);

            con->num = 0;
            num = ferCDGeomSeparate(cd, pair->g[0], pair->g[1], con);
            if (num > 0){
                con2 = ferCDCPUpdate(cd->cp, pair->g[0], pair->g[1], con);
                cb(cd, pair->g[0], pair->g[1], con2, data);
            }
        }
    }

    if (cd->cp)
        ferCDCPMaintainance(cd->cp);
}

static void ferCDSeparateSAPThreads(fer_cd_t *cd,
                                    fer_cd_separate_cb cb, void *data)
{
    size_t i;
    sep_task_t s;

    // first of all update all dirty geoms
    updateDirtyGeoms(cd);

    s.cd = cd;
    s.cb = cb;
    s.data = data;
    //s.size = ferTasksSize(cd->tasks);
    s.size = ferCDSAPCollidePairsBuckets(cd->sap);

    for (i = 0; i < s.size; i++){
        ferTasksAdd(cd->tasks, __sepTask, (int)i, &s);
    }

    ferTasksBarrier(cd->tasks);

    if (cd->cp)
        ferCDCPMaintainance(cd->cp);
}

void ferCDSeparate(fer_cd_t *cd, fer_cd_separate_cb cb, void *data)
{
    if (!cd->sap){
        if (cd->tasks){
            ferCDSeparateBruteForceThreads(cd, cb, data);
        }else{
            ferCDSeparateBruteForce(cd, cb, data);
        }
    }else{
        if (cd->tasks){
            ferCDSeparateSAPThreads(cd, cb, data);
        }else{
            ferCDSeparateSAP(cd, cb, data);
        }
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

int __ferCDShapeSeparate(struct _fer_cd_t *cd,
                         const fer_cd_shape_t *s1,
                         const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                         const fer_cd_shape_t *s2,
                         const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                         fer_cd_contacts_t *con)
{
    int type1, type2, num = 0;
    size_t i;

    type1 = s1->cl->type;
    type2 = s2->cl->type;

    if (cd->separate[type1][type2]){
        num = cd->separate[type1][type2](cd, s1, rot1, tr1, s2, rot2, tr2, con);
    }else if (cd->separate[type2][type1]){
        num = cd->separate[type2][type1](cd, s2, rot2, tr2, s1, rot1, tr1, con);
        if (num > 0){
            for (i = 0; i < num; i++){
                ferVec3Scale(&con->dir[con->num - i - 1], -FER_ONE);
            }
        }
    }else{
        fprintf(stderr, "Error: No separator for %d-%d\n", type1, type2);
    }

    return num;
}

#include <fermat/timer.h>
static void updateDirtyGeoms(fer_cd_t *cd)
{
    fer_list_t *item;
    fer_cd_geom_t *g;

    fer_timer_t timer;
    ferTimerStart(&timer);
    while (!ferListEmpty(&cd->geoms_dirty)){
        item = ferListNext(&cd->geoms_dirty);
        g    = FER_LIST_ENTRY(item, fer_cd_geom_t, list_dirty);
        ferCDSAPUpdate(cd->sap, g);
        __ferCDGeomResetDirty(cd, g);
    }
    ferTimerStop(&timer);
    DBG("%lu us", ferTimerElapsedInUs(&timer));

    ferCDSAPProcess(cd->sap);
}
