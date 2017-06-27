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

#ifndef __BOR_SET_H__
#define __BOR_SET_H__

#include <boruvka/compiler.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Array-based set.
 */
struct bor_set {
    TYPE *s;
    int size;
    int alloc;
};
typedef struct bor_set bor_set_t;

#define BOR_SET_FOR_EACH(S, V) \
    for (int __i = 0; __i < (S)->size && ((V) = (S)->s[__i], 1); ++__i)

/**
 * Initialize the set.
 */
void borSetInit(bor_set_t *s);

/**
 * Frees allocated memory.
 */
void borSetFree(bor_set_t *s);

/**
 * Returns ith element from the set.
 */
_bor_inline TYPE borSetGet(bor_set_t *s, int i);

/**
 * Returns size of the set.
 */
_bor_inline int borSetSize(bor_set_t *s);

/**
 * Returns true if the set contains val.
 */
int borSetHas(const bor_set_t *s, TYPE val);

/**
 * Makes the set empty.
 */
_bor_inline void borSetEmpty(bor_set_t *s);

/**
 * Adds a new value into the set if not already there.
 */
void borSetAdd(bor_set_t *s, TYPE val);

/**
 * dst = dst \cup src
 */
void borSetUnion(bor_set_t *dst, const bor_set_t *src);

/**
 * Allocate enough memory for size elements.
 * (It does not change s->size.)
 */
void borSetResize(bor_set_t *s, int size);

/**
 * Returns true if the sets are equal.
 */
_bor_inline int borSetEq(const bor_set_t *s1, const bor_set_t *s2);

/**
 * s1 = s1 \setminus s2
 */
void borSetMinus(bor_set_t *s1, const bor_set_t *s2);

/**
 * s = s \setminus {val}
 * Returns true if val was found in s.
 */
int borSetRm(bor_set_t *s, TYPE val);



/**** INLINES: ****/
_bor_inline TYPE borSetGet(bor_set_t *s, int i)
{
    return s->s[i];
}

_bor_inline int borSetSize(bor_set_t *s)
{
    return s->size;
}

_bor_inline void borSetEmpty(bor_set_t *s)
{
    s->size = 0;
}

_bor_inline int borSetEq(const bor_set_t *s1, const bor_set_t *s2)
{
    return s1->size == s2->size
            && memcmp(s1->s, s2->s, sizeof(TYPE) * s1->size) == 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_SET_H__ */
