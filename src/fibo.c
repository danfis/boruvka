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

#include <boruvka/fibo.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

/** Consolidates heap. */
_bor_inline void __borFiboConsolidate(bor_fibo_t *f);

/** Links subtrees n1 and n2, n1 and n2 must be (!) both root nodes.
 *  It is also assumed n2 is smaller than n1.
 *  After this n1 becomes child of n2. */
_bor_inline void __borFiboLink(bor_fibo_t *f, bor_fibo_node_t *n1,
                                              bor_fibo_node_t *n2);

/** Cuts x from y's list of children */
_bor_inline void __borFiboCut(bor_fibo_t *f, bor_fibo_node_t *x,
                                             bor_fibo_node_t *y);

/** Cuts n from heap and cascadely the same with parents */
_bor_inline void __borFiboCutCascade(bor_fibo_t *f, bor_fibo_node_t *n);


bor_fibo_t *borFiboNew(bor_fibo_lt lt, void *data)
{
    bor_fibo_t *fibo;

    fibo = BOR_ALLOC(bor_fibo_t);

    bzero(fibo, sizeof(bor_fibo_t));
    borListInit(&fibo->root);

    fibo->max_degree = 0;

    fibo->lt = lt;
    fibo->data = data;

    return fibo;
}

void borFiboDel(bor_fibo_t *fibo)
{
    BOR_FREE(fibo);
}

bor_fibo_node_t *borFiboExtractMin(bor_fibo_t *f)
{
    bor_list_t *item;
    bor_fibo_node_t *min, *n;

    if (!f->min)
        return NULL;

    min = f->min;

    // put all children to root list
    while (!borListEmpty(&min->children)){
        item = borListNext(&min->children);
        n    = BOR_LIST_ENTRY(item, bor_fibo_node_t, list);

        borListDel(item);
        borListAppend(&f->root, item);
        n->parent = NULL;
    }

    // remove minimum from root list
    borListDel(&min->list);

    __borFiboConsolidate(f);

    return min;
}

void borFiboDecreaseKey(bor_fibo_t *f, bor_fibo_node_t *n)
{
    __borFiboCutCascade(f, n);

    if (n != f->min && (!f->min || f->lt(n, f->min, f->data)))
        f->min = n;
}

void borFiboRemove(bor_fibo_t *f, bor_fibo_node_t *n)
{
    bor_list_t *list, *item, *item_tmp;
    bor_fibo_node_t *c;

    __borFiboCutCascade(f, n);

    // remove all its children
    list = &n->children;
    BOR_LIST_FOR_EACH_SAFE(list, item, item_tmp){
        c = bor_container_of(item, bor_fibo_node_t, list);
        __borFiboCut(f, c, n);
    }

    // remove from root list
    borListDel(&n->list);

    __borFiboConsolidate(f);
}


_bor_inline void __borFiboConsolidate(bor_fibo_t *f)
{
    bor_list_t *list, *item, *tmp_item;
    bor_fibo_node_t *n;
    unsigned int degree;
    size_t i;

    for (i = 0; i <= f->max_degree; i++){
        f->cons[i] = NULL;
    }

    f->max_degree = 0;

    list = &f->root;
    BOR_LIST_FOR_EACH_SAFE(list, item, tmp_item){
        n = BOR_LIST_ENTRY(item, bor_fibo_node_t, list);

        degree = n->degree;
        while (f->cons[degree] != NULL){
            if (f->lt(f->cons[degree], n, f->data)){
                __borFiboLink(f, n, f->cons[degree]);
                n = f->cons[degree];
            }else{
                __borFiboLink(f, f->cons[degree], n);
            }

            f->cons[degree] = NULL;
            degree++;
        }

        f->cons[degree] = n;
        if (degree > f->max_degree)
            f->max_degree = degree;
    }

    // find minimum
    f->min = NULL;
    for (i = 0; i <= f->max_degree; i++){
        if (!f->min || (f->cons[i] && f->lt(f->cons[i], f->min, f->data))){
            f->min = f->cons[i];
        }
    }
}

_bor_inline void __borFiboLink(bor_fibo_t *f, bor_fibo_node_t *n1,
                                              bor_fibo_node_t *n2)
{
    borListDel(&n1->list);
    borListAppend(&n2->children, &n1->list);
    n1->parent = n2;
    n2->degree += 1;
    n1->mark = 0;
}

_bor_inline void __borFiboCut(bor_fibo_t *f, bor_fibo_node_t *x,
                                             bor_fibo_node_t *y)
{
    borListDel(&x->list);
    y->degree--;
    borListAppend(&f->root, &x->list);
    x->parent = NULL;
    x->mark = 0;
}

_bor_inline void __borFiboCutCascade(bor_fibo_t *f, bor_fibo_node_t *n)
{
    bor_fibo_node_t *p;

    p = n->parent;

    if (p && f->lt(n, p, f->data)){
        __borFiboCut(f, n, p);

        while (p){
            if (!p->mark){
                p->mark = 1;
                break;
            }

            __borFiboCut(f, n, p);
            p = p->parent;
        }
    }
}
