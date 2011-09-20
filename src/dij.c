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

#include <fermat/dij.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** Compares two nodes and returns true if n1 < n2. */
static int heapLT(const fer_pairheap_node_t *n1,
                  const fer_pairheap_node_t *n2,
                  void *_);

fer_dij_t *ferDijNew(const fer_dij_ops_t *ops)
{
    fer_dij_t *dij;

    dij = FER_ALLOC(fer_dij_t);
    dij->ops   = *ops;

    dij->heap = NULL;

    return dij;
}

void ferDijDel(fer_dij_t *dij)
{
    if (dij->heap)
        ferPairHeapDel(dij->heap);
    FER_FREE(dij);
}

int ferDijRun(fer_dij_t *dij, fer_dij_node_t *start,
                              fer_dij_node_t *end)
{
    fer_pairheap_node_t *heapnode;
    fer_dij_node_t *node, *nextnode;
    fer_list_t list, *item;
    fer_real_t dist;

    if (dij->heap)
        ferPairHeapDel(dij->heap);

    // create priority heap
    dij->heap = ferPairHeapNew(heapLT, NULL);

    // push start node on heap
    start->dist = FER_ZERO;
    start->prev = NULL;
    start->state = FER_DIJ_STATE_OPEN;
    ferPairHeapAdd(dij->heap, &start->_heap);

    // run algorithm
    while (!ferPairHeapEmpty(dij->heap)){
        // Get minimal node from priority heap
        heapnode = ferPairHeapExtractMin(dij->heap);
        node = fer_container_of(heapnode, fer_dij_node_t, _heap);

        // set state to CLOSED
        node->state = FER_DIJ_STATE_CLOSED;

        // we found end point - terminate algorithm
        if (node == end)
            return 0;

        // Expand node

        // empty list
        ferListInit(&list);
        // let user's function to fill list with nodes
        dij->ops.expand(node, &list, dij->ops.data);
        // iterate over all nodes in list
        FER_LIST_FOR_EACH(&list, item){
            nextnode = FER_LIST_ENTRY(item, fer_dij_node_t, _list);

            // skip closed nodes
            // user shouldn't put them in list, but to be sure...
            if (fer_unlikely(nextnode->state == FER_DIJ_STATE_CLOSED))
                continue;

            // Relax operation.
            dist = node->dist + nextnode->_loc_dist;
            // Let assume that nextnode->dist is always higher than dist if
            // nextnode wasn't touched until now - user was responsible to
            // call ferDijNodeInit() function which sets .dist to
            // FER_REAL_MAX.
            if (dist < nextnode->dist){
                // store new distance
                nextnode->dist = dist;
                nextnode->prev = node;

                // and update its position in heap or add it on heap if it
                // is not already on heap
                if (nextnode->state == FER_DIJ_STATE_OPEN){
                    ferPairHeapDecreaseKey(dij->heap, &nextnode->_heap);
                }else{
                    ferPairHeapAdd(dij->heap, &nextnode->_heap);
                    nextnode->state = FER_DIJ_STATE_OPEN;
                }
            }
        }
    }

    return -1;
}

void ferDijPath(fer_dij_node_t *endnode, fer_list_t *list)
{
    fer_dij_node_t *node;

    node = endnode;
    while (node){
        ferListPrepend(list, &node->_list);
        node = node->prev;
    }
}


static int heapLT(const fer_pairheap_node_t *h1,
                  const fer_pairheap_node_t *h2,
                  void *_)
{
    fer_dij_node_t *n1, *n2;
    n1 = fer_container_of(h1, fer_dij_node_t, _heap);
    n2 = fer_container_of(h2, fer_dij_node_t, _heap);

    return n1->dist < n2->dist;
}
