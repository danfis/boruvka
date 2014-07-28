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

#include "boruvka/alloc.h"
#include "boruvka/splaytree_int.h"

#define BOR_SPLAY_TREE_NODE_T bor_splaytree_int_node_t
#define BOR_SPLAY_TREE_T bor_splaytree_int_t
#define BOR_SPLAY_KEY_T int
#define BOR_SPLAY_NODE_KEY(node) (node)->key
#define BOR_SPLAY_NODE_SET_KEY(head, node, Key) ((node)->key = (Key))
#define BOR_SPLAY_KEY_CMP(head, key1, key2) ((key1) - (key2))
#define BOR_SPLAY_KEY_EQ(head, key1, key2) ((key1) == (key2))
#include "boruvka/splaytree_def.h"

bor_splaytree_int_t *borSplayTreeIntNew(void)
{
    bor_splaytree_int_t *st;

    st = BOR_ALLOC(bor_splaytree_int_t);
    borSplayTreeIntInit(st);
    return st;
}

void borSplayTreeIntDel(bor_splaytree_int_t *splaytree)
{
    borSplayFree(splaytree);
    BOR_FREE(splaytree);
}

void borSplayTreeIntInit(bor_splaytree_int_t *st)
{
    borSplayInit(st);
}

void borSplayTreeIntFree(bor_splaytree_int_t *st)
{
    borSplayFree(st);
}


bor_splaytree_int_node_t *borSplayTreeIntInsert(bor_splaytree_int_t *head,
                                                int key,
                                                bor_splaytree_int_node_t *elm)
{
    return borSplayInsert(head, key, elm);
}

bor_splaytree_int_node_t *borSplayTreeIntRemove(bor_splaytree_int_t *head,
                                                bor_splaytree_int_node_t *elm)
{
    return borSplayRemove(head, elm);
}

bor_splaytree_int_node_t *borSplayTreeIntFind(bor_splaytree_int_t *head,
                                              int key)
{
    return borSplayFind(head, key);
}

bor_splaytree_int_node_t *borSplayTreeIntNext(bor_splaytree_int_t *head,
                                              bor_splaytree_int_node_t *elm)
{
    return borSplayNext(head, elm);
}

bor_splaytree_int_node_t *borSplayTreeIntPrev(bor_splaytree_int_t *head,
                                              bor_splaytree_int_node_t *elm)
{
    return borSplayPrev(head, elm);
}

bor_splaytree_int_node_t *borSplayTreeIntMin(bor_splaytree_int_t *head)
{
    return borSplayMin(head);
}

bor_splaytree_int_node_t *borSplayTreeIntMax(bor_splaytree_int_t *head)
{
    return borSplayMax(head);
}
