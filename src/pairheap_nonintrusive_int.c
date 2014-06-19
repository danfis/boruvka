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
#include "boruvka/pairheap_nonintrusive_int.h"

struct _node_t {
    int key;             /*!< Key of the node */
    void *data;          /*!< Inserted data */
    bor_list_t children; /*!< List of children */
    bor_list_t list;     /*!< Connection into list of siblings */
};
typedef struct _node_t node_t;


/** Consolidates pairing heap. */
static void consolidate(bor_pairheap_nonintr_int_t *ph);
/** Returns minimal node */
static node_t *minNode(bor_pairheap_nonintr_int_t *ph);
/** Removes node from the heap */
static void removeNode(bor_pairheap_nonintr_int_t *ph,
                       node_t *node);

bor_pairheap_nonintr_int_t *borPairHeapNonIntrIntNew(void)
{
    bor_pairheap_nonintr_int_t *heap;

    heap = BOR_ALLOC(bor_pairheap_nonintr_int_t);
    borListInit(&heap->root);
    return heap;
}

void borPairHeapNonIntrIntDel(bor_pairheap_nonintr_int_t *ph)
{
    borPairHeapNonIntrIntClear(ph);
    BOR_FREE(ph);
}

void *borPairHeapNonIntrIntMin(bor_pairheap_nonintr_int_t *ph, int *key)
{
    node_t *el;

    el = minNode(ph);
    if (el == NULL)
        return NULL;

    if (key)
        *key = el->key;
    return el->data;
}

void borPairHeapNonIntrIntAdd(bor_pairheap_nonintr_int_t *ph,
                              int key, void *data)
{
    node_t *node;

    node = BOR_ALLOC(node_t);
    node->key = key;
    node->data = data;
    borListInit(&node->children);
    borListAppend(&ph->root, &node->list);
}

void *borPairHeapNonIntrIntExtractMin(bor_pairheap_nonintr_int_t *ph, int *key)
{
    node_t *node;
    void *data;

    node = minNode(ph);
    if (node == NULL)
        return NULL;

    if (key)
        *key = node->key;
    data = node->data;

    removeNode(ph, node);

    return data;
}

static void recursiveClear(node_t *node)
{
    bor_list_t *lnode, *ltmp;
    node_t *ch_node;

    BOR_LIST_FOR_EACH_SAFE(&node->children, lnode, ltmp){
        ch_node = BOR_LIST_ENTRY(lnode, node_t, list);
        recursiveClear(ch_node);
    }

    borListDel(&node->list);
    BOR_FREE(node);
}

void borPairHeapNonIntrIntClear(bor_pairheap_nonintr_int_t *ph)
{
    bor_list_t *lnode, *ltmp;
    node_t *node;

    BOR_LIST_FOR_EACH_SAFE(&ph->root, lnode, ltmp){
        node = BOR_LIST_ENTRY(lnode, node_t, list);
        recursiveClear(node);
    }
}


static node_t *minNode(bor_pairheap_nonintr_int_t *ph)
{
    node_t *el;
    bor_list_t *item;

    if (borPairHeapNonIntrIntEmpty(ph))
        return NULL;

    item = borListNext(&ph->root);

    /* if root doesn't contain only one node, heap must be consolidated */
    if (borListNext(item) != &ph->root){
        consolidate(ph);
        item = borListNext(&ph->root);
    }

    el = BOR_LIST_ENTRY(item, node_t, list);
    return el;
}

static void removeNode(bor_pairheap_nonintr_int_t *ph,
                       node_t *node)
{
    node_t *c;
    bor_list_t *list, *item, *item_tmp;

    list = &node->children;
    BOR_LIST_FOR_EACH_SAFE(list, item, item_tmp){
        c = BOR_LIST_ENTRY(item, node_t, list);

        // remove from n
        borListDel(&c->list);
        // add it to root list
        borListAppend(&ph->root, &c->list);
    }

    // remove n itself
    borListDel(&node->list);
    BOR_FREE(node);
}

void consolidate(bor_pairheap_nonintr_int_t *ph)
{
    bor_list_t *root, *item, *item_next;
    node_t *n1, *n2;

    root = &ph->root;

    // 1. First pairing from left to righ
    item = borListNext(root);
    item_next = borListNext(item);
    while (item != root && item_next != root){
        // get nodes
        n1 = BOR_LIST_ENTRY(item, node_t, list);
        n2 = BOR_LIST_ENTRY(item_next, node_t, list);

        // compare them
        if (n1->key < n2->key){
            borListDel(&n2->list);
            borListAppend(&n1->children, &n2->list);
            item = borListNext(&n1->list);
        }else{
            borListDel(&n1->list);
            borListAppend(&n2->children, &n1->list);
            item = borListNext(&n2->list);
        }

        item_next = borListNext(item);
    }

    // 2. Finish mergin from right to left
    // To be honest, I really don't understand should it be from right to
    // left, so let's do it ordinary way...
    item = borListNext(root);
    item_next = borListNext(item);
    while (item != root && item_next != root){
        // get nodes
        n1 = BOR_LIST_ENTRY(item, node_t, list);
        n2 = BOR_LIST_ENTRY(item_next, node_t, list);

        if (n1->key < n2->key){
            borListDel(&n2->list);
            borListAppend(&n1->children, &n2->list);
        }else{
            borListDel(&n1->list);
            borListAppend(&n2->children, &n1->list);
            item = item_next;
        }
        item_next = borListNext(item);
    }
}
