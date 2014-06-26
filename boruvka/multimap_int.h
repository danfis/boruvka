/***
 * Boruvka
 * --------
 * Copyright (c)2014 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.bhp>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __BOR_MULTIMAP_INT_H__
#define __BOR_MULTIMAP_INT_H__

#include <boruvka/list.h>
#include <boruvka/rbtree_int.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Multi Map with Integer Key
 * ===========================
 *
 * This structure can stored multiple nodes associated with the (possibly
 * same) integer key.
 */

/**
 * Connector to the multi-map.
 */
typedef bor_list_t bor_multimap_int_node_t;



struct _bor_multimap_int_keynode_t {
    bor_list_t nodes;
    bor_rbtree_int_node_t node;
};
typedef struct _bor_multimap_int_keynode_t bor_multimap_int_keynode_t;

struct _bor_multimap_int_t {
    bor_rbtree_int_t tree;
    bor_multimap_int_keynode_t *pre_keynode; /*!< Preallocated key-node */
};
typedef struct _bor_multimap_int_t bor_multimap_int_t;

/**
 * Creates a new multimap.
 */
bor_multimap_int_t *borMultiMapIntNew(void);

/**
 * Deletes the multi-map.
 */
void borMultiMapIntDel(bor_multimap_int_t *mm);

/**
 * Returns true if the multimap is empty.
 */
_bor_inline int borMultiMapIntEmpty(const bor_multimap_int_t *mm);

/**
 * Inserts a new node into the multimap and associate it with the given
 * key.
 */
void borMultiMapIntInsert(bor_multimap_int_t *mm,
                          int key, bor_multimap_int_node_t *node);

/**
 * Removes a node from the map.
 */
void borMultiMapIntRemove(bor_multimap_int_t *mm,
                          bor_multimap_int_node_t *node);

/**
 * Returns a list of values associated with the key, or NULL if there is no
 * such a key. See BOR_MULTIMAP_INT_FOR_EACH() for more convinient
 * way to iterate over all values.
 */
bor_list_t *borMultiMapIntFind(bor_multimap_int_t *mm, int key);

/**
 * Returns a list of values associated with the minimal key in map.
 * See BOR_MULTIMAP_INT_MIN_FOR_EACH().
 */
bor_list_t *borMultiMapIntMin(bor_multimap_int_t *mm, int *key);

/**
 * Removes from the multimap and returns the node associated with the
 * minimal key. This function returns the node in FIFO manner.
 */
bor_multimap_int_node_t *borMultiMapIntExtractMinNodeFifo(
            bor_multimap_int_t *mm, int *key);

/**
 * Removes from the multimap and returns the node associated with the
 * minimal key. This function returns the node in LIFO manner.
 */
bor_multimap_int_node_t *borMultiMapIntExtractMinNodeLifo(
            bor_multimap_int_t *mm, int *key);

/**
 * Iterates over all values associated with the specified key.
 * The argument {list} is used for storing list's head and in {node} will
 * be available pointer to the bor_multimap_int_node_t.
 */
#define BOR_MULTIMAP_INT_FOR_EACH(multimap, key, list, node) \
    if (((list) = borMultiMapIntFind((multimap), (key))) != NULL) \
        BOR_LIST_FOR_EACH(list, node)

/**
 * Iterates over all values associated with the minimal key.
 * The argument {list} is used for storing list's head and in {node} will
 * be available pointer to the bor_multimap_int_node_t. To the argument
 * {min_key} will be stored value of the minimal key.
 */
#define BOR_MULTIMAP_INT_MIN_FOR_EACH(multimap, list, node, min_key) \
    if (((list) = borMultiMapIntMin(multimap, &(min_key))) != NULL) \
        BOR_LIST_FOR_EACH(list, node)

/**** INLINES: ****/
_bor_inline int borMultiMapIntEmpty(const bor_multimap_int_t *mm)
{
    return borRBTreeIntEmpty(&mm->tree);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_MULTIMAP_INT_H__ */
