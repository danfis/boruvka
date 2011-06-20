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

#ifndef __FER_CD_SAP_H__
#define __FER_CD_SAP_H__

#include <fermat/cd.h>
#include <fermat/barrier.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


struct _fer_cd_geom_t;
struct _fer_cd_sap_t;

struct _fer_cd_sap_geom_t {
    struct _fer_cd_geom_t *g;
    uint32_t min[3], max[3];
};
typedef struct _fer_cd_sap_geom_t fer_cd_sap_geom_t;

struct _fer_cd_sap_minmax_t {
    uint32_t geom_ismax; /*!< 31: geom id, 1: ismax */
    fer_real_t val;
};
typedef struct _fer_cd_sap_minmax_t fer_cd_sap_minmax_t;

struct _fer_cd_sap_radix_sort_t {
    fer_cd_sap_minmax_t *minmax;
    size_t minmax_len, minmax_alloc;
    uint32_t *counter;  /*!< Array for count-sort */
    uint32_t *negative; /*!< Number of negative values */

    fer_barrier_t *barrier;

    struct {
        fer_cd_sap_minmax_t *src, *dst;
        fer_uint_t mask, shift;
        int axis;
        fer_uint_t i, len;
    } context;
};
typedef struct _fer_cd_sap_radix_sort_t fer_cd_sap_radix_sort_t;

/**
 * Sweep and Prune
 * ----------------
 */
struct _fer_cd_sap_t {
    fer_cd_t *cd; /*!< Back pointer to main struct */
    size_t par;   /*!< Level of parallelization */

    fer_vec3_t axis[3];

    fer_cd_sap_geom_t *geoms;
    size_t geoms_len, geoms_alloc;
    fer_cd_sap_minmax_t *minmax[3];

    int dirty; /*!< True if any geom was changed */

    fer_cd_sap_radix_sort_t radix_sort; /*!< Cached radix sort struct */

    fer_hmap_t *pairs_reg;           /*!< Register (hash map) of collide pairs */
    pthread_mutex_t *pairs_reg_lock; /*!< Array of locks for .pairs_reg */
    size_t pairs_reg_lock_len;       /*!< Length of .pairs_reg_lock */

    fer_list_t *collide_pairs; /*!< Array of lists of possible collide pairs
                                    (fer_cd_sap_pair_t's connected by .list).
                                    Length of this array is .par */
    size_t collide_pairs_len;  /*!< Overall number of collide pairs in all
                                    buckets of .collide_pairs */
    size_t collide_pairs_next; /*!< Next .collide_pairs bucket that will be
                                    used */
} fer_packed fer_aligned(16);
typedef struct _fer_cd_sap_t fer_cd_sap_t;

/**
 * Struct representing collide pair of geoms
 */
struct _fer_cd_sap_pair_t {
    struct _fer_cd_geom_t *g[2]; /*!< Reference to geoms */
    uint8_t num_axis;            /*!< This counts number of axis where
                                      these two geoms overlap */
    fer_list_t hmap;             /*!< Connection into hash map */
    fer_list_t list;             /*!< Connection into sap.collide_pairs
                                      list */
};
typedef struct _fer_cd_sap_pair_t fer_cd_sap_pair_t;

/**
 * Creates new SAP instance.
 * Note that for one fer_cd_t instance must be maximally one SAP instance.
 */
fer_cd_sap_t *ferCDSAPNew(fer_cd_t *cd, size_t par, size_t hash_table_size);

/**
 * Deletes SAP
 */
void ferCDSAPDel(fer_cd_sap_t *sap);

/**
 * Adds geom to SAP
 */
void ferCDSAPAdd(fer_cd_sap_t *sap, struct _fer_cd_geom_t *geom);

/**
 * Updates given geom in SAP
 */
void ferCDSAPUpdate(fer_cd_sap_t *sap, struct _fer_cd_geom_t *geom);

/**
 * Remove given geom from SAP
 */
void ferCDSAPRemove(fer_cd_sap_t *sap, struct _fer_cd_geom_t *geom);

void ferCDSAPProcess(fer_cd_sap_t *sap);

/**
 * Returns number of buckets of collide pairs lists
 */
_fer_inline size_t ferCDSAPCollidePairsBuckets(const fer_cd_sap_t *sap);

/**
 * Returns list of possible collide pairs.
 * fer_cd_sap_pair_t structure is connected into this list by member .list.
 * Please don't change a list, but you can read it for example this way:
 * ~~~~~
 * const fer_list_t *list;
 * fer_list_t *item;
 * fer_cd_sap_pair_t *pair;
 *
 * list = ferCDSAPCollidePairs(sap, 0);
 * FER_LIST_FOR_EACH(list, item){
 *     pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, list);
 *
 *     // do something with a pair
 *     // ...
 * }
 */
_fer_inline const fer_list_t *ferCDSAPCollidePairs(const fer_cd_sap_t *sap,
                                                   size_t bucket);

/** For debugging purposes */
void ferCDSAPDumpPairs(fer_cd_sap_t *sap, FILE *out);
void ferCDSAPDump(fer_cd_sap_t *sap);


/**** INLINES ****/
_fer_inline size_t ferCDSAPCollidePairsBuckets(const fer_cd_sap_t *sap)
{
    return (size_t)sap->par;
}

_fer_inline const fer_list_t *ferCDSAPCollidePairs(const fer_cd_sap_t *sap,
                                                   size_t bucket)
{
    return &sap->collide_pairs[bucket];
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_SAP_H__ */

