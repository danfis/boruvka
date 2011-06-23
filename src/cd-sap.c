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
static fer_timer_t timer;


#define MINMAX_ISMAX(minmax) \
    ((minmax)->geom_ismax & 0x1)
#define MINMAX_GEOM(minmax) \
    ((minmax)->geom_ismax >> 0x1)

/** Find all collide pairs */
static void ferCDSAPFindPairs(fer_cd_sap_t *sap);
/** Find collide pairs of {g} */
static void ferCDSAPFindGeomPairs(fer_cd_sap_t *sap, fer_cd_sap_geom_t *g,
                                  int bucket);
/** Process all geoms - radix sort approach */
static void ferCDSAPProcessAll(fer_cd_sap_t *sap);

/** Create and delete radix sort struct */
static void radixSortInit(fer_cd_sap_radix_sort_t *rs);
static void radixSortDestroy(fer_cd_sap_radix_sort_t *rs);
_fer_inline fer_cd_sap_t *radixSortSAP(fer_cd_sap_radix_sort_t *rs);
/** Sorts min-max values along specified axis */
static void radixSort(fer_cd_sap_t *sap, int axis);

/** Creates new pair from other fer_cd_sap_pair_t struct */
static fer_cd_sap_pair_t *pairNew(fer_cd_geom_t *g1, fer_cd_geom_t *g2);
/** Deletes collide pair */
static void pairDel(fer_cd_sap_pair_t *p);

/** Callbacks for hash table */
static uint32_t pairHash(fer_list_t *key, void *sap);
static int pairEq(const fer_list_t *key1, const fer_list_t *key2, void *sap);

/** Removes all pairs from all buckets */
static void pairRemoveAll(fer_cd_sap_t *sap);



fer_cd_sap_t *ferCDSAPNew(fer_cd_t *cd, size_t par, size_t hash_table_size)
{
    fer_cd_sap_t *sap;
    size_t i;

    sap = FER_ALLOC_ALIGN(fer_cd_sap_t, 16);

    sap->cd = cd;
    sap->par = par;

    // set up axis
    ferVec3Set(&sap->axis[0], FER_ONE,  FER_ONE, FER_ZERO);
    ferVec3Set(&sap->axis[1], FER_ZERO, FER_ONE,  FER_ONE);
    ferVec3Set(&sap->axis[2], FER_ONE, FER_ZERO, FER_ONE);

    // init geoms and minmax arrays
    sap->geoms_alloc = 100;
    sap->geoms_len = 0;
    sap->geoms = FER_ALLOC_ARR(fer_cd_sap_geom_t, sap->geoms_alloc);
    for (i = 0; i < 3; i++){
        sap->minmax[i] = FER_ALLOC_ARR(fer_cd_sap_minmax_t,
                                       sap->geoms_alloc * 2);
    }

    radixSortInit(&sap->radix_sort);

    sap->dirty = 0;

    sap->pairs_reg = ferHMapNew(hash_table_size, pairHash, pairEq, (void *)sap);
    sap->pairs_reg_lock_len = sap->par;
    sap->pairs_reg_lock = FER_ALLOC_ARR(pthread_mutex_t,
                                        sap->pairs_reg_lock_len);
    for (i = 0; i < sap->pairs_reg_lock_len; i++){
        pthread_mutex_init(&sap->pairs_reg_lock[i], NULL);
    }

    // initialize list of possible collide pairs
    sap->collide_pairs = FER_ALLOC_ARR(fer_list_t, par);
    sap->collide_pairs_len = 0;
    for (i = 0; i < par; i++){
        ferListInit(&sap->collide_pairs[i]);
    }

    sap->gpu = NULL;

    return sap;
}

void ferCDSAPDel(fer_cd_sap_t *sap)
{
    fer_list_t allpairs, *item;
    fer_cd_sap_pair_t *pair;
    int i;

    if (sap->geoms)
        free(sap->geoms);
    for (i = 0; i < 3; i++){
        if (sap->minmax[i])
            free(sap->minmax[i]);
    }

    radixSortDestroy(&sap->radix_sort);

    // delete all pairs from hash table
    ferListInit(&allpairs);
    ferHMapGather(sap->pairs_reg, &allpairs);
    while (!ferListEmpty(&allpairs)){
        item = ferListNext(&allpairs);
        ferListDel(item);
        pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, hmap);
        pairDel(pair);
    }

    ferHMapDel(sap->pairs_reg);
    for (i = 0; i < sap->pairs_reg_lock_len; i++){
        pthread_mutex_destroy(&sap->pairs_reg_lock[i]);
    }
    free(sap->pairs_reg_lock);

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

        for (i = 0; i < 3; i++){
            sap->minmax[i] = FER_REALLOC_ARR(sap->minmax[i],
                                             fer_cd_sap_minmax_t,
                                             sap->geoms_alloc * 2);
        }
    }

    sap->geoms[sap->geoms_len].g = geom;
    for (i = 0; i < 3; i++){
        sap->minmax[i][sap->geoms_len * 2].geom_ismax = sap->geoms_len << 1;
        sap->minmax[i][sap->geoms_len * 2 + 1].geom_ismax = sap->geoms_len << 1;
        sap->minmax[i][sap->geoms_len * 2 + 1].geom_ismax |= 0x1;
        sap->geoms[sap->geoms_len].min[i] = sap->geoms_len * 2;
        sap->geoms[sap->geoms_len].max[i] = sap->geoms_len * 2 + 1;
    }

    geom->sap = sap->geoms_len;

    sap->geoms_len++;
    sap->dirty = 1;
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
        sap->minmax[i][g->min[i]].val = min;
        sap->minmax[i][g->max[i]].val = max;
    }

    sap->dirty = 1;
}

void ferCDSAPRemove(fer_cd_sap_t *sap, fer_cd_geom_t *geom)
{
    // TODO
    sap->dirty = 1;
}



static void ferCDSAPProcessGPU(fer_cd_sap_t *sap);

void ferCDSAPProcess(fer_cd_sap_t *sap)
{
    if (sap->dirty){
        ferCDSAPProcessAll(sap);
    }

    sap->dirty = 0;
}


static void ferCDSAPFindGeomPairs(fer_cd_sap_t *sap, fer_cd_sap_geom_t *g,
                                  int bucket)
{
    int diff[3], d, j;
    fer_cd_sap_minmax_t *minmax;
    fer_cd_geom_t *g2;
    fer_cd_sap_pair_t *pair;
    uint32_t id;
    int lockid;

    diff[0] = g->max[0] - g->min[0];
    diff[1] = g->max[1] - g->min[1];
    diff[2] = g->max[2] - g->min[2];

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
    for (j = g->min[d] + 1; j < g->max[d]; j++){
        g2 = sap->geoms[MINMAX_GEOM(&minmax[j])].g;
        if (ferCDGeomOBBOverlap(g->g, g2)){
            // create new pair
            pair = pairNew(g->g, g2);

            // add this pair to list of pairs (into specified bucket)
            ferListAppend(&sap->collide_pairs[bucket], &pair->list);
            sap->collide_pairs_len++;

            // obtain hash ID
            id = ferHMapID(sap->pairs_reg, &pair->hmap);

            // find out mutext corresponding to id
            lockid = id % sap->pairs_reg_lock_len;

            // check if this pair isn't already registered
            pthread_mutex_lock(&sap->pairs_reg_lock[lockid]);
            if (ferHMapIDGet(sap->pairs_reg, id, &pair->hmap) == NULL){
                ferHMapIDPut(sap->pairs_reg, id, &pair->hmap);
            }else{
                pairDel(pair);
                sap->collide_pairs_len--;
            }
            pthread_mutex_unlock(&sap->pairs_reg_lock[lockid]);
        }
    }
}

static void __findPairsTask(int id, void *data,
                            const fer_tasks_thinfo_t *thinfo)
{
    fer_cd_sap_t *sap = (fer_cd_sap_t *)data;
    int from, to, len, i;

    len  = sap->geoms_len / sap->par;
    from = id * len;
    to   = from + len;
    if (id == sap->par - 1)
        to = sap->geoms_len;

    for (i = from; i < to; i++){
        ferCDSAPFindGeomPairs(sap, &sap->geoms[i], id);
    }
}

static void ferCDSAPFindPairs(fer_cd_sap_t *sap)
{
    size_t i;

    if (sap->par > 1){
        for (i = 0; i < sap->par; i++){
            ferTasksAdd(sap->cd->tasks, __findPairsTask, i, sap);
        }
        ferTasksBarrier(sap->cd->tasks);
    }else{
        __findPairsTask(0, sap, NULL);
    }
}

static void ferCDSAPProcessAll(fer_cd_sap_t *sap)
{
    int i;

    ferCDSAPProcessGPU(sap);

    ferTimerStart(&timer);
    // do radix sort for all min/max values
    for (i = 0; i < 3; i++){
        radixSort(sap, i);
    }
    ferTimerStop(&timer);
    fprintf(stderr, "Radix Sort: %lu us\n", ferTimerElapsedInUs(&timer));
    exit(-1);

    // remove all current pairs - we will reevaluate it all
    pairRemoveAll(sap);

    // obtain all pairs from min-max arrays
    ferCDSAPFindPairs(sap);
}





static void radixSortInit(fer_cd_sap_radix_sort_t *rs)
{
    fer_cd_sap_t *sap = radixSortSAP(rs);

    rs->minmax = NULL;
    rs->minmax_len = 0;
    rs->minmax_alloc = 0;

    rs->counter  = FER_ALLOC_ARR(uint32_t, (1 << 8) * sap->par);
    rs->negative = FER_ALLOC_ARR(uint32_t, sap->par);

    rs->barrier = NULL;
    if (sap->par > 1)
        rs->barrier = ferBarrierNew(sap->par);

    rs->context.src = rs->context.dst = NULL;
}

static void radixSortDestroy(fer_cd_sap_radix_sort_t *rs)
{
    if (rs->minmax)
        free(rs->minmax);
    if (rs->counter)
        free(rs->counter);
    if (rs->negative)
        free(rs->negative);
    if (rs->barrier)
        ferBarrierDel(rs->barrier);
}

_fer_inline fer_cd_sap_t *radixSortSAP(fer_cd_sap_radix_sort_t *rs)
{
    return fer_container_of(rs, fer_cd_sap_t, radix_sort);
}


/** Zeroize 256 position in counter from offset */
_fer_inline void radixSortZeroizeCounter(fer_cd_sap_radix_sort_t *rs,
                                         int offset);
/** Fix counter values to point to correct position */
_fer_inline void radixSortFixCounter(fer_cd_sap_radix_sort_t *rs);

static void __radixSortTask(int id, void *data,
                            const fer_tasks_thinfo_t *thinfo);

/** Perform one cycle of sort */
static void radixSortSort(fer_cd_sap_radix_sort_t *rs);

static void radixSort(fer_cd_sap_t *sap, int axis)
{
    fer_cd_sap_radix_sort_t *rs = &sap->radix_sort;
    fer_cd_sap_minmax_t *tmp;
    fer_uint_t i;

    // allocate temporary array
    if (rs->minmax_alloc < 2 * sap->geoms_len){
        if (rs->minmax)
            free(rs->minmax);
        rs->minmax_alloc = 2 * sap->geoms_alloc;
        rs->minmax_len = 2 * sap->geoms_len;
        rs->minmax = FER_ALLOC_ARR(fer_cd_sap_minmax_t, rs->minmax_alloc);
    }
    rs->minmax_len = 2 * sap->geoms_len;

    // set up context
    rs->context.src  = sap->minmax[axis];
    rs->context.dst  = rs->minmax;
    rs->context.axis = axis;
    rs->context.len  = (fer_uint_t)sizeof(fer_real_t);

    for (i = 0; i < rs->context.len; i++){
        // set up context
        rs->context.shift = i * 8;
        rs->context.mask = ((fer_uint_t)0xffu) << rs->context.shift;
        rs->context.i = i;

        // run sorting
        radixSortSort(rs);

        // swap src and dst min-max arrays
        tmp = rs->context.src;
        rs->context.src = rs->context.dst;
        rs->context.dst = tmp;
    }
}

_fer_inline void radixSortZeroizeCounter(fer_cd_sap_radix_sort_t *rs,
                                         int offset)
{
    int i;
    for (i = 0; i < 256; i++)
        rs->counter[offset + i] = 0;
}

_fer_inline void radixSortFixCounter(fer_cd_sap_radix_sort_t *rs)
{
    int t, i, s, c, par;

    par = radixSortSAP(rs)->par;

    for (s = 0, c = 0, i = 0; i < 256; i++){
        for (t = 0; t < par; t++){
            c += rs->counter[256 * t + i];
            rs->counter[256 * t + i] = s;
            s = c;
        }
    }
}

static void __radixSortCount(int id, int from, int to, int offset,
                             fer_cd_sap_radix_sort_t *rs)
{
    int i;
    fer_uint_t val;

    radixSortZeroizeCounter(rs, offset);
    if (rs->context.i == rs->context.len - 1){
        rs->negative[id] = 0;
        for (i = from; i < to; i++){
            val  = ferRealAsUInt(rs->context.src[i].val);
            val &= rs->context.mask;
            val >>= rs->context.shift;
            rs->counter[offset + val] += 1;

            if (rs->context.src[i].val < FER_ZERO)
                ++rs->negative[id];
        }
    }else{
        for (i = from; i < to; i++){
            val  = ferRealAsUInt(rs->context.src[i].val);
            val &= rs->context.mask;
            val >>= rs->context.shift;
            rs->counter[offset + val] += 1;
        }
    }
}

static void __radixSortSort(int id, int from, int to, int offset,
                            fer_cd_sap_radix_sort_t *rs)
{
    fer_cd_sap_t *sap = radixSortSAP(rs);
    fer_uint_t val, pos;
    int i, neg;

    if (rs->context.i == rs->context.len - 1){
        // compute number of negative values
        neg = 0;
        for (i = 0; i < sap->par; i++){
            neg += rs->negative[i];
        }

        for (i = from; i < to; i++){
            val  = ferRealAsUInt(rs->context.src[i].val);
            val &= rs->context.mask;
            val >>= rs->context.shift;

            // compute dst position considering negative numbers
            pos = rs->counter[offset + val];
            if (pos >= rs->minmax_len - neg){
                pos = rs->minmax_len - pos - 1;
            }else{
                pos = neg + rs->counter[offset + val];
            }

            // write to dst
            rs->context.dst[pos] = rs->context.src[i];

            // update geom's info
            if (MINMAX_ISMAX(&rs->context.dst[pos])){
                sap->geoms[MINMAX_GEOM(&rs->context.dst[pos])].max[rs->context.axis] = pos;
            }else{
                sap->geoms[MINMAX_GEOM(&rs->context.dst[pos])].min[rs->context.axis] = pos;
            }

            ++rs->counter[offset + val];
        }
    }else{
        for (i = from; i < to; i++){
            val  = ferRealAsUInt(rs->context.src[i].val);
            val &= rs->context.mask;
            val >>= rs->context.shift;

            pos = rs->counter[offset + val];
            rs->context.dst[pos] = rs->context.src[i];

            ++rs->counter[offset + val];
        }
    }
}

static void __radixSortTask(int id, void *data,
                            const fer_tasks_thinfo_t *thinfo)
{
    fer_cd_sap_radix_sort_t *rs = (fer_cd_sap_radix_sort_t *)data;
    fer_cd_sap_t *sap = radixSortSAP(rs);
    int len, from, to, offset;

    len  = rs->minmax_len / sap->par;
    from = id * len;
    to   = from + len;
    if (id == sap->par - 1)
        to = rs->minmax_len;
    offset = (1 << 8) * id;

    __radixSortCount(id, from, to, offset, rs);
    ferBarrier(rs->barrier);

    if (id == 0)
        radixSortFixCounter(rs);
    ferBarrier(rs->barrier);

    __radixSortSort(id, from, to, offset, rs);
}

static void radixSortSort(fer_cd_sap_radix_sort_t *rs)
{
    fer_cd_sap_t *sap = radixSortSAP(rs);
    size_t i;

    if (sap->par > 1){
        for (i = 0; i < sap->par; i++){
            ferTasksAdd(sap->cd->tasks, __radixSortTask, i, rs);
        }
        ferTasksBarrier(sap->cd->tasks);
    }else{
        __radixSortCount(0, 0, rs->minmax_len, 0, rs);
        radixSortFixCounter(rs);
        __radixSortSort(0, 0, rs->minmax_len, 0, rs);
    }
}





void ferCDSAPDump(fer_cd_sap_t *sap)
{
    size_t i;

    fprintf(stderr, "geoms_len: %d, geoms_alloc: %d\n", (int)sap->geoms_len, (int)sap->geoms_alloc);
    fprintf(stderr, "geoms:\n");
    for (i = 0; i < sap->geoms_len; i++){
        fprintf(stderr, "    [%02d]: %02d, %02d, %02d; %02d, %02d, %02d (%lx)\n",
                (int)i, sap->geoms[i].min[0], sap->geoms[i].min[1],
                sap->geoms[i].min[2], sap->geoms[i].max[0],
                sap->geoms[i].max[1], sap->geoms[i].max[2],
                (long)sap->geoms[i].g);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "minmax:\n");
    for (i = 0; i < 2 * sap->geoms_len; i++){
        fprintf(stderr, "    [%02d]: %02d, %d, % 10f (%x)  |  %02d, %d, % 10f (%x)  |  %02d, %d, % 10f (%x)\n",
                (int)i, MINMAX_GEOM(&sap->minmax[0][i]), MINMAX_ISMAX(&sap->minmax[0][i]),
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

    for (i = 0; i < sap->pairs_reg->size; i++){
        list = &sap->pairs_reg->table[i];

        FER_LIST_FOR_EACH(list, item){
            pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, hmap);
            fprintf(out, "[%04d]: (%lx) g[0]: %lx, g[1]: %lx\n",
                    (int)i, (long)pair, (long)pair->g[0], (long)pair->g[1]);
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

    for (d = 0; d < sap->par; d++)
    {
        fprintf(out, ".collide_pairs:\n");
        FER_LIST_FOR_EACH(&sap->collide_pairs[d], item){
            pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, list);
            fprintf(out, "    (%lx) g[0]: %lx, g[1]: %lx\n",
                    (long)pair, (long)pair->g[0], (long)pair->g[1]);
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
    free(p);
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

    for (i = 0; i < sap->par; i++){
        while (!ferListEmpty(&sap->collide_pairs[i])){
            item = ferListNext(&sap->collide_pairs[i]);
            pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, list);
            pairDel(pair);
        }
    }
    sap->collide_pairs_len = 0;
}



#include <fermat/opencl.h>
#include <fermat/timer.h>
struct _gpu_t {
    fer_cl_t *cl;
    int kernel_radix_sort;
    uint32_t num_threads;
    fer_cd_sap_minmax_t *minmax, *minmax_tmp;
    uint32_t minmax_len;
    cl_uint *counter;
    cl_uint *counter_sum;
    cl_uint *negative;
};
typedef struct _gpu_t gpu_t;

static fer_timer_t timer;

static void gpuUpdate(fer_cd_sap_t *sap)
{
    gpu_t *gpu;

    if (!sap->gpu){
        gpu = FER_ALLOC(gpu_t);
        gpu->cl = ferCLNewSimple(ferCLProgramFromFile("sap.cl"), "");
        //gpu->kernel_radix_sort = ferCLKernelNew(gpu->cl, "radixSort");
        gpu->num_threads = 16;
        gpu->minmax = gpu->minmax_tmp = NULL;
        gpu->minmax_len = 0;
        //gpu->counter  = FER_CL_ALLOC_ARR(gpu->cl, cl_uint, gpu->num_threads * (1 << 4));
        gpu->counter  = FER_CL_ALLOC_ARR(gpu->cl, cl_uint, 16 * 16 * 100);
        gpu->counter_sum = FER_CL_ALLOC_ARR(gpu->cl, cl_uint, 16 * 100);
        gpu->negative = FER_CL_ALLOC_ARR(gpu->cl, cl_uint, 100);
        sap->gpu = gpu;
    }

    gpu = (gpu_t *)sap->gpu;

    if (gpu->minmax_len < 2 * sap->geoms_len){
        if (gpu->minmax){
            FER_CL_FREE(gpu->cl, gpu->minmax);
            FER_CL_FREE(gpu->cl, gpu->minmax_tmp);
        }

        gpu->minmax_len = 2 * sap->geoms_len;
        gpu->minmax = FER_CL_ALLOC_ARR(gpu->cl, fer_cd_sap_minmax_t, gpu->minmax_len);
        gpu->minmax_tmp = FER_CL_ALLOC_ARR(gpu->cl, fer_cd_sap_minmax_t, gpu->minmax_len);
    }
}

static void gpuDel(fer_cd_sap_t *sap)
{
    gpu_t *gpu = (gpu_t *)sap->gpu;

    if (!gpu)
        return;

    if (gpu->minmax){
        FER_CL_FREE(gpu->cl, gpu->minmax);
        FER_CL_FREE(gpu->cl, gpu->minmax_tmp);
    }
    FER_CL_FREE(gpu->cl, gpu->counter);
    FER_CL_FREE(gpu->cl, gpu->counter_sum);
    FER_CL_FREE(gpu->cl, gpu->negative);
    free(gpu);
}

static void gpuLoad(fer_cd_sap_t *sap)
{
    gpu_t *gpu = (gpu_t *)sap->gpu;

    ferTimerStart(&timer);

    FER_CL_COPY_FROM_HOST(gpu->cl, sap->minmax[0], gpu->minmax,
                          fer_cd_sap_minmax_t, gpu->minmax_len);

    ferTimerStop(&timer);
    DBG("%lu us", ferTimerElapsedInUs(&timer));
}

static void gpuSave(fer_cd_sap_t *sap)
{
    gpu_t *gpu = (gpu_t *)sap->gpu;

    ferTimerStart(&timer);

    FER_CL_COPY_TO_HOST(gpu->cl, gpu->minmax, sap->minmax[0],
                          fer_cd_sap_minmax_t, gpu->minmax_len);

    ferTimerStop(&timer);
    DBG("%lu us", ferTimerElapsedInUs(&timer));
}

static void gpuRun(fer_cd_sap_t *sap)
{
    gpu_t *gpu = (gpu_t *)sap->gpu;
    size_t glob[1], loc[1], glob_fix[1];
    uint32_t num_groups;
    uint32_t i, shift, use_negative;
    fer_cd_sap_minmax_t *src, *dst, *tmp;

    num_groups = 50;
    glob[0] = 16 * num_groups;
    loc[0] = 16;
    glob_fix[0] = 2 * loc[0];

    int k_reduce, k_fix_counter1, k_fix_counter2, k_copy;

    ferCLKernelNew(gpu->cl, "radixSortReduce");
    k_reduce = ferCLKernelsLen(gpu->cl) - 1;
    DBG("k_reduce: %d", k_reduce);

    ferCLKernelNew(gpu->cl, "radixSortFixCounter1");
    k_fix_counter1 = ferCLKernelsLen(gpu->cl) - 1;
    DBG("k_fix_counter1: %d", k_fix_counter1);

    ferCLKernelNew(gpu->cl, "radixSortFixCounter2");
    k_fix_counter2 = ferCLKernelsLen(gpu->cl) - 1;
    DBG("k_fix_counter2: %d", k_fix_counter2);

    ferCLKernelNew(gpu->cl, "radixSortCopy");
    k_copy = ferCLKernelsLen(gpu->cl) - 1;
    DBG("k_copy: %d", k_copy);

    ferTimerStart(&timer);

    shift = 0;
    src = gpu->minmax;
    dst = gpu->minmax_tmp;
    for (i = 0; i < 8; i++){
        use_negative = (i == 7);

        FER_CL_KERNEL_SET_ARG(gpu->cl, k_reduce, 0, src);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_reduce, 1, gpu->minmax_len);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_reduce, 2, gpu->counter);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_reduce, 3, gpu->counter_sum);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_reduce, 4, gpu->negative);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_reduce, 5, shift);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_reduce, 6, use_negative);
        ferCLKernelEnqueue(gpu->cl, k_reduce, 1, glob, loc);

        FER_CL_KERNEL_SET_ARG(gpu->cl, k_fix_counter1, 0, gpu->counter_sum);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_fix_counter1, 1, num_groups);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_fix_counter1, 2, gpu->negative);
        if (use_negative){
            ferCLKernelEnqueue(gpu->cl, k_fix_counter1, 1, glob_fix, loc);
        }else{
            ferCLKernelEnqueue(gpu->cl, k_fix_counter1, 1, loc, loc);
        }

        FER_CL_KERNEL_SET_ARG(gpu->cl, k_fix_counter2, 0, gpu->counter);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_fix_counter2, 1, gpu->counter_sum);
        ferCLKernelEnqueue(gpu->cl, k_fix_counter2, 1, glob, loc);

        FER_CL_KERNEL_SET_ARG(gpu->cl, k_copy, 0, src);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_copy, 1, dst);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_copy, 2, gpu->minmax_len);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_copy, 3, gpu->counter);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_copy, 4, gpu->negative);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_copy, 5, shift);
        FER_CL_KERNEL_SET_ARG(gpu->cl, k_copy, 6, use_negative);
        ferCLKernelEnqueue(gpu->cl, k_copy, 1, glob, loc);

        tmp = src;
        src = dst;
        dst = tmp;
        shift += 4;
    }

    ferCLFinish(gpu->cl);


    ferTimerStop(&timer);
    DBG("%lu us", ferTimerElapsedInUs(&timer));

    {
    uint32_t *counter = FER_ALLOC_ARR(uint32_t, 16 * 16 * 5);
    uint32_t *counter_sum = FER_ALLOC_ARR(uint32_t, 16 * 5);
    uint32_t *negative = FER_ALLOC_ARR(uint32_t, 5);
    FER_CL_COPY_TO_HOST(gpu->cl, gpu->counter, counter, uint, 16 * 16 * 5);
    FER_CL_COPY_TO_HOST(gpu->cl, gpu->counter_sum, counter_sum, uint, 16 * 5);
    FER_CL_COPY_TO_HOST(gpu->cl, gpu->negative, negative, uint, 5);

    {
        uint32_t i;

        fprintf(stderr, "counter:\n");
        for (i = 0; i < 16 * 16 * 2; i++){
            if (i % 16 == 0)
                fprintf(stderr, "[%02d]: ", i / 16);
            fprintf(stderr, " %u", counter[i]);
            if ((i + 1) % 16 == 0)
                fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");

        fprintf(stderr, "counter_sum:\n");
        for (i = 0; i < 16 * 2; i++){
            fprintf(stderr, " %u", counter_sum[i]);
            if ((i + 1) % 16 == 0)
                fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");

        fprintf(stderr, "negative:\n");
        for (i = 0; i < 5; i++){
            fprintf(stderr, " %u", negative[i]);
        }
        fprintf(stderr, "\n");
    }

    free(counter);
    free(counter_sum);
    free(negative);
    }

    {
    FER_CL_COPY_TO_HOST(gpu->cl, gpu->minmax_tmp, sap->minmax[0],
                          fer_cd_sap_minmax_t, gpu->minmax_len);

    {
        uint32_t i;

        fprintf(stderr, "minmax_tmp:\n");
        for (i = 0; i < 20 && i < gpu->minmax_len; i++){
            fprintf(stderr, " % 10f (%x)", sap->minmax[0][i].val,
                    ferRealAsUInt(sap->minmax[0][i].val));
            if ((i + 1) % 5 == 0)
                fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");
    }
    }

    {
    FER_CL_COPY_TO_HOST(gpu->cl, gpu->minmax, sap->minmax[0],
                          fer_cd_sap_minmax_t, gpu->minmax_len);

    {
        uint32_t i;

        fprintf(stderr, "minmax:\n");
        for (i = 0; i < 20 && i < gpu->minmax_len; i++){
            fprintf(stderr, " % 10f (%x)", sap->minmax[0][i].val,
                    ferRealAsUInt(sap->minmax[0][i].val));
            if ((i + 1) % 5 == 0)
                fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");
    }
    }

}

static void ferCDSAPProcessGPU(fer_cd_sap_t *sap)
{
    ferCLPrintPlatforms(stdout);
    gpuUpdate(sap);

    gpuLoad(sap);
    gpuRun(sap);
    gpuSave(sap);

    gpuDel(sap);
    //exit(-1);
}
