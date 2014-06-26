/***
 * Boruvka
 * --------
 * Copyright (c)2014 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.splaytreep>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include "boruvka/alloc.h"
#include "boruvka/splaytree.h"

#define SPLAY_LEFT(elm)   (elm)->spe_left
#define SPLAY_RIGHT(elm)  (elm)->spe_right
#define SPLAY_ROOT(head)  (head)->root
#define SPLAY_EMPTY(head) (SPLAY_ROOT(head) == NULL)

/* SPLAY_ROTATE_{LEFT,RIGHT} expect that tmp hold SPLAY_{RIGHT,LEFT} */
#define SPLAY_ROTATE_RIGHT(head, tmp) do { \
    SPLAY_LEFT((head)->root) = SPLAY_RIGHT(tmp); \
    SPLAY_RIGHT(tmp) = (head)->root; \
    (head)->root = tmp; \
} while (/*CONSTCOND*/ 0)

#define SPLAY_ROTATE_LEFT(head, tmp) do { \
    SPLAY_RIGHT((head)->root) = SPLAY_LEFT(tmp); \
    SPLAY_LEFT(tmp) = (head)->root; \
    (head)->root = tmp; \
} while (/*CONSTCOND*/ 0)

#define SPLAY_LINKLEFT(head, tmp) do { \
    SPLAY_LEFT(tmp) = (head)->root; \
    tmp = (head)->root; \
    (head)->root = SPLAY_LEFT((head)->root); \
} while (/*CONSTCOND*/ 0)

#define SPLAY_LINKRIGHT(head, tmp) do { \
    SPLAY_RIGHT(tmp) = (head)->root; \
    tmp = (head)->root; \
    (head)->root = SPLAY_RIGHT((head)->root); \
} while (/*CONSTCOND*/ 0)

#define SPLAY_ASSEMBLE(head, node, left, right) do { \
    SPLAY_RIGHT(left) = SPLAY_LEFT((head)->root); \
    SPLAY_LEFT(right) = SPLAY_RIGHT((head)->root); \
    SPLAY_LEFT((head)->root) = SPLAY_RIGHT(node); \
    SPLAY_RIGHT((head)->root) = SPLAY_LEFT(node); \
} while (/*CONSTCOND*/ 0)



bor_splaytree_t *borSplayTreeNew(bor_splaytree_cmp cmp, void *data)
{
    bor_splaytree_t *st;

    st = BOR_ALLOC(bor_splaytree_t);
    borSplayTreeInit(st, cmp, data);
    return st;
}

void borSplayTreeDel(bor_splaytree_t *splaytree)
{
    BOR_FREE(splaytree);
}

void borSplayTreeInit(bor_splaytree_t *st, bor_splaytree_cmp cmp, void *data)
{
    st->root = NULL;
    st->cmp = cmp;
    st->data = data;
}

void borSplayTreeFree(bor_splaytree_t *st)
{
    st->root = NULL;
    st->cmp = NULL;
    st->data = NULL;
}

static void splay(bor_splaytree_t *head, bor_splaytree_node_t *elm)
{
    bor_splaytree_node_t __node, *__left, *__right, *__tmp;
    int __comp;

    SPLAY_LEFT(&__node) = SPLAY_RIGHT(&__node) = NULL;
    __left = __right = &__node;

    while ((__comp = head->cmp(elm, (head)->root, head->data)) != 0) {
        if (__comp < 0) {
            __tmp = SPLAY_LEFT((head)->root);
            if (__tmp == NULL)
                break;
            if (head->cmp(elm, __tmp, head->data) < 0){
                SPLAY_ROTATE_RIGHT(head, __tmp);
                if (SPLAY_LEFT((head)->root) == NULL)
                    break;
            }
            SPLAY_LINKLEFT(head, __right);
        } else if (__comp > 0) {
            __tmp = SPLAY_RIGHT((head)->root);
            if (__tmp == NULL)
                break;
            if (head->cmp(elm, __tmp, head->data) > 0){
                SPLAY_ROTATE_LEFT(head, __tmp);
                if (SPLAY_RIGHT((head)->root) == NULL)
                    break;
            }
            SPLAY_LINKRIGHT(head, __left);
        }
    }
    SPLAY_ASSEMBLE(head, &__node, __left, __right);
}

bor_splaytree_node_t *borSplayTreeInsert(bor_splaytree_t *head,
                                         bor_splaytree_node_t *elm)
{
    if (SPLAY_EMPTY(head)) {
        SPLAY_LEFT(elm) = SPLAY_RIGHT(elm) = NULL;
    } else {
        int __comp;
        splay(head, elm);
        __comp = head->cmp(elm, (head)->root, head->data);
        if(__comp < 0) {
            SPLAY_LEFT(elm) = SPLAY_LEFT((head)->root);
            SPLAY_RIGHT(elm) = (head)->root;
            SPLAY_LEFT((head)->root) = NULL;
        } else if (__comp > 0) {
            SPLAY_RIGHT(elm) = SPLAY_RIGHT((head)->root);
            SPLAY_LEFT(elm) = (head)->root;
            SPLAY_RIGHT((head)->root) = NULL;
        } else
            return ((head)->root);
    }
    (head)->root = (elm);
    return (NULL);
}

bor_splaytree_node_t *borSplayTreeRemove(bor_splaytree_t *head,
                                         bor_splaytree_node_t *elm)
{
    bor_splaytree_node_t *__tmp;
    if (SPLAY_EMPTY(head))
        return (NULL);
    splay(head, elm);
    if (head->cmp(elm, (head)->root, head->data) == 0) {
        if (SPLAY_LEFT((head)->root) == NULL) {
            (head)->root = SPLAY_RIGHT((head)->root);
        } else {
            __tmp = SPLAY_RIGHT((head)->root);
            (head)->root = SPLAY_LEFT((head)->root);
            splay(head, elm);
            SPLAY_RIGHT((head)->root) = __tmp;
        }
        return (elm);
    }
    return (NULL);
}

bor_splaytree_node_t *borSplayTreeFind(bor_splaytree_t *head,
                                       bor_splaytree_node_t *elm)
{
    if (SPLAY_EMPTY(head))                        
        return(NULL);                        
    splay(head, elm);
    if (head->cmp(elm, (head)->root, head->data) == 0)                
        return (head->root);                
    return (NULL);                            
}

bor_splaytree_node_t *borSplayTreeNext(bor_splaytree_t *head,
                                       bor_splaytree_node_t *elm)
{
    splay(head, elm);
    if (SPLAY_RIGHT(elm) != NULL) {
        elm = SPLAY_RIGHT(elm);
        while (SPLAY_LEFT(elm) != NULL) {
            elm = SPLAY_LEFT(elm);
        }
    } else
        elm = NULL;
    return (elm);
}

bor_splaytree_node_t *borSplayTreePrev(bor_splaytree_t *head,
                                       bor_splaytree_node_t *elm)
{
    splay(head, elm);
    if (SPLAY_LEFT(elm) != NULL) {
        elm = SPLAY_LEFT(elm);
        while (SPLAY_RIGHT(elm) != NULL) {
            elm = SPLAY_RIGHT(elm);
        }
    } else
        elm = NULL;
    return (elm);
}

bor_splaytree_node_t *borSplayTreeMin(bor_splaytree_t *head)
{
    bor_splaytree_node_t __node, *__left, *__right, *__tmp;

    if (head->root == NULL)
        return NULL;

    SPLAY_LEFT(&__node) = SPLAY_RIGHT(&__node) = NULL;
    __left = __right = &__node;

    while (1) {
        __tmp = SPLAY_LEFT((head)->root);
        if (__tmp == NULL)
            break;
        SPLAY_ROTATE_RIGHT(head, __tmp);
        if (SPLAY_LEFT((head)->root) == NULL)
            break;
        SPLAY_LINKLEFT(head, __right);
    }
    SPLAY_ASSEMBLE(head, &__node, __left, __right);
    return (SPLAY_ROOT(head));
}

bor_splaytree_node_t *borSplayTreeMax(bor_splaytree_t *head)
{
    bor_splaytree_node_t __node, *__left, *__right, *__tmp;

    if (head->root == NULL)
        return NULL;

    SPLAY_LEFT(&__node) = SPLAY_RIGHT(&__node) = NULL;
    __left = __right = &__node;

    while (1) {
        __tmp = SPLAY_RIGHT((head)->root);
        if (__tmp == NULL)
            break;
        SPLAY_ROTATE_LEFT(head, __tmp);
        if (SPLAY_RIGHT((head)->root) == NULL)
            break;
        SPLAY_LINKRIGHT(head, __left);
    }
    SPLAY_ASSEMBLE(head, &__node, __left, __right);
    return (SPLAY_ROOT(head));
}
