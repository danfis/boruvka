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
#include "boruvka/splaytree.h"

#define BOR_SPLAY_TREE_NODE_T bor_splaytree_node_t
#define BOR_SPLAY_TREE_T bor_splaytree_t
#define BOR_SPLAY_KEY_T const bor_splaytree_node_t *
#define BOR_SPLAY_NODE_KEY(node) (node)
#define BOR_SPLAY_NODE_SET_KEY(head, node, key)
#define BOR_SPLAY_KEY_CMP(head, key1, key2) \
    (head)->cmp((key1), (key2), (head)->data)
#include "boruvka/splaytree_def.h"


bor_splaytree_t *borSplayTreeNew(bor_splaytree_cmp cmp, void *data)
{
    bor_splaytree_t *st;

    st = BOR_ALLOC(bor_splaytree_t);
    borSplayTreeInit(st, cmp, data);
    return st;
}

void borSplayTreeDel(bor_splaytree_t *splaytree)
{
    BOR_FREE(splaytree);
}

void borSplayTreeInit(bor_splaytree_t *st, bor_splaytree_cmp cmp, void *data)
{
    borSplayInit(st);
    st->cmp = cmp;
    st->data = data;
}

void borSplayTreeFree(bor_splaytree_t *st)
{
    st->cmp = NULL;
    st->data = NULL;
    borSplayFree(st);
}

bor_splaytree_node_t *borSplayTreeInsert(bor_splaytree_t *head,
                                         bor_splaytree_node_t *elm)
{
    return borSplayInsert(head, elm, elm);
}

bor_splaytree_node_t *borSplayTreeRemove(bor_splaytree_t *head,
                                         bor_splaytree_node_t *elm)
{
    return borSplayRemove(head, elm);
}

bor_splaytree_node_t *borSplayTreeFind(bor_splaytree_t *head,
                                       bor_splaytree_node_t *elm)
{
    return borSplayFind(head, elm);
}

bor_splaytree_node_t *borSplayTreeNext(bor_splaytree_t *head,
                                       bor_splaytree_node_t *elm)
{
    return borSplayNext(head, elm);
}

bor_splaytree_node_t *borSplayTreePrev(bor_splaytree_t *head,
                                       bor_splaytree_node_t *elm)
{
    return borSplayPrev(head, elm);
}

bor_splaytree_node_t *borSplayTreeMin(bor_splaytree_t *head)
{
    return borSplayMin(head);
}

bor_splaytree_node_t *borSplayTreeMax(bor_splaytree_t *head)
{
    return borSplayMax(head);
}
