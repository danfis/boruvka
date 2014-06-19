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

#ifndef __BOR_PAIRHEAP_NONINTRUSIVE_INT_H__
#define __BOR_PAIRHEAP_NONINTRUSIVE_INT_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Pairing Heap -- Non-intrusive Version with Integer Key
 * =======================================================
 *
 * Fredman, Michael L.; Sedgewick, Robert; Sleator, Daniel D.; Tarjan,
 * Robert E. (1986), "The pairing heap: a new form of self-adjusting heap",
 * Algorithmica 1 (1): 111–129.
 *
 */

/** vvvv */

struct _bor_pairheap_nonintr_int_t {
    bor_list_t root; /*!< List of root nodes. In fact, pairing heap has
                          always one root node, but we need this to make
                          effecient (lazy) merging. */
};
typedef struct _bor_pairheap_nonintr_int_t bor_pairheap_nonintr_int_t;
/** ^^^^ */


/**
 * Functions
 * ----------
 */

/**
 * Creates new empty Pairing heap.
 */
bor_pairheap_nonintr_int_t *borPairHeapNonIntrIntNew(void);

/**
 * Deletes pairing heap.
 */
void borPairHeapNonIntrIntDel(bor_pairheap_nonintr_int_t *ph);

/**
 * Returns true if heap is empty.
 */
_bor_inline int borPairHeapNonIntrIntEmpty(const bor_pairheap_nonintr_int_t *ph);

/**
 * Returns minimal node. If {key} is non-NULL it will be filled with the
 * key value of the minimal node.
 */
void *borPairHeapNonIntrIntMin(bor_pairheap_nonintr_int_t *ph, int *key);

/**
 * Adds a new node to the heap.
 */
void borPairHeapNonIntrIntAdd(bor_pairheap_nonintr_int_t *ph,
                              int key, void *data);

/**
 * Removes and returns minimal node from heap.
 */
void *borPairHeapNonIntrIntExtractMin(bor_pairheap_nonintr_int_t *ph, int *key);


/**
 * Removes all nodes from the heap.
 */
void borPairHeapNonIntrIntClear(bor_pairheap_nonintr_int_t *ph);


/**** INLINES ****/
_bor_inline int borPairHeapNonIntrIntEmpty(const bor_pairheap_nonintr_int_t *ph)
{
    return borListEmpty(&ph->root);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_PAIRHEAP_NONINTRUSIVE_INT_H__ */
