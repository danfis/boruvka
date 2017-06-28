/***
 * Boruvka
 * --------
 * Copyright (c)2017 Daniel Fiser <danfis@danfis.cz>
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

#include <boruvka/alloc.h>
#include "boruvka/set.h"

void borSetInit(bor_set_t *s)
{
    bzero(s, sizeof(*s));
}

void borSetFree(bor_set_t *s)
{
    if (s->s)
        BOR_FREE(s->s);
}

void borSetResize(bor_set_t *s, int size)
{
    s->s = BOR_REALLOC_ARR(s->s, TYPE, size);
    s->size = size;
}

int borSetHas(const bor_set_t *s, TYPE v)
{
    // TODO: binary search
    for (int i = 0; i < s->size; ++i){
        if (s->s[i] == v)
            return 1;
    }
    return 0;
}

int borSetIsSubset(const bor_set_t *s1, const bor_set_t *s2)
{
    int i, j, size;

    if (s1->size > s2->size)
        return 0;

    size = s1->size;
    for (i = j = 0; i < size && j < s2->size;){
        if (s1->s[i] == s2->s[j]){
            ++i;
            ++j;
        }else if (s1->s[i] < s2->s[j]){
            return 0;
        }else{
            ++j;
        }
    }
    return i == size;
}

int borSetIntersectionSize(const bor_set_t *s1, const bor_set_t *s2)
{
    int i, j, size, setsize;

    setsize = 0;
    size = s1->size;
    for (i = j = 0; i < size && j < s2->size;){
        if (s1->s[i] == s2->s[j]){
            ++setsize;
            ++i;
            ++j;
        }else if (s1->s[i] < s2->s[j]){
            ++i;
        }else{
            ++j;
        }
    }
    return setsize;
}

int borSetIntersectionSizeAtLeast(const bor_set_t *s1, const bor_set_t *s2,
                                  int limit)
{
    int i, j, size, setsize;

    if (limit == 0)
        return 1;

    setsize = 0;
    size = s1->size;
    for (i = j = 0; i < size && j < s2->size;){
        if (s1->s[i] == s2->s[j]){
            if (++setsize == limit)
                return 1;
            ++i;
            ++j;
        }else if (s1->s[i] < s2->s[j]){
            ++i;
        }else{
            ++j;
        }
    }
    return 0;
}

void borSetAdd(bor_set_t *s, TYPE v)
{
    if (s->size >= s->alloc){
        if (s->alloc == 0)
            s->alloc = 1;
        s->alloc *= 2;
        s->s = BOR_REALLOC_ARR(s->s, TYPE, s->alloc);
    }
    s->s[s->size++] = v;

    if (s->size > 1 && v <= s->s[s->size - 2]){
        TYPE *f = s->s + s->size - 1;
        for (; f > s->s && f[0] < f[-1]; --f){
            TYPE tmp = f[0];
            f[0] = f[-1];
            f[-1] = tmp;
        }
        if (f > s->s && f[0] == f[-1]){
            for (--s->size; f != s->s + s->size; ++f)
                *f = f[1];
        }
    }
}

int borSetRm(bor_set_t *s, TYPE v)
{
    int i;

    for (i = 0; i < s->size && s->s[i] < v; ++i);
    if (i < s->size && s->s[i] == v){
        for (++i; i < s->size; ++i)
            s->s[i - 1] = s->s[i];
        --s->size;
        return 1;
    }
    return 0;
}

void borSetUnion(bor_set_t *dst, const bor_set_t *src)
{
    for (int i = 0; i < src->size; ++i)
        borSetAdd(dst, src->s[i]);
}

void borSetIntersect(bor_set_t *dst, const bor_set_t *src)
{
    int w, i, j, size;

    size = dst->size;
    for (w = i = j = 0; i < size && j < src->size;){
        if (dst->s[i] == src->s[j]){
            dst->s[w++] = dst->s[i];
            ++i;
            ++j;
        }else if (dst->s[i] < src->s[j]){
            ++i;
        }else{
            ++j;
        }
    }
    dst->size = w;
}

void borSetMinus(bor_set_t *s1, const bor_set_t *s2)
{
    int w, i, j;

    for (w = i = j = 0; i < s1->size && j < s2->size;){
        if (s1->s[i] == s2->s[j]){
            ++i;
            ++j;
        }else if (s1->s[i] < s2->s[j]){
            s1->s[w++] = s1->s[i++];
        }else{
            ++j;
        }
    }
    for (; i < s1->size; ++i, ++w)
        s1->s[w] = s1->s[i];
    s1->size = w;
}
