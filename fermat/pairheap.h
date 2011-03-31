/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __FER_PAIRHEAP_H__
#define __FER_PAIRHEAP_H__

#include <fermat/core.h>
#include <fermat/list.h>

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

struct _fer_pairheap_node_t {
    fer_list_t children;             /*!< List of children */
    fer_list_t list;                 /*!< Connection into list of siblings */
};
typedef struct _fer_pairheap_node_t fer_pairheap_node_t;

/**
 * Callback that should return true if {n1} is smaller than {n2}.
 */
typedef int (*fer_pairheap_lt)(const fer_pairheap_node_t *n1,
                               const fer_pairheap_node_t *n2,
                               void *data);

struct _fer_pairheap_t {
    fer_list_t root; /*!< List of root nodes. In fact, pairing heap has
                          always one root node, but we need this to make
                          effecient (lazy) merging. */
    fer_pairheap_lt lt; /*!< "Less than" callback provided by user */
    void *data;
};
typedef struct _fer_pairheap_t fer_pairheap_t;


/**
 * Creates new empty Pairing heap.
 * Callback for comparison must be provided.
 */
fer_pairheap_t *ferPairHeapNew(fer_pairheap_lt less_than, void *data);


/**
 * Deletes pairing heap.
 * Note that individual nodes are not disconnected from heap.
 */
void ferPairHeapDel(fer_pairheap_t *ph);

/**
 * Returns true if heap is empty.
 */
_fer_inline int ferPairHeapEmpty(const fer_pairheap_t *ph);

/**
 * Returns minimal node.
 */
_fer_inline fer_pairheap_node_t *ferPairHeapMin(fer_pairheap_t *ph);

/**
 * Adds node to heap.
 */
_fer_inline void ferPairHeapAdd(fer_pairheap_t *ph, fer_pairheap_node_t *n);

/**
 * Removes and returns minimal node from heap.
 */
_fer_inline fer_pairheap_node_t *ferPairHeapExtractMin(fer_pairheap_t *ph);

/**
 * Update position of node in heap in case its value was decreased.
 * If value wasn't decreased (or you are not sure) call ferPairHeapUpdate()
 * instead.
 */
_fer_inline void ferPairHeapDecreaseKey(fer_pairheap_t *ph, fer_pairheap_node_t *n);

/**
 * Updates position of node in heap.
 */
_fer_inline void ferPairHeapUpdate(fer_pairheap_t *ph, fer_pairheap_node_t *n);

/**
 * Del node from heap.
 */
void ferPairHeapRemove(fer_pairheap_t *ph, fer_pairheap_node_t *n);


/**
 * Consolidates pairing heap.
 */
void __ferPairHeapConsolidate(fer_pairheap_t *ph);


/**** INLINES ****/
_fer_inline int ferPairHeapEmpty(const fer_pairheap_t *ph)
{
    return ferListEmpty(&ph->root);
}

_fer_inline fer_pairheap_node_t *ferPairHeapMin(fer_pairheap_t *ph)
{
    fer_pairheap_node_t *el;
    fer_list_t *item;

    if (ferPairHeapEmpty(ph))
        return NULL;

    item = ferListNext(&ph->root);

    /* if root doesn't contain only one node, heap must be consolidated */
    if (ferListNext(item) != &ph->root){
        __ferPairHeapConsolidate(ph);
        item = ferListNext(&ph->root);
    }

    el = ferListEntry(item, fer_pairheap_node_t, list);
    return el;
}

_fer_inline void ferPairHeapAdd(fer_pairheap_t *ph, fer_pairheap_node_t *n)
{
    ferListInit(&n->children);
    ferListAppend(&ph->root, &n->list);
}

_fer_inline void ferPairHeapDecreaseKey(fer_pairheap_t *ph, fer_pairheap_node_t *n)
{
    ferListDel(&n->list);
    ferListAppend(&ph->root, &n->list);
}

_fer_inline void ferPairHeapUpdate(fer_pairheap_t *ph, fer_pairheap_node_t *n)
{
    ferPairHeapRemove(ph, n);
    ferPairHeapAdd(ph, n);
}

_fer_inline fer_pairheap_node_t *ferPairHeapExtractMin(fer_pairheap_t *ph)
{
    fer_pairheap_node_t *n;

    n = ferPairHeapMin(ph);
    ferPairHeapRemove(ph, n);

    return n;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_PAIRHEAP_H__ */


