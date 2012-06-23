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

#include <boruvka/dij.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

/** Compares two nodes and returns true if n1 < n2. */
static int heapLT(const bor_pairheap_node_t *n1,
                  const bor_pairheap_node_t *n2,
                  void *_);

bor_dij_t *borDijNew(const bor_dij_ops_t *ops)
{
    bor_dij_t *dij;

    dij = BOR_ALLOC(bor_dij_t);
    dij->ops   = *ops;

    dij->heap = NULL;

    return dij;
}

void borDijDel(bor_dij_t *dij)
{
    if (dij->heap)
        borPairHeapDel(dij->heap);
    BOR_FREE(dij);
}

int borDijRun(bor_dij_t *dij, bor_dij_node_t *start,
                              bor_dij_node_t *end)
{
    bor_pairheap_node_t *heapnode;
    bor_dij_node_t *node, *nextnode;
    bor_list_t list, *item;
    bor_real_t dist;

    if (dij->heap)
        borPairHeapDel(dij->heap);

    // create priority heap
    dij->heap = borPairHeapNew(heapLT, NULL);

    // push start node on heap
    start->dist = BOR_ZERO;
    start->prev = NULL;
    start->state = BOR_DIJ_STATE_OPEN;
    borPairHeapAdd(dij->heap, &start->_heap);

    // run algorithm
    while (!borPairHeapEmpty(dij->heap)){
        // Get minimal node from priority heap
        heapnode = borPairHeapExtractMin(dij->heap);
        node = bor_container_of(heapnode, bor_dij_node_t, _heap);

        // set state to CLOSED
        node->state = BOR_DIJ_STATE_CLOSED;

        // we found end point - terminate algorithm
        if (node == end)
            return 0;

        // Expand node

        // empty list
        borListInit(&list);
        // let user's function to fill list with nodes
        dij->ops.expand(node, &list, dij->ops.data);
        // iterate over all nodes in list
        BOR_LIST_FOR_EACH(&list, item){
            nextnode = BOR_LIST_ENTRY(item, bor_dij_node_t, _list);

            // skip closed nodes
            // user shouldn't put them in list, but to be sure...
            if (bor_unlikely(nextnode->state == BOR_DIJ_STATE_CLOSED))
                continue;

            // Relax operation.
            dist = node->dist + nextnode->_loc_dist;
            // Let assume that nextnode->dist is always higher than dist if
            // nextnode wasn't touched until now - user was responsible to
            // call borDijNodeInit() function which sets .dist to
            // BOR_REAL_MAX.
            if (dist < nextnode->dist){
                // store new distance
                nextnode->dist = dist;
                nextnode->prev = node;

                // and update its position in heap or add it on heap if it
                // is not already on heap
                if (nextnode->state == BOR_DIJ_STATE_OPEN){
                    borPairHeapDecreaseKey(dij->heap, &nextnode->_heap);
                }else{
                    borPairHeapAdd(dij->heap, &nextnode->_heap);
                    nextnode->state = BOR_DIJ_STATE_OPEN;
                }
            }
        }
    }

    return -1;
}

void borDijPath(bor_dij_node_t *endnode, bor_list_t *list)
{
    bor_dij_node_t *node;

    node = endnode;
    while (node){
        borListPrepend(list, &node->_list);
        node = node->prev;
    }
}


static int heapLT(const bor_pairheap_node_t *h1,
                  const bor_pairheap_node_t *h2,
                  void *_)
{
    bor_dij_node_t *n1, *n2;
    n1 = bor_container_of(h1, bor_dij_node_t, _heap);
    n2 = bor_container_of(h2, bor_dij_node_t, _heap);

    return n1->dist < n2->dist;
}
