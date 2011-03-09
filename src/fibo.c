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

#include <fermat/fibo.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** Consolidates heap. */
_fer_inline void __ferFiboConsolidate(fer_fibo_t *f);

/** Links subtrees n1 and n2, n1 and n2 must be (!) both root nodes.
 *  It is also assumed n2 is smaller than n1.
 *  After this n1 becomes child of n2. */
_fer_inline void __ferFiboLink(fer_fibo_t *f, fer_fibo_node_t *n1,
                                              fer_fibo_node_t *n2);

/** Cuts x from y's list of children */
_fer_inline void __ferFiboCut(fer_fibo_t *f, fer_fibo_node_t *x,
                                             fer_fibo_node_t *y);

/** Cuts n from heap and cascadely the same with parents */
_fer_inline void __ferFiboCutCascade(fer_fibo_t *f, fer_fibo_node_t *n);


fer_fibo_t *ferFiboNew(fer_fibo_lt lt, void *data)
{
    fer_fibo_t *fibo;

    fibo = FER_ALLOC(fer_fibo_t);

    bzero(fibo, sizeof(fer_fibo_t));
    ferListInit(&fibo->root);

    fibo->max_degree = 0;

    fibo->lt = lt;
    fibo->data = data;

    return fibo;
}

void ferFiboDel(fer_fibo_t *fibo)
{
    free(fibo);
}

fer_fibo_node_t *ferFiboExtractMin(fer_fibo_t *f)
{
    fer_list_t *item;
    fer_fibo_node_t *min, *n;

    if (!f->min)
        return NULL;

    min = f->min;

    // put all children to root list
    while (!ferListEmpty(&min->children)){
        item = ferListNext(&min->children);
        n    = ferListEntry(item, fer_fibo_node_t, list);

        ferListDel(item);
        ferListAppend(&f->root, item);
        n->parent = NULL;
    }

    // remove minimum from root list
    ferListDel(&min->list);

    __ferFiboConsolidate(f);

    return min;
}

void ferFiboDecreaseKey(fer_fibo_t *f, fer_fibo_node_t *n)
{
    __ferFiboCutCascade(f, n);

    if (n != f->min && (!f->min || f->lt(n, f->min, f->data)))
        f->min = n;
}

void ferFiboRemove(fer_fibo_t *f, fer_fibo_node_t *n)
{
    fer_list_t *list, *item, *item_tmp;
    fer_fibo_node_t *c;

    __ferFiboCutCascade(f, n);

    // remove all its children
    list = &n->children;
    ferListForEachSafe(list, item, item_tmp){
        c = fer_container_of(item, fer_fibo_node_t, list);
        __ferFiboCut(f, c, n);
    }

    // remove from root list
    ferListDel(&n->list);

    __ferFiboConsolidate(f);
}


_fer_inline void __ferFiboConsolidate(fer_fibo_t *f)
{
    fer_list_t *list, *item, *tmp_item;
    fer_fibo_node_t *n;
    unsigned int degree;
    size_t i;

    for (i = 0; i <= f->max_degree; i++){
        f->cons[i] = NULL;
    }

    f->max_degree = 0;

    list = &f->root;
    ferListForEachSafe(list, item, tmp_item){
        n = ferListEntry(item, fer_fibo_node_t, list);

        degree = n->degree;
        while (f->cons[degree] != NULL){
            if (f->lt(f->cons[degree], n, f->data)){
                __ferFiboLink(f, n, f->cons[degree]);
                n = f->cons[degree];
            }else{
                __ferFiboLink(f, f->cons[degree], n);
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

_fer_inline void __ferFiboLink(fer_fibo_t *f, fer_fibo_node_t *n1,
                                              fer_fibo_node_t *n2)
{
    ferListDel(&n1->list);
    ferListAppend(&n2->children, &n1->list);
    n1->parent = n2;
    n2->degree += 1;
    n1->mark = 0;
}

_fer_inline void __ferFiboCut(fer_fibo_t *f, fer_fibo_node_t *x,
                                             fer_fibo_node_t *y)
{
    ferListDel(&x->list);
    y->degree--;
    ferListAppend(&f->root, &x->list);
    x->parent = NULL;
    x->mark = 0;
}

_fer_inline void __ferFiboCutCascade(fer_fibo_t *f, fer_fibo_node_t *n)
{
    fer_fibo_node_t *p;

    p = n->parent;

    if (p && f->lt(n, p, f->data)){
        __ferFiboCut(f, n, p);

        while (p){
            if (!p->mark){
                p->mark = 1;
                break;
            }

            __ferFiboCut(f, n, p);
            p = p->parent;
        }
    }
}
