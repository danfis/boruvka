ifdef(`PRELUDE', `PRELUDE')
ifdef(`ARR_NAME',, `define(`ARR_NAME', `v')')dnl
`#ifndef 'GUARD
`#define 'GUARD

#include <boruvka/compiler.h>
#include <boruvka/alloc.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * Array-based sets.
 */

struct STRUCT_NAME {
    TYPE *ARR_NAME;
    int size;
    int alloc;
};
typedef struct STRUCT_NAME STRUCT_NAME`_t';

ifdef(`FOR_EACH_NAME', dnl
`#define 'FOR_EACH_NAME`(S, V) \'
`    for (int __i = 0; __i < (S)->size && ((V) = (S)->ARR_NAME[__i], 1); ++__i)')

/**
 * Initialize the set.
 */
_bor_inline void FUNC_PREFIX`Init'(STRUCT_NAME`_t' *s);

/**
 * Frees allocated memory.
 */
_bor_inline void FUNC_PREFIX`Free'(STRUCT_NAME`_t' *s);

/**
 * Returns true if the set contains val.
 */
int FUNC_PREFIX`Has'(const STRUCT_NAME`_t' *s, int val);

/**
 * Makes the set empty.
 */
_bor_inline void FUNC_PREFIX`Empty'(STRUCT_NAME`_t' *s);

/**
 * Adds a new value into the set if not already there.
 */
void FUNC_PREFIX`Add'(STRUCT_NAME`_t' *s, TYPE val);

/**
 * dst = dst \cup src
 */
void FUNC_PREFIX`Union'(STRUCT_NAME`_t' *dst, const STRUCT_NAME`_t' *src);

/**
 * Allocate enough memory for size elements.
 * (It does not change s->size.)
 */
_bor_inline void FUNC_PREFIX`Resize'(STRUCT_NAME`_t' *s, int size);

/**
 * Returns true if the sets are equal.
 */
_bor_inline int FUNC_PREFIX`Eq'(const STRUCT_NAME`_t' *s1, const STRUCT_NAME`_t' *s2);

/**
 * s1 = s1 \setminus s2
 */
void FUNC_PREFIX`Minus'(STRUCT_NAME`_t' *s1, const STRUCT_NAME`_t' *s2);

/**
 * s = s \setminus {val}
 */
int FUNC_PREFIX`Rm'(STRUCT_NAME`_t' *s, TYPE val);



/**** INLINES: ****/
_bor_inline void FUNC_PREFIX`Init'(STRUCT_NAME`_t' *s)
{
    bzero(s, sizeof(*s));
}

_bor_inline void FUNC_PREFIX`Free'(STRUCT_NAME`_t' *s)
{
    if (s->ARR_NAME)
        BOR_FREE(s->ARR_NAME);
}

_bor_inline void FUNC_PREFIX`Empty'(STRUCT_NAME`_t' *s)
{
    s->size = 0;
}

_bor_inline void FUNC_PREFIX`Resize'(STRUCT_NAME`_t' *s, int size)
{
    s->ARR_NAME = BOR_REALLOC_ARR(s->ARR_NAME, TYPE, size);
    s->size = size;
}

_bor_inline int FUNC_PREFIX`Eq'(const STRUCT_NAME`_t' *s1, const STRUCT_NAME`_t' *s2)
{
    return s1->size == s2->size
            && memcmp(s1->ARR_NAME, s2->ARR_NAME, sizeof(TYPE) * s1->size) == 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

`#endif '/* GUARD */
