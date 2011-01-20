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

#ifndef __FER_MAT_H__
#define __FER_MAT_H__

#include <fermat/core.h>
#include <fermat/gsl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * TODO
 */
struct _fer_mat_t {
    fer_gsl_matrix *m;
};
typedef struct _fer_mat_t fer_mat_t;

/**
 * Allocates and initializes new matrix.
 */
fer_mat_t *ferMatNew(size_t rows, size_t cols);

/**
 * Free matrix created by ferMatNew().
 */
void ferMatDel(fer_mat_t *m);

/**
 * Initializes matrix (staticaly declared).
 */
_fer_inline void ferMatInit(fer_mat_t *m, size_t rows, size_t cols);

/**
 * Destroys previously Init'ed matrix.
 */
_fer_inline void ferMatDestroy(fer_mat_t *m);

/**
 * Clones given matrix.
 */
_fer_inline fer_mat_t *ferMatClone(const fer_mat_t *m);

/**
 * Copies matrix from s to d.
 */
_fer_inline int ferMatCopy(fer_mat_t *d, const fer_mat_t *s);

/**
 * Returns number of rows.
 */
_fer_inline size_t ferMatRows(const fer_mat_t *m);

/**
 * Returns number of cols.
 */
_fer_inline size_t ferMatCols(const fer_mat_t *m);

/**
 * Returns element from specified position.
 */
_fer_inline fer_real_t ferMatGet(const fer_mat_t *m, size_t row, size_t col);

/**
 * Sets element at specified position.
 */
_fer_inline void ferMatSet(fer_mat_t *m, size_t row, size_t col,
                           fer_real_t val);

/**
 * Sets all elements to val.
 */
_fer_inline void ferMatSetAll(fer_mat_t *m, fer_real_t val);

/**
 * Set identity matrix.
 *     | 1 0 0 .. |
 * m = | 0 1 0 .. |
 *     | 0 0 1 .. |
 *     | . . . .. |
 */
_fer_inline void ferMatSetIdentity(fer_mat_t *m);

/**
 * Set zero matrix.
 */
_fer_inline void ferMatSetZero(fer_mat_t *m);

/**
 * Set diagonal.
 *     | s 0 0 .. |
 * m = | 0 s 0 .. |
 *     | 0 0 s .. |
 *     | . . . .. |
 */
_fer_inline void ferMatSetDiag(fer_mat_t *m, fer_real_t s);

/** TODO: operators must return 0/1 and check dimensions of matrices */

/**
 * a = a + b
 */
_fer_inline int ferMatAdd(fer_mat_t *a, const fer_mat_t *b);

/**
 * d = a + b
 */
_fer_inline int ferMatAdd2(fer_mat_t *d, const fer_mat_t *a,
                                         const fer_mat_t *b);

/**
 * a = a - b
 */
_fer_inline int ferMatSub(fer_mat_t *a, const fer_mat_t *b);

/**
 * d = a - b
 */
_fer_inline int ferMatSub2(fer_mat_t *d, const fer_mat_t *a,
                                         const fer_mat_t *b);

/**
 * d = d * s
 */
_fer_inline void ferMatScale(fer_mat_t *d, fer_real_t s);

/**
 * d = a * s
 */
_fer_inline int ferMatScale2(fer_mat_t *d, const fer_mat_t *a, fer_real_t s);

/**
 * d = d + c
 */
_fer_inline void ferMatAddConst(fer_mat_t *d, fer_real_t c);

/**
 * d = a + c
 */
_fer_inline int ferMatAddConst2(fer_mat_t *d, const fer_mat_t *a, fer_real_t c);

/**
 * d = d - c
 */
_fer_inline void ferMatSubConst(fer_mat_t *d, fer_real_t c);

/**
 * d = a - c
 */
_fer_inline int ferMatSubConst2(fer_mat_t *d, const fer_mat_t *a, fer_real_t c);


/**
 * a = a * b
 */
int ferMatMul(fer_mat_t *a, const fer_mat_t *b);

/**
 * d = a * b
 */
_fer_inline int ferMatMul2(fer_mat_t *d, const fer_mat_t *a,
                                         const fer_mat_t *b);

/**
 * a = b * a
 */
int ferMatMulLeft(fer_mat_t *a, const fer_mat_t *b);


/**
 * Multiplies two matrices by components.
 *     | a11*b11 a12*b12 a13*b13 .. |
 * a = | a21*b21 a22*b22 a23*b23 .. |
 *     | a31*b31 a32*b32 a33*b33 .. |
 *     | . . . . . . . . . . . . .. |
 */
_fer_inline int ferMatMulComp(fer_mat_t *a, const fer_mat_t *b);
_fer_inline int ferMatMulComp2(fer_mat_t *d, const fer_mat_t *a,
                                             const fer_mat_t *b);

/**
 * Transposes matrix. Note that this can change swaps dimensions.
 */
_fer_inline void ferMatTrans(fer_mat_t *d);

/**
 * Store transposition of matrix a in d. Matrix d must must have correct
 * dimensions.
 * d = a^t
 */
_fer_inline int ferMatTrans2(fer_mat_t *d, const fer_mat_t *a);

/**
 * Returns true if matrix is regular.
 */
_fer_inline int ferMatRegular(const fer_mat_t *m);

/**
 * Returns true if matrix is singular.
 */
_fer_inline int ferMatSingular(const fer_mat_t *m);

/**
 * Returns determinant of matrix.
 */
fer_real_t ferMatDet(const fer_mat_t *m);

/**
 * Inverts matrix.
 * Returns 0 on success, -1 if matrix is singular.
 */
int ferMatInv(fer_mat_t *m);

/**
 * Computes invertion matrix and stores it in m:
 *  m = inv(a)
 * Returns 0 on success, -1 if matrix is singular.
 */
int ferMatInv2(fer_mat_t *m, const fer_mat_t *a);

#if 0
/**
 * Multiplies vector by matrix (vectors are considered to be colunmal).
 * v = m * w
 */
_fer_inline void ferMatMulVec(fer_vec_t *v, const fer_mat_t *m,
                                            const fer_vec_t *w);
#endif

//TODO: get submatrix, get column, get row



/**** INLINES ****/
#define __FER_CHECKDIM(A, B) \
    if (ferMatRows(A) != ferMatRows(B) || ferMatCols(A) != ferMatCols(B)) \
        return -1


_fer_inline void ferMatInit(fer_mat_t *m, size_t rows, size_t cols)
{
    m->m = fer_gsl_matrix_alloc(rows, cols);
}

_fer_inline void ferMatDestroy(fer_mat_t *m)
{
    fer_gsl_matrix_free(m->m);
}

_fer_inline fer_mat_t *ferMatClone(const fer_mat_t *m)
{
    fer_mat_t *n;

    n = ferMatNew(ferMatRows(m), ferMatCols(m));
    ferMatCopy(n, m);
    return n;
}

_fer_inline int ferMatCopy(fer_mat_t *d, const fer_mat_t *s)
{
    __FER_CHECKDIM(d, s);
    fer_gsl_matrix_memcpy(d->m, s->m);
    return 0;
}

_fer_inline size_t ferMatRows(const fer_mat_t *m)
{
    return m->m->size1;
}

_fer_inline size_t ferMatCols(const fer_mat_t *m)
{
    return m->m->size2;
}

_fer_inline fer_real_t ferMatGet(const fer_mat_t *m, size_t row, size_t col)
{
    return fer_gsl_matrix_get(m->m, row, col);
}

_fer_inline void ferMatSet(fer_mat_t *m, size_t row, size_t col,
                           fer_real_t val)
{
    fer_gsl_matrix_set(m->m, row, col, val);
}

_fer_inline void ferMatSetAll(fer_mat_t *m, fer_real_t val)
{
    fer_gsl_matrix_set_all(m->m, val);
}

_fer_inline void ferMatSetIdentity(fer_mat_t *m)
{
    fer_gsl_matrix_set_identity(m->m);
}

_fer_inline void ferMatSetZero(fer_mat_t *m)
{
    fer_gsl_matrix_set_zero(m->m);
}

_fer_inline void ferMatSetDiag(fer_mat_t *m, fer_real_t s)
{
    ferMatSetIdentity(m);
    ferMatScale(m, s);
}

_fer_inline int ferMatAdd(fer_mat_t *a, const fer_mat_t *b)
{
    __FER_CHECKDIM(a, b);
    fer_gsl_matrix_add(a->m, b->m);
    return 0;
}

_fer_inline int ferMatAdd2(fer_mat_t *d, const fer_mat_t *a,
                                         const fer_mat_t *b)
{
    if (ferMatCopy(d, a) != 0)
        return -1;
    return ferMatAdd(d, b);
}

_fer_inline int ferMatSub(fer_mat_t *a, const fer_mat_t *b)
{
    __FER_CHECKDIM(a, b);
    fer_gsl_matrix_sub(a->m, b->m);
    return 0;
}

_fer_inline int ferMatSub2(fer_mat_t *d, const fer_mat_t *a,
                                         const fer_mat_t *b)
{
    if (ferMatCopy(d, a) != 0)
        return -1;
    return ferMatSub(d, b);
}

_fer_inline void ferMatScale(fer_mat_t *d, fer_real_t s)
{
    fer_gsl_matrix_scale(d->m, s);
}

_fer_inline int ferMatScale2(fer_mat_t *d, const fer_mat_t *a, fer_real_t s)
{
    if (ferMatCopy(d, a) != 0)
        return -1;
    ferMatScale(d, s);
    return 0;
}

_fer_inline void ferMatAddConst(fer_mat_t *d, fer_real_t c)
{
    fer_gsl_matrix_add_constant(d->m, c);
}

_fer_inline int ferMatAddConst2(fer_mat_t *d, const fer_mat_t *a, fer_real_t c)
{
    if (ferMatCopy(d, a) != 0)
        return -1;

    ferMatAddConst(d, c);
    return 0;
}

_fer_inline void ferMatSubConst(fer_mat_t *d, fer_real_t c)
{
    fer_gsl_matrix_add_constant(d->m, -c);
}

_fer_inline int ferMatSubConst2(fer_mat_t *d, const fer_mat_t *a, fer_real_t c)
{
    if (ferMatCopy(d, a) != 0)
        return -1;

    ferMatSubConst(d, c);
    return 0;
}

_fer_inline int ferMatMul2(fer_mat_t *d, const fer_mat_t *a,
                                         const fer_mat_t *b)
{
    size_t x, y, z;
    x = ferMatRows(a);
    y = ferMatCols(a);
    z = ferMatCols(b);

    if (ferMatRows(b) != y || ferMatRows(d) != x || ferMatCols(d) != z)
        return -1;

    fer_gsl_blas_gemm(CblasNoTrans, CblasNoTrans, FER_ONE, a->m, b->m, FER_ZERO, d->m);
    return 0;
}


_fer_inline int ferMatMulComp(fer_mat_t *a, const fer_mat_t *b)
{
    __FER_CHECKDIM(a, b);
    fer_gsl_matrix_mul_elements(a->m, b->m);
}

_fer_inline int ferMatMulComp2(fer_mat_t *d, const fer_mat_t *a,
                                             const fer_mat_t *b)
{
    if (ferMatCopy(d, a) != 0)
        return -1;
    return ferMatMulComp(d, b);
}

_fer_inline void ferMatTrans(fer_mat_t *d)
{
    fer_gsl_matrix_transpose(d->m);
}

_fer_inline int ferMatTrans2(fer_mat_t *d, const fer_mat_t *a)
{
    if (ferMatRows(d) != ferMatCols(d) || ferMatCols(d) != ferMatRows(a))
        return -1;

    fer_gsl_matrix_transpose_memcpy(d->m, a->m);
    return 0;
}

_fer_inline int ferMatRegular(const fer_mat_t *m)
{
    fer_real_t det;
    det = ferMatDet(m);
    return !ferIsZero(det);
}

_fer_inline int ferMatSingular(const fer_mat_t *m)
{
    return !ferMatRegular(m);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_MAT_H__ */

