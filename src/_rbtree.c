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

/**
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

#define RB_SET_BLACKRED(black, red) \
    do { \
        RB_SET_BLACK(black); \
        RB_SET_RED(red); \
    } while (0)

#define RB_ROTATE_LEFT(head, elm, tmp) do { \
    (tmp) = RB_RIGHT(elm); \
    if ((RB_RIGHT(elm) = RB_LEFT(tmp)) != NULL) { \
        RB_PARENT(RB_LEFT(tmp)) = (elm); \
    } \
    if ((RB_PARENT(tmp) = RB_PARENT(elm)) != NULL) { \
        if ((elm) == RB_LEFT(RB_PARENT(elm))) \
            RB_LEFT(RB_PARENT(elm)) = (tmp); \
        else \
            RB_RIGHT(RB_PARENT(elm)) = (tmp); \
    } else \
        (head)->root = (tmp); \
    RB_LEFT(tmp) = (elm); \
    RB_PARENT(elm) = (tmp); \
} while (0)

#define RB_ROTATE_RIGHT(head, elm, tmp) do { \
    (tmp) = RB_LEFT(elm); \
    if ((RB_LEFT(elm) = RB_RIGHT(tmp)) != NULL) { \
        RB_PARENT(RB_RIGHT(tmp)) = (elm); \
    } \
    if ((RB_PARENT(tmp) = RB_PARENT(elm)) != NULL) { \
        if ((elm) == RB_LEFT(RB_PARENT(elm))) \
            RB_LEFT(RB_PARENT(elm)) = (tmp); \
        else \
            RB_RIGHT(RB_PARENT(elm)) = (tmp); \
    } else \
        (head)->root = (tmp); \
    RB_RIGHT(tmp) = (elm); \
    RB_PARENT(elm) = (tmp); \
} while (/*CONSTCOND*/ 0)

_bor_inline void rbSet(RB_NODE *elm, RB_NODE *parent)
{
    RB_PARENT(elm) = parent;
    RB_LEFT(elm) = RB_RIGHT(elm) = NULL;
    RB_SET_RED(elm);
}

void RB_DEL(RB_TREE *rbtree)
{
    BOR_FREE(rbtree);
}

static void rbInsertColor(RB_TREE *rbtree,
                          RB_NODE *elm)
{
    RB_NODE *parent, *gparent, *tmp;
    while ((parent = RB_PARENT(elm)) != NULL && RB_IS_RED(parent)){
        gparent = RB_PARENT(parent);
        if (parent == RB_LEFT(gparent)){
            tmp = RB_RIGHT(gparent);
            if (tmp && RB_IS_RED(tmp)){
                RB_SET_BLACK(tmp);
                RB_SET_BLACKRED(parent, gparent);
                elm = gparent;
                continue;
            }
            if (RB_RIGHT(parent) == elm) {
                RB_ROTATE_LEFT(rbtree, parent, tmp);
                tmp = parent;
                parent = elm;
                elm = tmp;
            }
            RB_SET_BLACKRED(parent, gparent);
            RB_ROTATE_RIGHT(rbtree, gparent, tmp);
        } else {
            tmp = RB_LEFT(gparent);
            if (tmp && RB_IS_RED(tmp)) {
                RB_SET_BLACK(tmp);
                RB_SET_BLACKRED(parent, gparent);
                elm = gparent;
                continue;
            }
            if (RB_LEFT(parent) == elm) {
                RB_ROTATE_RIGHT(rbtree, parent, tmp);
                tmp = parent;
                parent = elm;
                elm = tmp;
            }
            RB_SET_BLACKRED(parent, gparent);
            RB_ROTATE_LEFT(rbtree, gparent, tmp);
        }
    }
    RB_SET_BLACK(rbtree->root);
}

RB_INSERT
{
    RB_NODE *tmp;
    RB_NODE *parent = NULL;
    int comp = 0;

    RB_SET_UP_KEY

    tmp = rbtree->root;
    while (tmp){
        parent = tmp;
        comp = RB_CMP;
        if (comp < 0){
            tmp = RB_LEFT(tmp);
        }else if (comp > 0){
            tmp = RB_RIGHT(tmp);
        }else{
            return tmp;
        }
    }

    rbSet(n, parent);
    if (parent != NULL){
        if (comp < 0){
            RB_LEFT(parent) = n;
        }else{
            RB_RIGHT(parent) = n;
        }
    }else{
        rbtree->root = n;
    }

    rbInsertColor(rbtree, n);
    return NULL;
}

static void rbRemoveColor(RB_TREE *rbtree, RB_NODE *parent,
                          RB_NODE *elm)
{
    RB_NODE *tmp;

    while ((elm == NULL || RB_IS_BLACK(elm)) &&
        elm != rbtree->root) {
        if (RB_LEFT(parent) == elm) {
            tmp = RB_RIGHT(parent);
            if (RB_IS_RED(tmp)) {
                RB_SET_BLACKRED(tmp, parent);
                RB_ROTATE_LEFT(rbtree, parent, tmp);
                tmp = RB_RIGHT(parent);
            }
            if ((RB_LEFT(tmp) == NULL ||
                RB_IS_BLACK(RB_LEFT(tmp))) &&
                (RB_RIGHT(tmp) == NULL ||
                RB_IS_BLACK(RB_RIGHT(tmp)))) {
                RB_SET_RED(tmp);
                elm = parent;
                parent = RB_PARENT(elm);
            } else {
                if (RB_RIGHT(tmp) == NULL ||
                    RB_IS_BLACK(RB_RIGHT(tmp))) {
                    RB_NODE *oleft;
                    if ((oleft = RB_LEFT(tmp))
                        != NULL)
                        RB_SET_BLACK(oleft);
                    RB_SET_RED(tmp);
                    RB_ROTATE_RIGHT(rbtree, tmp, oleft);
                    tmp = RB_RIGHT(parent);
                }
                RB_COPY_COLOR(tmp, parent);
                RB_SET_BLACK(parent);
                if (RB_RIGHT(tmp))
                    RB_SET_BLACK(RB_RIGHT(tmp));
                RB_ROTATE_LEFT(rbtree, parent, tmp);
                elm = rbtree->root;
                break;
            }
        } else {
            tmp = RB_LEFT(parent);
            if (RB_IS_RED(tmp)) {
                RB_SET_BLACKRED(tmp, parent);
                RB_ROTATE_RIGHT(rbtree, parent, tmp);
                tmp = RB_LEFT(parent);
            }
            if ((RB_LEFT(tmp) == NULL ||
                RB_IS_BLACK(RB_LEFT(tmp))) &&
                (RB_RIGHT(tmp) == NULL ||
                RB_IS_BLACK(RB_RIGHT(tmp)))) {
                RB_SET_RED(tmp);
                elm = parent;
                parent = RB_PARENT(elm);
            } else {
                if (RB_LEFT(tmp) == NULL ||
                    RB_IS_BLACK(RB_LEFT(tmp))) {
                    RB_NODE *oright;
                    if ((oright = RB_RIGHT(tmp))
                        != NULL)
                        RB_SET_BLACK(oright);
                    RB_SET_RED(tmp);
                    RB_ROTATE_LEFT(rbtree, tmp, oright);
                    tmp = RB_LEFT(parent);
                }
                RB_COPY_COLOR(tmp, parent);
                RB_SET_BLACK(parent);
                if (RB_LEFT(tmp))
                    RB_SET_BLACK(RB_LEFT(tmp));
                RB_ROTATE_RIGHT(rbtree, parent, tmp);
                elm = rbtree->root;
                break;
            }
        }
    }
    if (elm)
        RB_SET_BLACK(elm);
}

RB_NODE *RB_REMOVE(RB_TREE *rbtree, RB_NODE *elm)
{
    RB_NODE *child, *parent, *old = elm;
    int color;

    if (RB_LEFT(elm) == NULL){
        child = RB_RIGHT(elm);
    }else if (RB_RIGHT(elm) == NULL){
        child = RB_LEFT(elm);
    }else{
        RB_NODE *left;
        elm = RB_RIGHT(elm);
        while ((left = RB_LEFT(elm)) != NULL)
            elm = left;
        child = RB_RIGHT(elm);
        parent = RB_PARENT(elm);
        color = RB_IS_BLACK(elm);
        if (child)
            RB_PARENT(child) = parent;
        if (parent) {
            if (RB_LEFT(parent) == elm)
                RB_LEFT(parent) = child;
            else
                RB_RIGHT(parent) = child;
        } else
            rbtree->root = child;
        if (RB_PARENT(elm) == old)
            parent = elm;
        RB_CP(elm, old);
        if (RB_PARENT(old)) {
            if (RB_LEFT(RB_PARENT(old)) == old)
                RB_LEFT(RB_PARENT(old)) = elm;
            else
                RB_RIGHT(RB_PARENT(old)) = elm;
        } else
            rbtree->root = elm;
        RB_PARENT(RB_LEFT(old)) = elm;
        if (RB_RIGHT(old))
            RB_PARENT(RB_RIGHT(old)) = elm;
        if (parent) {
            left = parent;
        }
        goto color;
    }
    parent = RB_PARENT(elm);
    color = RB_IS_BLACK(elm);
    if (child)
        RB_PARENT(child) = parent;
    if (parent) {
        if (RB_LEFT(parent) == elm)
            RB_LEFT(parent) = child;
        else
            RB_RIGHT(parent) = child;
    } else
        rbtree->root = child;
color:
    if (color)
        rbRemoveColor(rbtree, parent, child);
    return (old);
}
