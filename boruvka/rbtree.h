/***
 * Boruvka
 * --------
 * Copyright (c)2014 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.rbtreep>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __BOR_RBTREE_H__
#define __BOR_RBTREE_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Red Black Tree
 * ===============
 *
 * A red-black tree is a binary search tree with the node color as an
 * extra attribute.  It fulfills a set of conditions:
 *	- every search path from the root to a leaf consists of the
 *	  same number of black nodes,
 *	- each red node (except for the root) has a black parent,
 *	- each leaf node is black.
 *
 * Every operation on a red-black tree is bounded as O(lg n).
 * The maximum height of a red-black tree is 2lg (n+1).
 *
 * The code was adopted from the FreeBSD's sys/tree.h:
 *
 * Copyright 2002 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** vvvv */
struct _bor_rbtree_node_t {
	struct _bor_rbtree_node_t *rbe_left;   /*!< left element */
	struct _bor_rbtree_node_t *rbe_right;  /*!< right element */
	struct _bor_rbtree_node_t *rbe_parent; /*!< parent element */
	int rbe_color;                         /*!< node color */
};
typedef struct _bor_rbtree_node_t bor_rbtree_node_t;

/**
 * Callback that should return 0 if nodes equal, negative number if n1 < n2
 * and positive number if n1 > n2.
 */
typedef int (*bor_rbtree_cmp)(const bor_rbtree_node_t *n1,
                              const bor_rbtree_node_t *n2,
                              void *data);

struct _bor_rbtree_t {
    bor_rbtree_node_t *root;
    bor_rbtree_cmp cmp;
    void *data;
};
typedef struct _bor_rbtree_t bor_rbtree_t;
/** ^^^^ */

/**
 * Functions
 * ----------
 */

/**
 * Creates a new empty Red Black Tree.
 * Callback for comparison must be provided.
 */
bor_rbtree_t *borRBTreeNew(bor_rbtree_cmp cmp, void *data);


/**
 * Deletes a rbtree.
 * Note that individual nodes are not disconnected from the tree.
 */
void borRBTreeDel(bor_rbtree_t *rbtree);

/**
 * In-place initialization of RB-Tree.
 */
void borRBTreeInit(bor_rbtree_t *rbtree, bor_rbtree_cmp cmp, void *data);

/**
 * Pair free() function for borRBTreeInit().
 */
void borRBTreeFree(bor_rbtree_t *rbtree);

/**
 * Returns true if the tree is empty.
 */
_bor_inline int borRBTreeEmpty(const bor_rbtree_t *rbtree);

/**
 * Inserts a new node into the tree.
 * If an equal node is already in the tree (see bor_rbtree_cmp callback
 * above) the given node is not inserted and the equal node from the tree
 * is returned.
 * If the given node isn't in the tree, the node is inserted and NULL is
 * returned.
 */
bor_rbtree_node_t *borRBTreeInsert(bor_rbtree_t *rbtree,
                                   bor_rbtree_node_t *n);

/**
 * Del a node from the tree.
 */
bor_rbtree_node_t *borRBTreeRemove(bor_rbtree_t *rbtree,
                                   bor_rbtree_node_t *n);


/**
 * Finds the node with the same key as elm.
 */
_bor_inline bor_rbtree_node_t *borRBTreeFind(bor_rbtree_t *rbtree,
                                             bor_rbtree_node_t *elm);

/**
 * Finds the first node greater than or equal to the search key.
 */
_bor_inline bor_rbtree_node_t *borRBTreeFindNearestGE(bor_rbtree_t *rbtree,
                                                      bor_rbtree_node_t *elm);


/**
 * Returns next higher node.
 */
_bor_inline bor_rbtree_node_t *borRBTreeNext(bor_rbtree_node_t *n);

/**
 * Returns previous smaller node.
 */
_bor_inline bor_rbtree_node_t *borRBTreePrev(bor_rbtree_node_t *n);

/**
 * Minimal node from the tree.
 */
_bor_inline bor_rbtree_node_t *borRBTreeMin(bor_rbtree_t *rbtree);

/**
 * Maximal node from the tree.
 */
_bor_inline bor_rbtree_node_t *borRBTreeMax(bor_rbtree_t *rbtree);

/**
 * Removes and returns minimal node from heap.
 */
_bor_inline bor_rbtree_node_t *borRBTreeExtractMin(bor_rbtree_t *rbtree);


#define BOR_RBTREE_FOR_EACH(rbtree, node) \
    for ((node) = borRBTreeMin(rbtree); \
         (node) != NULL; \
         (node) = borRBTreeNext(node))

#define BOR_RBTREE_FOR_EACH_FROM(rbtree, node) \
    for (; \
         (node) != NULL; \
         (node) = borRBTreeNext(node))

#define BOR_RBTREE_FOR_EACH_SAFE(rbtree, node, tmp) \
    for ((node) = borRBTreeMin(rbtree); \
         (node) != NULL && ((tmp) = borRBTreeNext(node), (node) != NULL); \
         (node) = (tmp))

#define BOR_RBTREE_FOR_EACH_REVERSE(rbtree, node) \
    for ((node) = borRBTreeMax(rbtree); \
         (node) != NULL; \
         (node) = borRBTreePrev(node))

#define BOR_RBTREE_FOR_EACH_REVERSE_FROM(rbtree, node) \
    for (; \
         (node) != NULL; \
         (node) = borRBTreePrev(node))

#define BOR_RBTREE_FOR_EACH_REVERSE_SAFE(rbtree, node, tmp) \
    for ((node) = borRBTreeMax(rbtree); \
         (node) != NULL && ((tmp) = borRBTreePrev(node), (node) != NULL); \
         (node) = (tmp))

/**** INLINES ****/
_bor_inline int borRBTreeEmpty(const bor_rbtree_t *rbtree)
{
    return rbtree->root == NULL;
}

_bor_inline bor_rbtree_node_t *borRBTreeFind(bor_rbtree_t *rbtree,
                                             bor_rbtree_node_t *elm)
{
    bor_rbtree_node_t *tmp = rbtree->root;
    int comp;

    while (tmp) {
        comp = rbtree->cmp(elm, tmp, rbtree->data);
        if (comp < 0){
            tmp = tmp->rbe_left;
        }else if (comp > 0){
            tmp = tmp->rbe_right;
        }else{
            return tmp;
        }
    }
    return NULL;
}

_bor_inline bor_rbtree_node_t *borRBTreeFindNearestGE(bor_rbtree_t *rbtree,
                                                      bor_rbtree_node_t *elm)
{
    bor_rbtree_node_t *tmp = rbtree->root;
    bor_rbtree_node_t *res = NULL;
    int comp;

    while (tmp) {
        comp = rbtree->cmp(elm, tmp, rbtree->data);
        if (comp < 0) {
            res = tmp;
            tmp = tmp->rbe_left;
        }else if (comp > 0){
            tmp = tmp->rbe_right;
        }else{
            return tmp;
        }
    }
    return res;
}

_bor_inline bor_rbtree_node_t *borRBTreeNext(bor_rbtree_node_t *elm)
{
    if (elm->rbe_right) {
        elm = elm->rbe_right;
        while (elm->rbe_left)
            elm = elm->rbe_left;
    }else{
        if (elm->rbe_parent && elm == elm->rbe_parent->rbe_left){
            elm = elm->rbe_parent;
        }else{
            while (elm->rbe_parent && elm == elm->rbe_parent->rbe_right)
                elm = elm->rbe_parent;
            elm = elm->rbe_parent;
        }
    }
    return elm;
}

bor_rbtree_node_t *borRBTreePrev(bor_rbtree_node_t *elm)
{
    if (elm->rbe_left) {
        elm = elm->rbe_left;
        while (elm->rbe_right)
            elm = elm->rbe_right;
    }else{
        if (elm->rbe_parent && elm == elm->rbe_parent->rbe_right){
            elm = elm->rbe_parent;
        }else{
            while (elm->rbe_parent && elm == elm->rbe_parent->rbe_left)
                elm = elm->rbe_parent;
            elm = elm->rbe_parent;
        }
    }
    return elm;
}

_bor_inline bor_rbtree_node_t *borRBTreeMin(bor_rbtree_t *rbtree)
{
    bor_rbtree_node_t *tmp = rbtree->root;
    bor_rbtree_node_t *parent = NULL;

    while (tmp) {
        parent = tmp;
        tmp = tmp->rbe_left;
    }
    return parent;
}

_bor_inline bor_rbtree_node_t *borRBTreeMax(bor_rbtree_t *rbtree)
{
    bor_rbtree_node_t *tmp = rbtree->root;
    bor_rbtree_node_t *parent = NULL;

    while (tmp) {
        parent = tmp;
        tmp = tmp->rbe_right;
    }
    return parent;
}

_bor_inline bor_rbtree_node_t *borRBTreeExtractMin(bor_rbtree_t *rbtree)
{
    bor_rbtree_node_t *n;
    n = borRBTreeMin(rbtree);
    if (n)
        borRBTreeRemove(rbtree, n);
    return n;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_RBTREE_H__ */
