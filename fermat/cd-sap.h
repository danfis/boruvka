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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


struct _fer_cd_geom_t;

#define FER_CD_SAP_NUM_AXIS 3

/**
 * Sweep and Prune
 * ----------------
 */
struct _fer_cd_sap_t {
    fer_vec3_t axis[FER_CD_SAP_NUM_AXIS];
    fer_hmap_t *pairs; /*!< Hash map of collide pairs */
    fer_list_t minmax[FER_CD_SAP_NUM_AXIS]; /*!< Sorted lists for min/max
                                                 values along each axis */
    fer_list_t collide_pairs; /*!< List of possible collide pairs
                                  (fer_cd_sap_pair_t's connected by .list) */
} fer_packed fer_aligned(16);
typedef struct _fer_cd_sap_t fer_cd_sap_t;

/**
 * Struct representing collide pair of geoms
 */
struct _fer_cd_sap_pair_t {
    struct _fer_cd_geom_t *g[2]; /*!< Reference to geoms */
    int num_axis;                /*!< This counts number of axis where
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
fer_cd_sap_t *ferCDSAPNew(size_t hash_table_size);

/**
 * Deletes SAP
 */
void ferCDSAPDel(fer_cd_sap_t *sap);

/**
 * Updates given geom in SAP
 */
void ferCDSAPUpdate(fer_cd_sap_t *sap, struct _fer_cd_geom_t *geom);

/**
 * Returns list of possible collide pairs.
 * fer_cd_sap_pair_t structure is connected into this list by member .list.
 * Please don't change a list, but you can read it for example this way:
 * ~~~~~
 * const fer_list_t *list;
 * fer_list_t *item;
 * fer_cd_sap_pair_t *pair;
 *
 * list = ferCDSAPCollidePairs(sap);
 * FER_LIST_FOR_EACH(list, item){
 *     pair = FER_LIST_ENTRY(item, fer_cd_sap_pair_t, list);
 *
 *     // do something with a pair
 *     // ...
 * }
 */
_fer_inline const fer_list_t *ferCDSAPCollidePairs(const fer_cd_sap_t *sap);

/**
 * Remove given grom from SAP
 */
void ferCDSAPRemove(fer_cd_sap_t *sap, struct _fer_cd_geom_t *geom);

/** For debugging purposes */
void ferCDSAPDumpPairs(fer_cd_sap_t *sap, FILE *out);


/**** INLINES ****/
_fer_inline const fer_list_t *ferCDSAPCollidePairs(const fer_cd_sap_t *sap)
{
    return &sap->collide_pairs;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_SAP_H__ */

