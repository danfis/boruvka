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

#ifndef __BOR_MAT4_H__
#define __BOR_MAT4_H__

#include <boruvka/core.h>
#include <boruvka/vec3.h>
#include <boruvka/vec4.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Mat4 - 4x4 matrix
 * ==================
 */
union _bor_mat4_t {
    bor_vec4_t v[4];
    bor_real_t f[4 * 4];
};
typedef union _bor_mat4_t bor_mat4_t;

/**
 * Holds identity matrix. Read only variable!
 */
extern const bor_mat4_t *bor_mat4_identity;

/**
 * Holds zero matrix. Read only variable!
 */
extern const bor_mat4_t *bor_mat4_zero;

#define BOR_MAT4_STATIC(f11, f12, f13, f14, \
                        f21, f22, f23, f24, \
                        f31, f32, f33, f34, \
                        f41, f42, f43, f44) \
    { .f = { (f11), (f12), (f13), (f14), \
             (f21), (f22), (f23), (f24), \
             (f31), (f32), (f33), (f34), \
             (f41), (f42), (f43), (f44) } }

#define BOR_MAT4(name, f11, f12, f13, f14, \
                       f21, f22, f23, f24, \
                       f31, f32, f33, f34, \
                       f41, f42, f43, f44) \
    bor_mat4_t name = BOR_MAT4_STATIC((f11), (f12), (f13), (f14), \
                                      (f21), (f22), (f23), (f24), \
                                      (f31), (f32), (f33), (f34), \
                                      (f41), (f42), (f43), (f44))

/**
 * Functions
 * ----------
 */

/**
 * Allocates uninitialized new matrix.
 */
bor_mat4_t *borMat4New(void);

/**
 * Deletes matrix.
 */
void borMat4Del(bor_mat4_t *m);

/**
 * Clones given matrix.
 */
_bor_inline bor_mat4_t *borMat4Clone(const bor_mat4_t *m);

/**
 * Copies matrix from s to d.
 */
_bor_inline void borMat4Copy(bor_mat4_t *d, const bor_mat4_t *s);


/**
 * Returns element from specified position.
 */
_bor_inline bor_real_t borMat4Get(const bor_mat4_t *m, size_t row, size_t col);

/**
 * Sets element at specified position.
 */
_bor_inline void borMat4Set1(bor_mat4_t *m, size_t row, size_t col,
                             bor_real_t val);

/**
 * Sets all elements to val.
 */
_bor_inline void borMat4SetAll(bor_mat4_t *m, bor_real_t val);

/**
 * Sets whole matrix.
 */
_bor_inline void borMat4Set(bor_mat4_t *m,
                            bor_real_t f11, bor_real_t f12, bor_real_t f13, bor_real_t f14,
                            bor_real_t f21, bor_real_t f22, bor_real_t f23, bor_real_t f24,
                            bor_real_t f31, bor_real_t f32, bor_real_t f33, bor_real_t f34,
                            bor_real_t f41, bor_real_t f42, bor_real_t f43, bor_real_t f44);

/**
 * Set identity matrix.
 * ~~~~
 *     | 1 0 0 0 |
 * m = | 0 1 0 0 |
 *     | 0 0 1 0 |
 *     | 0 0 0 1 |
 */
_bor_inline void borMat4SetIdentity(bor_mat4_t *m);

/**
 * Set zero matrix.
 */
_bor_inline void borMat4SetZero(bor_mat4_t *m);

/**
 * Set scale matrix.
 * ~~~~
 *     | s 0 0 0 |
 * m = | 0 s 0 0 |
 *     | 0 0 s 0 |
 *     | 0 0 0 1 |
 */
_bor_inline void borMat4SetScale(bor_mat4_t *m, bor_real_t s);

/**
 * Set diagonal.
 * ~~~~
 *     | s 0 0 0 |
 * m = | 0 s 0 0 |
 *     | 0 0 s 0 |
 *     | 0 0 0 s |
 */
_bor_inline void borMat4SetDiag(bor_mat4_t *m, bor_real_t s);

/**
 * Set translation matrix (translation along v vector).
 * ~~~~
 *     | 1 0 0 x |
 * m = | 0 1 0 y |
 *     | 0 0 1 z |
 *     | 0 0 0 1 |
 */
_bor_inline void borMat4SetTranslate(bor_mat4_t *m, const bor_vec3_t *v);

/**
 * Set rotation matrix (3D).
 */
_bor_inline void borMat4SetRot(bor_mat4_t *m,
                               bor_real_t angle, const bor_vec3_t *axis);

/**
 * Apply scale transfomation.
 * ~~~~
 *     | s 0 0 0 |
 * m = | 0 s 0 0 | . m
 *     | 0 0 s 0 |
 *     | 0 0 0 1 |
 */
_bor_inline void borMat4TrScale(bor_mat4_t *m, bor_real_t s);

/**
 * Translate transformation matrix towards vector v.
 * ~~~~
 *     | 1 0 0 x |
 * m = | 0 1 0 y | . m
 *     | 0 0 0 z |
 *     | 0 0 0 1 |
 */
_bor_inline void borMat4Translate(bor_mat4_t *m, const bor_vec3_t *v);

/**
 * Rotate transformation matrix.
 */
_bor_inline void borMat4Rot(bor_mat4_t *m,
                            bor_real_t angle, const bor_vec3_t *axis);


/**
 * Composes transformation:
 * A = B . A
 */
_bor_inline void borMat4Compose(bor_mat4_t *A, const bor_mat4_t *B);


/**
 * a = a + b
 */
_bor_inline void borMat4Add(bor_mat4_t *a, const bor_mat4_t *b);

/**
 * d = a + b
 */
_bor_inline void borMat4Add2(bor_mat4_t *d, const bor_mat4_t *a,
                                            const bor_mat4_t *b);

/**
 * a = a - b
 */
_bor_inline void borMat4Sub(bor_mat4_t *a, const bor_mat4_t *b);

/**
 * d = a - b
 */
_bor_inline void borMat4Sub2(bor_mat4_t *d, const bor_mat4_t *a,
                                            const bor_mat4_t *b);

/**
 * d = d * s
 */
_bor_inline void borMat4Scale(bor_mat4_t *d, bor_real_t s);

/**
 * d = a * s
 */
_bor_inline void borMat4Scale2(bor_mat4_t *d, const bor_mat4_t *a, bor_real_t s);

/**
 * d = d + c
 */
_bor_inline void borMat4AddConst(bor_mat4_t *d, bor_real_t c);

/**
 * d = a + c
 */
_bor_inline void borMat4AddConst2(bor_mat4_t *d, const bor_mat4_t *a, bor_real_t c);

/**
 * d = d - c
 */
_bor_inline void borMat4SubConst(bor_mat4_t *d, bor_real_t c);

/**
 * d = a - c
 */
_bor_inline void borMat4SubConst2(bor_mat4_t *d, const bor_mat4_t *a, bor_real_t c);


/**
 * a = a * b
 */
_bor_inline void borMat4Mul(bor_mat4_t *a, const bor_mat4_t *b);

/**
 * d = a * b
 */
_bor_inline void borMat4Mul2(bor_mat4_t *d, const bor_mat4_t *a,
                                            const bor_mat4_t *b);

/**
 * a = b * a
 */
_bor_inline void borMat4MulLeft(bor_mat4_t *a, const bor_mat4_t *b);


/**
 * Multiplies two matrices by components.
 * ~~~~
 *     | a11*b11 a12*b12 a13*b13 a14*b14 |
 * a = | a21*b21 a22*b22 a23*b23 a24*b24 |
 *     | a31*b31 a32*b32 a33*b33 a34*b34 |
 *     | a41*b41 a42*b42 a43*b43 a44*b44 |
 */
_bor_inline void borMat4MulComp(bor_mat4_t *a, const bor_mat4_t *b);
_bor_inline void borMat4MulComp2(bor_mat4_t *d, const bor_mat4_t *a,
                                                const bor_mat4_t *b);

/**
 * Transposes matrix.
 */
_bor_inline void borMat4Trans(bor_mat4_t *d);
_bor_inline void borMat4Trans2(bor_mat4_t *d, const bor_mat4_t *a);

/**
 * Returns true if matrix is regular.
 */
_bor_inline int borMat4Regular(const bor_mat4_t *m);

/**
 * Returns true if matrix is singular.
 */
_bor_inline int borMat4Singular(const bor_mat4_t *m);

/**
 * Returns determinant of matrix.
 */
_bor_inline bor_real_t borMat4Det(const bor_mat4_t *m);

/**
 * Inverts matrix.
 * Returns 0 on success, -1 if matrix is singular.
 */
_bor_inline int borMat4Inv(bor_mat4_t *m);

/**
 * Computes invertion matrix and stores it in m:
 * m = inv(a)
 * Returns 0 on success, -1 if matrix is singular.
 */
int borMat4Inv2(bor_mat4_t *m, const bor_mat4_t *a);


/**
 * Multiplies 4D vector by matrix (vectors are considered to be colunmal).
 * v = m * w
 */
_bor_inline void borMat4MulVec(bor_vec4_t *v, const bor_mat4_t *m,
                                              const bor_vec4_t *w);

/**
 * Multiplies matrix with 3D vector.
 * Third coordinate is filled with 1 and after multiplication x, y and z
 * coordinate is divided by resulting third coordinate.
 * In fact, this operation is applying transform matrix on vector w.
 *
 * ~~~~
 *         | w.x |
 * v = m * | w.y |
 *         | w.z |
 *         |  1  |
 * v.x = v.x / v.w
 * v.y = v.y / v.w
 * v.z = v.z / v.w
 */
_bor_inline void borMat4MulVec3(bor_vec3_t *v, const bor_mat4_t *m,
                                               const bor_vec3_t *w);


/**
 * Copies c'th column in col vector.
 */
_bor_inline void borMat4CopyCol(bor_vec4_t *col, const bor_mat4_t *m, size_t c);

/**
 * Copies r'th row in row vector.
 */
_bor_inline void borMat4CopyRow(bor_vec4_t *row, const bor_mat4_t *m, size_t r);

/**
 * Computes dot product of r'th row of matrix m and c'th column of matrix n.
 */
_bor_inline bor_real_t borMat4DotRowCol(const bor_mat4_t *m, size_t r,
                                        const bor_mat4_t *n, size_t c);

/**
 * Returns dot product of col'th column of matrix m with vector v.
 */
_bor_inline bor_real_t borMat4DotCol(const bor_mat4_t *m, size_t c,
                                     const bor_vec4_t *v);

/**
 * Returns dot product of row'th row of matrix m with vector v.
 */
_bor_inline bor_real_t borMat4DotRow(const bor_mat4_t *m, size_t r,
                                     const bor_vec4_t *v);


/**** INLINES ****/
_bor_inline bor_mat4_t *borMat4Clone(const bor_mat4_t *m)
{
    bor_mat4_t *n;

    n = borMat4New();
    borMat4Copy(n, m);
    return n;
}

_bor_inline void borMat4Copy(bor_mat4_t *d, const bor_mat4_t *s)
{
    *d = *s;
}


_bor_inline bor_real_t borMat4Get(const bor_mat4_t *m, size_t row, size_t col)
{
    return m->f[row * 4 + col];
}

_bor_inline void borMat4Set1(bor_mat4_t *m, size_t row, size_t col,
                             bor_real_t val)
{
    m->f[row * 4 + col] = val;
}

_bor_inline void borMat4SetAll(bor_mat4_t *m, bor_real_t val)
{
    size_t i;
    for (i = 0; i < 4 * 4; i++){
        m->f[i] = val;
    }
}

_bor_inline void borMat4Set(bor_mat4_t *m,
                            bor_real_t f11, bor_real_t f12, bor_real_t f13, bor_real_t f14,
                            bor_real_t f21, bor_real_t f22, bor_real_t f23, bor_real_t f24,
                            bor_real_t f31, bor_real_t f32, bor_real_t f33, bor_real_t f34,
                            bor_real_t f41, bor_real_t f42, bor_real_t f43, bor_real_t f44)
{
    m->f[0] = f11;
    m->f[1] = f12;
    m->f[2] = f13;
    m->f[3] = f14;
    m->f[4] = f21;
    m->f[5] = f22;
    m->f[6] = f23;
    m->f[7] = f24;
    m->f[8] = f31;
    m->f[9] = f32;
    m->f[10] = f33;
    m->f[11] = f34;
    m->f[12] = f41;
    m->f[13] = f42;
    m->f[14] = f43;
    m->f[15] = f44;
}

_bor_inline void borMat4SetIdentity(bor_mat4_t *m)
{
    borMat4Set(m, BOR_ONE, BOR_ZERO, BOR_ZERO, BOR_ZERO,
                  BOR_ZERO, BOR_ONE, BOR_ZERO, BOR_ZERO,
                  BOR_ZERO, BOR_ZERO, BOR_ONE, BOR_ZERO,
                  BOR_ZERO, BOR_ZERO, BOR_ZERO, BOR_ONE);
}

_bor_inline void borMat4SetZero(bor_mat4_t *m)
{
    borMat4SetAll(m, BOR_ZERO);
}

_bor_inline void borMat4SetScale(bor_mat4_t *m, bor_real_t s)
{
    borMat4Set(m, s, BOR_ZERO, BOR_ZERO, BOR_ZERO,
                  BOR_ZERO, s, BOR_ZERO, BOR_ZERO,
                  BOR_ZERO, BOR_ZERO, s, BOR_ZERO,
                  BOR_ZERO, BOR_ZERO, BOR_ZERO, BOR_ONE);
}

_bor_inline void borMat4SetDiag(bor_mat4_t *m, bor_real_t s)
{
    borMat4Set(m, s, BOR_ZERO, BOR_ZERO, BOR_ZERO,
                  BOR_ZERO, s, BOR_ZERO, BOR_ZERO,
                  BOR_ZERO, BOR_ZERO, s, BOR_ZERO,
                  BOR_ZERO, BOR_ZERO, BOR_ZERO, s);
}

_bor_inline void borMat4SetTranslate(bor_mat4_t *m, const bor_vec3_t *v)
{
    borMat4Set(m, BOR_ONE, BOR_ZERO, BOR_ZERO, borVec3X(v),
                  BOR_ZERO, BOR_ONE, BOR_ZERO, borVec3Y(v),
                  BOR_ZERO, BOR_ZERO, BOR_ONE, borVec3Z(v),
                  BOR_ZERO, BOR_ZERO, BOR_ZERO, BOR_ONE);
}

_bor_inline void borMat4SetRot(bor_mat4_t *m,
                               bor_real_t angle, const bor_vec3_t *axis)
{
    bor_real_t x2, y2, z2, len2;
    bor_real_t k1, k2, k3, k1xy, k1xz, k1yz, k3x, k3y, k3z;

    x2 = borVec3X(axis) * borVec3X(axis);
    y2 = borVec3Y(axis) * borVec3Y(axis);
    z2 = borVec3Z(axis) * borVec3Z(axis);
    len2 = x2 + y2 + z2;

    k2   = BOR_COS(angle);
    k1   = (BOR_ONE - k2) / len2;
    k3   = BOR_SIN(angle) / BOR_SQRT(len2);
    k1xy = k1 * borVec3X(axis) * borVec3Y(axis);
    k1xz = k1 * borVec3X(axis) * borVec3Z(axis);
    k1yz = k1 * borVec3Y(axis) * borVec3Z(axis);
    k3x  = k3 * borVec3X(axis);
    k3y  = k3 * borVec3Y(axis);
    k3z  = k3 * borVec3Z(axis);

    m->f[0] = k1 * x2 + k2;
    m->f[1] = k1xy - k3z;
    m->f[2] = k1xz + k3y;
    m->f[3] = BOR_ZERO;

    m->f[4] = k1xy + k3z;
    m->f[5] = k1 * y2 + k2;
    m->f[6] = k1yz - k3x;
    m->f[7] = BOR_ZERO;

    m->f[8] = k1xy - k3y;
    m->f[9] = k1yz + k3x;
    m->f[10] = k1 * z2 + k2;
    m->f[11] = BOR_ZERO;

    m->f[12] = BOR_ZERO;
    m->f[13] = BOR_ZERO;
    m->f[14] = BOR_ZERO;
    m->f[15] = BOR_ONE;
}

_bor_inline void borMat4TrScale(bor_mat4_t *m, bor_real_t s)
{
    bor_mat4_t scale;
    borMat4SetScale(&scale, s);
    borMat4Compose(m, &scale);
}

_bor_inline void borMat4Translate(bor_mat4_t *m, const bor_vec3_t *v)
{
    bor_mat4_t tr;
    borMat4SetTranslate(&tr, v);
    borMat4Compose(m, &tr);
}

_bor_inline void borMat4Rot(bor_mat4_t *m,
                            bor_real_t angle, const bor_vec3_t *axis)
{
    bor_mat4_t rot;
    borMat4SetRot(&rot, angle, axis);
    borMat4Compose(m, &rot);
}


_bor_inline void borMat4Compose(bor_mat4_t *A, const bor_mat4_t *B)
{
    borMat4MulLeft(A, B);
}


_bor_inline void borMat4Add(bor_mat4_t *a, const bor_mat4_t *b)
{
    size_t i;
    for (i = 0; i < 4; i++){
        borVec4Add(a->v + i, b->v + i);
    }
}

_bor_inline void borMat4Add2(bor_mat4_t *d, const bor_mat4_t *a,
                                            const bor_mat4_t *b)
{
    size_t i;
    for (i = 0; i < 4; i++){
        borVec4Add2(d->v + i, a->v + i, b->v + i);
    }
}

_bor_inline void borMat4Sub(bor_mat4_t *a, const bor_mat4_t *b)
{
    size_t i;
    for (i = 0; i < 4; i++){
        borVec4Sub(a->v + i, b->v + i);
    }
}

_bor_inline void borMat4Sub2(bor_mat4_t *d, const bor_mat4_t *a,
                                            const bor_mat4_t *b)
{
    size_t i;
    for (i = 0; i < 4; i++){
        borVec4Sub2(d->v + i, a->v + i, b->v + i);
    }
}

_bor_inline void borMat4Scale(bor_mat4_t *d, bor_real_t s)
{
    size_t i;
    for (i = 0; i < 4; i++){
        borVec4Scale(d->v + i, s);
    }
}

_bor_inline void borMat4Scale2(bor_mat4_t *d, const bor_mat4_t *a, bor_real_t s)
{
    size_t i;
    for (i = 0; i < 4; i++){
        borVec4Scale2(d->v + i, a->v + i, s);
    }
}

_bor_inline void borMat4AddConst(bor_mat4_t *d, bor_real_t c)
{
    size_t i;
    for (i = 0; i < 4; i++){
        borVec4AddConst(d->v + i, c);
    }
}

_bor_inline void borMat4AddConst2(bor_mat4_t *d, const bor_mat4_t *a, bor_real_t c)
{
    size_t i;
    for (i = 0; i < 4; i++){
        borVec4AddConst2(d->v + i, a->v + i, c);
    }
}

_bor_inline void borMat4SubConst(bor_mat4_t *d, bor_real_t c)
{
    size_t i;
    for (i = 0; i < 4; i++){
        borVec4SubConst(d->v + i, c);
    }
}

_bor_inline void borMat4SubConst2(bor_mat4_t *d, const bor_mat4_t *a, bor_real_t c)
{
    size_t i;
    for (i = 0; i < 4; i++){
        borVec4SubConst2(d->v + i, a->v + i, c);
    }
}

_bor_inline void borMat4Mul(bor_mat4_t *a, const bor_mat4_t *b)
{
    bor_vec4_t v;

    borVec4Copy(&v, a->v + 0);
    a->f[0]  = borMat4DotCol(b, 0, &v);
    a->f[1]  = borMat4DotCol(b, 1, &v);
    a->f[2]  = borMat4DotCol(b, 2, &v);
    a->f[3]  = borMat4DotCol(b, 3, &v);

    borVec4Copy(&v, a->v + 1);
    a->f[4]  = borMat4DotCol(b, 0, &v);
    a->f[5]  = borMat4DotCol(b, 1, &v);
    a->f[6]  = borMat4DotCol(b, 2, &v);
    a->f[7]  = borMat4DotCol(b, 3, &v);

    borVec4Copy(&v, a->v + 2);
    a->f[8]  = borMat4DotCol(b, 0, &v);
    a->f[9]  = borMat4DotCol(b, 1, &v);
    a->f[10] = borMat4DotCol(b, 2, &v);
    a->f[11] = borMat4DotCol(b, 3, &v);

    borVec4Copy(&v, a->v + 3);
    a->f[12] = borMat4DotCol(b, 0, &v);
    a->f[13] = borMat4DotCol(b, 1, &v);
    a->f[14] = borMat4DotCol(b, 2, &v);
    a->f[15] = borMat4DotCol(b, 3, &v);

}

_bor_inline void borMat4Mul2(bor_mat4_t *d, const bor_mat4_t *a,
                                            const bor_mat4_t *b)
{
    bor_vec4_t v;

    borMat4CopyCol(&v, b, 0);
    d->f[0]  = borMat4DotRow(a, 0, &v);
    d->f[4]  = borMat4DotRow(a, 1, &v);
    d->f[8]  = borMat4DotRow(a, 2, &v);
    d->f[12] = borMat4DotRow(a, 3, &v);

    borMat4CopyCol(&v, b, 1);
    d->f[1]  = borMat4DotRow(a, 0, &v);
    d->f[5]  = borMat4DotRow(a, 1, &v);
    d->f[9]  = borMat4DotRow(a, 2, &v);
    d->f[13] = borMat4DotRow(a, 3, &v);

    borMat4CopyCol(&v, b, 2);
    d->f[2]  = borMat4DotRow(a, 0, &v);
    d->f[6]  = borMat4DotRow(a, 1, &v);
    d->f[10] = borMat4DotRow(a, 2, &v);
    d->f[14] = borMat4DotRow(a, 3, &v);

    borMat4CopyCol(&v, b, 3);
    d->f[3]  = borMat4DotRow(a, 0, &v);
    d->f[7]  = borMat4DotRow(a, 1, &v);
    d->f[11] = borMat4DotRow(a, 2, &v);
    d->f[15] = borMat4DotRow(a, 3, &v);
}

_bor_inline void borMat4MulLeft(bor_mat4_t *a, const bor_mat4_t *b)
{
    bor_vec4_t v;

    borMat4CopyCol(&v, a, 0);
    a->f[0]  = borMat4DotRow(b, 0, &v);
    a->f[4]  = borMat4DotRow(b, 1, &v);
    a->f[8]  = borMat4DotRow(b, 2, &v);
    a->f[12] = borMat4DotRow(b, 3, &v);

    borMat4CopyCol(&v, a, 1);
    a->f[1]  = borMat4DotRow(b, 0, &v);
    a->f[5]  = borMat4DotRow(b, 1, &v);
    a->f[9]  = borMat4DotRow(b, 2, &v);
    a->f[13] = borMat4DotRow(b, 3, &v);

    borMat4CopyCol(&v, a, 2);
    a->f[2]  = borMat4DotRow(b, 0, &v);
    a->f[6]  = borMat4DotRow(b, 1, &v);
    a->f[10] = borMat4DotRow(b, 2, &v);
    a->f[14] = borMat4DotRow(b, 3, &v);

    borMat4CopyCol(&v, a, 3);
    a->f[3]  = borMat4DotRow(b, 0, &v);
    a->f[7]  = borMat4DotRow(b, 1, &v);
    a->f[11] = borMat4DotRow(b, 2, &v);
    a->f[15] = borMat4DotRow(b, 3, &v);
}

_bor_inline void borMat4MulComp(bor_mat4_t *a, const bor_mat4_t *b)
{
    size_t i;
    for (i = 0; i < 4; i++){
        borVec4MulComp(a->v + i, b->v + i);
    }
}

_bor_inline void borMat4MulComp2(bor_mat4_t *d, const bor_mat4_t *a,
                                                const bor_mat4_t *b)
{
    size_t i;
    for (i = 0; i < 4; i++){
        borVec4MulComp2(d->v + i, a->v + i, b->v + i);
    }
}

_bor_inline void borMat4Trans(bor_mat4_t *d)
{
    bor_real_t a, b, c, e, f, g;

    a = d->f[1];
    b = d->f[2];
    c = d->f[6];
    e = d->f[3];
    f = d->f[7];
    g = d->f[11];
    borMat4Set(d, d->f[0], d->f[4], d->f[8],  d->f[12],
                  a,       d->f[5], d->f[9],  d->f[13],
                  b,       c,       d->f[10], d->f[14],
                  e,       f,       g,        d->f[15]);
}

_bor_inline void borMat4Trans2(bor_mat4_t *d, const bor_mat4_t *a)
{
    borMat4CopyCol(d->v + 0, a, 0);
    borMat4CopyCol(d->v + 1, a, 1);
    borMat4CopyCol(d->v + 2, a, 2);
    borMat4CopyCol(d->v + 3, a, 3);
}

_bor_inline int borMat4Regular(const bor_mat4_t *m)
{
    bor_real_t det;
    det = borMat4Det(m);
    return !borIsZero(det);
}

_bor_inline int borMat4Singular(const bor_mat4_t *m)
{
    return !borMat4Regular(m);
}

_bor_inline bor_real_t borMat4Det(const bor_mat4_t *m)
{
    bor_real_t det;

    det  = m->f[0] * m->f[5] * m->f[10] * m->f[15];
    det -= m->f[0] * m->f[5] * m->f[11] * m->f[14];
    det += m->f[0] * m->f[6] * m->f[11] * m->f[13];
    det -= m->f[0] * m->f[6] * m->f[9]  * m->f[15];
    det += m->f[0] * m->f[7] * m->f[9]  * m->f[14];
    det -= m->f[0] * m->f[7] * m->f[10] * m->f[13];
    det -= m->f[1] * m->f[6] * m->f[11] * m->f[12];
    det += m->f[1] * m->f[6] * m->f[8]  * m->f[15];
    det -= m->f[1] * m->f[7] * m->f[8]  * m->f[14];
    det += m->f[1] * m->f[7] * m->f[10] * m->f[12];
    det -= m->f[1] * m->f[4] * m->f[10] * m->f[15];
    det += m->f[1] * m->f[4] * m->f[11] * m->f[14];
    det += m->f[2] * m->f[7] * m->f[8]  * m->f[13];
    det -= m->f[2] * m->f[7] * m->f[9]  * m->f[12];
    det += m->f[2] * m->f[4] * m->f[9]  * m->f[15];
    det -= m->f[2] * m->f[4] * m->f[11] * m->f[13];
    det += m->f[2] * m->f[5] * m->f[11] * m->f[12];
    det -= m->f[2] * m->f[5] * m->f[8]  * m->f[15];
    det -= m->f[3] * m->f[4] * m->f[9]  * m->f[14];
    det += m->f[3] * m->f[4] * m->f[10] * m->f[13];
    det -= m->f[3] * m->f[5] * m->f[10] * m->f[12];
    det += m->f[3] * m->f[5] * m->f[8]  * m->f[14];
    det -= m->f[3] * m->f[6] * m->f[8]  * m->f[13];
    det += m->f[3] * m->f[6] * m->f[9]  * m->f[12];

    return det;
}

_bor_inline int borMat4Inv(bor_mat4_t *m)
{
    bor_mat4_t n;
    borMat4Copy(&n, m);
    return borMat4Inv2(m, &n);
}

_bor_inline void borMat4MulVec(bor_vec4_t *v, const bor_mat4_t *m,
                                              const bor_vec4_t *w)
{
    borVec4SetX(v, borVec4Dot(m->v + 0, w));
    borVec4SetY(v, borVec4Dot(m->v + 1, w));
    borVec4SetZ(v, borVec4Dot(m->v + 2, w));
    borVec4SetW(v, borVec4Dot(m->v + 3, w));
}

_bor_inline void borMat4MulVec3(bor_vec3_t *v, const bor_mat4_t *m,
                                               const bor_vec3_t *_w)
{
    bor_real_t denom;
    bor_vec4_t w;

    borVec4SetX(&w, borVec3X(_w));
    borVec4SetY(&w, borVec3Y(_w));
    borVec4SetZ(&w, borVec3Z(_w));
    borVec4SetW(&w, BOR_ONE);

    borVec3SetX(v, borVec4Dot(m->v + 0, &w));
    borVec3SetY(v, borVec4Dot(m->v + 1, &w));
    borVec3SetZ(v, borVec4Dot(m->v + 2, &w));
    denom = borVec4Dot(m->v + 3, &w);
    borVec3Scale(v, borRecp(denom));
}



_bor_inline void borMat4CopyCol(bor_vec4_t *col, const bor_mat4_t *m, size_t c)
{
    borVec4Set(col, m->f[c], m->f[c + 4], m->f[c + 8], m->f[c + 12]);
}

_bor_inline void borMat4CopyRow(bor_vec4_t *row, const bor_mat4_t *m, size_t r)
{
    borVec4Copy(row, m->v + r);
}

_bor_inline bor_real_t borMat4DotRowCol(const bor_mat4_t *m, size_t r,
                                        const bor_mat4_t *n, size_t c)
{
    bor_vec4_t col;
    borMat4CopyCol(&col, n, c);
    return borVec4Dot(m->v + r, &col);
}

_bor_inline bor_real_t borMat4DotCol(const bor_mat4_t *m, size_t c,
                                     const bor_vec4_t *v)
{
    bor_vec4_t col;
    borMat4CopyCol(&col, m, c);
    return borVec4Dot(&col, v);
}

_bor_inline bor_real_t borMat4DotRow(const bor_mat4_t *m, size_t r,
                                     const bor_vec4_t *v)
{
    return borVec4Dot(m->v + r, v);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_MAT4_H__ */
