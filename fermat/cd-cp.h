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

#ifndef __FER_CD_CP_H__
#define __FER_CD_CP_H__

#include <fermat/cd.h>
#include <fermat/cd-geom.h>
#include <fermat/hmap.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Contact Persistence
 * ====================
 */
struct _fer_cd_cp_t {
    fer_hmap_t *cs;      /*!< Hash table of contacts */
    fer_real_t max_dist; /*!< Maximal squared distance a contact point can
                              move to be considered "still" */

    fer_list_t active;   /*!< List of active contacts */
    fer_list_t unactive; /*!< List of unactive contacts */

    pthread_mutex_t lock; /*!< Global lock */
};
typedef struct _fer_cd_cp_t fer_cd_cp_t;

/**
 * Creates new contact persistence structure
 */
fer_cd_cp_t *ferCDCPNew(size_t hashsize, fer_real_t max_dist);

/**
 * Deletes the structure
 */
void ferCDCPDel(fer_cd_cp_t *cp);

/**
 * 1. If {cp} is NULL {con} is returned.
 * 2. If both {g1} and {g2} doesn't support contact presistence, {con} is
 *    returned.
 * 3. If either {g1} or {g2} support contact presistence, the contacts
 *    between {g1} and {g2} are updated and returned.
 */
const fer_cd_contacts_t *ferCDCPUpdate(fer_cd_cp_t *cp,
                                       const fer_cd_geom_t *g1,
                                       const fer_cd_geom_t *g2,
                                       const fer_cd_contacts_t *con);

/**
 * Performs maintainance of the register of the contacts.
 * This must be called at the end of each collision detection cycle.
 */
void ferCDCPMaintainance(fer_cd_cp_t *cp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_CP_H__ */
