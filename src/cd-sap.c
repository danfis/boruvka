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
#include <fermat/hfunc.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** TODO **/
#include <fermat/timer.h>

#define RADIX_SORT_MASK 0xffu


static void ferCDSAPInit(fer_cd_t *cd, fer_cd_sap_t *sap, size_t buckets);
static void ferCDSAPDestroy(fer_cd_sap_t *sap);

/** Estimation of variance accros min array */
static fer_real_t ferCDSAPMinMaxVariance(const fer_cd_sap_min_t *m,
                                         size_t len);

/** Creates new pair from other fer_cd_sap_pair_t struct */
static fer_cd_sap_pair_t *pairNew(fer_cd_geom_t *g1, fer_cd_geom_t *g2);
/** Deletes collide pair */
static void pairDel(fer_cd_sap_pair_t *p);

/** Callbacks for hash table */
static uint32_t pairHash(fer_list_t *key, void *sap);
static int pairEq(const fer_list_t *key1, const fer_list_t *key2, void *sap);

/** Removes all pairs from all buckets */
static void pairRemoveAll(fer_cd_sap_t *sap);

#include "cd-sap-1.c"
#include "cd-sap-threads.c"
#include "cd-sap-gpu.c"


fer_cd_sap_t *ferCDSAPNew(fer_cd_t *cd, uint64_t flags)
{
    if (__FER_CD_SAP_GPU(flags)){
        return ferCDSAPGPUNew(cd, flags);
    }else if (__FER_CD_SAP_THREADS(flags) > 1){
        return ferCDSAPThreadsNew(cd, flags);
    }

    return ferCDSAP1New(cd, flags);
}

void ferCDSAPDel(fer_cd_sap_t *sap)
{
    if (sap->type == FER_CD_SAP_TYPE_1){
        ferCDSAP1Del(sap);
    }else if (sap->type == FER_CD_SAP_TYPE_THREADS){
        ferCDSAPThreadsDel(sap);
    }else if (sap->type == FER_CD_SAP_TYPE_GPU){
        ferCDSAPGPUDel(sap);
    }
}

void ferCDSAPAdd(fer_cd_sap_t *sap, fer_cd_geom_t *geom)
{
    int i;

    if (sap->geoms_len >= sap->geoms_alloc){
        sap->geoms_alloc *= 2;
        sap->geoms = FER_REALLOC_ARR(sap->geoms, fer_cd_sap_geom_t, sap->geoms_alloc);

        for (i = 0; i < 3; i++){
            sap->min[i] = FER_REALLOC_ARR(sap->min[i], fer_cd_sap_min_t,
                                          sap->geoms_alloc);
        }
    }

    sap->geoms[sap->geoms_len].g = geom;
    for (i = 0; i < 3; i++){
        sap->min[i][sap->geoms_len].geom = sap->geoms_len;
        sap->geoms[sap->geoms_len].min[i] = sap->geoms_len;
    }

    geom->sap = sap->geoms_len;

    sap->geoms_len++;
    sap->dirty = 1;
    sap->added++;
}

void ferCDSAPUpdate(fer_cd_sap_t *sap, fer_cd_geom_t *geom)
{
    int i;
    fer_cd_sap_geom_t *g;
    fer_real_t min, max;

    g = &sap->geoms[geom->sap];

    for (i = 0; i < 3; i++){
        // update min/max values
        __ferCDGeomSetMinMax(geom, &sap->axis[i], &min, &max);
        sap->min[i][g->min[i]].val = min;
        g->max[i] = max;
    }

    sap->dirty = 1;
}

void ferCDSAPRemove(fer_cd_sap_t *sap, fer_cd_geom_t *geom)
{
    int i;
    fer_cd_sap_geom_t *glast, *g;

    g     = &sap->geoms[geom->sap];
    glast = &sap->geoms[sap->geoms_len - 1];
    for (i = 0; i < 3; i++){
        sap->min[i][glast->min[i]].geom = geom->sap;

        sap->min[i][g->min[i]] = sap->min[i][glast->min[i]];

        glast->min[i] = g->min[i];
    }

    *g = *glast;

    sap->geoms_len--;

    sap->dirty = 1;
}


void ferCDSAPProcess(fer_cd_sap_t *sap)
{
    int i;
    fer_timer_t timer;

    if (sap->dirty){
        ferTimerStart(&timer);
        // do radix sort for all min values
        for (i = 0; i < 3; i++){
            sap->radix_sort(sap, i);
        }
        ferTimerStop(&timer);
        fprintf(stderr, "Radix Sort: %lu us\n", ferTimerElapsedInUs(&timer));

        ferTimerStart(&timer);
        // remove all current pairs - we will reevaluate it all
        pairRemoveAll(sap);

        // obtain all pairs from min-max arrays
        sap->find_pairs(sap);
        ferTimerStop(&timer);
        fprintf(stderr, "Find Pairs: %lu us\n", ferTimerElapsedInUs(&timer));
    }

    sap->dirty = 0;
    sap->added = 0;
}


static void ferCDSAPInit(fer_cd_t *cd, fer_cd_sap_t *sap, size_t buckets)
{
    int i;

    sap->cd = cd;

    // set up axis
    ferVec3Set(&sap->axis[0], FER_ONE,  FER_ONE, FER_ZERO);
    ferVec3Set(&sap->axis[1], FER_ZERO, FER_ONE,  FER_ONE);
    ferVec3Set(&sap->axis[2], FER_ONE, FER_ZERO, FER_ONE);

    // init geoms and min arrays
    sap->geoms_alloc = 100;
    sap->geoms_len = 0;
    sap->geoms = FER_ALLOC_ARR(fer_cd_sap_geom_t, sap->geoms_alloc);
    for (i = 0; i < 3; i++){
        sap->min[i] = FER_ALLOC_ARR(fer_cd_sap_min_t, sap->geoms_alloc);
    }

    sap->dirty = 0;
    sap->added = 0;

    sap->pairs = FER_ALLOC_ARR(fer_list_t, buckets);
    sap->pairs_buckets = buckets;
    for (i = 0; i < buckets; i++){
        ferListInit(&sap->pairs[i]);
    }
    sap->pairs_len = 0;
}

static void ferCDSAPDestroy(fer_cd_sap_t *sap)
{
    fer_list_t *item;
    fer_cd_sap_pair_t *pair;
    int i;

    if (sap->geoms)
        FER_FREE(sap->geoms);
    for (i = 0; i < 3; i++){
        if (sap->min[i])
            FER_FREE(sap->min[i]);
    }

    for (i = 0; i < sap->pairs_buckets; i++){
        while (!ferListEmpty(&sap->pairs[i])){
            item = ferListNext(&sap->pairs[i]);
            pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, list);
            pairDel(pair);
        }
    }
}


static fer_cd_sap_pair_t *pairNew(fer_cd_geom_t *g1, fer_cd_geom_t *g2)
{
    fer_cd_sap_pair_t *pair;

    pair = FER_ALLOC(fer_cd_sap_pair_t);
    pair->g[0] = g1;
    pair->g[1] = g2;
    ferListInit(&pair->hmap);
    ferListInit(&pair->list);

    return pair;
}

static void pairDel(fer_cd_sap_pair_t *p)
{
    // delete it from hash table
    ferListDel(&p->hmap);
    ferListDel(&p->list);
    FER_FREE(p);
}

static uint32_t pairHash(fer_list_t *key, void *sap)
{
    fer_cd_sap_pair_t *pair = FER_LIST_ENTRY(key, fer_cd_sap_pair_t, hmap);
    uint32_t k[2];

    if (pair->g[0] > pair->g[1]){
        k[0] = (uint32_t)(long)pair->g[0];
        k[1] = (uint32_t)(long)pair->g[1];
    }else{
        k[1] = (uint32_t)(long)pair->g[0];
        k[0] = (uint32_t)(long)pair->g[1];
    }

    return ferHashJenkins(k, 2, 0);
}

static int pairEq(const fer_list_t *key1, const fer_list_t *key2, void *sap)
{
    fer_cd_sap_pair_t *p1 = FER_LIST_ENTRY(key1, fer_cd_sap_pair_t, hmap);
    fer_cd_sap_pair_t *p2 = FER_LIST_ENTRY(key2, fer_cd_sap_pair_t, hmap);

    // geoms must be same
    return (p1->g[0] == p2->g[0] && p1->g[1] == p2->g[1])
            || (p1->g[0] == p2->g[1] && p1->g[1] == p2->g[0]);
}

static void pairRemoveAll(fer_cd_sap_t *sap)
{
    fer_list_t *item;
    fer_cd_sap_pair_t *pair;
    size_t i;

    for (i = 0; i < sap->pairs_buckets; i++){
        while (!ferListEmpty(&sap->pairs[i])){
            item = ferListNext(&sap->pairs[i]);
            pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, list);
            pairDel(pair);
        }
    }
    sap->pairs_len = 0;
}

static fer_real_t ferCDSAPMinMaxVariance(const fer_cd_sap_min_t *m,
                                         size_t mlen)
{
    size_t i;
    fer_real_t avg, len, var, tr1, tr2;

    tr1 = -FER_REAL_MAX / FER_REAL(2.);
    tr2 =  FER_REAL_MAX / FER_REAL(2.);
    avg = len = var = 0;
    for (i = 0; i < mlen; ++i, ++m){
        if (m->val > tr1 && m->val < tr2){
            avg += m->val;
            len += FER_ONE;
            var += FER_CUBE(m->val);
        }
    }

    avg /= len;
    var /= len;
    var -= FER_CUBE(avg);

    return var;
}
