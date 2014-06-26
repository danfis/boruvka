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

#include "boruvka/alloc.h"
#include "boruvka/multimap_int.h"

bor_multimap_int_t *borMultiMapIntNew(void)
{
    bor_multimap_int_t *mm;

    mm = BOR_ALLOC(bor_multimap_int_t);
    borRBTreeIntInit(&mm->tree);
    mm->pre_keynode = BOR_ALLOC(bor_multimap_int_keynode_t);
    borListInit(&mm->pre_keynode->nodes);
    return mm;
}

void borMultiMapIntDel(bor_multimap_int_t *mm)
{
    bor_rbtree_int_node_t *tree_node, *tmp;
    bor_multimap_int_keynode_t *keynode;

    BOR_RBTREE_INT_FOR_EACH_SAFE(&mm->tree, tree_node, tmp){
        // TODO: This can be done more efficiently but requires support
        // from rb-tree
        borRBTreeIntRemove(&mm->tree, tree_node);
        keynode = bor_container_of(tree_node,
                                   bor_multimap_int_keynode_t, node);
        BOR_FREE(keynode);
    }

    borRBTreeIntFree(&mm->tree);

    if (mm->pre_keynode)
        BOR_FREE(mm->pre_keynode);
    BOR_FREE(mm);
}

void borMultiMapIntInsert(bor_multimap_int_t *mm,
                          int key, bor_multimap_int_node_t *node)
{
    bor_multimap_int_keynode_t *keynode;
    bor_rbtree_int_node_t *kn;

    kn = borRBTreeIntInsert(&mm->tree, key, &mm->pre_keynode->node);
    if (kn == NULL){
        // A new key-node was inserted, so pre-allocate a next key-node and
        // remember the old one.
        keynode = mm->pre_keynode;

        mm->pre_keynode = BOR_ALLOC(bor_multimap_int_keynode_t);
        borListInit(&mm->pre_keynode->nodes);
    }else{
        // The key is already in tree, infer the main struct
        keynode = bor_container_of(kn, bor_multimap_int_keynode_t, node);
    }

    // Add the node to the end of the key-node list
    borListAppend(&keynode->nodes, node);
}

void borMultiMapIntRemove(bor_multimap_int_t *mm,
                          bor_multimap_int_node_t *node)
{
    bor_multimap_int_keynode_t *keynode;
    bor_list_t *node_list;

    // Remember the next node in the list.
    // Later, we can check this node whether it isn't the head of the list
    // and if so (and it is empty) than the key-node can be removed.
    node_list = borListNext(node);

    // Disconnect node from the multimap key-node
    borListDel(node);

    // Check whether the list isn't empty.
    // This is a bit tricky. In the node_list can or cannot be stored the
    // head of the list, but if this node is pointing to itself we now that
    // it is the head of the list and we also know it is empty (thus
    // borListEmpty() function). And if it is the head of the list we can
    // infere the main key-node structure from it.
    if (borListEmpty(node_list)){
        keynode = BOR_LIST_ENTRY(node_list, bor_multimap_int_keynode_t, nodes);
        borRBTreeIntRemove(&mm->tree, &keynode->node);
        BOR_FREE(keynode);
    }
}

bor_list_t *borMultiMapIntFind(bor_multimap_int_t *mm, int key)
{
    bor_multimap_int_keynode_t *keynode;
    bor_rbtree_int_node_t *kn;

    kn = borRBTreeIntFind(&mm->tree, key);
    if (kn == NULL)
        return NULL;

    keynode = bor_container_of(kn, bor_multimap_int_keynode_t, node);
    return &keynode->nodes;
}

bor_list_t *borMultiMapIntMin(bor_multimap_int_t *mm, int *key)
{
    bor_multimap_int_keynode_t *keynode;
    bor_rbtree_int_node_t *kn;

    kn = borRBTreeIntMin(&mm->tree);
    if (kn == NULL)
        return NULL;

    keynode = bor_container_of(kn, bor_multimap_int_keynode_t, node);
    if (key)
        *key = borRBTreeIntKey(kn);

    return &keynode->nodes;
}

bor_multimap_int_node_t *borMultiMapIntExtractMinNodeFifo(
            bor_multimap_int_t *mm, int *key)
{
    bor_list_t *nodes;
    bor_multimap_int_node_t *node;

    nodes = borMultiMapIntMin(mm, key);
    node = borListNext(nodes);
    borMultiMapIntRemove(mm, node);
    return node;
}

bor_multimap_int_node_t *borMultiMapIntExtractMinNodeLifo(
            bor_multimap_int_t *mm, int *key)
{
    bor_list_t *nodes;
    bor_multimap_int_node_t *node;

    nodes = borMultiMapIntMin(mm, key);
    node = borListPrev(nodes);
    borMultiMapIntRemove(mm, node);
    return node;
}
