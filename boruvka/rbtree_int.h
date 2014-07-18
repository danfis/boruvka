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

#ifndef __BOR_RBTREE_INT_H__
#define __BOR_RBTREE_INT_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Red Black Tree with Integer Key
 * ================================
 *
 * This algorithm provides red-black tree structure with integer as a key.
 * Note that one bit from the key is used to store color of the node, so in
 * case of 32 bit ints only 31 bits are available. This means that you
 * should provide integer in range [INT_MIN/2, INT_MAX/2] inclusive.
 * The range is not checked and behaviour with keys out of the permitted
 * range is undefined.
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
struct _bor_rbtree_int_node_t {
	struct _bor_rbtree_int_node_t *left;   /*!< left element */
	struct _bor_rbtree_int_node_t *right;  /*!< right element */
	struct _bor_rbtree_int_node_t *parent; /*!< parent element */
	int color_key;                         /*!< node color and key value*/
};
typedef struct _bor_rbtree_int_node_t bor_rbtree_int_node_t;

struct _bor_rbtree_int_t {
    bor_rbtree_int_node_t *root;
};
typedef struct _bor_rbtree_int_t bor_rbtree_int_t;
/** ^^^^ */

/**
 * Functions
 * ----------
 */

/**
 * Creates a new empty Red Black Tree.
 */
bor_rbtree_int_t *borRBTreeIntNew(void);

/**
 * Deletes a rbtree.
 * Note that individual nodes are not disconnected from the tree.
 */
void borRBTreeIntDel(bor_rbtree_int_t *rbtree);

/**
 * In-place initialization of RB-tree
 */
void borRBTreeIntInit(bor_rbtree_int_t *rbtree);

/**
 * Pair free() for borRBTreeInit().
 */
void borRBTreeIntFree(bor_rbtree_int_t *rbtree);

/**
 * Returns true if the tree is empty.
 */
_bor_inline int borRBTreeIntEmpty(const bor_rbtree_int_t *rbtree);

/**
 * Inserts a new node into the tree.
 * If an equal node is already in the tree the given node is not inserted
 * and the equal node from the tree is returned.
 * If the given node isn't in the tree, the node is inserted and NULL is
 * returned.
 */
bor_rbtree_int_node_t *borRBTreeIntInsert(bor_rbtree_int_t *rbtree,
                                          int key,
                                          bor_rbtree_int_node_t *n);

/**
 * Del a node from the tree.
 */
bor_rbtree_int_node_t *borRBTreeIntRemove(bor_rbtree_int_t *rbtree,
                                          bor_rbtree_int_node_t *n);


/**
 * Finds the node with the key.
 */
_bor_inline bor_rbtree_int_node_t *borRBTreeIntFind(bor_rbtree_int_t *rbtree,
                                                    int key);

/**
 * Returns next higher node.
 */
_bor_inline bor_rbtree_int_node_t *borRBTreeIntNext(bor_rbtree_int_node_t *n);

/**
 * Returns previous smaller node.
 */
_bor_inline bor_rbtree_int_node_t *borRBTreeIntPrev(bor_rbtree_int_node_t *n);

/**
 * Minimal node from the tree.
 */
_bor_inline bor_rbtree_int_node_t *borRBTreeIntMin(bor_rbtree_int_t *rbtree);

/**
 * Maximal node from the tree.
 */
_bor_inline bor_rbtree_int_node_t *borRBTreeIntMax(bor_rbtree_int_t *rbtree);

/**
 * Removes and returns minimal node from heap.
 */
_bor_inline bor_rbtree_int_node_t *borRBTreeIntExtractMin(bor_rbtree_int_t *rbtree);

/**
 * Returns a key value of the node as was inserted into the tree.
 */
_bor_inline int borRBTreeIntKey(const bor_rbtree_int_node_t *n);

#define BOR_RBTREE_INT_FOR_EACH(rbtree, node) \
    for ((node) = borRBTreeIntMin(rbtree); \
         (node) != NULL; \
         (node) = borRBTreeIntNext(node))

#define BOR_RBTREE_INT_FOR_EACH_FROM(rbtree, node) \
    for (; \
         (node) != NULL; \
         (node) = borRBTreeIntNext(node))

#define BOR_RBTREE_INT_FOR_EACH_SAFE(rbtree, node, tmp) \
    for ((node) = borRBTreeIntMin(rbtree); \
         (node) != NULL && ((tmp) = borRBTreeIntNext(node), (node) != NULL); \
         (node) = (tmp))

#define BOR_RBTREE_INT_FOR_EACH_REVERSE(rbtree, node) \
    for ((node) = borRBTreeIntMax(rbtree); \
         (node) != NULL; \
         (node) = borRBTreeIntPrev(node))

#define BOR_RBTREE_INT_FOR_EACH_REVERSE_FROM(rbtree, node) \
    for (; \
         (node) != NULL; \
         (node) = borRBTreeIntPrev(node))

#define BOR_RBTREE_INT_FOR_EACH_REVERSE_SAFE(rbtree, node, tmp) \
    for ((node) = borRBTreeIntMax(rbtree); \
         (node) != NULL && ((tmp) = borRBTreeIntPrev(node), (node) != NULL); \
         (node) = (tmp))

/**** INLINES ****/
_bor_inline int borRBTreeIntEmpty(const bor_rbtree_int_t *rbtree)
{
    return rbtree->root == NULL;
}

_bor_inline bor_rbtree_int_node_t *borRBTreeIntFind(bor_rbtree_int_t *rbtree,
                                                    int key)
{
    bor_rbtree_int_node_t *tmp = rbtree->root;
    key = (key << 1) | 0x1;

    while (tmp) {
        if (key < (tmp->color_key | 0x1)){
            tmp = tmp->left;
        }else if (key > (tmp->color_key | 0x1)){
            tmp = tmp->right;
        }else{
            return tmp;
        }
    }
    return NULL;
}

_bor_inline bor_rbtree_int_node_t *borRBTreeIntNext(bor_rbtree_int_node_t *elm)
{
    if (elm->right) {
        elm = elm->right;
        while (elm->left)
            elm = elm->left;
    }else{
        if (elm->parent && elm == elm->parent->left){
            elm = elm->parent;
        }else{
            while (elm->parent && elm == elm->parent->right)
                elm = elm->parent;
            elm = elm->parent;
        }
    }
    return elm;
}

_bor_inline bor_rbtree_int_node_t *borRBTreeIntPrev(bor_rbtree_int_node_t *elm)
{
    if (elm->left) {
        elm = elm->left;
        while (elm->right)
            elm = elm->right;
    }else{
        if (elm->parent && elm == elm->parent->right){
            elm = elm->parent;
        }else{
            while (elm->parent && elm == elm->parent->left)
                elm = elm->parent;
            elm = elm->parent;
        }
    }
    return elm;
}

_bor_inline bor_rbtree_int_node_t *borRBTreeIntMin(bor_rbtree_int_t *rbtree)
{
    bor_rbtree_int_node_t *tmp = rbtree->root;
    bor_rbtree_int_node_t *parent = NULL;

    while (tmp) {
        parent = tmp;
        tmp = tmp->left;
    }
    return parent;
}

_bor_inline bor_rbtree_int_node_t *borRBTreeIntMax(bor_rbtree_int_t *rbtree)
{
    bor_rbtree_int_node_t *tmp = rbtree->root;
    bor_rbtree_int_node_t *parent = NULL;

    while (tmp) {
        parent = tmp;
        tmp = tmp->right;
    }
    return parent;
}

_bor_inline bor_rbtree_int_node_t *borRBTreeIntExtractMin(bor_rbtree_int_t *rbtree)
{
    bor_rbtree_int_node_t *n;
    n = borRBTreeIntMin(rbtree);
    if (n)
        borRBTreeIntRemove(rbtree, n);
    return n;
}

_bor_inline int borRBTreeIntKey(const bor_rbtree_int_node_t *n)
{
    int key;

    key = n->color_key >> 1;

    // fix a sign of key explicitly
    if (n->color_key < 0){
        key |= ~(((unsigned)-1) >> 1);
    }

    return key;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_RBTREE_INT_H__ */
