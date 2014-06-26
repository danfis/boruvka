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
#include "boruvka/rbtree_int.h"
#include <stdio.h>

#define RB_NODE bor_rbtree_int_node_t
#define RB_TREE bor_rbtree_int_t
#define RB_DEL borRBTreeIntDel
#define RB_INSERT \
    RB_NODE *borRBTreeIntInsert(RB_TREE *rbtree, int key, RB_NODE *n)
#define RB_REMOVE borRBTreeIntRemove
#define RB_SET_UP_KEY n->color_key = key = (key << 1) | 0x1;
#define RB_CMP key - (parent->color_key | 0x1)
#define RB_CP(dst, src) \
    do { \
        (dst)->left = (src)->left; \
        (dst)->right = (src)->right; \
        (dst)->parent = (src)->parent; \
        RB_COPY_COLOR((dst), (src)); \
    } while (0)

#define RB_BLACK 0x0
#define RB_RED   0x1

#define RB_LEFT(node)   (node)->left
#define RB_RIGHT(node)  (node)->right
#define RB_PARENT(node) (node)->parent
#define RB_COLOR(node)  ((node)->color_key & 0x1)

#define RB_SET_RED(node)   ((node)->color_key |= 0x1)
#define RB_SET_BLACK(node) ((node)->color_key &= ~0x1)
#define RB_IS_RED(node)    (RB_COLOR(node) == RB_RED)
#define RB_IS_BLACK(node)  (RB_COLOR(node) == RB_BLACK)
#define RB_COPY_COLOR(dst, src) \
    ((dst)->color_key = ((dst)->color_key & -2) | RB_COLOR(src))

#include "_rbtree.c"

RB_TREE *borRBTreeIntNew(void)
{
    RB_TREE *rb;

    rb = BOR_ALLOC(RB_TREE);
    borRBTreeIntInit(rb);

    return rb;
}

void borRBTreeIntInit(bor_rbtree_int_t *rbtree)
{
    rbtree->root = NULL;
}

void borRBTreeIntFree(bor_rbtree_int_t *rbtree)
{
    rbtree->root = NULL;
}
