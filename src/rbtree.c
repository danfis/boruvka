/***
 * Boruvka
 * --------
 * Copyright (c)2014 Daniel Fiser <danfis@danfis.cz>
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

#include "boruvka/alloc.h"
#include "boruvka/rbtree.h"


#define RB_BLACK 0
#define RB_RED   1

#define RB_LEFT(node)   (node)->rbe_left
#define RB_RIGHT(node)  (node)->rbe_right
#define RB_PARENT(node) (node)->rbe_parent
#define RB_COLOR(node)  (node)->rbe_color

#define RB_SET_RED(node)   (RB_COLOR(node) = RB_RED)
#define RB_SET_BLACK(node) (RB_COLOR(node) = RB_BLACK)
#define RB_IS_RED(node)    (RB_COLOR(node) == RB_RED)
#define RB_IS_BLACK(node)  (RB_COLOR(node) == RB_BLACK)
#define RB_SET_BLACKRED(black, red) \
    do { \
        RB_SET_BLACK(black); \
        RB_SET_RED(red); \
    } while (0)

#define RB_ROTATE_LEFT(head, elm, tmp) do { \
    (tmp) = RB_RIGHT(elm); \
    if ((RB_RIGHT(elm) = RB_LEFT(tmp)) != NULL) { \
        RB_PARENT(RB_LEFT(tmp)) = (elm); \
    } \
    if ((RB_PARENT(tmp) = RB_PARENT(elm)) != NULL) { \
        if ((elm) == RB_LEFT(RB_PARENT(elm))) \
            RB_LEFT(RB_PARENT(elm)) = (tmp); \
        else \
            RB_RIGHT(RB_PARENT(elm)) = (tmp); \
    } else \
        (head)->root = (tmp); \
    RB_LEFT(tmp) = (elm); \
    RB_PARENT(elm) = (tmp); \
} while (0)

#define RB_ROTATE_RIGHT(head, elm, tmp) do { \
    (tmp) = RB_LEFT(elm); \
    if ((RB_LEFT(elm) = RB_RIGHT(tmp)) != NULL) { \
        RB_PARENT(RB_RIGHT(tmp)) = (elm); \
    } \
    if ((RB_PARENT(tmp) = RB_PARENT(elm)) != NULL) { \
        if ((elm) == RB_LEFT(RB_PARENT(elm))) \
            RB_LEFT(RB_PARENT(elm)) = (tmp); \
        else \
            RB_RIGHT(RB_PARENT(elm)) = (tmp); \
    } else \
        (head)->root = (tmp); \
    RB_RIGHT(tmp) = (elm); \
    RB_PARENT(elm) = (tmp); \
} while (/*CONSTCOND*/ 0)

_bor_inline void rbSet(bor_rbtree_node_t *elm, bor_rbtree_node_t *parent)
{
    RB_PARENT(elm) = parent;
    RB_LEFT(elm) = RB_RIGHT(elm) = NULL;
    RB_SET_RED(elm);
}


bor_rbtree_t *borRBTreeNew(bor_rbtree_cmp cmp, void *data)
{
    bor_rbtree_t *rb;

    rb = BOR_ALLOC(bor_rbtree_t);
    rb->root = NULL;
    rb->cmp = cmp;
    rb->data = data;

    return rb;
}

void borRBTreeDel(bor_rbtree_t *rbtree)
{
    BOR_FREE(rbtree);
}

static void rbInsertColor(bor_rbtree_t *rbtree,
                          bor_rbtree_node_t *elm)
{
    bor_rbtree_node_t *parent, *gparent, *tmp;
    while ((parent = RB_PARENT(elm)) != NULL && RB_IS_RED(parent)){
        gparent = RB_PARENT(parent);
        if (parent == RB_LEFT(gparent)){
            tmp = RB_RIGHT(gparent);
            if (tmp && RB_IS_RED(tmp)){
                RB_SET_BLACK(tmp);
                RB_SET_BLACKRED(parent, gparent);
                elm = gparent;
                continue;
            }
            if (RB_RIGHT(parent) == elm) {
                RB_ROTATE_LEFT(rbtree, parent, tmp);
                tmp = parent;
                parent = elm;
                elm = tmp;
            }
            RB_SET_BLACKRED(parent, gparent);
            RB_ROTATE_RIGHT(rbtree, gparent, tmp);
        } else {
            tmp = RB_LEFT(gparent);
            if (tmp && RB_IS_RED(tmp)) {
                RB_SET_BLACK(tmp);
                RB_SET_BLACKRED(parent, gparent);
                elm = gparent;
                continue;
            }
            if (RB_LEFT(parent) == elm) {
                RB_ROTATE_RIGHT(rbtree, parent, tmp);
                tmp = parent;
                parent = elm;
                elm = tmp;
            }
            RB_SET_BLACKRED(parent, gparent);
            RB_ROTATE_LEFT(rbtree, gparent, tmp);
        }
    }
    RB_SET_BLACK(rbtree->root);
}

bor_rbtree_node_t *borRBTreeInsert(bor_rbtree_t *rbtree,
                                   bor_rbtree_node_t *n)
{
    bor_rbtree_node_t *tmp;
    bor_rbtree_node_t *parent = NULL;
    int comp = 0;

    tmp = rbtree->root;
    while (tmp){
        parent = tmp;
        comp = rbtree->cmp(n, parent, rbtree->data);
        if (comp < 0){
            tmp = RB_LEFT(tmp);
        }else if (comp > 0){
            tmp = RB_RIGHT(tmp);
        }else{
            return tmp;
        }
    }

    rbSet(n, parent);
    if (parent != NULL){
        if (comp < 0){
            RB_LEFT(parent) = n;
        }else{
            RB_RIGHT(parent) = n;
        }
    }else{
        rbtree->root = n;
    }

    rbInsertColor(rbtree, n);
    return NULL;
}

static void rbRemoveColor(bor_rbtree_t *rbtree, bor_rbtree_node_t *parent,
                          bor_rbtree_node_t *elm)
{
    bor_rbtree_node_t *tmp;

    while ((elm == NULL || RB_COLOR(elm) == RB_BLACK) &&
        elm != rbtree->root) {
        if (RB_LEFT(parent) == elm) {
            tmp = RB_RIGHT(parent);
            if (RB_COLOR(tmp) == RB_RED) {
                RB_SET_BLACKRED(tmp, parent);
                RB_ROTATE_LEFT(rbtree, parent, tmp);
                tmp = RB_RIGHT(parent);
            }
            if ((RB_LEFT(tmp) == NULL ||
                RB_COLOR(RB_LEFT(tmp)) == RB_BLACK) &&
                (RB_RIGHT(tmp) == NULL ||
                RB_COLOR(RB_RIGHT(tmp)) == RB_BLACK)) {
                RB_COLOR(tmp) = RB_RED;
                elm = parent;
                parent = RB_PARENT(elm);
            } else {
                if (RB_RIGHT(tmp) == NULL ||
                    RB_COLOR(RB_RIGHT(tmp)) == RB_BLACK) {
                    bor_rbtree_node_t *oleft;
                    if ((oleft = RB_LEFT(tmp))
                        != NULL)
                        RB_COLOR(oleft) = RB_BLACK;
                    RB_COLOR(tmp) = RB_RED;
                    RB_ROTATE_RIGHT(rbtree, tmp, oleft);
                    tmp = RB_RIGHT(parent);
                }
                RB_COLOR(tmp) = RB_COLOR(parent);
                RB_COLOR(parent) = RB_BLACK;
                if (RB_RIGHT(tmp))
                    RB_COLOR(RB_RIGHT(tmp)) = RB_BLACK;
                RB_ROTATE_LEFT(rbtree, parent, tmp);
                elm = rbtree->root;
                break;
            }
        } else {
            tmp = RB_LEFT(parent);
            if (RB_COLOR(tmp) == RB_RED) {
                RB_SET_BLACKRED(tmp, parent);
                RB_ROTATE_RIGHT(rbtree, parent, tmp);
                tmp = RB_LEFT(parent);
            }
            if ((RB_LEFT(tmp) == NULL ||
                RB_COLOR(RB_LEFT(tmp)) == RB_BLACK) &&
                (RB_RIGHT(tmp) == NULL ||
                RB_COLOR(RB_RIGHT(tmp)) == RB_BLACK)) {
                RB_COLOR(tmp) = RB_RED;
                elm = parent;
                parent = RB_PARENT(elm);
            } else {
                if (RB_LEFT(tmp) == NULL ||
                    RB_COLOR(RB_LEFT(tmp)) == RB_BLACK) {
                    bor_rbtree_node_t *oright;
                    if ((oright = RB_RIGHT(tmp))
                        != NULL)
                        RB_COLOR(oright) = RB_BLACK;
                    RB_COLOR(tmp) = RB_RED;
                    RB_ROTATE_LEFT(rbtree, tmp, oright);
                    tmp = RB_LEFT(parent);
                }
                RB_COLOR(tmp) = RB_COLOR(parent);
                RB_COLOR(parent) = RB_BLACK;
                if (RB_LEFT(tmp))
                    RB_COLOR(RB_LEFT(tmp)) = RB_BLACK;
                RB_ROTATE_RIGHT(rbtree, parent, tmp);
                elm = rbtree->root;
                break;
            }
        }
    }
    if (elm)
        RB_COLOR(elm) = RB_BLACK;
}

bor_rbtree_node_t *borRBTreeRemove(bor_rbtree_t *rbtree,
                                   bor_rbtree_node_t *elm)
{
    bor_rbtree_node_t *child, *parent, *old = elm;
    int color;

    if (RB_LEFT(elm) == NULL){
        child = RB_RIGHT(elm);
    }else if (RB_RIGHT(elm) == NULL){
        child = RB_LEFT(elm);
    }else{
        bor_rbtree_node_t *left;
        elm = RB_RIGHT(elm);
        while ((left = RB_LEFT(elm)) != NULL)
            elm = left;
        child = RB_RIGHT(elm);
        parent = RB_PARENT(elm);
        color = RB_COLOR(elm);
        if (child)
            RB_PARENT(child) = parent;
        if (parent) {
            if (RB_LEFT(parent) == elm)
                RB_LEFT(parent) = child;
            else
                RB_RIGHT(parent) = child;
        } else
            rbtree->root = child;
        if (RB_PARENT(elm) == old)
            parent = elm;
        *elm = *old;
        if (RB_PARENT(old)) {
            if (RB_LEFT(RB_PARENT(old)) == old)
                RB_LEFT(RB_PARENT(old)) = elm;
            else
                RB_RIGHT(RB_PARENT(old)) = elm;
        } else
            rbtree->root = elm;
        RB_PARENT(RB_LEFT(old)) = elm;
        if (RB_RIGHT(old))
            RB_PARENT(RB_RIGHT(old)) = elm;
        if (parent) {
            left = parent;
        }
        goto color;
    }
    parent = RB_PARENT(elm);
    color = RB_COLOR(elm);
    if (child)
        RB_PARENT(child) = parent;
    if (parent) {
        if (RB_LEFT(parent) == elm)
            RB_LEFT(parent) = child;
        else
            RB_RIGHT(parent) = child;
    } else
        rbtree->root = child;
color:
    if (color == RB_BLACK)
        rbRemoveColor(rbtree, parent, child);
    return (old);
}


bor_rbtree_node_t *borRBTreeFind(bor_rbtree_t *rbtree,
                                 bor_rbtree_node_t *elm)
{
    // TODO: Inline, refactor
    bor_rbtree_node_t *tmp = rbtree->root;
    int comp;

    while (tmp) {
        comp = rbtree->cmp(elm, tmp, rbtree->data);
        if (comp < 0)
            tmp = RB_LEFT(tmp);
        else if (comp > 0)
            tmp = RB_RIGHT(tmp);
        else
            return (tmp);
    }
    return NULL;
}

bor_rbtree_node_t *borRBTreeFindNearestGE(bor_rbtree_t *rbtree,
                                          bor_rbtree_node_t *elm)
{
    // TODO: Inline
    bor_rbtree_node_t *tmp = rbtree->root;
    bor_rbtree_node_t *res = NULL;
    int comp;

    while (tmp) {
        comp = rbtree->cmp(elm, tmp, rbtree->data);
        if (comp < 0) {
            res = tmp;
            tmp = RB_LEFT(tmp);
        }
        else if (comp > 0)
            tmp = RB_RIGHT(tmp);
        else
            return (tmp);
    }
    return (res);
}

bor_rbtree_node_t *borRBTreeNext(bor_rbtree_node_t *elm)
{
    // TODO: Inline
    if (RB_RIGHT(elm)) {
        elm = RB_RIGHT(elm);
        while (RB_LEFT(elm))
            elm = RB_LEFT(elm);
    } else {
        if (RB_PARENT(elm) &&
            (elm == RB_LEFT(RB_PARENT(elm))))
            elm = RB_PARENT(elm);
        else {
            while (RB_PARENT(elm) &&
                (elm == RB_RIGHT(RB_PARENT(elm))))
                elm = RB_PARENT(elm);
            elm = RB_PARENT(elm);
        }
    }
    return (elm);
}

bor_rbtree_node_t *borRBTreePrev(bor_rbtree_node_t *elm)
{
    // TODO: Inline
    if (RB_LEFT(elm)) {
        elm = RB_LEFT(elm);
        while (RB_RIGHT(elm))
            elm = RB_RIGHT(elm);
    } else {
        if (RB_PARENT(elm) &&
            (elm == RB_RIGHT(RB_PARENT(elm))))
            elm = RB_PARENT(elm);
        else {
            while (RB_PARENT(elm) &&
                (elm == RB_LEFT(RB_PARENT(elm))))
                elm = RB_PARENT(elm);
            elm = RB_PARENT(elm);
        }
    }
    return (elm);
}

bor_rbtree_node_t *borRBTreeMin(bor_rbtree_t *rbtree)
{
    // TODO: Inline
    bor_rbtree_node_t *tmp = rbtree->root;
    bor_rbtree_node_t *parent = NULL;

    while (tmp) {
        parent = tmp;
        tmp = RB_LEFT(tmp);
    }
    return (parent);
}

bor_rbtree_node_t *borRBTreeMax(bor_rbtree_t *rbtree)
{
    // TODO: Inline
    bor_rbtree_node_t *tmp = rbtree->root;
    bor_rbtree_node_t *parent = NULL;

    while (tmp) {
        parent = tmp;
        tmp = RB_RIGHT(tmp);
    }
    return (parent);
}
