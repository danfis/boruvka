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

#include <string.h>
#include <boruvka/compiler.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Array-based set.
 * The elements in .s are always sorted.
 */
struct bor_set {
    TYPE *s;
    int size;
    int alloc;
};
typedef struct bor_set bor_set_t;

#define BOR_SET_INIT { NULL, 0, 0 }
#define BOR_SET(NAME) bor_set_t NAME = BOR_SET_INIT

#define BOR_SET_FOR_EACH(S, V) \
    for (int __i = 0; __i < (S)->size && ((V) = (S)->s[__i], 1); ++__i)

#define BOR_SET_ADD(S, ...) \
    do { \
        TYPE ___bor_set_vals[] = {__VA_ARGS__}; \
        int ___bor_set_size = sizeof(___bor_set_vals) / sizeof(TYPE); \
        for (int i = 0; i < ___bor_set_size; ++i) \
            borSetAdd((S), ___bor_set_vals[i]); \
    }while (0)

#define BOR_SET_SET(S, ...) \
    do { \
        borISetEmpty(S); \
        BOR_SET_ADD((S), __VA_ARGS__); \
    } while (0)

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
_bor_inline TYPE borSetGet(const bor_set_t *s, int i);

/**
 * Returns size of the set.
 */
_bor_inline int borSetSize(const bor_set_t *s);


/**
 * Returns true if val \in s
 */
int borSetHas(const bor_set_t *s, TYPE val);
_bor_inline int borSetIn(TYPE val, const bor_set_t *s);

/**
 * Return true if s1 \subset s2
 */
int borSetIsSubset(const bor_set_t *s1, const bor_set_t *s2);

/**
 * Returns size of s1 \cap s2.
 */
int borSetIntersectionSize(const bor_set_t *s1, const bor_set_t *s2);

/**
 * Returns true if | s1 \cap s2 | >= limit
 */
int borSetIntersectionSizeAtLeast(const bor_set_t *s1, const bor_set_t *s2,
                                  int limit);

/**
 * Returns true if | s1 \cap s2 \cap s3 | >= limit
 */
int borSetIntersectionSizeAtLeast3(const bor_set_t *s1,
                                   const bor_set_t *s2,
                                   const bor_set_t *s3,
                                   int limit);

/**
 * Returns true if the sets are disjunct.
 */
_bor_inline int borSetIsDisjunct(const bor_set_t *s1, const bor_set_t *s2);

/**
 * s = \emptyset
 */
_bor_inline void borSetEmpty(bor_set_t *s);

/**
 * d = s
 */
void borSetSet(bor_set_t *d, const bor_set_t *s);

/**
 * s = s \cup {val}
 */
void borSetAdd(bor_set_t *s, TYPE val);

/**
 * s = s \setminus {val}
 * Returns true if val was found in s.
 */
int borSetRm(bor_set_t *s, TYPE val);

/**
 * dst = dst \cup src
 */
void borSetUnion(bor_set_t *dst, const bor_set_t *src);

/**
 * dst = s1 \cup s2
 */
void borSetUnion2(bor_set_t *dst, const bor_set_t *s1, const bor_set_t *s2);

/**
 * dst = dst \cap src
 */
void borSetIntersect(bor_set_t *dst, const bor_set_t *src);

/**
 * dst = s1 \cap s2
 */
void borSetIntersect2(bor_set_t *dst, const bor_set_t *s1, const bor_set_t *s2);

/**
 * s1 = s1 \setminus s2
 */
void borSetMinus(bor_set_t *s1, const bor_set_t *s2);

/**
 * d = s1 \setminus s2
 */
void borSetMinus2(bor_set_t *d, const bor_set_t *s1, const bor_set_t *s2);


/**
 * Returns true if the sets are equal.
 */
_bor_inline int borSetEq(const bor_set_t *s1, const bor_set_t *s2);

/**
 * Compares sets, return values are the same as by memcmp().
 */
_bor_inline int borSetCmp(const bor_set_t *s1, const bor_set_t *s2);

/**
 * Remaps the elements of the set using remap array containing maping from
 * the old value to the new value. The mapping must be monotonically
 * increasing and it is assumed that the values in the set are >= 0.
 */
void borSetRemap(bor_set_t *s, const TYPE *remap);



/**** INLINES: ****/
_bor_inline TYPE borSetGet(const bor_set_t *s, int i)
{
    return s->s[i];
}

_bor_inline int borSetSize(const bor_set_t *s)
{
    return s->size;
}

_bor_inline int borSetIn(TYPE val, const bor_set_t *s)
{
    return borSetHas(s, val);
}

_bor_inline int borSetIsDisjunct(const bor_set_t *s1, const bor_set_t *s2)
{
    return !borSetIntersectionSizeAtLeast(s1, s2, 1);
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

_bor_inline int borSetCmp(const bor_set_t *s1, const bor_set_t *s2)
{
    int cmp;
    cmp = memcmp(s1->s, s2->s,
                 sizeof(TYPE) * (s1->size < s2->size ?  s1->size : s2->size));
    if (cmp == 0)
        return s1->size - s2->size;
    return cmp;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_SET_H__ */
