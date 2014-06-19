/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_PAIRHEAP_H__
#define __BOR_PAIRHEAP_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * PairHeap - Pairing Heap
 * ========================
 *
 * Fredman, Michael L.; Sedgewick, Robert; Sleator, Daniel D.; Tarjan,
 * Robert E. (1986), "The pairing heap: a new form of self-adjusting heap",
 * Algorithmica 1 (1): 111–129.
 *
 */

/** vvvv */
struct _bor_pairheap_node_t {
    bor_list_t children;             /*!< List of children */
    bor_list_t list;                 /*!< Connection into list of siblings */
};
typedef struct _bor_pairheap_node_t bor_pairheap_node_t;

/**
 * Callback that should return true if {n1} is smaller than {n2}.
 */
typedef int (*bor_pairheap_lt)(const bor_pairheap_node_t *n1,
                               const bor_pairheap_node_t *n2,
                               void *data);

/**
 * Callback for borPairHeapClear() function.
 */
typedef void (*bor_pairheap_clear)(bor_pairheap_node_t *n,
                                   void *data);

struct _bor_pairheap_t {
    bor_list_t root; /*!< List of root nodes. In fact, pairing heap has
                          always one root node, but we need this to make
                          effecient (lazy) merging. */
    bor_pairheap_lt lt; /*!< "Less than" callback provided by user */
    void *data;
};
typedef struct _bor_pairheap_t bor_pairheap_t;
/** ^^^^ */


/**
 * Functions
 * ----------
 */

/**
 * Creates new empty Pairing heap.
 * Callback for comparison must be provided.
 */
bor_pairheap_t *borPairHeapNew(bor_pairheap_lt less_than, void *data);


/**
 * Deletes pairing heap.
 * Note that individual nodes are not disconnected from heap.
 */
void borPairHeapDel(bor_pairheap_t *ph);

/**
 * Returns true if heap is empty.
 */
_bor_inline int borPairHeapEmpty(const bor_pairheap_t *ph);

/**
 * Returns minimal node.
 */
_bor_inline bor_pairheap_node_t *borPairHeapMin(bor_pairheap_t *ph);

/**
 * Adds node to heap.
 */
_bor_inline void borPairHeapAdd(bor_pairheap_t *ph, bor_pairheap_node_t *n);

/**
 * Removes and returns minimal node from heap.
 */
_bor_inline bor_pairheap_node_t *borPairHeapExtractMin(bor_pairheap_t *ph);

/**
 * Update position of node in heap in case its value was decreased.
 * If value wasn't decreased (or you are not sure) call borPairHeapUpdate()
 * instead.
 */
_bor_inline void borPairHeapDecreaseKey(bor_pairheap_t *ph, bor_pairheap_node_t *n);

/**
 * Updates position of node in heap.
 */
_bor_inline void borPairHeapUpdate(bor_pairheap_t *ph, bor_pairheap_node_t *n);

/**
 * Del node from heap.
 */
void borPairHeapRemove(bor_pairheap_t *ph, bor_pairheap_node_t *n);

/**
 * Removes all data from the heap call for each disconnected node a given
 * callback. There is no guarantee for any particular order of the removed
 * nodes.
 */
void borPairHeapClear(bor_pairheap_t *ph,
                      bor_pairheap_clear clear_fn,
                      void *user_data);


/**
 * Consolidates pairing heap.
 */
void __borPairHeapConsolidate(bor_pairheap_t *ph);


/**** INLINES ****/
_bor_inline int borPairHeapEmpty(const bor_pairheap_t *ph)
{
    return borListEmpty(&ph->root);
}

_bor_inline bor_pairheap_node_t *borPairHeapMin(bor_pairheap_t *ph)
{
    bor_pairheap_node_t *el;
    bor_list_t *item;

    if (borPairHeapEmpty(ph))
        return NULL;

    item = borListNext(&ph->root);

    /* if root doesn't contain only one node, heap must be consolidated */
    if (borListNext(item) != &ph->root){
        __borPairHeapConsolidate(ph);
        item = borListNext(&ph->root);
    }

    el = BOR_LIST_ENTRY(item, bor_pairheap_node_t, list);
    return el;
}

_bor_inline void borPairHeapAdd(bor_pairheap_t *ph, bor_pairheap_node_t *n)
{
    borListInit(&n->children);
    borListAppend(&ph->root, &n->list);
}

_bor_inline void borPairHeapDecreaseKey(bor_pairheap_t *ph, bor_pairheap_node_t *n)
{
    borListDel(&n->list);
    borListAppend(&ph->root, &n->list);
}

_bor_inline void borPairHeapUpdate(bor_pairheap_t *ph, bor_pairheap_node_t *n)
{
    borPairHeapRemove(ph, n);
    borPairHeapAdd(ph, n);
}

_bor_inline bor_pairheap_node_t *borPairHeapExtractMin(bor_pairheap_t *ph)
{
    bor_pairheap_node_t *n;

    n = borPairHeapMin(ph);
    borPairHeapRemove(ph, n);

    return n;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_PAIRHEAP_H__ */
