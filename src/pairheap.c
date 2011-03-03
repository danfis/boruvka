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

#include <fermat/pairheap.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

fer_pairheap_t *ferPairHeapNew(fer_pairheap_lt less_than)
{
    fer_pairheap_t *ph;

    ph = FER_ALLOC(fer_pairheap_t);
    ferListInit(&ph->root);
    ph->lt = less_than;

    return ph;
}

void ferPairHeapDel(fer_pairheap_t *ph)
{
    free(ph);
}

void ferPairHeapRemove(fer_pairheap_t *ph, fer_pairheap_node_t *n)
{
    fer_pairheap_node_t *c;
    fer_list_t *list, *item, *item_tmp;

    list = &n->children;
    ferListForEachSafe(list, item, item_tmp){
        c = ferListEntry(item, fer_pairheap_node_t, list);

        // remove from n
        ferListDel(&c->list);
        // add it to root list
        ferListAppend(&ph->root, &c->list);
    }

    // remove n itself
    ferListDel(&n->list);
}

void __ferPairHeapConsolidate(fer_pairheap_t *ph)
{
    fer_list_t *root, *item, *item_next;
    fer_pairheap_node_t *n1, *n2;

    root = &ph->root;

    // 1. First pairing from left to righ
    item = ferListNext(root);
    item_next = ferListNext(item);
    while (item != root && item_next != root){
        // get nodes
        n1 = ferListEntry(item, fer_pairheap_node_t, list);
        n2 = ferListEntry(item_next, fer_pairheap_node_t, list);

        // compare them
        if (ph->lt(n1, n2)){ // n1 < n2
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
        n1 = ferListEntry(item, fer_pairheap_node_t, list);
        n2 = ferListEntry(item_next, fer_pairheap_node_t, list);

        if (ph->lt(n1, n2)){ // n1 < n2
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
