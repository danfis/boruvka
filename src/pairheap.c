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

#include <boruvka/pairheap.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

bor_pairheap_t *ferPairHeapNew(fer_pairheap_lt less_than, void *data)
{
    bor_pairheap_t *ph;

    ph = BOR_ALLOC(bor_pairheap_t);
    ferListInit(&ph->root);
    ph->lt = less_than;
    ph->data = data;

    return ph;
}

void ferPairHeapDel(bor_pairheap_t *ph)
{
    BOR_FREE(ph);
}

void ferPairHeapRemove(bor_pairheap_t *ph, bor_pairheap_node_t *n)
{
    bor_pairheap_node_t *c;
    bor_list_t *list, *item, *item_tmp;

    list = &n->children;
    BOR_LIST_FOR_EACH_SAFE(list, item, item_tmp){
        c = BOR_LIST_ENTRY(item, bor_pairheap_node_t, list);

        // remove from n
        ferListDel(&c->list);
        // add it to root list
        ferListAppend(&ph->root, &c->list);
    }

    // remove n itself
    ferListDel(&n->list);
}

void __ferPairHeapConsolidate(bor_pairheap_t *ph)
{
    bor_list_t *root, *item, *item_next;
    bor_pairheap_node_t *n1, *n2;

    root = &ph->root;

    // 1. First pairing from left to righ
    item = ferListNext(root);
    item_next = ferListNext(item);
    while (item != root && item_next != root){
        // get nodes
        n1 = BOR_LIST_ENTRY(item, bor_pairheap_node_t, list);
        n2 = BOR_LIST_ENTRY(item_next, bor_pairheap_node_t, list);

        // compare them
        if (ph->lt(n1, n2, ph->data)){ // n1 < n2
            ferListDel(&n2->list);
            ferListAppend(&n1->children, &n2->list);
            item = ferListNext(&n1->list);
        }else{
            ferListDel(&n1->list);
            ferListAppend(&n2->children, &n1->list);
            item = ferListNext(&n2->list);
        }

        item_next = ferListNext(item);
    }

    // 2. Finish mergin from right to left
    // To be honest, I really don't understand should it be from right to
    // left, so let's do it ordinary way...
    item = ferListNext(root);
    item_next = ferListNext(item);
    while (item != root && item_next != root){
        // get nodes
        n1 = BOR_LIST_ENTRY(item, bor_pairheap_node_t, list);
        n2 = BOR_LIST_ENTRY(item_next, bor_pairheap_node_t, list);

        if (ph->lt(n1, n2, ph->data)){ // n1 < n2
            ferListDel(&n2->list);
            ferListAppend(&n1->children, &n2->list);
        }else{
            ferListDel(&n1->list);
            ferListAppend(&n2->children, &n1->list);
            item = item_next;
        }
        item_next = ferListNext(item);
    }
}
