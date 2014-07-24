/***
 * Boruvka
 * --------
 * Copyright (c)2014 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.splaytreep>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __BOR_SPLAYTREE_INT_H__
#define __BOR_SPLAYTREE_INT_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Splay Tree with Integer Keys
 * =============================
 *
 */

/** vvvv */
struct _bor_splaytree_int_node_t {
    int key;
    struct _bor_splaytree_int_node_t *spe_left;
    struct _bor_splaytree_int_node_t *spe_right;
};
typedef struct _bor_splaytree_int_node_t bor_splaytree_int_node_t;


struct _bor_splaytree_int_t {
    bor_splaytree_int_node_t *root;
};
typedef struct _bor_splaytree_int_t bor_splaytree_int_t;
/** ^^^^ */

/**
 * Functions
 * ----------
 */

/**
 * Creates a new empty Red Black Tree.
 */
bor_splaytree_int_t *borSplayTreeIntNew(void);


/**
 * Deletes a splaytree.
 * Note that individual nodes are not disconnected from the tree.
 */
void borSplayTreeIntDel(bor_splaytree_int_t *splaytree_int);

/**
 * In-place initialization.
 */
void borSplayTreeIntInit(bor_splaytree_int_t *st);

/**
 * Pair free() for borSplayTreeIntInit().
 */
void borSplayTreeIntFree(bor_splaytree_int_t *st);

/**
 * Returns true if the tree is empty.
 */
_bor_inline int borSplayTreeIntEmpty(const bor_splaytree_int_t *splaytree_int);

/**
 * Inserts a new node into the tree.
 * If an equal node is already in the tree the given node is not inserted
 * and the equal node from the tree is returned.
 * If the given node isn't in the tree, the node is inserted and NULL is
 * returned.
 */
bor_splaytree_int_node_t *borSplayTreeIntInsert(bor_splaytree_int_t *splaytree_int,
                                                int key,
                                                bor_splaytree_int_node_t *n);

/**
 * Del a node from the tree.
 */
bor_splaytree_int_node_t *borSplayTreeIntRemove(bor_splaytree_int_t *splaytree_int,
                                                bor_splaytree_int_node_t *n);


/**
 * Finds the node with the same key as elm.
 */
bor_splaytree_int_node_t *borSplayTreeIntFind(bor_splaytree_int_t *splaytree_int,
                                              int key);

/**
 * Returns next higher node.
 */
bor_splaytree_int_node_t *borSplayTreeIntNext(bor_splaytree_int_t *st,
                                              bor_splaytree_int_node_t *n);

/**
 * Returns previous smaller node.
 */
bor_splaytree_int_node_t *borSplayTreeIntPrev(bor_splaytree_int_t *st,
                                              bor_splaytree_int_node_t *n);

/**
 * Minimal node from the tree.
 */
bor_splaytree_int_node_t *borSplayTreeIntMin(bor_splaytree_int_t *splaytree_int);

/**
 * Maximal node from the tree.
 */
bor_splaytree_int_node_t *borSplayTreeIntMax(bor_splaytree_int_t *splaytree_int);

/**
 * Removes and returns minimal node from heap.
 */
_bor_inline bor_splaytree_int_node_t *borSplayTreeIntExtractMin(bor_splaytree_int_t *splaytree_int);

/**
 * Returns a key value of the node as was inserted into the tree.
 */
_bor_inline int borSplayTreeIntKey(const bor_splaytree_int_node_t *n);


#define BOR_SPLAYTREE_INT_FOR_EACH(tree, node) \
    for ((node) = borSplayTreeIntMin(tree); \
         (node) != NULL; \
         (node) = borSplayTreeIntNext((tree), (node)))

#define BOR_SPLAYTREE_INT_FOR_EACH_FROM(tree, node) \
    for (; \
         (node) != NULL; \
         (node) = borSplayTreeIntNext((tree), (node)))

#define BOR_SPLAYTREE_INT_FOR_EACH_SAFE(tree, node, tmp) \
    for ((node) = borSplayTreeIntMin(tree); \
         (node) != NULL && ((tmp) = borSplayTreeIntNext((tree), (node)), 1); \
         (node) = (tmp))

#define BOR_SPLAYTREE_INT_FOR_EACH_REVERSE(tree, node) \
    for ((node) = borSplayTreeIntMax(tree); \
         (node) != NULL; \
         (node) = borSplayTreeIntPrev((tree), (node)))

#define BOR_SPLAYTREE_INT_FOR_EACH_REVERSE_FROM(tree, node) \
    for (; \
         (node) != NULL; \
         (node) = borSplayTreeIntPrev((tree), (node)))

#define BOR_SPLAYTREE_INT_FOR_EACH_REVERSE_SAFE(tree, node, tmp) \
    for ((node) = borSplayTreeIntMax(tree); \
         (node) != NULL && ((tmp) = borSplayTreeIntPrev((tree), (node)), 1); \
         (node) = (tmp))

/**** INLINES ****/
_bor_inline int borSplayTreeIntEmpty(const bor_splaytree_int_t *splaytree_int)
{
    return splaytree_int->root == NULL;
}

_bor_inline bor_splaytree_int_node_t *borSplayTreeIntExtractMin(bor_splaytree_int_t *splaytree_int)
{
    bor_splaytree_int_node_t *n;
    n = borSplayTreeIntExtractMin(splaytree_int);
    if (n)
        borSplayTreeIntRemove(splaytree_int, n);
    return n;
}

_bor_inline int borSplayTreeIntKey(const bor_splaytree_int_node_t *n)
{
    return n->key;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_SPLAYTREE_INT_H__ */
