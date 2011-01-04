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

#ifndef __FER_MAT3_H__
#define __FER_MAT3_H__

#include <fermat/core.h>
#include <fermat/vec2.h>
#include <fermat/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * 3 x 3 matrix
 */
struct _fer_mat3_t {
    fer_real_t f[3 * 4];
};
typedef struct _fer_mat3_t fer_mat3_t;

/**
 * Holds identity matrix. Read only variable!
 */
extern const fer_mat3_t *fer_mat3_identity;

/**
 * Holds zero matrix. Read only variable!
 */
extern const fer_mat3_t *fer_mat3_zero;

#define FER_MAT3_STATIC(f11, f12, f13, \
                        f21, f22, f23, \
                        f31, f32, f33) \
    { .f = { (f11), (f12), (f13), FER_ZERO, \
             (f21), (f22), (f23), FER_ZERO, \
             (f31), (f32), (f33), FER_ZERO } }

#define FER_MAT3(name, f11, f12, f13, \
                       f21, f22, f23, \
                       f31, f32, f33) \
    fer_mat3_t name = FER_MAT3_STATIC(f11, f12, f13, \
                                      f21, f22, f23, \
                                      f31, f32, f33)

/**
 * Allocates uninitialized new matrix.
 */
fer_mat3_t *ferMat3New(void);

/**
 * Deletes matrix.
 */
void ferMat3Del(fer_mat3_t *m);

/**
 * Clones given matrix.
 */
_fer_inline fer_mat3_t *ferMat3Clone(const fer_mat3_t *m);

/**
 * Copies matrix from s to d.
 */
_fer_inline void ferMat3Copy(fer_mat3_t *d, const fer_mat3_t *s);


/**
 * Returns element from specified position.
 */
_fer_inline fer_real_t ferMat3Get(const fer_mat3_t *m, size_t row, size_t col);

/**
 * Sets element at specified position.
 */
_fer_inline void ferMat3Set1(fer_mat3_t *m, size_t row, size_t col,
                             fer_real_t val);

/**
 * Sets all elements to val.
 */
_fer_inline void ferMat3SetAll(fer_mat3_t *m, fer_real_t val);

/**
 * Sets whole matrix.
 */
_fer_inline void ferMat3Set(fer_mat3_t *m,
                            fer_real_t f11, fer_real_t f12, fer_real_t f13,
                            fer_real_t f21, fer_real_t f22, fer_real_t f23,
                            fer_real_t f31, fer_real_t f32, fer_real_t f33);

/**
 * Set identity matrix.
 *     | 1 0 0 |
 * m = | 0 1 0 |
 *     | 0 0 1 |
 */
_fer_inline void ferMat3SetIdentity(fer_mat3_t *m);

/**
 * Set zero matrix.
 */
_fer_inline void ferMat3SetZero(fer_mat3_t *m);

/**
 * Set scale matrix.
 *     | s 0 0 |
 * m = | 0 s 0 |
 *     | 0 0 s |
 */
_fer_inline void ferMat3SetScale(fer_mat3_t *m, fer_real_t s);
#define ferMat3SetDiag ferMat3SetScale

/**
 * Set translation matrix (translation along v vector).
 *     | 1 0 x |
 * m = | 0 1 y |
 *     | 0 0 1 |
 */
_fer_inline void ferMat3SetTranslate(fer_mat3_t *m, const fer_vec2_t *v);

/**
 * Set rotation matrix (2D).
 *     | cos(a) -sin(a) 0 |
 * m = | sin(a)  cos(a) 0 |
 *     |   0       0    1 |
 */
_fer_inline void ferMat3SetRot(fer_mat3_t *m, fer_real_t angle);

/**
 * Composes transformation:
 * A = B . A
 */
_fer_inline void ferMat3Compose(fer_mat3_t *A, const fer_mat3_t *B);


/**
 * a = a + b
 */
_fer_inline void ferMat3Add(fer_mat3_t *a, const fer_mat3_t *b);

/**
 * d = a + b
 */
_fer_inline void ferMat3Add2(fer_mat3_t *d, const fer_mat3_t *a,
                                            const fer_mat3_t *b);

/**
 * a = a - b
 */
_fer_inline void ferMat3Sub(fer_mat3_t *a, const fer_mat3_t *b);

/**
 * d = a - b
 */
_fer_inline void ferMat3Sub2(fer_mat3_t *d, const fer_mat3_t *a,
                                            const fer_mat3_t *b);

/**
 * d = d * s
 */
_fer_inline void ferMat3Scale(fer_mat3_t *d, fer_real_t s);

/**
 * d = a * s
 */
_fer_inline void ferMat3Scale2(fer_mat3_t *d, const fer_mat3_t *a, fer_real_t s);

/**
 * d = d + c
 */
_fer_inline void ferMat3AddConst(fer_mat3_t *d, fer_real_t c);

/**
 * d = a + c
 */
_fer_inline void ferMat3AddConst2(fer_mat3_t *d, const fer_mat3_t *a, fer_real_t c);

/**
 * a = a * b
 */
_fer_inline void ferMat3Mul(fer_mat3_t *a, const fer_mat3_t *b);

/**
 * d = a * b
 */
_fer_inline void ferMat3Mul2(fer_mat3_t *d, const fer_mat3_t *a,
                                            const fer_mat3_t *b);

/**
 * a = b * a
 */
_fer_inline void ferMat3MulLeft(fer_mat3_t *a, const fer_mat3_t *b);


/**
 * Multiplies two matrices by components.
 *     | a11*b11 a12*b12 a13*b13 |
 * a = | a21*b21 a22*b22 a23*b23 |
 *     | a31*b31 a32*b32 a33*313 |
 */
_fer_inline void ferMat3MulComp(fer_mat3_t *a, const fer_mat3_t *b);
_fer_inline void ferMat3MulComp2(fer_mat3_t *d, const fer_mat3_t *a,
                                                const fer_mat3_t *b);

/**
 * Transposes matrix.
 */
_fer_inline void ferMat3Trans(fer_mat3_t *d);
_fer_inline void ferMat3Trans2(fer_mat3_t *d, const fer_mat3_t *a);

/**
 * Returns true if matrix is regular.
 */
_fer_inline int ferMat3Regular(const fer_mat3_t *m);

/**
 * Returns true if matrix is singular.
 */
_fer_inline int ferMat3Singular(const fer_mat3_t *m);

/**
 * Returns determinant of matrix.
 */
_fer_inline fer_real_t ferMat3Det(const fer_mat3_t *m);

/**
 * Inverts matrix.
 * Returns 0 on success, -1 if matrix is singular.
 */
_fer_inline int ferMat3Inv(fer_mat3_t *m);

/**
 * Computes invertion matrix and stores it in m:
 *  m = inv(a)
 * Returns 0 on success, -1 if matrix is singular.
 */
_fer_inline int ferMat3Inv2(fer_mat3_t *m, const fer_mat3_t *a);


/**
 * Multiplies 3D vector by matrix (vectors are considered to be colunmal).
 * v = m * w
 */
_fer_inline void ferMat3MulVec(fer_vec3_t *v, const fer_mat3_t *m,
                                              const fer_vec3_t *w);

/**
 * Multiplies matrix with 2D vector.
 * Third coordinate is filled with 1 and after multiplication x and y
 * coordinate is divided by resulting third coordinate.
 * In fact, this operation is applying transform matrix on vector w.
 *
 *         | w.x |
 * v = m * | w.y |
 *         |  1  |
 * v.x = v.x / w.z
 * v.y = v.y / w.z
 */
_fer_inline void ferMat3MulVec2(fer_vec2_t *v, const fer_mat3_t *m,
                                               const fer_vec2_t *w);


/**
 * Computes dot product of r'th row of matrix m and c'th column of matrix n.
 */
_fer_inline fer_real_t ferMat3DotRowCol(const fer_mat3_t *m, size_t r,
                                        const fer_mat3_t *n, size_t c);

/**
 * Returns dot product of col'th column of matrix m with vector (a, b, c).
 */
_fer_inline fer_real_t ferMat3DotCol(const fer_mat3_t *m, size_t col,
                                     fer_real_t a, fer_real_t b, fer_real_t c);

/**
 * Returns dot product of row'th row of matrix m with vector (a, b, c).
 */
_fer_inline fer_real_t ferMat3DotRow(const fer_mat3_t *m, size_t row,
                                     fer_real_t a, fer_real_t b, fer_real_t c);

/**** INLINES ****/
_fer_inline fer_mat3_t *ferMat3Clone(const fer_mat3_t *m)
{
    fer_mat3_t *n;

    n = ferMat3New();
    ferMat3Copy(n, m);
    return n;
}

_fer_inline void ferMat3Copy(fer_mat3_t *d, const fer_mat3_t *s)
{
    *d = *s;
}


_fer_inline fer_real_t ferMat3Get(const fer_mat3_t *m, size_t row, size_t col)
{
    return m->f[row * 4 + col];
}

_fer_inline void ferMat3Set1(fer_mat3_t *m, size_t row, size_t col,
                             fer_real_t val)
{
    m->f[row * 4 + col] = val;
}

_fer_inline void ferMat3SetAll(fer_mat3_t *m, fer_real_t val)
{
    size_t i, j;
    for (i = 0; i < 3; i++){
        for (j = 0; j < 3; j++){
            ferMat3Set1(m, i, j, val);
        }
    }
}

_fer_inline void ferMat3Set(fer_mat3_t *m,
                            fer_real_t f11, fer_real_t f12, fer_real_t f13,
                            fer_real_t f21, fer_real_t f22, fer_real_t f23,
                            fer_real_t f31, fer_real_t f32, fer_real_t f33)
{
    m->f[0] = f11;
    m->f[1] = f12;
    m->f[2] = f13;
    m->f[4] = f21;
    m->f[5] = f22;
    m->f[6] = f23;
    m->f[8] = f31;
    m->f[9] = f32;
    m->f[10] = f33;
}

_fer_inline void ferMat3SetIdentity(fer_mat3_t *m)
{
    ferMat3Set(m, FER_ONE, FER_ZERO, FER_ZERO,
                  FER_ZERO, FER_ONE, FER_ZERO,
                  FER_ZERO, FER_ZERO, FER_ONE);
}

_fer_inline void ferMat3SetZero(fer_mat3_t *m)
{
    ferMat3SetAll(m, FER_ZERO);
}

_fer_inline void ferMat3SetScale(fer_mat3_t *m, fer_real_t s)
{
    ferMat3Set(m, s, FER_ZERO, FER_ZERO,
                  FER_ZERO, s, FER_ZERO,
                  FER_ZERO, FER_ZERO, s);
}

_fer_inline void ferMat3SetTranslate(fer_mat3_t *m, const fer_vec2_t *v)
{
    ferMat3Set(m, FER_ONE, FER_ZERO, ferVec2X(v),
                  FER_ZERO, FER_ONE, ferVec2Y(v),
                  FER_ZERO, FER_ZERO, FER_ONE);
}

_fer_inline void ferMat3SetRot(fer_mat3_t *m, fer_real_t angle)
{
    ferMat3Set(m, FER_COS(angle), -FER_SIN(angle), FER_ZERO,
                  FER_SIN(angle),  FER_COS(angle), FER_ZERO,
                  FER_ZERO, FER_ZERO, FER_ONE);
}

/**
 * Composes transformation:
 * A = B . A
 */
_fer_inline void ferMat3Compose(fer_mat3_t *A, const fer_mat3_t *B)
{
    ferMat3MulLeft(A, B);
}


_fer_inline void ferMat3Add(fer_mat3_t *a, const fer_mat3_t *b)
{
    a->f[0] += b->f[0];
    a->f[1] += b->f[1];
    a->f[2] += b->f[2];
    a->f[4] += b->f[4];
    a->f[5] += b->f[5];
    a->f[6] += b->f[6];
    a->f[8] += b->f[8];
    a->f[9] += b->f[9];
    a->f[10] += b->f[10];
}

_fer_inline void ferMat3Add2(fer_mat3_t *d, const fer_mat3_t *a,
                                            const fer_mat3_t *b)
{
    d->f[0] = a->f[0] + b->f[0];
    d->f[1] = a->f[1] + b->f[1];
    d->f[2] = a->f[2] + b->f[2];
    d->f[4] = a->f[4] + b->f[4];
    d->f[5] = a->f[5] + b->f[5];
    d->f[6] = a->f[6] + b->f[6];
    d->f[8] = a->f[8] + b->f[8];
    d->f[9] = a->f[9] + b->f[9];
    d->f[10] = a->f[10] + b->f[10];
}

_fer_inline void ferMat3Sub(fer_mat3_t *a, const fer_mat3_t *b)
{
    a->f[0] -= b->f[0];
    a->f[1] -= b->f[1];
    a->f[2] -= b->f[2];
    a->f[4] -= b->f[4];
    a->f[5] -= b->f[5];
    a->f[6] -= b->f[6];
    a->f[8] -= b->f[8];
    a->f[9] -= b->f[9];
    a->f[10] -= b->f[10];
}

_fer_inline void ferMat3Sub2(fer_mat3_t *d, const fer_mat3_t *a,
                                            const fer_mat3_t *b)
{
    d->f[0] = a->f[0] - b->f[0];
    d->f[1] = a->f[1] - b->f[1];
    d->f[2] = a->f[2] - b->f[2];
    d->f[4] = a->f[4] - b->f[4];
    d->f[5] = a->f[5] - b->f[5];
    d->f[6] = a->f[6] - b->f[6];
    d->f[8] = a->f[8] - b->f[8];
    d->f[9] = a->f[9] - b->f[9];
    d->f[10] = a->f[10] - b->f[10];
}

_fer_inline void ferMat3Scale(fer_mat3_t *d, fer_real_t s)
{
    d->f[0] *= s;
    d->f[1] *= s;
    d->f[2] *= s;
    d->f[4] *= s;
    d->f[5] *= s;
    d->f[6] *= s;
    d->f[8] *= s;
    d->f[9] *= s;
    d->f[10] *= s;
}

_fer_inline void ferMat3Scale2(fer_mat3_t *d, const fer_mat3_t *a, fer_real_t s)
{
    d->f[0] = a->f[0] * s;
    d->f[1] = a->f[1] * s;
    d->f[2] = a->f[2] * s;
    d->f[4] = a->f[4] * s;
    d->f[5] = a->f[5] * s;
    d->f[6] = a->f[6] * s;
    d->f[8] = a->f[8] * s;
    d->f[9] = a->f[9] * s;
    d->f[10] = a->f[10] * s;
}

_fer_inline void ferMat3AddConst(fer_mat3_t *d, fer_real_t c)
{
    d->f[0] += c;
    d->f[1] += c;
    d->f[2] += c;
    d->f[4] += c;
    d->f[5] += c;
    d->f[6] += c;
    d->f[8] += c;
    d->f[9] += c;
    d->f[10] += c;
}

_fer_inline void ferMat3AddConst2(fer_mat3_t *d, const fer_mat3_t *a, fer_real_t c)
{
    d->f[0] = a->f[0] + c;
    d->f[1] = a->f[1] + c;
    d->f[2] = a->f[2] + c;
    d->f[4] = a->f[4] + c;
    d->f[5] = a->f[5] + c;
    d->f[6] = a->f[6] + c;
    d->f[8] = a->f[8] + c;
    d->f[9] = a->f[9] + c;
    d->f[10] = a->f[10] + c;
}

_fer_inline void ferMat3Mul(fer_mat3_t *a, const fer_mat3_t *b)
{
    fer_real_t x, y, z;

    x = a->f[0];
    y = a->f[1];
    z = a->f[2];
    a->f[0]  = ferMat3DotCol(b, 0, x, y, z);
    a->f[1]  = ferMat3DotCol(b, 1, x, y, z);
    a->f[2]  = ferMat3DotCol(b, 2, x, y, z);
    x = a->f[4];
    y = a->f[5];
    z = a->f[6];
    a->f[4]  = ferMat3DotCol(b, 0, x, y, z);
    a->f[5]  = ferMat3DotCol(b, 1, x, y, z);
    a->f[6]  = ferMat3DotCol(b, 2, x, y, z);
    x = a->f[8];
    y = a->f[9];
    z = a->f[10];
    a->f[8]  = ferMat3DotCol(b, 0, x, y, z);
    a->f[9]  = ferMat3DotCol(b, 1, x, y, z);
    a->f[10] = ferMat3DotCol(b, 2, x, y, z);

}

_fer_inline void ferMat3Mul2(fer_mat3_t *d, const fer_mat3_t *a,
                                            const fer_mat3_t *b)
{
    d->f[0]  = ferMat3DotRowCol(a, 0, b, 0);
    d->f[1]  = ferMat3DotRowCol(a, 0, b, 1);
    d->f[2]  = ferMat3DotRowCol(a, 0, b, 2);
    d->f[4]  = ferMat3DotRowCol(a, 1, b, 0);
    d->f[5]  = ferMat3DotRowCol(a, 1, b, 1);
    d->f[6]  = ferMat3DotRowCol(a, 1, b, 2);
    d->f[8]  = ferMat3DotRowCol(a, 2, b, 0);
    d->f[9]  = ferMat3DotRowCol(a, 2, b, 1);
    d->f[10] = ferMat3DotRowCol(a, 2, b, 2);
}

_fer_inline void ferMat3MulLeft(fer_mat3_t *a, const fer_mat3_t *b)
{
    fer_real_t x, y, z;

    x = a->f[0];
    y = a->f[4];
    z = a->f[8];
    a->f[0]  = ferMat3DotRow(b, 0, x, y, z);
    a->f[4]  = ferMat3DotRow(b, 1, x, y, z);
    a->f[8]  = ferMat3DotRow(b, 2, x, y, z);
    x = a->f[1];
    y = a->f[5];
    z = a->f[9];
    a->f[1]  = ferMat3DotRow(b, 0, x, y, z);
    a->f[5]  = ferMat3DotRow(b, 1, x, y, z);
    a->f[9]  = ferMat3DotRow(b, 2, x, y, z);
    x = a->f[2];
    y = a->f[6];
    z = a->f[10];
    a->f[2]  = ferMat3DotRow(b, 0, x, y, z);
    a->f[6]  = ferMat3DotRow(b, 1, x, y, z);
    a->f[10] = ferMat3DotRow(b, 2, x, y, z);

}

_fer_inline void ferMat3MulComp(fer_mat3_t *a, const fer_mat3_t *b)
{
    a->f[0] *= b->f[0];
    a->f[1] *= b->f[1];
    a->f[2] *= b->f[2];
    a->f[4] *= b->f[4];
    a->f[5] *= b->f[5];
    a->f[6] *= b->f[6];
    a->f[8] *= b->f[8];
    a->f[9] *= b->f[9];
    a->f[10] *= b->f[10];
}

_fer_inline void ferMat3MulComp2(fer_mat3_t *d, const fer_mat3_t *a,
                                                const fer_mat3_t *b)
{
    d->f[0] = a->f[0] * b->f[0];
    d->f[1] = a->f[1] * b->f[1];
    d->f[2] = a->f[2] * b->f[2];
    d->f[4] = a->f[4] * b->f[4];
    d->f[5] = a->f[5] * b->f[5];
    d->f[6] = a->f[6] * b->f[6];
    d->f[8] = a->f[8] * b->f[8];
    d->f[9] = a->f[9] * b->f[9];
    d->f[10] = a->f[10] * b->f[10];
}

_fer_inline void ferMat3Trans(fer_mat3_t *d)
{
    fer_real_t a, b, c;

    a = d->f[1];
    b = d->f[2];
    c = d->f[6];
    ferMat3Set(d, d->f[0], d->f[4], d->f[8],
                  a,       d->f[5], d->f[9],
                  b,       c,       d->f[10]);
}

_fer_inline void ferMat3Trans2(fer_mat3_t *d, const fer_mat3_t *a)
{
    ferMat3Set(d, a->f[0], a->f[4], a->f[8],
                  a->f[1], a->f[5], a->f[9],
                  a->f[2], a->f[6], a->f[10]);
}

_fer_inline int ferMat3Regular(const fer_mat3_t *m)
{
    fer_real_t det;
    det = ferMat3Det(m);
    return !ferIsZero(det);
}

_fer_inline int ferMat3Singular(const fer_mat3_t *m)
{
    return !ferMat3Regular(m);
}

_fer_inline fer_real_t ferMat3Det(const fer_mat3_t *m)
{
    fer_real_t det;

    det =  m->f[0] * m->f[5] * m->f[10];
    det += m->f[1] * m->f[6] * m->f[8];
    det += m->f[2] * m->f[4] * m->f[9];
    det -= m->f[0] * m->f[6] * m->f[9];
    det -= m->f[1] * m->f[4] * m->f[10];
    det -= m->f[2] * m->f[5] * m->f[8];

    return det;
}

_fer_inline int ferMat3Inv(fer_mat3_t *m)
{
    fer_mat3_t n;
    ferMat3Copy(&n, m);
    return ferMat3Inv2(m, &n);
}

_fer_inline int ferMat3Inv2(fer_mat3_t *m, const fer_mat3_t *a)
{
    fer_real_t det, invdet;

    det = ferMat3Det(a);
    if (ferIsZero(det))
        return -1;

    invdet = ferRecp(det);

    m->f[0]  = a->f[5] * a->f[10] - a->f[6] * a->f[9];
    m->f[1]  = a->f[2] * a->f[9]  - a->f[1] * a->f[10];
    m->f[2]  = a->f[1] * a->f[6]  - a->f[2] * a->f[5];
    m->f[4]  = a->f[6] * a->f[8]  - a->f[4] * a->f[10];
    m->f[5]  = a->f[0] * a->f[10] - a->f[2] * a->f[8];
    m->f[6]  = a->f[2] * a->f[4]  - a->f[0] * a->f[6];
    m->f[8]  = a->f[4] * a->f[9]  - a->f[5] * a->f[8];
    m->f[9]  = a->f[1] * a->f[8]  - a->f[0] * a->f[9];
    m->f[10] = a->f[0] * a->f[5]  - a->f[1] * a->f[4];

    ferMat3Scale(m, invdet);

    return 0;
}

_fer_inline void ferMat3MulVec(fer_vec3_t *v, const fer_mat3_t *m,
                                              const fer_vec3_t *w)
{
    ferVec3SetX(v, ferMat3DotRow(m, 0, ferVec3X(w), ferVec3Y(w), ferVec3Z(w)));
    ferVec3SetY(v, ferMat3DotRow(m, 1, ferVec3X(w), ferVec3Y(w), ferVec3Z(w)));
    ferVec3SetZ(v, ferMat3DotRow(m, 2, ferVec3X(w), ferVec3Y(w), ferVec3Z(w)));
}

_fer_inline void ferMat3MulVec2(fer_vec2_t *v, const fer_mat3_t *m,
                                               const fer_vec2_t *w)
{
    fer_real_t denom;

    ferVec2SetX(v, ferMat3DotRow(m, 0, ferVec2X(w), ferVec2Y(w), FER_ONE));
    ferVec2SetY(v, ferMat3DotRow(m, 1, ferVec2X(w), ferVec2Y(w), FER_ONE));
    denom = ferMat3DotRow(m, 2, ferVec2X(w), ferVec2Y(w), FER_ONE);
    ferVec2Scale(v, ferRecp(denom));
}



_fer_inline fer_real_t ferMat3DotRowCol(const fer_mat3_t *m, size_t r,
                                        const fer_mat3_t *n, size_t c)
{
    fer_real_t dot;

    dot  = m->f[r * 4] * n->f[c];
    dot += m->f[r * 4 + 1] * n->f[c + 4];
    dot += m->f[r * 4 + 2] * n->f[c + 8];

    return dot;
}

_fer_inline fer_real_t ferMat3DotCol(const fer_mat3_t *m, size_t col,
                                     fer_real_t a, fer_real_t b, fer_real_t c)
{
    fer_real_t dot;

    dot  = m->f[col] * a;
    dot += m->f[col + 4] * b;
    dot += m->f[col + 8] * c;

    return dot;
}

_fer_inline fer_real_t ferMat3DotRow(const fer_mat3_t *m, size_t row,
                                     fer_real_t a, fer_real_t b, fer_real_t c)
{
    fer_real_t dot;

    dot  = m->f[row * 4] * a;
    dot += m->f[row * 4 + 1] * b;
    dot += m->f[row * 4 + 2] * c;

    return dot;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_MAT3_H__ */
