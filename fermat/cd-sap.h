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

/**
 * Sweep and Prune
 * ----------------
 */

struct _fer_cd_geom_t;

struct _fer_cd_sap_geom_t {
    struct _fer_cd_geom_t *g;
    uint32_t min[3], max[3];
} fer_packed;
typedef struct _fer_cd_sap_geom_t fer_cd_sap_geom_t;

struct _fer_cd_sap_minmax_t {
    uint32_t geom_ismax; /*!< 31: geom id, 1: ismax */
    fer_real_t val;
} fer_packed;
typedef struct _fer_cd_sap_minmax_t fer_cd_sap_minmax_t;

#define FER_CD_SAP_TYPE_1       0x0
#define FER_CD_SAP_TYPE_THREADS 0x1
#define FER_CD_SAP_TYPE_GPU     0x2

struct _fer_cd_sap_t {
    int type;                 /*!< Type of SAP */
    void (*radix_sort)(struct _fer_cd_sap_t *, int axis);
    void (*find_pairs)(struct _fer_cd_sap_t *);

    fer_cd_t *cd;             /*!< Back pointer to main struct */
    fer_vec3_t axis[3];       /*!< Projection axes */
    fer_cd_sap_geom_t *geoms; /*!< Array of geoms */
    size_t geoms_len;         /*!< Length of .geoms[] */
    size_t geoms_alloc;       /*!< Length of allocated memory for .geoms[] */
    fer_cd_sap_minmax_t *minmax[3]; /*!< Array of min/max values */
    uint8_t dirty;            /*!< True if any geom was changed */
    uint32_t added;           /*!< Number of geoms added from last call of
                                   ferCDSAPProcess() */

    fer_list_t *pairs;    /*!< Array of lists of possible collide pairs
                               (fer_cd_sap_pair_t is connected by .list). */
    size_t pairs_buckets; /*!< Length of .pairs[] */
    size_t pairs_len;     /*!< Overall number of collide pairs in all
                               buckets of .pairs[] */
} fer_packed fer_aligned(16);
typedef struct _fer_cd_sap_t fer_cd_sap_t;

/**
 * Struct representing collide pair of geoms
 */
struct _fer_cd_sap_pair_t {
    struct _fer_cd_geom_t *g[2]; /*!< Reference to geoms */
    fer_list_t hmap;             /*!< Connection into hash map */
    fer_list_t list;             /*!< Connection into sap.collide_pairs
                                      list */
} fer_packed;
typedef struct _fer_cd_sap_pair_t fer_cd_sap_pair_t;

/**
 * Flags has this layout (from most significant bits):
 * 24 bits: hash table size
 *  8 bits: num threads
 * 32 bits: reserved
 */

/**
 * Sets size of hash table.
 * Default 1024.
 */
#define FER_CD_SAP_HASH_TABLE_SIZE(size) \
    (((uint64_t)((uint32_t)size & ~((~0u) << 24u))) << (64u - 24u))
#define __FER_CD_SAP_HASH_TABLE_SIZE(flags) \
    (((uint64_t)flags) >> (64u - 24u))

/**
 * Sets number of threads used by SAP.
 * Default 0.
 */
#define FER_CD_SAP_THREADS(num_threads) \
    (((uint64_t)((uint32_t)num_threads & 0xffu)) << (64u - 24u - 8u))
#define __FER_CD_SAP_THREADS(flags) \
    ((((uint64_t)flags) >> (64u - 24u - 8u)) & 0xffu)

/**
 * Creates new SAP instance.
 * Note that for one fer_cd_t instance must be maximally one SAP instance.
 */
fer_cd_sap_t *ferCDSAPNew(fer_cd_t *cd, uint64_t flags);

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

/**
 * Find out all collide pairs.
 */
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
    return (size_t)sap->pairs_buckets;
}

_fer_inline const fer_list_t *ferCDSAPCollidePairs(const fer_cd_sap_t *sap,
                                                   size_t bucket)
{
    return &sap->pairs[bucket];
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_SAP_H__ */

