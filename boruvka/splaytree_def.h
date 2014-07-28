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

#ifndef __BOR_SPLAYTREE_DEF_H__
#define __BOR_SPLAYTREE_DEF_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Splay Tree Definition
 * ======================
 *
 * This header should never be part of public API of any project that uses
 * boruvka library. The purpose of this is to provided macro-driven
 * definition of splay-tree routines that can be modified to your own goal.
 *
 * How to use this file, see splaytree and splaytree_int modules.
 *
 * The code was adopted from the FreeBSD's sys/tree.h:
 *
 * Copyright 2002 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef BOR_SPLAY_KEY_EQ
# define BOR_SPLAY_KEY_EQ(head, key1, key2) \
    (BOR_SPLAY_KEY_CMP((head), (key1), (key2)) == 0)
#endif /* BOR_SPLAY_KEY_EQ */

#define BOR_SPLAY_LEFT(elm)   (elm)->spe_left
#define BOR_SPLAY_RIGHT(elm)  (elm)->spe_right
#define BOR_SPLAY_ROOT(head)  (head)->root
#define BOR_SPLAY_EMPTY(head) (BOR_SPLAY_ROOT(head) == NULL)

/* BOR_SPLAY_ROTATE_{LEFT,RIGHT} expect that tmp hold BOR_SPLAY_{RIGHT,LEFT} */
#define BOR_SPLAY_ROTATE_RIGHT(head, tmp) do { \
    BOR_SPLAY_LEFT((head)->root) = BOR_SPLAY_RIGHT(tmp); \
    BOR_SPLAY_RIGHT(tmp) = (head)->root; \
    (head)->root = tmp; \
} while (/*CONSTCOND*/ 0)

#define BOR_SPLAY_ROTATE_LEFT(head, tmp) do { \
    BOR_SPLAY_RIGHT((head)->root) = BOR_SPLAY_LEFT(tmp); \
    BOR_SPLAY_LEFT(tmp) = (head)->root; \
    (head)->root = tmp; \
} while (/*CONSTCOND*/ 0)

#define BOR_SPLAY_LINKLEFT(head, tmp) do { \
    BOR_SPLAY_LEFT(tmp) = (head)->root; \
    tmp = (head)->root; \
    (head)->root = BOR_SPLAY_LEFT((head)->root); \
} while (/*CONSTCOND*/ 0)

#define BOR_SPLAY_LINKRIGHT(head, tmp) do { \
    BOR_SPLAY_RIGHT(tmp) = (head)->root; \
    tmp = (head)->root; \
    (head)->root = BOR_SPLAY_RIGHT((head)->root); \
} while (/*CONSTCOND*/ 0)

#define BOR_SPLAY_ASSEMBLE(head, node, left, right) do { \
    BOR_SPLAY_RIGHT(left) = BOR_SPLAY_LEFT((head)->root); \
    BOR_SPLAY_LEFT(right) = BOR_SPLAY_RIGHT((head)->root); \
    BOR_SPLAY_LEFT((head)->root) = BOR_SPLAY_RIGHT(node); \
    BOR_SPLAY_RIGHT((head)->root) = BOR_SPLAY_LEFT(node); \
} while (/*CONSTCOND*/ 0)



_bor_inline void borSplayInit(BOR_SPLAY_TREE_T *head)
{
    head->root = NULL;
}

_bor_inline void borSplayFree(BOR_SPLAY_TREE_T *head)
{
    head->root = NULL;
}

_bor_inline void borSplay(BOR_SPLAY_TREE_T *head,
                          BOR_SPLAY_KEY_T key)
{
    BOR_SPLAY_TREE_NODE_T __node, *__left, *__right, *__tmp;
    int __comp;

    BOR_SPLAY_LEFT(&__node) = BOR_SPLAY_RIGHT(&__node) = NULL;
    __left = __right = &__node;

    while ((__comp = BOR_SPLAY_KEY_CMP(head, key, BOR_SPLAY_NODE_KEY((head)->root))) != 0) {
        if (__comp < 0) {
            __tmp = BOR_SPLAY_LEFT((head)->root);
            if (__tmp == NULL)
                break;
            if (BOR_SPLAY_KEY_CMP(head, key, BOR_SPLAY_NODE_KEY(__tmp)) < 0){
                BOR_SPLAY_ROTATE_RIGHT(head, __tmp);
                if (BOR_SPLAY_LEFT((head)->root) == NULL)
                    break;
            }
            BOR_SPLAY_LINKLEFT(head, __right);
        } else if (__comp > 0) {
            __tmp = BOR_SPLAY_RIGHT((head)->root);
            if (__tmp == NULL)
                break;
            if (BOR_SPLAY_KEY_CMP(head, key, BOR_SPLAY_NODE_KEY(__tmp)) > 0){
                BOR_SPLAY_ROTATE_LEFT(head, __tmp);
                if (BOR_SPLAY_RIGHT((head)->root) == NULL)
                    break;
            }
            BOR_SPLAY_LINKRIGHT(head, __left);
        }
    }
    BOR_SPLAY_ASSEMBLE(head, &__node, __left, __right);
}

_bor_inline BOR_SPLAY_TREE_NODE_T *borSplayInsert(BOR_SPLAY_TREE_T *head,
                                                  BOR_SPLAY_KEY_T key,
                                                  BOR_SPLAY_TREE_NODE_T *elm)
{
    BOR_SPLAY_NODE_SET_KEY(head, elm, key);

    if (BOR_SPLAY_EMPTY(head)) {
        BOR_SPLAY_LEFT(elm) = BOR_SPLAY_RIGHT(elm) = NULL;
    } else {
        int __comp;
        borSplay(head, key);
        __comp = BOR_SPLAY_KEY_CMP(head, key, BOR_SPLAY_NODE_KEY(head->root));
        if(__comp < 0) {
            BOR_SPLAY_LEFT(elm) = BOR_SPLAY_LEFT((head)->root);
            BOR_SPLAY_RIGHT(elm) = (head)->root;
            BOR_SPLAY_LEFT((head)->root) = NULL;
        } else if (__comp > 0) {
            BOR_SPLAY_RIGHT(elm) = BOR_SPLAY_RIGHT((head)->root);
            BOR_SPLAY_LEFT(elm) = (head)->root;
            BOR_SPLAY_RIGHT((head)->root) = NULL;
        } else
            return ((head)->root);
    }
    (head)->root = (elm);
    return (NULL);
}

_bor_inline BOR_SPLAY_TREE_NODE_T *borSplayRemove(BOR_SPLAY_TREE_T *head,
                                                  BOR_SPLAY_TREE_NODE_T *elm)
{
    BOR_SPLAY_TREE_NODE_T *__tmp;
    if (BOR_SPLAY_EMPTY(head))
        return (NULL);
    borSplay(head, BOR_SPLAY_NODE_KEY(elm));
    if (BOR_SPLAY_KEY_EQ(head, BOR_SPLAY_NODE_KEY(elm), BOR_SPLAY_NODE_KEY((head)->root))) {
        if (BOR_SPLAY_LEFT((head)->root) == NULL) {
            (head)->root = BOR_SPLAY_RIGHT((head)->root);
        } else {
            __tmp = BOR_SPLAY_RIGHT((head)->root);
            (head)->root = BOR_SPLAY_LEFT((head)->root);
            borSplay(head, BOR_SPLAY_NODE_KEY(elm));
            BOR_SPLAY_RIGHT((head)->root) = __tmp;
        }
        return (elm);
    }
    return (NULL);
}

_bor_inline BOR_SPLAY_TREE_NODE_T *borSplayFind(BOR_SPLAY_TREE_T *head,
                                                BOR_SPLAY_KEY_T key)
{
    if (BOR_SPLAY_EMPTY(head))
        return(NULL);
    borSplay(head, key);
    if (BOR_SPLAY_KEY_EQ(head, key, BOR_SPLAY_NODE_KEY((head)->root)))
        return (head->root);
    return (NULL);
}

_bor_inline BOR_SPLAY_TREE_NODE_T *borSplayNext(BOR_SPLAY_TREE_T *head,
                                                BOR_SPLAY_TREE_NODE_T *elm)
{
    borSplay(head, BOR_SPLAY_NODE_KEY(elm));
    if (BOR_SPLAY_RIGHT(elm) != NULL) {
        elm = BOR_SPLAY_RIGHT(elm);
        while (BOR_SPLAY_LEFT(elm) != NULL) {
            elm = BOR_SPLAY_LEFT(elm);
        }
    } else
        elm = NULL;
    return (elm);
}

_bor_inline BOR_SPLAY_TREE_NODE_T *borSplayPrev(BOR_SPLAY_TREE_T *head,
                                                BOR_SPLAY_TREE_NODE_T *elm)
{
    borSplay(head, BOR_SPLAY_NODE_KEY(elm));
    if (BOR_SPLAY_LEFT(elm) != NULL) {
        elm = BOR_SPLAY_LEFT(elm);
        while (BOR_SPLAY_RIGHT(elm) != NULL) {
            elm = BOR_SPLAY_RIGHT(elm);
        }
    } else
        elm = NULL;
    return (elm);
}

_bor_inline BOR_SPLAY_TREE_NODE_T *borSplayMin(BOR_SPLAY_TREE_T *head)
{
    BOR_SPLAY_TREE_NODE_T __node, *__left, *__right, *__tmp;

    if (head->root == NULL)
        return NULL;

    BOR_SPLAY_LEFT(&__node) = BOR_SPLAY_RIGHT(&__node) = NULL;
    __left = __right = &__node;

    while (1) {
        __tmp = BOR_SPLAY_LEFT((head)->root);
        if (__tmp == NULL)
            break;
        BOR_SPLAY_ROTATE_RIGHT(head, __tmp);
        if (BOR_SPLAY_LEFT((head)->root) == NULL)
            break;
        BOR_SPLAY_LINKLEFT(head, __right);
    }
    BOR_SPLAY_ASSEMBLE(head, &__node, __left, __right);
    return (BOR_SPLAY_ROOT(head));
}

_bor_inline BOR_SPLAY_TREE_NODE_T *borSplayMax(BOR_SPLAY_TREE_T *head)
{
    BOR_SPLAY_TREE_NODE_T __node, *__left, *__right, *__tmp;

    if (head->root == NULL)
        return NULL;

    BOR_SPLAY_LEFT(&__node) = BOR_SPLAY_RIGHT(&__node) = NULL;
    __left = __right = &__node;

    while (1) {
        __tmp = BOR_SPLAY_RIGHT((head)->root);
        if (__tmp == NULL)
            break;
        BOR_SPLAY_ROTATE_LEFT(head, __tmp);
        if (BOR_SPLAY_RIGHT((head)->root) == NULL)
            break;
        BOR_SPLAY_LINKRIGHT(head, __left);
    }
    BOR_SPLAY_ASSEMBLE(head, &__node, __left, __right);
    return (BOR_SPLAY_ROOT(head));
}

#endif /* __BOR_SPLAYTREE_DEF_H__ */
