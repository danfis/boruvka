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


#define MINMAX_ISMAX(minmax) \
    ((minmax)->geom_ismax & 0x1)
#define MINMAX_GEOM(minmax) \
    ((minmax)->geom_ismax >> 0x1)

/** Process all geoms - radix sort approach */
static void ferCDSAPProcessAll(fer_cd_t *cd, fer_cd_sap_t *sap);

/** Create and delete radix sort struct */
static fer_cd_sap_radix_sort_t *radixSortNew(void);
static void radixSortDel(fer_cd_sap_radix_sort_t *rs);
/** Sorts min-max values along specified axis */
static void radixSort(fer_cd_sap_t *sap, int axis);

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



fer_cd_sap_t *ferCDSAPNew(size_t buckets, size_t hash_table_size)
{
    fer_cd_sap_t *sap;
    size_t i;

    sap = FER_ALLOC_ALIGN(fer_cd_sap_t, 16);
    sap->pairs = ferHMapNew(hash_table_size, pairHash, pairEq, (void *)sap);

    // set up axis
    ferVec3Set(&sap->axis[0], FER_ONE,  FER_ONE, FER_ZERO);
    ferVec3Set(&sap->axis[1], FER_ZERO, FER_ONE,  FER_ONE);
    ferVec3Set(&sap->axis[2], FER_ONE, FER_ZERO, FER_ONE);

    // init geoms and minmax arrays
    sap->geoms_alloc = 100;
    sap->geoms_len = 0;
    sap->geoms = FER_ALLOC_ARR(fer_cd_sap_geom_t, sap->geoms_alloc);
    for (i = 0; i < FER_CD_SAP_NUM_AXIS; i++){
        sap->minmax[i] = FER_ALLOC_ARR(fer_cd_sap_minmax_t,
                                       sap->geoms_alloc * 2);
    }

    sap->radix_sort = radixSortNew();

    sap->added = 0;


    // initialize list of possible collide pairs
    sap->collide_pairs = FER_ALLOC_ARR(fer_list_t, buckets);
    sap->collide_pairs_buckets = buckets;
    sap->collide_pairs_len = 0;
    sap->collide_pairs_next = 0;
    for (i = 0; i < buckets; i++){
        ferListInit(&sap->collide_pairs[i]);
    }

    return sap;
}

void ferCDSAPDel(fer_cd_sap_t *sap)
{
    fer_list_t allpairs, *item;
    fer_cd_sap_pair_t *pair;
    int i;

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

    if (sap->geoms)
        free(sap->geoms);
    for (i = 0; i < FER_CD_SAP_NUM_AXIS; i++){
        if (sap->minmax[i])
            free(sap->minmax[i]);
    }

    if (sap->radix_sort)
        radixSortDel(sap->radix_sort);

    if (sap->collide_pairs)
        free(sap->collide_pairs);

    free(sap);
}

void ferCDSAPAdd(fer_cd_sap_t *sap, fer_cd_geom_t *geom)
{
    int i;

    if (sap->geoms_len >= sap->geoms_alloc){
        sap->geoms_alloc *= 2;
        sap->geoms = FER_REALLOC_ARR(sap->geoms, fer_cd_sap_geom_t, sap->geoms_alloc);

        for (i = 0; i < FER_CD_SAP_NUM_AXIS; i++){
            sap->minmax[i] = FER_REALLOC_ARR(sap->minmax[i],
                                             fer_cd_sap_minmax_t,
                                             sap->geoms_alloc * 2);
        }
    }

    sap->geoms[sap->geoms_len].g = geom;
    for (i = 0; i < FER_CD_SAP_NUM_AXIS; i++){
        sap->minmax[i][sap->geoms_len * 2].geom_ismax = sap->geoms_len << 1;
        sap->minmax[i][sap->geoms_len * 2 + 1].geom_ismax = sap->geoms_len << 1;
        sap->minmax[i][sap->geoms_len * 2 + 1].geom_ismax |= 0x1;
        sap->geoms[sap->geoms_len].min[i] = sap->geoms_len * 2;
        sap->geoms[sap->geoms_len].max[i] = sap->geoms_len * 2 + 1;
    }

    geom->sap = sap->geoms_len;

    sap->geoms_len++;
    sap->added++;
}

static void ferCDSAPUpdateMinMax(fer_cd_sap_t *sap, fer_cd_geom_t *geom,
                                 int axis)
{
    fer_cd_sap_minmax_t *minmax, *minmax_last, *min, *max, tmp;

    minmax = sap->minmax[axis];
    minmax_last = minmax + (2 * sap->geoms_len) - 1;
    min = &sap->minmax[axis][sap->geoms[geom->sap].min[axis]];
    max = &sap->minmax[axis][sap->geoms[geom->sap].max[axis]];

    // try to move MIN to LEFT
    while (minmax != min && min->val < (min - 1)->val){
        FER_SWAP(*min, *(min - 1), tmp);

        if (MINMAX_ISMAX(min)){
            pairAdd(sap, sap->geoms[MINMAX_GEOM(min)].g,
                         sap->geoms[MINMAX_GEOM(min - 1)].g);
        }
        min = min - 1;
    }

    // try to move MAX to RIGHT
    while (minmax_last != max && max->val > (max + 1)->val){
        FER_SWAP(*max, *(max + 1), tmp);

        if (!MINMAX_ISMAX(max)){
            pairAdd(sap, sap->geoms[MINMAX_GEOM(max)].g,
                         sap->geoms[MINMAX_GEOM(max + 1)].g);
        }
        max = max + 1;
    }

    // try to move MIN to RIGHT
    while (minmax_last != min && min->val > (min + 1)->val){
        FER_SWAP(*min, *(min + 1), tmp);

        if (MINMAX_ISMAX(min)){
            pairRemove(sap, sap->geoms[MINMAX_GEOM(min)].g,
                            sap->geoms[MINMAX_GEOM(min + 1)].g);
        }
        min = min + 1;
    }

    // try to move MAX to LEFT
    while (minmax != max && max->val < (max - 1)->val){
        FER_SWAP(*max, *(max - 1), tmp);

        if (MINMAX_ISMAX(max)){
            pairRemove(sap, sap->geoms[MINMAX_GEOM(max)].g,
                            sap->geoms[MINMAX_GEOM(max - 1)].g);
        }
        max = max - 1;
    }
}

void ferCDSAPUpdate(fer_cd_sap_t *sap, fer_cd_geom_t *geom)
{
    int i;
    fer_cd_sap_geom_t *g;
    fer_real_t min, max;

    //if (sap->added == 0)
    //    ferCDSAPDump(sap);

    g = &sap->geoms[geom->sap];
    for (i = 0; i < FER_CD_SAP_NUM_AXIS; i++){
        // update min/max values
        __ferCDGeomSetMinMax(geom, &sap->axis[i], &min, &max);
        //DBG("[%d %d] %f %f -> %f %f", g->min[i], g->max[i], sap->minmax[i][g->min[i]].val,
        //        sap->minmax[i][g->max[i]].val, min, max);
        sap->minmax[i][g->min[i]].val = min;
        sap->minmax[i][g->max[i]].val = max;

        if (sap->added == 0){
            //ferCDSAPUpdateMinMax(sap, geom, i);
        }
    }

}

void ferCDSAPRemove(fer_cd_sap_t *sap, fer_cd_geom_t *geom)
{
    // TODO
}



#include <fermat/timer.h>

static void ferCDSAPProcessAll(fer_cd_t *cd, fer_cd_sap_t *sap)
{
    fer_list_t *item;
    fer_cd_sap_geom_t *g1;
    fer_cd_sap_minmax_t *minmax;
    fer_cd_sap_pair_t *pair;
    int i, j, d, diff[3];

    fer_timer_t timer;

    ferTimerStart(&timer);
    // do radix sort for all min/max values
    for (i = 0; i < FER_CD_SAP_NUM_AXIS; i++){
        radixSort(sap, i);
    }
    ferTimerStop(&timer);
    fprintf(stderr, "radix sort: %lu us\n", ferTimerElapsedInUs(&timer));

    ferTimerStart(&timer);
    // remove all current pairs - we will reevaluate it all
    for (i = 0; i < sap->collide_pairs_buckets; i++){
        while (!ferListEmpty(&sap->collide_pairs[i])){
            item = ferListNext(&sap->collide_pairs[i]);
            pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, list);
            pairDel(pair);
        }
    }
    ferTimerStop(&timer);
    fprintf(stderr, "remove pairs: %lu us\n", ferTimerElapsedInUs(&timer));


    ferTimerStart(&timer);
    // obtain all pairs from min-max arrays
    for (i = 0; i < sap->geoms_len; i++){
        g1 = &sap->geoms[i];
        diff[0] = g1->max[0] - g1->min[0];
        diff[1] = g1->max[1] - g1->min[1];
        diff[2] = g1->max[2] - g1->min[2];

        if (diff[0] < diff[1]){
            if (diff[0] < diff[2]){
                d = 0;
            }else{
                d = 2;
            }
        }else if (diff[1] < diff[2]){
            d = 1;
        }else{
            d = 2;
        }

        minmax = sap->minmax[d];
        for (j = g1->min[d] + 1; j < g1->max[d]; j++){
            if (!MINMAX_ISMAX(&minmax[j])){
                pairAdd(sap, g1->g, sap->geoms[MINMAX_GEOM(&minmax[j])].g);
            }
        }
    }
    ferTimerStop(&timer);
    fprintf(stderr, "create pairs: %lu us\n", ferTimerElapsedInUs(&timer));

    sap->added = 0;
}

void ferCDSAPProcess(fer_cd_t *cd, fer_cd_sap_t *sap)
{
    if (sap->added > 0){
        ferCDSAPProcessAll(cd, sap);
    }else{
        ferCDSAPProcessAll(cd, sap);
    }
}

static fer_cd_sap_radix_sort_t *radixSortNew(void)
{
    fer_cd_sap_radix_sort_t *rs;

    rs = FER_ALLOC(fer_cd_sap_radix_sort_t);
    rs->minmax = NULL;
    rs->minmax_len = 0;
    rs->minmax_alloc = 0;

    return rs;
}

static void radixSortDel(fer_cd_sap_radix_sort_t *rs)
{
    if (rs->minmax)
        free(rs->minmax);
    free(rs);
}


_fer_inline void radixSortZeroizeCounter(fer_cd_sap_radix_sort_t *rs)
{
    int i;
    for (i = 0; i < 256; i++)
        rs->counter[i] = 0;
}

_fer_inline void radixSortFixCounter(fer_cd_sap_radix_sort_t *rs)
{
    int i, s, c;

    for (s = 0, c = 0, i = 0; i < 256; i++){
        c += rs->counter[i];
        rs->counter[i] = s;
        s = c;
    }
}

_fer_inline void radixSortCount(fer_cd_sap_radix_sort_t *rs,
                                fer_cd_sap_minmax_t *minmax,
                                fer_uint_t mask, fer_uint_t shift)
{
    int i;
    fer_uint_t val;

    radixSortZeroizeCounter(rs);
    for (i = 0; i < rs->minmax_len; i++){
        val  = ferRealAsUInt(minmax[i].val);
        val &= mask;
        val >>= shift;
        rs->counter[val] += 1;
    }
    radixSortFixCounter(rs);
}

_fer_inline void radixSortSort(fer_cd_sap_radix_sort_t *rs,
                               fer_cd_sap_minmax_t *src,
                               fer_cd_sap_minmax_t *dst,
                               int radix)
{
    fer_uint_t mask, shift, i, val;

    shift = radix * 8;
    mask = ((fer_uint_t)0xffu) << shift;

    // run counting
    radixSortCount(rs, src, mask, shift);

    // and sort the values to dst
    for (i = 0; i < rs->minmax_len; i++){
        val  = ferRealAsUInt(src[i].val);
        val &= mask;
        val >>= shift;
        dst[rs->counter[val]] = src[i];
        ++rs->counter[val];
    }
}

_fer_inline void radixSortCountFinal(fer_cd_sap_radix_sort_t *rs,
                                     fer_cd_sap_minmax_t *minmax,
                                     fer_uint_t mask, fer_uint_t shift)
{
    int i;
    fer_uint_t val;

    radixSortZeroizeCounter(rs);
    rs->negative = 0;
    for (i = 0; i < rs->minmax_len; i++){
        val  = ferRealAsUInt(minmax[i].val);
        val &= mask;
        val >>= shift;
        rs->counter[val] += 1;

        if (minmax[i].val < FER_ZERO)
            ++rs->negative;
    }
    radixSortFixCounter(rs);
}

_fer_inline void radixSortSortFinal(fer_cd_sap_t *sap,
                                    fer_cd_sap_radix_sort_t *rs,
                                    fer_cd_sap_minmax_t *src,
                                    fer_cd_sap_minmax_t *dst,
                                    int radix, int axis)
{
    fer_uint_t mask, shift, val, pos;
    int i;

    shift = radix * 8;
    mask = ((fer_uint_t)0xffu) << shift;

    // run counting
    radixSortCountFinal(rs, src, mask, shift);

    // and sort the values to dst
    for (i = 0; i < rs->minmax_len; i++){
        val  = ferRealAsUInt(src[i].val);
        val &= mask;
        val >>= shift;

        pos = rs->counter[val];
        if (pos >= rs->minmax_len - rs->negative){
            pos = rs->minmax_len - pos - 1;
        }else{
            pos = rs->negative + rs->counter[val];
        }

        dst[pos] = src[i];

        if (MINMAX_ISMAX(&dst[pos])){
            sap->geoms[MINMAX_GEOM(&dst[pos])].max[axis] = pos;
        }else{
            sap->geoms[MINMAX_GEOM(&dst[pos])].min[axis] = pos;
        }

        ++rs->counter[val];
    }
}

static void radixSort(fer_cd_sap_t *sap, int axis)
{
    fer_cd_sap_radix_sort_t *rs = sap->radix_sort;

    // allocate temporary array
    if (rs->minmax_alloc < 2 * sap->geoms_len){
        if (rs->minmax)
            free(rs->minmax);
        rs->minmax_alloc = 2 * sap->geoms_alloc;
        rs->minmax_len = 2 * sap->geoms_len;
        rs->minmax = FER_ALLOC_ARR(fer_cd_sap_minmax_t, rs->minmax_alloc);
    }

    if (sizeof(fer_real_t) == 4){
        radixSortSort(rs, sap->minmax[axis], rs->minmax, 0);
        radixSortSort(rs, rs->minmax, sap->minmax[axis], 1);
        radixSortSort(rs, sap->minmax[axis], rs->minmax, 2);
        radixSortSortFinal(sap, rs, rs->minmax, sap->minmax[axis], 3, axis);
    }else if (sizeof(fer_real_t) == 8){
        radixSortSort(rs, sap->minmax[axis], rs->minmax, 0);
        radixSortSort(rs, rs->minmax, sap->minmax[axis], 1);
        radixSortSort(rs, sap->minmax[axis], rs->minmax, 2);
        radixSortSort(rs, rs->minmax, sap->minmax[axis], 3);
        radixSortSort(rs, sap->minmax[axis], rs->minmax, 4);
        radixSortSort(rs, rs->minmax, sap->minmax[axis], 5);
        radixSortSort(rs, sap->minmax[axis], rs->minmax, 6);
        radixSortSortFinal(sap, rs, rs->minmax, sap->minmax[axis], 7, axis);
    }
}




void ferCDSAPDump(fer_cd_sap_t *sap)
{
    size_t i;

    fprintf(stderr, "geoms_len: %d, geoms_alloc: %d\n", (int)sap->geoms_len, (int)sap->geoms_alloc);
    fprintf(stderr, "geoms:\n");
    for (i = 0; i < sap->geoms_len; i++){
        fprintf(stderr, "    [%02u]: %02d, %02d, %02d; %02d, %02d, %02d (%lx)\n",
                i, sap->geoms[i].min[0], sap->geoms[i].min[1],
                sap->geoms[i].min[2], sap->geoms[i].max[0],
                sap->geoms[i].max[1], sap->geoms[i].max[2],
                (long)sap->geoms[i].g);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "minmax:\n");
    for (i = 0; i < 2 * sap->geoms_len; i++){
        fprintf(stderr, "    [%02u]: %02d, %d, % 10f (%x)  |  %02d, %d, % 10f (%x)  |  %02d, %d, % 10f (%x)\n",
                i, MINMAX_GEOM(&sap->minmax[0][i]), MINMAX_ISMAX(&sap->minmax[0][i]),
                sap->minmax[0][i].val, (int)ferRealAsUInt(sap->minmax[0][i].val),
                MINMAX_GEOM(&sap->minmax[1][i]), MINMAX_ISMAX(&sap->minmax[1][i]),
                sap->minmax[1][i].val, (int)ferRealAsUInt(sap->minmax[1][i].val),
                MINMAX_GEOM(&sap->minmax[2][i]), MINMAX_ISMAX(&sap->minmax[2][i]),
                sap->minmax[2][i].val, (int)ferRealAsUInt(sap->minmax[2][i].val));
    }
    fprintf(stderr, "\n");
}

void ferCDSAPDumpPairs(fer_cd_sap_t *sap, FILE *out)
{
    fer_list_t *list, *item;
    fer_cd_sap_pair_t *pair;
    size_t i, d;

    for (i = 0; i < sap->pairs->size; i++){
        list = &sap->pairs->table[i];

        FER_LIST_FOR_EACH(list, item){
            pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, hmap);
            fprintf(out, "[%04d]: (%lx) g[0]: %lx, g[1]: %lx, num_axis: %d\n",
                    (int)i, (long)pair, (long)pair->g[0], (long)pair->g[1],
                    pair->num_axis);
        }
    }

    /*
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
    */

    for (d = 0; d < sap->collide_pairs_buckets; d++)
    {
        fprintf(out, ".collide_pairs:\n");
        FER_LIST_FOR_EACH(&sap->collide_pairs[d], item){
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
    ferListDel(&p->list);
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
    if (!hmap && ferCDGeomOBBOverlap(g1, g2)){
        p = pairNew(&pair);
        ferHMapPut(sap->pairs, &p->hmap);

        ferListAppend(&sap->collide_pairs[sap->collide_pairs_next], &p->list);
        sap->collide_pairs_next = (sap->collide_pairs_next + 1) % sap->collide_pairs_buckets;
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
        ferListDel(&p->list);
        sap->collide_pairs_len--;

        // we don't need to call ferHMapRemove() because it is safe to
        // remove a pair from hash table silently by disconnecting
        // .hmap list item - which is done in pairDel().
        //ferHMapRemove(sap->pairs, &p->hmap);
        pairDel(p);
    }
}

