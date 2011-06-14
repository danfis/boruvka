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


/** Creates new pair from other fer_cd_sap_pair_t struct */
static fer_cd_sap_pair_t *pairNew(const fer_cd_sap_pair_t *p);
/** Deletes collide pair */
static void pairDel(fer_cd_sap_pair_t *p);

/** Callbacks for hash table */
static uint32_t pairHash(fer_list_t *key, void *sap);
static int pairEq(const fer_list_t *key1, const fer_list_t *key2, void *sap);

/** Adds/Removes collide pair to hash table and possible to .collide_pairs
 *  list */
static void pairAdd(fer_cd_sap_t *sap, fer_cd_geom_t *g1, fer_cd_geom_t *g2);
static void pairRemove(fer_cd_sap_t *sap, fer_cd_geom_t *g1, fer_cd_geom_t *g2);

/** Struct representing min or max value of geom along some axis */
struct _fer_cd_sap_geom_minmax_t {
    fer_real_t val;  /*!< Min/Max value */
    fer_list_t list; /*!< Connection into sap.minmax[] list */
    uint16_t flags;  /*!< Lowest bit is 0 or 1 if this is min or max value
                          respectively.
                          Highest 15 bits is precomputed offset in
                          fer_cd_sap_geom_t structure. */
} fer_packed;
typedef struct _fer_cd_sap_geom_minmax_t fer_cd_sap_geom_minmax_t;

/** Macros for handling geom_minmax.flags */
#define MINMAX_FLAG(m) ((m)->flags & 0x1)
#define MINMAX_IS_MIN(m) (MINMAX_FLAG(m) == 0)
#define MINMAX_IS_MAX(m) (MINMAX_FLAG(m) == 1)
#define MINMAX_OFFSET(m) ((m)->flags >> 1)
#define SAP_GEOM_FROM_MINMAX(m) \
    (fer_cd_sap_geom_t *)((char *)(m) - MINMAX_OFFSET(m))


/** Data SAP needs to know about geom.
 *  Note that each geom has (void *) pointer .sap that points at this
 *  struct (or is NULL) */
struct _fer_cd_sap_geom_t {
    fer_cd_geom_t *geom; /*!< Reference to geom */
    fer_cd_sap_geom_minmax_t min[FER_CD_SAP_NUM_AXIS]; /*!< Min values */
    fer_cd_sap_geom_minmax_t max[FER_CD_SAP_NUM_AXIS]; /*!< Max values */
};
typedef struct _fer_cd_sap_geom_t fer_cd_sap_geom_t;

/** Creates and deletes sap_geom struct */
static fer_cd_sap_geom_t *sapGeomNew(fer_cd_geom_t *g);
static void sapGeomDel(fer_cd_sap_geom_t *);


fer_cd_sap_t *ferCDSAPNew(size_t hash_table_size)
{
    fer_cd_sap_t *sap;

    sap = FER_ALLOC_ALIGN(fer_cd_sap_t, 16);
    sap->pairs = ferHMapNew(hash_table_size, pairHash, pairEq, (void *)sap);

    // set up axis
    ferVec3Set(&sap->axis[0], FER_ONE,  FER_ONE, FER_ZERO);
    ferVec3Set(&sap->axis[1], FER_ZERO, FER_ONE,  FER_ONE);
    ferVec3Set(&sap->axis[2], FER_ONE, FER_ZERO, FER_ONE);

    // initialize lists
    ferListInit(&sap->minmax[0]);
    ferListInit(&sap->minmax[1]);
    ferListInit(&sap->minmax[2]);

    // initialize list of possible collide pairs
    ferListInit(&sap->collide_pairs);
    sap->collide_pairs_len = 0;

    return sap;
}

void ferCDSAPDel(fer_cd_sap_t *sap)
{
    fer_list_t allpairs, *item;
    fer_cd_sap_pair_t *pair;

    // delete all pairs from hash table
    ferListInit(&allpairs);
    ferHMapGather(sap->pairs, &allpairs);
    while (!ferListEmpty(&allpairs)){
        item = ferListNext(&allpairs);
        ferListDel(item);
        pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, hmap);
        pairDel(pair);
    }

    ferHMapDel(sap->pairs);
    free(sap);
}


#define updateBubbleUp(i, m1, item, m2) \
    for (item = ferListNext(&m1->list), \
         m2   = FER_LIST_ENTRY(item, fer_cd_sap_geom_minmax_t, list); \
         item != &sap->minmax[i] && m2->val < m1->val; \
         item = ferListNext(&m1->list), \
         m2   = FER_LIST_ENTRY(item, fer_cd_sap_geom_minmax_t, list))

#define updateBubbleDown(i, m1, item, m2) \
    for (item = ferListPrev(&m1->list), \
         m2   = FER_LIST_ENTRY(item, fer_cd_sap_geom_minmax_t, list); \
         item != &sap->minmax[i] && m2->val > m1->val; \
         item = ferListPrev(&m1->list), \
         m2   = FER_LIST_ENTRY(item, fer_cd_sap_geom_minmax_t, list))

static int updateBubbleUpMin(fer_cd_sap_t *sap, int i,
                             fer_cd_sap_geom_minmax_t *m1)
{
    fer_list_t *item;
    fer_cd_sap_geom_minmax_t *m2;
    fer_cd_sap_geom_t *g1, *g2;
    int ret = 0;

    g1 = SAP_GEOM_FROM_MINMAX(m1);
    updateBubbleUp(i, m1, item, m2){
        ferListDel(&m1->list);
        ferListPrepend(&m2->list, &m1->list);
        g2 = SAP_GEOM_FROM_MINMAX(m2);

        // min can't be places after max!
        if (g1 == g2)
            break;

        ret = 1;
        if (MINMAX_IS_MAX(m2)){
            pairRemove(sap, g1->geom, g2->geom);
        }
    }

    return ret;
}

static int updateBubbleDownMin(fer_cd_sap_t *sap, int i,
                               fer_cd_sap_geom_minmax_t *m1)
{
    fer_list_t *item;
    fer_cd_sap_geom_minmax_t *m2;
    fer_cd_sap_geom_t *g1, *g2;
    int ret = 0;

    g1 = SAP_GEOM_FROM_MINMAX(m1);
    updateBubbleDown(i, m1, item, m2){
        ferListDel(&m1->list);
        ferListAppend(&m2->list, &m1->list);
        g2 = SAP_GEOM_FROM_MINMAX(m2);
        ret = 1;

        if (g1 == g2)
            continue;

        if (MINMAX_IS_MAX(m2)){
            pairAdd(sap, g1->geom, g2->geom);
        }
    }

    return ret;
}

static int updateBubbleUpMax(fer_cd_sap_t *sap, int i,
                             fer_cd_sap_geom_minmax_t *m1)
{
    fer_list_t *item;
    fer_cd_sap_geom_minmax_t *m2;
    fer_cd_sap_geom_t *g1, *g2;
    int ret = 0;

    g1 = SAP_GEOM_FROM_MINMAX(m1);
    updateBubbleUp(i, m1, item, m2){
        ferListDel(&m1->list);
        ferListPrepend(&m2->list, &m1->list);
        g2 = SAP_GEOM_FROM_MINMAX(m2);
        ret = 1;

        if (g1 == g2)
            continue;

        if (MINMAX_IS_MIN(m2)){
            pairAdd(sap, g1->geom, g2->geom);
        }
    }

    return ret;
}

static int updateBubbleDownMax(fer_cd_sap_t *sap, int i,
                               fer_cd_sap_geom_minmax_t *m1)
{
    fer_list_t *item;
    fer_cd_sap_geom_minmax_t *m2;
    fer_cd_sap_geom_t *g1, *g2;
    int ret = 0;

    g1 = SAP_GEOM_FROM_MINMAX(m1);
    updateBubbleDown(i, m1, item, m2){
        ferListDel(&m1->list);
        ferListAppend(&m2->list, &m1->list);
        g2 = SAP_GEOM_FROM_MINMAX(m2);

        // max can't be placed before min
        if (g1 == g2)
            break;

        ret = 1;
        if (MINMAX_IS_MIN(m2)){
            pairRemove(sap, g1->geom, g2->geom);
        }
    }

    return ret;
}


void ferCDSAPUpdate(fer_cd_sap_t *sap, fer_cd_geom_t *geom)
{
    int i;
    fer_real_t min, max;
    fer_cd_sap_geom_t *g;

    // create geom_sap struct if not already created
    g = geom->sap;
    if (!g)
        g = sapGeomNew(geom);

    for (i = 0; i < FER_CD_SAP_NUM_AXIS; i++){
        // update min/max values
        __ferCDGeomSetMinMax(geom, &sap->axis[i], &min, &max);
        g->min[i].val = min;
        g->max[i].val = max;

        // add geom to list of not already there
        if (ferListEmpty(&g->min[i].list)){
            ferListPrepend(&sap->minmax[i], &g->max[i].list);
            ferListPrepend(&sap->minmax[i], &g->min[i].list);
        }

        // move min and max breakpoints to correct place
        updateBubbleDownMin(sap, i, &g->min[i]);
        updateBubbleUpMax(sap, i, &g->max[i]);
        updateBubbleUpMin(sap, i, &g->min[i]);
        updateBubbleDownMax(sap, i, &g->max[i]);
    }
}

void ferCDSAPRemove(fer_cd_sap_t *sap, fer_cd_geom_t *geom)
{
    if (geom->sap)
        sapGeomDel(geom->sap);
}

void ferCDSAPDumpPairs(fer_cd_sap_t *sap, FILE *out)
{
    fer_list_t *list, *item;
    fer_cd_sap_pair_t *pair;
    size_t i;

    for (i = 0; i < sap->pairs->size; i++){
        list = &sap->pairs->table[i];

        FER_LIST_FOR_EACH(list, item){
            pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, hmap);
            fprintf(out, "[%04d]: (%lx) g[0]: %lx, g[1]: %lx, num_axis: %d\n",
                    (int)i, (long)pair, (long)pair->g[0], (long)pair->g[1],
                    pair->num_axis);
        }
    }

    {
        fer_cd_sap_geom_minmax_t *minmax;
        fer_cd_sap_geom_t *g;
        for (i = 0; i < 3; i++){
            list = &sap->minmax[i];
            FER_LIST_FOR_EACH(list, item){
                minmax = FER_LIST_ENTRY(item, fer_cd_sap_geom_minmax_t, list);
                g = SAP_GEOM_FROM_MINMAX(minmax);
                fprintf(out, ".minmax[%d]: geom: %lx, m: %d, val: %f\n",
                        i, (long)g->geom, (int)MINMAX_FLAG(minmax), minmax->val);
            }
        }
    }

    {
        fprintf(out, ".collide_pairs:\n");
        FER_LIST_FOR_EACH(&sap->collide_pairs, item){
            pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, list);
            fprintf(out, "    (%lx) g[0]: %lx, g[1]: %lx, num_axis: %d\n",
                    (long)pair, (long)pair->g[0], (long)pair->g[1],
                    pair->num_axis);
        }
    }
}



static fer_cd_sap_pair_t *pairNew(const fer_cd_sap_pair_t *p)
{
    fer_cd_sap_pair_t *pair;

    pair = FER_ALLOC(fer_cd_sap_pair_t);
    pair->g[0] = p->g[0];
    pair->g[1] = p->g[1];
    pair->num_axis = 0;
    ferListInit(&pair->hmap);
    ferListInit(&pair->list);

    return pair;
}

static void pairDel(fer_cd_sap_pair_t *p)
{
    // delete it from hash table
    ferListDel(&p->hmap);
    free(p);
}

static uint32_t pairHash(fer_list_t *key, void *sap)
{
    fer_cd_sap_pair_t *pair = FER_LIST_ENTRY(key, fer_cd_sap_pair_t, hmap);
    uint32_t k[2];

    if (pair->g[0] > pair->g[1]){
        k[0] = (uint32_t)pair->g[0];
        k[1] = (uint32_t)pair->g[1];
    }else{
        k[1] = (uint32_t)pair->g[0];
        k[0] = (uint32_t)pair->g[1];
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

static void pairAdd(fer_cd_sap_t *sap, fer_cd_geom_t *g1, fer_cd_geom_t *g2)
{
    fer_cd_sap_pair_t pair, *p;
    fer_list_t *hmap;

    pair.g[0] = g1;
    pair.g[1] = g2;

    hmap = ferHMapGet(sap->pairs, &pair.hmap);
    if (hmap){
        p = FER_LIST_ENTRY(hmap, fer_cd_sap_pair_t, hmap);
    }else{
        p = pairNew(&pair);
        ferHMapPut(sap->pairs, &p->hmap);
    }
    p->num_axis += 1;

    if (p->num_axis == FER_CD_SAP_NUM_AXIS){
        ferListAppend(&sap->collide_pairs, &p->list);
        sap->collide_pairs_len++;
    }
}

static void pairRemove(fer_cd_sap_t *sap, fer_cd_geom_t *g1, fer_cd_geom_t *g2)
{
    fer_cd_sap_pair_t pair, *p;
    fer_list_t *hmap;

    pair.g[0] = g1;
    pair.g[1] = g2;

    hmap = ferHMapGet(sap->pairs, &pair.hmap);
    if (hmap){
        p = FER_LIST_ENTRY(hmap, fer_cd_sap_pair_t, hmap);
        p->num_axis -= 1;

        if (p->num_axis == FER_CD_SAP_NUM_AXIS - 1){
            ferListDel(&p->list);
            sap->collide_pairs_len--;
        }

        if (p->num_axis == 0){
            // we don't need to call ferHMapRemove() because it is safe to
            // remove a pair from hash table silently by disconnecting
            // .hmap list item - which is done in pairDel().
            //ferHMapRemove(sap->pairs, &p->hmap);
            pairDel(p);
        }
    }
}


static fer_cd_sap_geom_t *sapGeomNew(fer_cd_geom_t *geom)
{
    fer_cd_sap_geom_t *g;
    int i;
    uint16_t offset;

    if (geom->sap)
        sapGeomDel(geom->sap);

    g = FER_ALLOC(fer_cd_sap_geom_t);
    g->geom = geom;
    geom->sap = g;

    for (i = 0; i < FER_CD_SAP_NUM_AXIS; i++){
        ferListInit(&g->min[i].list);
        g->min[i].flags = 0;

        // compute offset
        offset  = fer_offsetof(fer_cd_sap_geom_t, min);
        offset += sizeof(fer_cd_sap_geom_minmax_t) * i;
        g->min[i].flags |= (offset << 1);


        ferListInit(&g->max[i].list);
        g->max[i].flags = 1;

        // compute offset
        offset  = fer_offsetof(fer_cd_sap_geom_t, max);
        offset += sizeof(fer_cd_sap_geom_minmax_t) * i;
        g->max[i].flags |= (offset << 1);
    }

    return g;
}

static void sapGeomDel(fer_cd_sap_geom_t *g)
{
    int i;

    for (i = 0; i < FER_CD_SAP_NUM_AXIS; i++){
        ferListDel(&g->min[i].list);
        ferListDel(&g->max[i].list);
    }

    g->geom->sap = NULL;

    free(g);
}
