/***
 * Boruvka
 * --------
 * Copyright (c)2019 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __BOR_HASHSET_H__
#define __BOR_HASHSET_H__

#include <boruvka/segmarr.h>
#include <boruvka/htable.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef bor_htable_key_t (*bor_hashset_hash_fn)(const void *el, void *ud);
typedef int (*bor_hashset_eq_fn)(const void *el1, const void *el2, void *ud);
typedef void *(*bor_hashset_clone_fn)(const void *el, void *ud);
typedef void (*bor_hashset_del_fn)(void *el, void *ud);

struct bor_hashset {
    bor_hashset_hash_fn hash_fn;
    bor_hashset_eq_fn eq_fn;
    bor_hashset_clone_fn clone_fn;
    bor_hashset_del_fn del_fn;
    void *userdata;

    bor_segmarr_t *el;
    bor_htable_t *htable;
    int size;
};
typedef struct bor_hashset bor_hashset_t;

/**
 * Initializes an empty set.
 */
void borHashSetInit(bor_hashset_t *s,
                    bor_hashset_hash_fn hash_fn,
                    bor_hashset_eq_fn eq_fn,
                    bor_hashset_clone_fn clone_fn,
                    bor_hashset_del_fn del_fn,
                    void *userdata);

/**
 * Initialize an empty set of {i,l,c}sets.
 */
void borHashSetInitISet(bor_hashset_t *s);
void borHashSetInitLSet(bor_hashset_t *s);
void borHashSetInitCSet(bor_hashset_t *s);

/**
 * Free allocated memory.
 */
void borHashSetFree(bor_hashset_t *s);

/**
 * Inserts the element into set and returns its ID within the sets if
 * successful.
 * If the element is already there it returns its ID and nothing is inserted.
 */
int borHashSetAdd(bor_hashset_t *s, const void *el);

/**
 * Returns ID of the element or -1 if it is not there.
 */
int borHashSetFind(const bor_hashset_t *s, const void *el);

/**
 * Returns the element with the corresponding ID.
 */
const void *borHashSetGet(const bor_hashset_t *s, int id);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_HASHSET_H__ */
