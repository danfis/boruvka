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

#ifndef __BOR_VEC_H__
#define __BOR_VEC_H__

#include <boruvka/core.h>
#include <boruvka/vec2.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Vec - N-D Vectors
 * =====================
 *
 */

/** vvvv */
typedef bor_real_t bor_vec_t;
/** ^^^^ */

/**
 * Static declaration of vector.
 */
#define BOR_VEC(name, size) \
    bor_vec_t name[size]

/**
 * Static declaration of array of vectors.
 */
#define BOR_VEC_ARR(name, size, arrsize) \
    bor_vec_t name[arrsize][size]


/** TODO: ToVec2/3/4 functions */

/**
 * Cast from Vec2 to Vec.
 */
_bor_inline bor_vec_t *borVecFromVec2(bor_vec2_t *v);

/**
 * Cast from Vec2 to Vec.
 */
_bor_inline const bor_vec_t *borVecFromVec2Const(const bor_vec2_t *v);


/**
 * Functions
 * ----------
 */

/**
 * Allocate and initialize new vector.
 */
bor_vec_t *borVecNew(size_t size);

/**
 * Delete vector.
 */
void borVecDel(bor_vec_t *);

/**
 * Clone given bor_vec_t that must be size length. This does deep copy.
 */
_bor_inline bor_vec_t *borVecClone(size_t size, const bor_vec_t *v);

/**
 * Returns vector with given offset
 */
_bor_inline bor_vec_t *borVecOff(bor_vec_t *v, size_t off);

/**
 * v = w
 */
_bor_inline void borVecCopy(size_t size, bor_vec_t *v, const bor_vec_t *w);

/**
 * Returns d'th coordinate.
 */
_bor_inline bor_real_t borVecGet(const bor_vec_t *v, size_t d);

/**
 * Set d'th coordinate to given value.
 */
_bor_inline void borVecSet(bor_vec_t *v, size_t d, bor_real_t val);

/**
 * Set {size} coordinates.
 * Note that the values should be double.
 */
void borVecSetN(size_t size, bor_vec_t *v, ...);

/**
 * Set all coordinates to given value.
 */
_bor_inline void borVecSetAll(size_t size, bor_vec_t *v, bor_real_t val);

/**
 * Zeroize whole vector.
 */
_bor_inline void borVecSetZero(size_t size, bor_vec_t *v);

/**
 * Returns true if two vectors equal
 */
_bor_inline int borVecEq(size_t size, const bor_vec_t *v, const bor_vec_t *w);

/**
 * Returns squared length of vector.
 */
_bor_inline bor_real_t borVecLen2(size_t size, const bor_vec_t *v);

/**
 * Returns length of vector
 */
_bor_inline bor_real_t borVecLen(size_t size, const bor_vec_t *v);

/**
 * Returns squared distance between a and b.
 */
_bor_inline bor_real_t borVecDist2(size_t size, const bor_vec_t *a, const bor_vec_t *b);

/**
 * Returns distance between a and b.
 */
_bor_inline bor_real_t borVecDist(size_t size, const bor_vec_t *a, const bor_vec_t *b);


/**
 * Adds coordinates of vector w to vector v.
 * v = v + w
 */
_bor_inline void borVecAdd(size_t size, bor_vec_t *v, const bor_vec_t *w);

/**
 * d = v + w
 */
_bor_inline void borVecAdd2(size_t size, bor_vec_t *d, const bor_vec_t *v, const bor_vec_t *w);


/**
 * Substracts coordinates of vector w from vector v. v = v - w
 */
_bor_inline void borVecSub(size_t size, bor_vec_t *v, const bor_vec_t *w);

/**
 * d = v - w
 */
_bor_inline void borVecSub2(size_t size, bor_vec_t *d, const bor_vec_t *v, const bor_vec_t *w);

_bor_inline void borVecAddConst(size_t size, bor_vec_t *v, bor_real_t f);
_bor_inline void borVecAddConst2(size_t size, bor_vec_t *d, const bor_vec_t *v, bor_real_t f);
_bor_inline void borVecSubConst(size_t size, bor_vec_t *v, bor_real_t f);
_bor_inline void borVecSubConst2(size_t size, bor_vec_t *d, const bor_vec_t *v, bor_real_t f);


/**
 * d = d * k;
 */
_bor_inline void borVecScale(size_t size, bor_vec_t *d, bor_real_t k);

/**
 * d = a * k;
 */
_bor_inline void borVecScale2(size_t size, bor_vec_t *d, const bor_vec_t *a, bor_real_t k);

/**
 * Dot product of two vectors.
 */
_bor_inline bor_real_t borVecDot(size_t size, const bor_vec_t *a, const bor_vec_t *b);

/**
 * Multiplies vectors component wise:
 * a.x = a.x * b.x
 * a.y = a.y * b.y
 * a.z = a.z * b.z
 */
_bor_inline void borVecMulComp(size_t size, bor_vec_t *a, const bor_vec_t *b);

/**
 * a.x = b.x * c.x
 * a.y = b.y * c.y
 * a.z = b.z * c.z
 */
_bor_inline void borVecMulComp2(size_t size, bor_vec_t *a, const bor_vec_t *b, const bor_vec_t *c);

/**
 * Prints vector to *out* in form "x y".
 */
_bor_inline void borVecPrint(size_t size, const bor_vec_t *v, FILE *out);


/**
 * Returns sum of components
 */
_bor_inline bor_real_t borVecSum(size_t size, const bor_vec_t *v);

/**** INLINES ****/
_bor_inline bor_vec_t *borVecFromVec2(bor_vec2_t *v)
{
    return (bor_vec_t *)v;
}

_bor_inline const bor_vec_t *borVecFromVec2Const(const bor_vec2_t *v)
{
    return (const bor_vec_t *)v;
}


_bor_inline bor_vec_t *borVecClone(size_t size, const bor_vec_t *v)
{
    bor_vec_t *w;
    w = borVecNew(size);
    borVecCopy(size, w, v);
    return w;
}

_bor_inline bor_vec_t *borVecOff(bor_vec_t *v, size_t off)
{
    return v + off;
}

_bor_inline bor_real_t borVecGet(const bor_vec_t *v, size_t d)
{
    return v[d];
}


_bor_inline bor_real_t borVecLen2(size_t size, const bor_vec_t *v)
{
    return borVecDot(size, v, v);
}
_bor_inline bor_real_t borVecLen(size_t size, const bor_vec_t *v)
{
    return BOR_SQRT(borVecLen2(size, v));
}

_bor_inline bor_real_t borVecDist2(size_t size, const bor_vec_t *a, const bor_vec_t *b)
{
    bor_real_t f, dot;
    size_t i;

    dot = BOR_ZERO;
    for (i = 0; i < size; i++){
        f = a[i] - b[i];
        dot += BOR_SQ(f);
    }

    return dot;
}
_bor_inline bor_real_t borVecDist(size_t size, const bor_vec_t *a, const bor_vec_t *b)
{
    return BOR_SQRT(borVecDist2(size, a, b));
}

_bor_inline void borVecSet(bor_vec_t *v, size_t d, bor_real_t val)
{
    v[d] = val;
}

_bor_inline void borVecSetAll(size_t size, bor_vec_t *v, bor_real_t val)
{
    size_t i;
    for (i = 0; i < size; i++){
        v[i] = val;
    }
}

_bor_inline void borVecSetZero(size_t size, bor_vec_t *v)
{
    borVecSetAll(size, v, BOR_ZERO);
}

_bor_inline int borVecEq(size_t size, const bor_vec_t *v, const bor_vec_t *w)
{
    size_t i;

    for (i = 0; i < size; i++){
        if (borNEq(borVecGet(v, i), borVecGet(w, i)))
            return 0;
    }

    return 1;
}


_bor_inline void borVecCopy(size_t size, bor_vec_t *v, const bor_vec_t *w)
{
    size_t i;
    for (i = 0; i < size; i++){
        v[i] = w[i];
    }
}

_bor_inline void borVecAdd(size_t size, bor_vec_t *v, const bor_vec_t *w)
{
    size_t i;
    for (i = 0; i < size; i++){
        v[i] += w[i];
    }
}

_bor_inline void borVecAdd2(size_t size, bor_vec_t *d, const bor_vec_t *v, const bor_vec_t *w)
{
    size_t i;
    for (i = 0; i < size; i++){
        d[i] = v[i] + w[i];
    }
}

_bor_inline void borVecSub(size_t size, bor_vec_t *v, const bor_vec_t *w)
{
    size_t i;
    for (i = 0; i < size; i++){
        v[i] -= w[i];
    }
}
_bor_inline void borVecSub2(size_t size, bor_vec_t *d, const bor_vec_t *v, const bor_vec_t *w)
{
    size_t i;
    for (i = 0; i < size; i++){
        d[i] = v[i] - w[i];
    }
}

_bor_inline void borVecAddConst(size_t size, bor_vec_t *v, bor_real_t f)
{
    size_t i;
    for (i = 0; i < size; i++){
        v[i] += f;
    }
}

_bor_inline void borVecAddConst2(size_t size, bor_vec_t *d, const bor_vec_t *v, bor_real_t f)
{
    size_t i;
    for (i = 0; i < size; i++){
        d[i] = v[i] + f;
    }
}

_bor_inline void borVecSubConst(size_t size, bor_vec_t *v, bor_real_t f)
{
    borVecAddConst(size, v, -f);
}

_bor_inline void borVecSubConst2(size_t size, bor_vec_t *d, const bor_vec_t *v, bor_real_t f)
{
    borVecAddConst2(size, d, v, -f);
}

_bor_inline void borVecScale(size_t size, bor_vec_t *d, bor_real_t k)
{
    size_t i;
    for (i = 0; i < size; i++){
        d[i] *= k;
    }
}

_bor_inline void borVecScale2(size_t size, bor_vec_t *d, const bor_vec_t *a, bor_real_t k)
{
    size_t i;
    for (i = 0; i < size; i++){
        d[i] = a[i] * k;
    }
}


_bor_inline bor_real_t borVecDot(size_t size, const bor_vec_t *a, const bor_vec_t *b)
{
    bor_real_t dot;
    size_t i;

    dot = BOR_ZERO;
    for (i = 0; i < size; i++){
        dot += a[i] * b[i];
    }

    return dot;
}

_bor_inline void borVecMulComp(size_t size, bor_vec_t *a, const bor_vec_t *b)
{
    size_t i;
    for (i = 0; i < size; i++){
        a[i] *= b[i];
    }
}

_bor_inline void borVecMulComp2(size_t size, bor_vec_t *a, const bor_vec_t *b, const bor_vec_t *c)
{
    size_t i;
    for (i = 0; i < size; i++){
        a[i] = b[i] * c[i];
    }
}

_bor_inline void borVecPrint(size_t size, const bor_vec_t *v, FILE *out)
{
    size_t i;
    fprintf(out, "%g", (double)borVecGet(v, 0));
    for (i = 1; i < size; i++){
        fprintf(out, " %g", (double)borVecGet(v, i));
    }
}

_bor_inline bor_real_t borVecSum(size_t size, const bor_vec_t *v)
{
    size_t i;
    bor_real_t sum = borVecGet(v, 0);

    for (i = 1; i < size; i++){
        sum += borVecGet(v, i);
    }

    return sum;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_VEC_H__ */
