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

#ifndef __BOR_SPLAYTREE_H__
#define __BOR_SPLAYTREE_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Splay Tree
 * ===========
 *
 * A splay tree is a self-organizing data structure.  Every operation
 * on the tree causes a splay to happen.  The splay moves the requested
 * node to the root of the tree and partly rebalances it.
 *
 * This has the benefit that request locality causes faster lookups as
 * the requested nodes move to the top of the tree.  On the other hand,
 * every lookup causes memory writes.
 *
 * The Balance Theorem bounds the total access time for m operations
 * and n inserts on an initially empty tree as O((m + n)lg n).  The
 * amortized cost for a sequence of m accesses to a splay tree is O(lg n);
 *
 */

/** vvvv */
struct _bor_splaytree_node_t {
    struct _bor_splaytree_node_t *spe_left;
    struct _bor_splaytree_node_t *spe_right;
};
typedef struct _bor_splaytree_node_t bor_splaytree_node_t;

/**
 * Callback that should return 0 if nodes equal, negative number if n1 < n2
 * and positive number if n1 > n2.
 */
typedef int (*bor_splaytree_cmp)(const bor_splaytree_node_t *n1,
                                 const bor_splaytree_node_t *n2,
                                 void *data);

struct _bor_splaytree_t {
    bor_splaytree_node_t *root;
    bor_splaytree_cmp cmp;
    void *data;
};
typedef struct _bor_splaytree_t bor_splaytree_t;
/** ^^^^ */

/**
 * Functions
 * ----------
 */

/**
 * Creates a new empty Red Black Tree.
 * Callback for comparison must be provided.
 */
bor_splaytree_t *borSplayTreeNew(bor_splaytree_cmp cmp, void *data);


/**
 * Deletes a splaytree.
 * Note that individual nodes are not disconnected from the tree.
 */
void borSplayTreeDel(bor_splaytree_t *splaytree);

/**
 * In-place initialization.
 */
void borSplayTreeInit(bor_splaytree_t *st, bor_splaytree_cmp cmp, void *data);

/**
 * Pair free() for borSplayTreeInit().
 */
void borSplayTreeFree(bor_splaytree_t *st);

/**
 * Returns true if the tree is empty.
 */
_bor_inline int borSplayTreeEmpty(const bor_splaytree_t *splaytree);

/**
 * Inserts a new node into the tree.
 * If an equal node is already in the tree (see bor_splaytree_cmp callback
 * above) the given node is not inserted and the equal node from the tree
 * is returned.
 * If the given node isn't in the tree, the node is inserted and NULL is
 * returned.
 */
bor_splaytree_node_t *borSplayTreeInsert(bor_splaytree_t *splaytree,
                                         bor_splaytree_node_t *n);

/**
 * Del a node from the tree.
 */
bor_splaytree_node_t *borSplayTreeRemove(bor_splaytree_t *splaytree,
                                         bor_splaytree_node_t *n);


/**
 * Finds the node with the same key as elm.
 */
bor_splaytree_node_t *borSplayTreeFind(bor_splaytree_t *splaytree,
                                       bor_splaytree_node_t *n);

/**
 * Returns next higher node.
 */
bor_splaytree_node_t *borSplayTreeNext(bor_splaytree_t *st,
                                       bor_splaytree_node_t *n);

/**
 * Returns previous smaller node.
 */
bor_splaytree_node_t *borSplayTreePrev(bor_splaytree_t *st,
                                       bor_splaytree_node_t *n);

/**
 * Minimal node from the tree.
 */
bor_splaytree_node_t *borSplayTreeMin(bor_splaytree_t *splaytree);

/**
 * Maximal node from the tree.
 */
bor_splaytree_node_t *borSplayTreeMax(bor_splaytree_t *splaytree);

/**
 * Removes and returns minimal node from heap.
 */
_bor_inline bor_splaytree_node_t *borSplayTreeExtractMin(bor_splaytree_t *splaytree);


#define BOR_SPLAYTREE_FOR_EACH(tree, node) \
    for ((node) = borSplayTreeMin(tree); \
         (node) != NULL; \
         (node) = borSplayTreeNext((tree), (node)))

#define BOR_SPLAYTREE_FOR_EACH_FROM(tree, node) \
    for (; \
         (node) != NULL; \
         (node) = borSplayTreeNext((tree), (node)))

#define BOR_SPLAYTREE_FOR_EACH_SAFE(tree, node, tmp) \
    for ((node) = borSplayTreeMin(tree); \
         (node) != NULL && ((tmp) = borSplayTreeNext((tree), (node)), 1); \
         (node) = (tmp))

#define BOR_SPLAYTREE_FOR_EACH_REVERSE(tree, node) \
    for ((node) = borSplayTreeMax(tree); \
         (node) != NULL; \
         (node) = borSplayTreePrev((tree), (node)))

#define BOR_SPLAYTREE_FOR_EACH_REVERSE_FROM(tree, node) \
    for (; \
         (node) != NULL; \
         (node) = borSplayTreePrev((tree), (node)))

#define BOR_SPLAYTREE_FOR_EACH_REVERSE_SAFE(tree, node, tmp) \
    for ((node) = borSplayTreeMax(tree); \
         (node) != NULL && ((tmp) = borSplayTreePrev((tree), (node)), 1); \
         (node) = (tmp))

/**** INLINES ****/
_bor_inline int borSplayTreeEmpty(const bor_splaytree_t *splaytree)
{
    return splaytree->root == NULL;
}

_bor_inline bor_splaytree_node_t *borSplayTreeExtractMin(bor_splaytree_t *splaytree)
{
    bor_splaytree_node_t *n;
    n = borSplayTreeExtractMin(splaytree);
    if (n)
        borSplayTreeRemove(splaytree, n);
    return n;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_SPLAYTREE_H__ */
