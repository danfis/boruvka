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


#define RB_NODE bor_rbtree_node_t
#define RB_TREE bor_rbtree_t
#define RB_DEL borRBTreeDel
#define RB_INSERT RB_NODE *borRBTreeInsert(RB_TREE *rbtree, RB_NODE *n)
#define RB_REMOVE borRBTreeRemove
#define RB_SET_UP_KEY
#define RB_CMP rbtree->cmp(n, parent, rbtree->data)
#define RB_CP(dst, src) *(dst) = *(src);

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
#define RB_COPY_COLOR(dst, src) ((dst)->rbe_color = (src)->rbe_color)

#include "_rbtree.c"

RB_TREE *borRBTreeNew(bor_rbtree_cmp cmp, void *data)
{
    RB_TREE *rb;

    rb = BOR_ALLOC(RB_TREE);
    borRBTreeInit(rb, cmp, data);

    return rb;
}

void borRBTreeInit(bor_rbtree_t *rb, bor_rbtree_cmp cmp, void *data)
{
    rb->root = NULL;
    rb->cmp  = cmp;
    rb->data = data;
}

void borRBTreeFree(bor_rbtree_t *rb)
{
    rb->root = NULL;
    rb->cmp  = NULL;
    rb->data = NULL;
}
