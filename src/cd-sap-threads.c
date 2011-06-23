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

#include <fermat/barrier.h>


struct _fer_cd_sap_th_radix_sort_t {
    fer_cd_sap_minmax_t *minmax;
    size_t minmax_len, minmax_alloc;
    uint32_t *counter;  /*!< Array for count-sort */
    uint32_t *negative; /*!< Number of negative values */

    fer_barrier_t *barrier;

    struct {
        fer_cd_sap_minmax_t *src, *dst;
        fer_uint_t shift;
        int axis;
        fer_uint_t i, len;
    } context;
};
typedef struct _fer_cd_sap_th_radix_sort_t fer_cd_sap_th_radix_sort_t;

struct _fer_cd_sap_th_t {
    fer_cd_sap_t sap;
    fer_cd_sap_th_radix_sort_t rs; /*!< Cached radix sort struct */

    uint32_t num_threads;

    fer_hmap_t *pairs_reg;           /*!< Register (hash map) of collide pairs */
    pthread_mutex_t *pairs_reg_lock; /*!< Array of locks for .pairs_reg */
    size_t pairs_reg_lock_len;       /*!< Length of .pairs_reg_lock */
} fer_packed fer_aligned(16);
typedef struct _fer_cd_sap_th_t fer_cd_sap_th_t;

static void sapthRadixSort(fer_cd_sap_t *_sap, int axis);
static void sapthFindPairs(fer_cd_sap_t *_sap);

static fer_cd_sap_t *ferCDSAPThreadsNew(fer_cd_t *cd, uint64_t flags)
{
    fer_cd_sap_th_t *sap;
    size_t regsize, num_threads, i;

    regsize     = __FER_CD_SAP_HASH_TABLE_SIZE(flags);
    num_threads = __FER_CD_SAP_THREADS(flags);

    sap = FER_ALLOC(fer_cd_sap_th_t);

    ferCDSAPInit(cd, &sap->sap, num_threads);
    sap->sap.type = FER_CD_SAP_TYPE_THREADS;
    sap->sap.radix_sort = sapthRadixSort;
    sap->sap.find_pairs = sapthFindPairs;

    sap->num_threads = num_threads;

    sap->pairs_reg = ferHMapNew(regsize, pairHash, pairEq, (void *)sap);
    sap->pairs_reg_lock_len = num_threads;
    sap->pairs_reg_lock = FER_ALLOC_ARR(pthread_mutex_t,
                                        sap->pairs_reg_lock_len);
    for (i = 0; i < sap->pairs_reg_lock_len; i++){
        pthread_mutex_init(&sap->pairs_reg_lock[i], NULL);
    }

    sap->rs.minmax = NULL;
    sap->rs.minmax_len = 0;
    sap->rs.minmax_alloc = 0;
    sap->rs.counter = FER_ALLOC_ARR(uint32_t, 256 * num_threads);
    sap->rs.negative = FER_ALLOC_ARR(uint32_t, num_threads);
    sap->rs.barrier  = ferBarrierNew(num_threads);
    sap->rs.context.src = sap->rs.context.dst = NULL;

    return (fer_cd_sap_t *)sap;
}

static void ferCDSAPThreadsDel(fer_cd_sap_t *_sap)
{
    fer_cd_sap_th_t *sap = (fer_cd_sap_th_t *)_sap;
    size_t i;

    ferHMapDel(sap->pairs_reg);
    for (i = 0; i < sap->pairs_reg_lock_len; i++){
        pthread_mutex_destroy(&sap->pairs_reg_lock[i]);
    }

    if (sap->rs.minmax)
        free(sap->rs.minmax);
    free(sap->rs.counter);
    free(sap->rs.negative);
    ferBarrierDel(sap->rs.barrier);

    ferCDSAPDestroy(&sap->sap);

    free(sap);
}

_fer_inline void thradixSortFixCounter(uint32_t *counter, uint32_t threads);
static void __thradixSortTask(int id, void *data,
                              const fer_tasks_thinfo_t *thinfo);
static void thRadixSortSort(fer_cd_sap_th_radix_sort_t *rs);

static void sapthRadixSort(fer_cd_sap_t *_sap, int axis)
{
    fer_cd_sap_th_t *sap = (fer_cd_sap_th_t *)_sap;
    fer_cd_sap_th_radix_sort_t *rs = &sap->rs;
    fer_cd_sap_minmax_t *tmp;
    fer_uint_t i;

    // allocate temporary array
    if (rs->minmax_alloc < 2 * sap->sap.geoms_len){
        if (rs->minmax)
            free(rs->minmax);
        rs->minmax_alloc = 2 * sap->sap.geoms_alloc;
        rs->minmax_len = 2 * sap->sap.geoms_len;
        rs->minmax = FER_ALLOC_ARR(fer_cd_sap_minmax_t, rs->minmax_alloc);
    }
    rs->minmax_len = 2 * sap->sap.geoms_len;

    // set up context
    rs->context.src  = sap->sap.minmax[axis];
    rs->context.dst  = rs->minmax;
    rs->context.axis = axis;
    rs->context.len  = (fer_uint_t)sizeof(fer_real_t);

    for (i = 0; i < rs->context.len; i++){
        // set up context
        rs->context.shift = i * 8;
        rs->context.i = i;

        // run sorting
        thRadixSortSort(rs);

        // swap src and dst min-max arrays
        FER_SWAP(rs->context.src, rs->context.dst, tmp);
    }
}

_fer_inline void thradixSortFixCounter(uint32_t *counter, uint32_t threads)
{
    uint32_t t, i, s, c;

    for (s = 0, c = 0, i = 0; i < 256; i++){
        for (t = 0; t < threads; t++){
            c += counter[256 * t + i];
            counter[256 * t + i] = s;
            s = c;
        }
    }
}

static void __thradixSortTask(int id, void *data,
                              const fer_tasks_thinfo_t *thinfo)
{
    fer_cd_sap_th_radix_sort_t *rs = (fer_cd_sap_th_radix_sort_t *)data;
    fer_cd_sap_th_t *sap = fer_container_of(rs, fer_cd_sap_th_t, rs);
    int len, from, i;
    uint32_t *counter;
    fer_cd_sap_minmax_t *minmax;

    // compute length of section managed by this thread and strating
    // position ({from})
    len  = rs->minmax_len / sap->num_threads;
    from = id * len;
    if (id == sap->num_threads - 1)
        len = rs->minmax_len - from;

    counter = rs->counter + (256 * id);
    minmax  = rs->context.src + from;

    radixSortZeroizeCounter(counter);
    if (rs->context.i == rs->context.len - 1){
        radixSortCountFinal(minmax, len, counter, rs->negative + id, rs->context.shift);
    }else{
        radixSortCount(minmax, len, counter, rs->context.shift);
    }
    ferBarrier(rs->barrier);


    if (id == 0){
        thradixSortFixCounter(rs->counter, sap->num_threads);
    }else if (rs->context.i == rs->context.len - 1 && id == 1){
        for (i = 1; i < sap->num_threads; i++){
            rs->negative[0] += rs->negative[i];
        }
    }
    ferBarrier(rs->barrier);


    if (rs->context.i == rs->context.len - 1){
        radixSortSortFinal(minmax, len,
                           rs->context.dst, rs->minmax_len,
                           counter, rs->negative[0], rs->context.shift,
                           sap->sap.geoms, rs->context.axis);
    }else{
        radixSortSort(minmax, rs->context.dst, len, counter, rs->context.shift);
    }
}

static void thRadixSortSort(fer_cd_sap_th_radix_sort_t *rs)
{
    fer_cd_sap_th_t *sap = fer_container_of(rs, fer_cd_sap_th_t, rs);
    size_t i;

    for (i = 0; i < sap->num_threads; i++){
        ferTasksAdd(sap->sap.cd->tasks, __thradixSortTask, i, rs);
    }
    ferTasksBarrier(sap->sap.cd->tasks);
}



static void sapthFindPairsGeom(fer_cd_sap_th_t *sap, fer_cd_sap_geom_t *g,
                               int bucket);
static void __findPairsTask(int id, void *data,
                            const fer_tasks_thinfo_t *thinfo);

static void sapthFindPairs(fer_cd_sap_t *_sap)
{
    fer_cd_sap_th_t *sap = (fer_cd_sap_th_t *)_sap;
    size_t i;

    for (i = 0; i < sap->num_threads; i++){
        ferTasksAdd(sap->sap.cd->tasks, __findPairsTask, i, sap);
    }
    ferTasksBarrier(sap->sap.cd->tasks);
}

static void sapthFindPairsGeom(fer_cd_sap_th_t *sap, fer_cd_sap_geom_t *g,
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

    minmax = sap->sap.minmax[d];
    for (j = g->min[d] + 1; j < g->max[d]; j++){
        g2 = sap->sap.geoms[MINMAX_GEOM(&minmax[j])].g;
        if (ferCDGeomOBBOverlap(g->g, g2)){
            // create new pair
            pair = pairNew(g->g, g2);

            // add this pair to list of pairs (into specified bucket)
            ferListAppend(&sap->sap.pairs[bucket], &pair->list);
            sap->sap.pairs_len++;

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
                sap->sap.pairs_len--;
            }
            pthread_mutex_unlock(&sap->pairs_reg_lock[lockid]);
        }
    }
}

static void __findPairsTask(int id, void *data,
                            const fer_tasks_thinfo_t *thinfo)
{
    fer_cd_sap_th_t *sap = (fer_cd_sap_th_t *)data;
    int from, to, len, i;

    len  = sap->sap.geoms_len / sap->num_threads;
    from = id * len;
    to   = from + len;
    if (id == sap->num_threads - 1)
        to = sap->sap.geoms_len;

    for (i = from; i < to; i++){
        sapthFindPairsGeom(sap, &sap->sap.geoms[i], id);
    }
}
