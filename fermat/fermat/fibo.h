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

#ifndef __FER_FIBO_H__
#define __FER_FIBO_H__

#include <fermat/core.h>
#include <fermat/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define FER_FIBO_CONS_SIZE (sizeof(unsigned int) << 3)

/**
 * Fibo - Fibonnacci Heap (priority queue)
 * ========================================
 *
 */

struct _fer_fibo_node_t {
    unsigned int degree; /*!< Degree of node */
    int mark; // TODO
    struct _fer_fibo_node_t *parent; /*!< Parent node */
    fer_list_t children;             /*!< List of children */
    fer_list_t list;                 /*!< Connection into list of siblings */

    int value;
};
typedef struct _fer_fibo_node_t fer_fibo_node_t;

struct _fer_fibo_t {
    fer_list_t root;      /*!< List of root nodes */
    fer_fibo_node_t *min; /*!< Minimal node */
    fer_fibo_node_t *cons[FER_FIBO_CONS_SIZE]; /*!< Consolidation array
                                                    with bitlength(degree)
                                                    elements */
};
typedef struct _fer_fibo_t fer_fibo_t;


/**
 * Creates new empty Fibonnacci heap
 */
fer_fibo_t *ferFiboNew(void);

/**
 * Deletes fibonnacci heap.
 * Note that individual nodes are not disconnected from heap.
 */
void ferFiboDel(fer_fibo_t *fibo);

/**
 * Returns minimal node.
 */
_fer_inline fer_fibo_node_t *ferFiboMin(fer_fibo_t *f);

/**
 * Adds node to heap.
 */
_fer_inline void ferFiboAdd(fer_fibo_t *f, fer_fibo_node_t *n);

/**
 * Removes and returns minimal node from heap.
 */
fer_fibo_node_t *ferFiboExtractMin(fer_fibo_t *f);

/**
 * Update position of node in heap in case its value was decreased.
 * If value wasn't decreased call ferFiboUpdate() instead.
 */
void ferFiboDecreaseKey(fer_fibo_t *f, fer_fibo_node_t *n);

/**
 * Updates position of node in heap.
 */
_fer_inline void ferFiboUpdate(fer_fibo_t *f, fer_fibo_node_t *n);

/**
 * Del node from heap.
 */
void ferFiboRemove(fer_fibo_t *f, fer_fibo_node_t *n);

/**** INLINES ****/
_fer_inline fer_fibo_node_t *ferFiboMin(fer_fibo_t *f)
{
    return f->min;
}

_fer_inline void ferFiboAdd(fer_fibo_t *f, fer_fibo_node_t *n)
{
    n->degree = 0;
    n->mark   = 0;
    n->parent = 0;
    ferListInit(&n->children);
    ferListAppend(&f->root, &n->list);

    // TODO:
    if (!f->min || n->value < f->min->value)
        f->min = n;
}

_fer_inline void ferFiboUpdate(fer_fibo_t *f, fer_fibo_node_t *n)
{
    ferFiboRemove(f, n);
    ferFiboAdd(f, n);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_FIBO_H__ */

