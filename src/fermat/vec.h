/***
 * fermat
 * -------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __FER_VEC_H__
#define __FER_VEC_H__

#include <fermat/core.h>
#include <fermat/gsl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Vec - wrapper around GSL vectors
 * =================================
 *
 * This is basically wrapper around GSL. Feel free to use .v member of
 * struct in any GSL function directly.
 */
struct _fer_vec_t {
    fer_gsl_vector *v;
};
typedef struct _fer_vec_t fer_vec_t;


/**
 * Allocate and initialize new vector.
 */
fer_vec_t *ferVecNew(size_t size);

/**
 * Delete vector.
 */
void ferVecDel(fer_vec_t *);

/**
 * Initialize staticaly defined vector.
 */
void ferVecInit(fer_vec_t *v, size_t size);

/**
 * Destroys previously initialized vector.
 */
void ferVecDestroy(fer_vec_t *v);

/**
 * Returns size of vector.
 */
_fer_inline size_t ferVecSize(const fer_vec_t *v);

/**
 * Clone given fer_vec_t. This does deep copy.
 */
_fer_inline fer_vec_t *ferVecClone(const fer_vec_t *v);

/**
 * v = w
 */
_fer_inline int ferVecCopy(fer_vec_t *v, const fer_vec_t *w);


_fer_inline fer_real_t ferVecGet(const fer_vec_t *v, size_t d);

_fer_inline void ferVecSet(fer_vec_t *v, size_t d, fer_real_t val);
_fer_inline void ferVecSetAll(fer_vec_t *v, fer_real_t val);
_fer_inline void ferVecSetZero(fer_vec_t *v);


/**
 * Returns squared length of vector.
 */
_fer_inline fer_real_t ferVecLen2(const fer_vec_t *v);
_fer_inline fer_real_t ferVecLen(const fer_vec_t *v);

/**
 * Returns squared distance between a and b.
 */
_fer_inline fer_real_t ferVecDist2(const fer_vec_t *a, const fer_vec_t *b);
_fer_inline fer_real_t ferVecDist(const fer_vec_t *a, const fer_vec_t *b);


/**
 * Adds coordinates of vector w to vector v. v = v + w
 */
_fer_inline int ferVecAdd(fer_vec_t *v, const fer_vec_t *w);

/**
 * d = v + w
 */
_fer_inline int ferVecAdd2(fer_vec_t *d, const fer_vec_t *v, const fer_vec_t *w);


/**
 * Substracts coordinates of vector w from vector v. v = v - w
 */
_fer_inline int ferVecSub(fer_vec_t *v, const fer_vec_t *w);

/**
 * d = v - w
 */
_fer_inline int ferVecSub2(fer_vec_t *d, const fer_vec_t *v, const fer_vec_t *w);

_fer_inline void ferVecAddConst(fer_vec_t *v, fer_real_t f);
_fer_inline int ferVecAddConst2(fer_vec_t *d, const fer_vec_t *v, fer_real_t f);
_fer_inline void ferVecSubConst(fer_vec_t *v, fer_real_t f);
_fer_inline int ferVecSubConst2(fer_vec_t *d, const fer_vec_t *v, fer_real_t f);


/**
 * d = d * k;
 */
_fer_inline void ferVecScale(fer_vec_t *d, fer_real_t k);

/**
 * Dot product of two vectors.
 */
_fer_inline fer_real_t ferVecDot(const fer_vec_t *a, const fer_vec_t *b);

/**
 * Multiplies vectors component wise:
 *  a.x = a.x * b.x
 *  a.y = a.y * b.y
 *  a.z = a.z * b.z
 */
_fer_inline int ferVecMulComp(fer_vec_t *a, const fer_vec_t *b);

/**
 * a.x = b.x * c.x
 * a.y = b.y * c.y
 * a.z = b.z * c.z
 */
_fer_inline int ferVecMulComp2(fer_vec_t *a, const fer_vec_t *b, const fer_vec_t *c);




/**** INLINES ****/
#define __FER_VEC_CHECKSIZE(a, b) \
    if (ferVecSize(a) != ferVecSize(b)) \
        return -1

_fer_inline size_t ferVecSize(const fer_vec_t *v)
{
    return v->v->size;
}

_fer_inline fer_vec_t *ferVecClone(const fer_vec_t *v)
{
    fer_vec_t *w;
    w = ferVecNew(ferVecSize(v));
    ferVecCopy(w, v);
    return w;
}

_fer_inline fer_real_t ferVecGet(const fer_vec_t *v, size_t d)
{
    return fer_gsl_vector_get(v->v, d);
}


_fer_inline fer_real_t ferVecLen2(const fer_vec_t *v)
{
    return ferVecDot(v, v);
}
_fer_inline fer_real_t ferVecLen(const fer_vec_t *v)
{
    return FER_SQRT(ferVecLen2(v));
}

_fer_inline fer_real_t ferVecDist2(const fer_vec_t *a, const fer_vec_t *b)
{
    fer_vec_t ab;
    fer_real_t dist2;

    ferVecInit(&ab, ferVecSize(a));
    ferVecSub2(&ab, a, b);
    dist2 = ferVecLen2(&ab);
    ferVecDestroy(&ab);
    return dist2;
}
_fer_inline fer_real_t ferVecDist(const fer_vec_t *a, const fer_vec_t *b)
{
    return FER_SQRT(ferVecDist2(a, b));
}

_fer_inline void ferVecSet(fer_vec_t *v, size_t d, fer_real_t val)
{
    fer_gsl_vector_set(v->v, d, val);
}

_fer_inline void ferVecSetAll(fer_vec_t *v, fer_real_t val)
{
    fer_gsl_vector_set_all(v->v, val);
}

_fer_inline void ferVecSetZero(fer_vec_t *v)
{
    fer_gsl_vector_set_zero(v->v);
}


_fer_inline int ferVecCopy(fer_vec_t *v, const fer_vec_t *w)
{
    __FER_VEC_CHECKSIZE(v, w);
    fer_gsl_vector_memcpy(v->v, w->v);
    return 0;
}

_fer_inline int ferVecAdd(fer_vec_t *v, const fer_vec_t *w)
{
    __FER_VEC_CHECKSIZE(v, w);
    fer_gsl_vector_add(v->v, w->v);
    return 0;
}

_fer_inline int ferVecAdd2(fer_vec_t *d, const fer_vec_t *v, const fer_vec_t *w)
{
    if (ferVecCopy(d, v) != 0)
        return -1;
    return ferVecAdd(d, w);
}

_fer_inline int ferVecSub(fer_vec_t *v, const fer_vec_t *w)
{
    __FER_VEC_CHECKSIZE(v, w);
    fer_gsl_vector_sub(v->v, w->v);
    return 0;
}
_fer_inline int ferVecSub2(fer_vec_t *d, const fer_vec_t *v, const fer_vec_t *w)
{
    if (ferVecCopy(d, v) != 0)
        return -1;
    return ferVecSub(d, w);
}

_fer_inline void ferVecAddConst(fer_vec_t *v, fer_real_t f)
{
    fer_gsl_vector_add_constant(v->v, f);
}

_fer_inline int ferVecAddConst2(fer_vec_t *d, const fer_vec_t *v, fer_real_t f)
{
    ferVecCopy(d, v);
    ferVecAddConst(d, f);
    return 0;
}

_fer_inline void ferVecSubConst(fer_vec_t *v, fer_real_t f)
{
    ferVecAddConst(v, -f);
}

_fer_inline int ferVecSubConst2(fer_vec_t *d, const fer_vec_t *v, fer_real_t f)
{
    return ferVecAddConst2(d, v, -f);
}

_fer_inline void ferVecScale(fer_vec_t *d, fer_real_t k)
{
    fer_gsl_vector_scale(d->v, k);
}


_fer_inline fer_real_t ferVecDot(const fer_vec_t *a, const fer_vec_t *b)
{
    fer_real_t res;
    fer_gsl_blas_dot(a->v, b->v, &res);
    return res;
}

_fer_inline int ferVecMulComp(fer_vec_t *a, const fer_vec_t *b)
{
    __FER_VEC_CHECKSIZE(a, b);
    fer_gsl_vector_mul(a->v, b->v);
    return 0;
}

_fer_inline int ferVecMulComp2(fer_vec_t *a, const fer_vec_t *b, const fer_vec_t *c)
{
    if (ferVecCopy(a, b) != 0)
        return -1;
    return ferVecMulComp(a, c);
}


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_VEC_H__ */
