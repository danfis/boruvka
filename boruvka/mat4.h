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

#ifndef __FER_MAT4_H__
#define __FER_MAT4_H__

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
extern const bor_mat4_t *fer_mat4_identity;

/**
 * Holds zero matrix. Read only variable!
 */
extern const bor_mat4_t *fer_mat4_zero;

#define FER_MAT4_STATIC(f11, f12, f13, f14, \
                        f21, f22, f23, f24, \
                        f31, f32, f33, f34, \
                        f41, f42, f43, f44) \
    { .f = { (f11), (f12), (f13), (f14), \
             (f21), (f22), (f23), (f24), \
             (f31), (f32), (f33), (f34), \
             (f41), (f42), (f43), (f44) } }

#define FER_MAT4(name, f11, f12, f13, f14, \
                       f21, f22, f23, f24, \
                       f31, f32, f33, f34, \
                       f41, f42, f43, f44) \
    bor_mat4_t name = FER_MAT4_STATIC((f11), (f12), (f13), (f14), \
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
bor_mat4_t *ferMat4New(void);

/**
 * Deletes matrix.
 */
void ferMat4Del(bor_mat4_t *m);

/**
 * Clones given matrix.
 */
_fer_inline bor_mat4_t *ferMat4Clone(const bor_mat4_t *m);

/**
 * Copies matrix from s to d.
 */
_fer_inline void ferMat4Copy(bor_mat4_t *d, const bor_mat4_t *s);


/**
 * Returns element from specified position.
 */
_fer_inline bor_real_t ferMat4Get(const bor_mat4_t *m, size_t row, size_t col);

/**
 * Sets element at specified position.
 */
_fer_inline void ferMat4Set1(bor_mat4_t *m, size_t row, size_t col,
                             bor_real_t val);

/**
 * Sets all elements to val.
 */
_fer_inline void ferMat4SetAll(bor_mat4_t *m, bor_real_t val);

/**
 * Sets whole matrix.
 */
_fer_inline void ferMat4Set(bor_mat4_t *m,
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
_fer_inline void ferMat4SetIdentity(bor_mat4_t *m);

/**
 * Set zero matrix.
 */
_fer_inline void ferMat4SetZero(bor_mat4_t *m);

/**
 * Set scale matrix.
 * ~~~~
 *     | s 0 0 0 |
 * m = | 0 s 0 0 |
 *     | 0 0 s 0 |
 *     | 0 0 0 1 |
 */
_fer_inline void ferMat4SetScale(bor_mat4_t *m, bor_real_t s);

/**
 * Set diagonal.
 * ~~~~
 *     | s 0 0 0 |
 * m = | 0 s 0 0 |
 *     | 0 0 s 0 |
 *     | 0 0 0 s |
 */
_fer_inline void ferMat4SetDiag(bor_mat4_t *m, bor_real_t s);

/**
 * Set translation matrix (translation along v vector).
 * ~~~~
 *     | 1 0 0 x |
 * m = | 0 1 0 y |
 *     | 0 0 1 z |
 *     | 0 0 0 1 |
 */
_fer_inline void ferMat4SetTranslate(bor_mat4_t *m, const bor_vec3_t *v);

/**
 * Set rotation matrix (3D).
 */
_fer_inline void ferMat4SetRot(bor_mat4_t *m,
                               bor_real_t angle, const bor_vec3_t *axis);

/**
 * Apply scale transfomation.
 * ~~~~
 *     | s 0 0 0 |
 * m = | 0 s 0 0 | . m
 *     | 0 0 s 0 |
 *     | 0 0 0 1 |
 */
_fer_inline void ferMat4TrScale(bor_mat4_t *m, bor_real_t s);

/**
 * Translate transformation matrix towards vector v.
 * ~~~~
 *     | 1 0 0 x |
 * m = | 0 1 0 y | . m
 *     | 0 0 0 z |
 *     | 0 0 0 1 |
 */
_fer_inline void ferMat4Translate(bor_mat4_t *m, const bor_vec3_t *v);

/**
 * Rotate transformation matrix.
 */
_fer_inline void ferMat4Rot(bor_mat4_t *m,
                            bor_real_t angle, const bor_vec3_t *axis);


/**
 * Composes transformation:
 * A = B . A
 */
_fer_inline void ferMat4Compose(bor_mat4_t *A, const bor_mat4_t *B);


/**
 * a = a + b
 */
_fer_inline void ferMat4Add(bor_mat4_t *a, const bor_mat4_t *b);

/**
 * d = a + b
 */
_fer_inline void ferMat4Add2(bor_mat4_t *d, const bor_mat4_t *a,
                                            const bor_mat4_t *b);

/**
 * a = a - b
 */
_fer_inline void ferMat4Sub(bor_mat4_t *a, const bor_mat4_t *b);

/**
 * d = a - b
 */
_fer_inline void ferMat4Sub2(bor_mat4_t *d, const bor_mat4_t *a,
                                            const bor_mat4_t *b);

/**
 * d = d * s
 */
_fer_inline void ferMat4Scale(bor_mat4_t *d, bor_real_t s);

/**
 * d = a * s
 */
_fer_inline void ferMat4Scale2(bor_mat4_t *d, const bor_mat4_t *a, bor_real_t s);

/**
 * d = d + c
 */
_fer_inline void ferMat4AddConst(bor_mat4_t *d, bor_real_t c);

/**
 * d = a + c
 */
_fer_inline void ferMat4AddConst2(bor_mat4_t *d, const bor_mat4_t *a, bor_real_t c);

/**
 * d = d - c
 */
_fer_inline void ferMat4SubConst(bor_mat4_t *d, bor_real_t c);

/**
 * d = a - c
 */
_fer_inline void ferMat4SubConst2(bor_mat4_t *d, const bor_mat4_t *a, bor_real_t c);


/**
 * a = a * b
 */
_fer_inline void ferMat4Mul(bor_mat4_t *a, const bor_mat4_t *b);

/**
 * d = a * b
 */
_fer_inline void ferMat4Mul2(bor_mat4_t *d, const bor_mat4_t *a,
                                            const bor_mat4_t *b);

/**
 * a = b * a
 */
_fer_inline void ferMat4MulLeft(bor_mat4_t *a, const bor_mat4_t *b);


/**
 * Multiplies two matrices by components.
 * ~~~~
 *     | a11*b11 a12*b12 a13*b13 a14*b14 |
 * a = | a21*b21 a22*b22 a23*b23 a24*b24 |
 *     | a31*b31 a32*b32 a33*b33 a34*b34 |
 *     | a41*b41 a42*b42 a43*b43 a44*b44 |
 */
_fer_inline void ferMat4MulComp(bor_mat4_t *a, const bor_mat4_t *b);
_fer_inline void ferMat4MulComp2(bor_mat4_t *d, const bor_mat4_t *a,
                                                const bor_mat4_t *b);

/**
 * Transposes matrix.
 */
_fer_inline void ferMat4Trans(bor_mat4_t *d);
_fer_inline void ferMat4Trans2(bor_mat4_t *d, const bor_mat4_t *a);

/**
 * Returns true if matrix is regular.
 */
_fer_inline int ferMat4Regular(const bor_mat4_t *m);

/**
 * Returns true if matrix is singular.
 */
_fer_inline int ferMat4Singular(const bor_mat4_t *m);

/**
 * Returns determinant of matrix.
 */
_fer_inline bor_real_t ferMat4Det(const bor_mat4_t *m);

/**
 * Inverts matrix.
 * Returns 0 on success, -1 if matrix is singular.
 */
_fer_inline int ferMat4Inv(bor_mat4_t *m);

/**
 * Computes invertion matrix and stores it in m:
 * m = inv(a)
 * Returns 0 on success, -1 if matrix is singular.
 */
int ferMat4Inv2(bor_mat4_t *m, const bor_mat4_t *a);


/**
 * Multiplies 4D vector by matrix (vectors are considered to be colunmal).
 * v = m * w
 */
_fer_inline void ferMat4MulVec(bor_vec4_t *v, const bor_mat4_t *m,
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
_fer_inline void ferMat4MulVec3(bor_vec3_t *v, const bor_mat4_t *m,
                                               const bor_vec3_t *w);


/**
 * Copies c'th column in col vector.
 */
_fer_inline void ferMat4CopyCol(bor_vec4_t *col, const bor_mat4_t *m, size_t c);

/**
 * Copies r'th row in row vector.
 */
_fer_inline void ferMat4CopyRow(bor_vec4_t *row, const bor_mat4_t *m, size_t r);

/**
 * Computes dot product of r'th row of matrix m and c'th column of matrix n.
 */
_fer_inline bor_real_t ferMat4DotRowCol(const bor_mat4_t *m, size_t r,
                                        const bor_mat4_t *n, size_t c);

/**
 * Returns dot product of col'th column of matrix m with vector v.
 */
_fer_inline bor_real_t ferMat4DotCol(const bor_mat4_t *m, size_t c,
                                     const bor_vec4_t *v);

/**
 * Returns dot product of row'th row of matrix m with vector v.
 */
_fer_inline bor_real_t ferMat4DotRow(const bor_mat4_t *m, size_t r,
                                     const bor_vec4_t *v);


/**** INLINES ****/
_fer_inline bor_mat4_t *ferMat4Clone(const bor_mat4_t *m)
{
    bor_mat4_t *n;

    n = ferMat4New();
    ferMat4Copy(n, m);
    return n;
}

_fer_inline void ferMat4Copy(bor_mat4_t *d, const bor_mat4_t *s)
{
    *d = *s;
}


_fer_inline bor_real_t ferMat4Get(const bor_mat4_t *m, size_t row, size_t col)
{
    return m->f[row * 4 + col];
}

_fer_inline void ferMat4Set1(bor_mat4_t *m, size_t row, size_t col,
                             bor_real_t val)
{
    m->f[row * 4 + col] = val;
}

_fer_inline void ferMat4SetAll(bor_mat4_t *m, bor_real_t val)
{
    size_t i;
    for (i = 0; i < 4 * 4; i++){
        m->f[i] = val;
    }
}

_fer_inline void ferMat4Set(bor_mat4_t *m,
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

_fer_inline void ferMat4SetIdentity(bor_mat4_t *m)
{
    ferMat4Set(m, FER_ONE, FER_ZERO, FER_ZERO, FER_ZERO,
                  FER_ZERO, FER_ONE, FER_ZERO, FER_ZERO,
                  FER_ZERO, FER_ZERO, FER_ONE, FER_ZERO,
                  FER_ZERO, FER_ZERO, FER_ZERO, FER_ONE);
}

_fer_inline void ferMat4SetZero(bor_mat4_t *m)
{
    ferMat4SetAll(m, FER_ZERO);
}

_fer_inline void ferMat4SetScale(bor_mat4_t *m, bor_real_t s)
{
    ferMat4Set(m, s, FER_ZERO, FER_ZERO, FER_ZERO,
                  FER_ZERO, s, FER_ZERO, FER_ZERO,
                  FER_ZERO, FER_ZERO, s, FER_ZERO,
                  FER_ZERO, FER_ZERO, FER_ZERO, FER_ONE);
}

_fer_inline void ferMat4SetDiag(bor_mat4_t *m, bor_real_t s)
{
    ferMat4Set(m, s, FER_ZERO, FER_ZERO, FER_ZERO,
                  FER_ZERO, s, FER_ZERO, FER_ZERO,
                  FER_ZERO, FER_ZERO, s, FER_ZERO,
                  FER_ZERO, FER_ZERO, FER_ZERO, s);
}

_fer_inline void ferMat4SetTranslate(bor_mat4_t *m, const bor_vec3_t *v)
{
    ferMat4Set(m, FER_ONE, FER_ZERO, FER_ZERO, ferVec3X(v),
                  FER_ZERO, FER_ONE, FER_ZERO, ferVec3Y(v),
                  FER_ZERO, FER_ZERO, FER_ONE, ferVec3Z(v),
                  FER_ZERO, FER_ZERO, FER_ZERO, FER_ONE);
}

_fer_inline void ferMat4SetRot(bor_mat4_t *m,
                               bor_real_t angle, const bor_vec3_t *axis)
{
    bor_real_t x2, y2, z2, len2;
    bor_real_t k1, k2, k3, k1xy, k1xz, k1yz, k3x, k3y, k3z;

    x2 = ferVec3X(axis) * ferVec3X(axis);
    y2 = ferVec3Y(axis) * ferVec3Y(axis);
    z2 = ferVec3Z(axis) * ferVec3Z(axis);
    len2 = x2 + y2 + z2;

    k2   = FER_COS(angle);
    k1   = (FER_ONE - k2) / len2;
    k3   = FER_SIN(angle) / FER_SQRT(len2);
    k1xy = k1 * ferVec3X(axis) * ferVec3Y(axis);
    k1xz = k1 * ferVec3X(axis) * ferVec3Z(axis);
    k1yz = k1 * ferVec3Y(axis) * ferVec3Z(axis);
    k3x  = k3 * ferVec3X(axis);
    k3y  = k3 * ferVec3Y(axis);
    k3z  = k3 * ferVec3Z(axis);

    m->f[0] = k1 * x2 + k2;
    m->f[1] = k1xy - k3z;
    m->f[2] = k1xz + k3y;
    m->f[3] = FER_ZERO;

    m->f[4] = k1xy + k3z;
    m->f[5] = k1 * y2 + k2;
    m->f[6] = k1yz - k3x;
    m->f[7] = FER_ZERO;

    m->f[8] = k1xy - k3y;
    m->f[9] = k1yz + k3x;
    m->f[10] = k1 * z2 + k2;
    m->f[11] = FER_ZERO;

    m->f[12] = FER_ZERO;
    m->f[13] = FER_ZERO;
    m->f[14] = FER_ZERO;
    m->f[15] = FER_ONE;
}

_fer_inline void ferMat4TrScale(bor_mat4_t *m, bor_real_t s)
{
    bor_mat4_t scale;
    ferMat4SetScale(&scale, s);
    ferMat4Compose(m, &scale);
}

_fer_inline void ferMat4Translate(bor_mat4_t *m, const bor_vec3_t *v)
{
    bor_mat4_t tr;
    ferMat4SetTranslate(&tr, v);
    ferMat4Compose(m, &tr);
}

_fer_inline void ferMat4Rot(bor_mat4_t *m,
                            bor_real_t angle, const bor_vec3_t *axis)
{
    bor_mat4_t rot;
    ferMat4SetRot(&rot, angle, axis);
    ferMat4Compose(m, &rot);
}


_fer_inline void ferMat4Compose(bor_mat4_t *A, const bor_mat4_t *B)
{
    ferMat4MulLeft(A, B);
}


_fer_inline void ferMat4Add(bor_mat4_t *a, const bor_mat4_t *b)
{
    size_t i;
    for (i = 0; i < 4; i++){
        ferVec4Add(a->v + i, b->v + i);
    }
}

_fer_inline void ferMat4Add2(bor_mat4_t *d, const bor_mat4_t *a,
                                            const bor_mat4_t *b)
{
    size_t i;
    for (i = 0; i < 4; i++){
        ferVec4Add2(d->v + i, a->v + i, b->v + i);
    }
}

_fer_inline void ferMat4Sub(bor_mat4_t *a, const bor_mat4_t *b)
{
    size_t i;
    for (i = 0; i < 4; i++){
        ferVec4Sub(a->v + i, b->v + i);
    }
}

_fer_inline void ferMat4Sub2(bor_mat4_t *d, const bor_mat4_t *a,
                                            const bor_mat4_t *b)
{
    size_t i;
    for (i = 0; i < 4; i++){
        ferVec4Sub2(d->v + i, a->v + i, b->v + i);
    }
}

_fer_inline void ferMat4Scale(bor_mat4_t *d, bor_real_t s)
{
    size_t i;
    for (i = 0; i < 4; i++){
        ferVec4Scale(d->v + i, s);
    }
}

_fer_inline void ferMat4Scale2(bor_mat4_t *d, const bor_mat4_t *a, bor_real_t s)
{
    size_t i;
    for (i = 0; i < 4; i++){
        ferVec4Scale2(d->v + i, a->v + i, s);
    }
}

_fer_inline void ferMat4AddConst(bor_mat4_t *d, bor_real_t c)
{
    size_t i;
    for (i = 0; i < 4; i++){
        ferVec4AddConst(d->v + i, c);
    }
}

_fer_inline void ferMat4AddConst2(bor_mat4_t *d, const bor_mat4_t *a, bor_real_t c)
{
    size_t i;
    for (i = 0; i < 4; i++){
        ferVec4AddConst2(d->v + i, a->v + i, c);
    }
}

_fer_inline void ferMat4SubConst(bor_mat4_t *d, bor_real_t c)
{
    size_t i;
    for (i = 0; i < 4; i++){
        ferVec4SubConst(d->v + i, c);
    }
}

_fer_inline void ferMat4SubConst2(bor_mat4_t *d, const bor_mat4_t *a, bor_real_t c)
{
    size_t i;
    for (i = 0; i < 4; i++){
        ferVec4SubConst2(d->v + i, a->v + i, c);
    }
}

_fer_inline void ferMat4Mul(bor_mat4_t *a, const bor_mat4_t *b)
{
    bor_vec4_t v;

    ferVec4Copy(&v, a->v + 0);
    a->f[0]  = ferMat4DotCol(b, 0, &v);
    a->f[1]  = ferMat4DotCol(b, 1, &v);
    a->f[2]  = ferMat4DotCol(b, 2, &v);
    a->f[3]  = ferMat4DotCol(b, 3, &v);

    ferVec4Copy(&v, a->v + 1);
    a->f[4]  = ferMat4DotCol(b, 0, &v);
    a->f[5]  = ferMat4DotCol(b, 1, &v);
    a->f[6]  = ferMat4DotCol(b, 2, &v);
    a->f[7]  = ferMat4DotCol(b, 3, &v);

    ferVec4Copy(&v, a->v + 2);
    a->f[8]  = ferMat4DotCol(b, 0, &v);
    a->f[9]  = ferMat4DotCol(b, 1, &v);
    a->f[10] = ferMat4DotCol(b, 2, &v);
    a->f[11] = ferMat4DotCol(b, 3, &v);

    ferVec4Copy(&v, a->v + 3);
    a->f[12] = ferMat4DotCol(b, 0, &v);
    a->f[13] = ferMat4DotCol(b, 1, &v);
    a->f[14] = ferMat4DotCol(b, 2, &v);
    a->f[15] = ferMat4DotCol(b, 3, &v);

}

_fer_inline void ferMat4Mul2(bor_mat4_t *d, const bor_mat4_t *a,
                                            const bor_mat4_t *b)
{
    bor_vec4_t v;

    ferMat4CopyCol(&v, b, 0);
    d->f[0]  = ferMat4DotRow(a, 0, &v);
    d->f[4]  = ferMat4DotRow(a, 1, &v);
    d->f[8]  = ferMat4DotRow(a, 2, &v);
    d->f[12] = ferMat4DotRow(a, 3, &v);

    ferMat4CopyCol(&v, b, 1);
    d->f[1]  = ferMat4DotRow(a, 0, &v);
    d->f[5]  = ferMat4DotRow(a, 1, &v);
    d->f[9]  = ferMat4DotRow(a, 2, &v);
    d->f[13] = ferMat4DotRow(a, 3, &v);

    ferMat4CopyCol(&v, b, 2);
    d->f[2]  = ferMat4DotRow(a, 0, &v);
    d->f[6]  = ferMat4DotRow(a, 1, &v);
    d->f[10] = ferMat4DotRow(a, 2, &v);
    d->f[14] = ferMat4DotRow(a, 3, &v);

    ferMat4CopyCol(&v, b, 3);
    d->f[3]  = ferMat4DotRow(a, 0, &v);
    d->f[7]  = ferMat4DotRow(a, 1, &v);
    d->f[11] = ferMat4DotRow(a, 2, &v);
    d->f[15] = ferMat4DotRow(a, 3, &v);
}

_fer_inline void ferMat4MulLeft(bor_mat4_t *a, const bor_mat4_t *b)
{
    bor_vec4_t v;

    ferMat4CopyCol(&v, a, 0);
    a->f[0]  = ferMat4DotRow(b, 0, &v);
    a->f[4]  = ferMat4DotRow(b, 1, &v);
    a->f[8]  = ferMat4DotRow(b, 2, &v);
    a->f[12] = ferMat4DotRow(b, 3, &v);

    ferMat4CopyCol(&v, a, 1);
    a->f[1]  = ferMat4DotRow(b, 0, &v);
    a->f[5]  = ferMat4DotRow(b, 1, &v);
    a->f[9]  = ferMat4DotRow(b, 2, &v);
    a->f[13] = ferMat4DotRow(b, 3, &v);

    ferMat4CopyCol(&v, a, 2);
    a->f[2]  = ferMat4DotRow(b, 0, &v);
    a->f[6]  = ferMat4DotRow(b, 1, &v);
    a->f[10] = ferMat4DotRow(b, 2, &v);
    a->f[14] = ferMat4DotRow(b, 3, &v);

    ferMat4CopyCol(&v, a, 3);
    a->f[3]  = ferMat4DotRow(b, 0, &v);
    a->f[7]  = ferMat4DotRow(b, 1, &v);
    a->f[11] = ferMat4DotRow(b, 2, &v);
    a->f[15] = ferMat4DotRow(b, 3, &v);
}

_fer_inline void ferMat4MulComp(bor_mat4_t *a, const bor_mat4_t *b)
{
    size_t i;
    for (i = 0; i < 4; i++){
        ferVec4MulComp(a->v + i, b->v + i);
    }
}

_fer_inline void ferMat4MulComp2(bor_mat4_t *d, const bor_mat4_t *a,
                                                const bor_mat4_t *b)
{
    size_t i;
    for (i = 0; i < 4; i++){
        ferVec4MulComp2(d->v + i, a->v + i, b->v + i);
    }
}

_fer_inline void ferMat4Trans(bor_mat4_t *d)
{
    bor_real_t a, b, c, e, f, g;

    a = d->f[1];
    b = d->f[2];
    c = d->f[6];
    e = d->f[3];
    f = d->f[7];
    g = d->f[11];
    ferMat4Set(d, d->f[0], d->f[4], d->f[8],  d->f[12],
                  a,       d->f[5], d->f[9],  d->f[13],
                  b,       c,       d->f[10], d->f[14],
                  e,       f,       g,        d->f[15]);
}

_fer_inline void ferMat4Trans2(bor_mat4_t *d, const bor_mat4_t *a)
{
    ferMat4CopyCol(d->v + 0, a, 0);
    ferMat4CopyCol(d->v + 1, a, 1);
    ferMat4CopyCol(d->v + 2, a, 2);
    ferMat4CopyCol(d->v + 3, a, 3);
}

_fer_inline int ferMat4Regular(const bor_mat4_t *m)
{
    bor_real_t det;
    det = ferMat4Det(m);
    return !ferIsZero(det);
}

_fer_inline int ferMat4Singular(const bor_mat4_t *m)
{
    return !ferMat4Regular(m);
}

_fer_inline bor_real_t ferMat4Det(const bor_mat4_t *m)
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

_fer_inline int ferMat4Inv(bor_mat4_t *m)
{
    bor_mat4_t n;
    ferMat4Copy(&n, m);
    return ferMat4Inv2(m, &n);
}

_fer_inline void ferMat4MulVec(bor_vec4_t *v, const bor_mat4_t *m,
                                              const bor_vec4_t *w)
{
    ferVec4SetX(v, ferVec4Dot(m->v + 0, w));
    ferVec4SetY(v, ferVec4Dot(m->v + 1, w));
    ferVec4SetZ(v, ferVec4Dot(m->v + 2, w));
    ferVec4SetW(v, ferVec4Dot(m->v + 3, w));
}

_fer_inline void ferMat4MulVec3(bor_vec3_t *v, const bor_mat4_t *m,
                                               const bor_vec3_t *_w)
{
    bor_real_t denom;
    bor_vec4_t w;

    ferVec4SetX(&w, ferVec3X(_w));
    ferVec4SetY(&w, ferVec3Y(_w));
    ferVec4SetZ(&w, ferVec3Z(_w));
    ferVec4SetW(&w, FER_ONE);

    ferVec3SetX(v, ferVec4Dot(m->v + 0, &w));
    ferVec3SetY(v, ferVec4Dot(m->v + 1, &w));
    ferVec3SetZ(v, ferVec4Dot(m->v + 2, &w));
    denom = ferVec4Dot(m->v + 3, &w);
    ferVec3Scale(v, ferRecp(denom));
}



_fer_inline void ferMat4CopyCol(bor_vec4_t *col, const bor_mat4_t *m, size_t c)
{
    ferVec4Set(col, m->f[c], m->f[c + 4], m->f[c + 8], m->f[c + 12]);
}

_fer_inline void ferMat4CopyRow(bor_vec4_t *row, const bor_mat4_t *m, size_t r)
{
    ferVec4Copy(row, m->v + r);
}

_fer_inline bor_real_t ferMat4DotRowCol(const bor_mat4_t *m, size_t r,
                                        const bor_mat4_t *n, size_t c)
{
    bor_vec4_t col;
    ferMat4CopyCol(&col, n, c);
    return ferVec4Dot(m->v + r, &col);
}

_fer_inline bor_real_t ferMat4DotCol(const bor_mat4_t *m, size_t c,
                                     const bor_vec4_t *v)
{
    bor_vec4_t col;
    ferMat4CopyCol(&col, m, c);
    return ferVec4Dot(&col, v);
}

_fer_inline bor_real_t ferMat4DotRow(const bor_mat4_t *m, size_t r,
                                     const bor_vec4_t *v)
{
    return ferVec4Dot(m->v + r, v);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_MAT4_H__ */
