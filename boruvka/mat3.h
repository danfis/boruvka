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

#ifndef __BOR_MAT3_H__
#define __BOR_MAT3_H__

#include <boruvka/core.h>
#include <boruvka/vec2.h>
#include <boruvka/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Mat3 - 3x3 matrix
 * ==================
 */
union _bor_mat3_t {
    bor_vec3_t v[3];
    bor_real_t f[3 * 4];
};
typedef union _bor_mat3_t bor_mat3_t;

/**
 * Holds identity matrix. Read only variable!
 */
extern const bor_mat3_t *bor_mat3_identity;

/**
 * Holds zero matrix. Read only variable!
 */
extern const bor_mat3_t *bor_mat3_zero;

#define BOR_MAT3_STATIC(f11, f12, f13, \
                        f21, f22, f23, \
                        f31, f32, f33) \
    { .f = { (f11), (f12), (f13), BOR_ZERO, \
             (f21), (f22), (f23), BOR_ZERO, \
             (f31), (f32), (f33), BOR_ZERO } }

#define BOR_MAT3(name, f11, f12, f13, \
                       f21, f22, f23, \
                       f31, f32, f33) \
    bor_mat3_t name = BOR_MAT3_STATIC(f11, f12, f13, \
                                      f21, f22, f23, \
                                      f31, f32, f33)

/**
 * Functions
 * ----------
 */

/**
 * Allocates uninitialized new matrix.
 */
bor_mat3_t *borMat3New(void);

/**
 * Deletes matrix.
 */
void borMat3Del(bor_mat3_t *m);

/**
 * Clones given matrix.
 */
_bor_inline bor_mat3_t *borMat3Clone(const bor_mat3_t *m);

/**
 * Copies matrix from s to d.
 */
_bor_inline void borMat3Copy(bor_mat3_t *d, const bor_mat3_t *s);


/**
 * Returns element from specified position.
 */
_bor_inline bor_real_t borMat3Get(const bor_mat3_t *m, size_t row, size_t col);

/**
 * Sets element at specified position.
 */
_bor_inline void borMat3Set1(bor_mat3_t *m, size_t row, size_t col,
                             bor_real_t val);

/**
 * Sets all elements to val.
 */
_bor_inline void borMat3SetAll(bor_mat3_t *m, bor_real_t val);

/**
 * Sets whole matrix.
 */
_bor_inline void borMat3Set(bor_mat3_t *m,
                            bor_real_t f11, bor_real_t f12, bor_real_t f13,
                            bor_real_t f21, bor_real_t f22, bor_real_t f23,
                            bor_real_t f31, bor_real_t f32, bor_real_t f33);

/**
 * Sets rows of matrix
 */
_bor_inline void borMat3SetRows(bor_mat3_t *m,
                                const bor_vec3_t *row1,
                                const bor_vec3_t *row2,
                                const bor_vec3_t *row3);

/**
 * Set identity matrix.
 * ~~~~
 *     | 1 0 0 |
 * m = | 0 1 0 |
 *     | 0 0 1 |
 */
_bor_inline void borMat3SetIdentity(bor_mat3_t *m);

/**
 * Set zero matrix.
 */
_bor_inline void borMat3SetZero(bor_mat3_t *m);

/**
 * Set scale matrix.
 * ~~~~
 *     | s 0 0 |
 * m = | 0 s 0 |
 *     | 0 0 1 |
 */
_bor_inline void borMat3SetScale(bor_mat3_t *m, bor_real_t s);

/**
 * Set diagonal.
 * ~~~~
 *     | s 0 0 |
 * m = | 0 s 0 |
 *     | 0 0 s |
 */
_bor_inline void borMat3SetDiag(bor_mat3_t *m, bor_real_t s);

/**
 * Set translation matrix (translation along v vector).
 * ~~~~
 *     | 1 0 x |
 * m = | 0 1 y |
 *     | 0 0 1 |
 */
_bor_inline void borMat3SetTranslate(bor_mat3_t *m, const bor_vec2_t *v);

/**
 * Set rotation matrix (2D).
 * ~~~~
 *     | cos(a) -sin(a) 0 |
 * m = | sin(a)  cos(a) 0 |
 *     |   0       0    1 |
 */
_bor_inline void borMat3SetRot(bor_mat3_t *m, bor_real_t angle);

/**
 * Set rotation matrix (3D) defined by rotation about x, y, z axis
 * respectively.
 */
_bor_inline void borMat3SetRot3D(bor_mat3_t *m,
                                 bor_real_t ax, bor_real_t ay, bor_real_t az);

/**
 * Apply scale transfomation.
 * ~~~~
 *     | s 0 0 |
 * m = | 0 s 0 | . m
 *     | 0 0 1 |
 */
_bor_inline void borMat3TrScale(bor_mat3_t *m, bor_real_t s);

/**
 * Translate transformation matrix towards vector v.
 * ~~~~
 *     | 1 0 x |
 * m = | 0 1 y | . m
 *     | 0 0 1 |
 */
_bor_inline void borMat3Translate(bor_mat3_t *m, const bor_vec2_t *v);

/**
 * Rotate transformation matrix.
 * ~~~~
 *     | cos(a) -sin(a) 0 |
 * m = | sin(a)  cos(a) 0 | . m
 *     |   0       0    1 |
 */
_bor_inline void borMat3Rot(bor_mat3_t *m, bor_real_t angle);


/**
 * Composes transformation:
 * A = B . A
 */
_bor_inline void borMat3Compose(bor_mat3_t *A, const bor_mat3_t *B);


/**
 * a = a + b
 */
_bor_inline void borMat3Add(bor_mat3_t *a, const bor_mat3_t *b);

/**
 * d = a + b
 */
_bor_inline void borMat3Add2(bor_mat3_t *d, const bor_mat3_t *a,
                                            const bor_mat3_t *b);

/**
 * a = a - b
 */
_bor_inline void borMat3Sub(bor_mat3_t *a, const bor_mat3_t *b);

/**
 * d = a - b
 */
_bor_inline void borMat3Sub2(bor_mat3_t *d, const bor_mat3_t *a,
                                            const bor_mat3_t *b);

/**
 * d = d * s
 */
_bor_inline void borMat3Scale(bor_mat3_t *d, bor_real_t s);

/**
 * d = a * s
 */
_bor_inline void borMat3Scale2(bor_mat3_t *d, const bor_mat3_t *a, bor_real_t s);

/**
 * d = d + c
 */
_bor_inline void borMat3AddConst(bor_mat3_t *d, bor_real_t c);

/**
 * d = a + c
 */
_bor_inline void borMat3AddConst2(bor_mat3_t *d, const bor_mat3_t *a, bor_real_t c);

/**
 * d = d - c
 */
_bor_inline void borMat3SubConst(bor_mat3_t *d, bor_real_t c);

/**
 * d = a - c
 */
_bor_inline void borMat3SubConst2(bor_mat3_t *d, const bor_mat3_t *a, bor_real_t c);


/**
 * a = a * b
 */
_bor_inline void borMat3Mul(bor_mat3_t *a, const bor_mat3_t *b);

/**
 * d = a * b
 */
_bor_inline void borMat3Mul2(bor_mat3_t *d, const bor_mat3_t *a,
                                            const bor_mat3_t *b);

/**
 * a = b * a
 */
_bor_inline void borMat3MulLeft(bor_mat3_t *a, const bor_mat3_t *b);

/**
 * a = b^t * a
 */
_bor_inline void borMat3MulLeftTrans(bor_mat3_t *a, const bor_mat3_t *b);

/**
 * d = a^t * b
 */
_bor_inline void borMat3MulTrans2(bor_mat3_t *d,
                                  const bor_mat3_t *a, const bor_mat3_t *b);

/**
 * Multiplies {a} by matrix defined by three column vectors.
 */
_bor_inline void borMat3MulColVecs2(bor_mat3_t *d, const bor_mat3_t *a,
                                    const bor_vec3_t *col1,
                                    const bor_vec3_t *col2,
                                    const bor_vec3_t *col3);

/**
 * Multiplies {a} from left by matrix defined by three row vectors.
 * ~~~~~
 *     |row1|
 * d = |row2| . a
 *     |row3|
 */
_bor_inline void borMat3MulLeftRowVecs2(bor_mat3_t *d, const bor_mat3_t *a,
                                        const bor_vec3_t *row1,
                                        const bor_vec3_t *row2,
                                        const bor_vec3_t *row3);


/**
 * Multiplies two matrices by components.
 * ~~~~
 *     | a11*b11 a12*b12 a13*b13 |
 * a = | a21*b21 a22*b22 a23*b23 |
 *     | a31*b31 a32*b32 a33*313 |
 */
_bor_inline void borMat3MulComp(bor_mat3_t *a, const bor_mat3_t *b);
_bor_inline void borMat3MulComp2(bor_mat3_t *d, const bor_mat3_t *a,
                                                const bor_mat3_t *b);

/**
 * Transposes matrix.
 */
_bor_inline void borMat3Trans(bor_mat3_t *d);
_bor_inline void borMat3Trans2(bor_mat3_t *d, const bor_mat3_t *a);

/**
 * Returns true if matrix is regular.
 */
_bor_inline int borMat3Regular(const bor_mat3_t *m);

/**
 * Returns true if matrix is singular.
 */
_bor_inline int borMat3Singular(const bor_mat3_t *m);

/**
 * Returns determinant of matrix.
 */
_bor_inline bor_real_t borMat3Det(const bor_mat3_t *m);

/**
 * Inverts matrix.
 * Returns 0 on success, -1 if matrix is singular.
 */
_bor_inline int borMat3Inv(bor_mat3_t *m);

/**
 * Computes invertion matrix and stores it in m:
 * m = inv(a)
 * Returns 0 on success, -1 if matrix is singular.
 */
_bor_inline int borMat3Inv2(bor_mat3_t *m, const bor_mat3_t *a);


/**
 * Multiplies 3D vector by matrix (vectors are considered to be colunmal).
 * v = m * w
 */
_bor_inline void borMat3MulVec(bor_vec3_t *v, const bor_mat3_t *m,
                                              const bor_vec3_t *w);

/**
 * Multiplies 3D vector by transposed matrix (vectors are considered to be
 * colunmal).
 * v = m^t * w
 */
_bor_inline void borMat3MulVecTrans(bor_vec3_t *v, const bor_mat3_t *m,
                                                   const bor_vec3_t *w);

/**
 * Multiplies matrix with 2D vector.
 * Third coordinate is filled with 1 and after multiplication x and y
 * coordinate is divided by resulting third coordinate.
 * In fact, this operation is applying transform matrix on vector w.
 * ~~~~
 *         | w.x |
 * v = m * | w.y |
 *         |  1  |
 * v.x = v.x / v.z
 * v.y = v.y / v.z
 */
_bor_inline void borMat3MulVec2(bor_vec2_t *v, const bor_mat3_t *m,
                                               const bor_vec2_t *w);

/**
 * dst = ABS(src)
 */
_bor_inline void borMat3Abs2(bor_mat3_t *dst, const bor_mat3_t *src);

/**
 * Compute eigenvectors of a given matrix.
 * If eigenvals is non-NULL it is filled with eigen values.
 *
 * Jacobi Rotation Algorithm is used.
 */
int borMat3Eigen(const bor_mat3_t *_m, bor_mat3_t *eigen,
                 bor_vec3_t *eigenvals);

/**
 * Computes dot product of r'th row of matrix m and c'th column of matrix n.
 */
_bor_inline bor_real_t borMat3DotRowCol(const bor_mat3_t *m, size_t r,
                                        const bor_mat3_t *n, size_t c);


_bor_inline void borMat3CopyCol(bor_vec3_t *col, const bor_mat3_t *m, size_t c);
_bor_inline void borMat3CopyRow(bor_vec3_t *row, const bor_mat3_t *m, size_t r);

/**
 * Returns dot product of col'th column of matrix m with vector (a, b, c).
 */
_bor_inline bor_real_t borMat3DotCol(const bor_mat3_t *m, size_t col,
                                     const bor_vec3_t *v);

/**
 * Returns dot product of row'th row of matrix m with vector (a, b, c).
 */
_bor_inline bor_real_t borMat3DotRow(const bor_mat3_t *m, size_t row,
                                     const bor_vec3_t *v);

/**** INLINES ****/
_bor_inline bor_mat3_t *borMat3Clone(const bor_mat3_t *m)
{
    bor_mat3_t *n;

    n = borMat3New();
    borMat3Copy(n, m);
    return n;
}

_bor_inline void borMat3Copy(bor_mat3_t *d, const bor_mat3_t *s)
{
    *d = *s;
}


_bor_inline bor_real_t borMat3Get(const bor_mat3_t *m, size_t row, size_t col)
{
    return m->f[row * 4 + col];
}

_bor_inline void borMat3Set1(bor_mat3_t *m, size_t row, size_t col,
                             bor_real_t val)
{
    m->f[row * 4 + col] = val;
}

_bor_inline void borMat3SetAll(bor_mat3_t *m, bor_real_t val)
{
    size_t i;
    for (i = 0; i < 3; i++){
        borVec3Set(m->v + i, val, val, val);
    }
}

_bor_inline void borMat3Set(bor_mat3_t *m,
                            bor_real_t f11, bor_real_t f12, bor_real_t f13,
                            bor_real_t f21, bor_real_t f22, bor_real_t f23,
                            bor_real_t f31, bor_real_t f32, bor_real_t f33)
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

_bor_inline void borMat3SetRows(bor_mat3_t *m,
                                const bor_vec3_t *row1,
                                const bor_vec3_t *row2,
                                const bor_vec3_t *row3)
{
    borVec3Copy(&m->v[0], row1);
    borVec3Copy(&m->v[1], row2);
    borVec3Copy(&m->v[2], row3);
}

_bor_inline void borMat3SetIdentity(bor_mat3_t *m)
{
    borMat3Set(m, BOR_ONE, BOR_ZERO, BOR_ZERO,
                  BOR_ZERO, BOR_ONE, BOR_ZERO,
                  BOR_ZERO, BOR_ZERO, BOR_ONE);
}

_bor_inline void borMat3SetZero(bor_mat3_t *m)
{
    borMat3SetAll(m, BOR_ZERO);
}

_bor_inline void borMat3SetScale(bor_mat3_t *m, bor_real_t s)
{
    borMat3Set(m, s, BOR_ZERO, BOR_ZERO,
                  BOR_ZERO, s, BOR_ZERO,
                  BOR_ZERO, BOR_ZERO, BOR_ONE);
}

_bor_inline void borMat3SetDiag(bor_mat3_t *m, bor_real_t s)
{
    borMat3Set(m, s, BOR_ZERO, BOR_ZERO,
                  BOR_ZERO, s, BOR_ZERO,
                  BOR_ZERO, BOR_ZERO, s);
}

_bor_inline void borMat3SetTranslate(bor_mat3_t *m, const bor_vec2_t *v)
{
    borMat3Set(m, BOR_ONE, BOR_ZERO, borVec2X(v),
                  BOR_ZERO, BOR_ONE, borVec2Y(v),
                  BOR_ZERO, BOR_ZERO, BOR_ONE);
}

_bor_inline void borMat3SetRot(bor_mat3_t *m, bor_real_t angle)
{
    borMat3Set(m, BOR_COS(angle), -BOR_SIN(angle), BOR_ZERO,
                  BOR_SIN(angle),  BOR_COS(angle), BOR_ZERO,
                  BOR_ZERO, BOR_ZERO, BOR_ONE);
}

_bor_inline void borMat3SetRot3D(bor_mat3_t *m,
                                 bor_real_t ax, bor_real_t ay, bor_real_t az)
{
    m->f[0] = BOR_COS(ay) * BOR_COS(az);
    m->f[1]  = -BOR_COS(ax) * BOR_SIN(az);
    m->f[1] += BOR_SIN(ax) * BOR_SIN(ay) * BOR_COS(az);
    m->f[2]  = BOR_SIN(ax) * BOR_SIN(az);
    m->f[2] += BOR_COS(ax) * BOR_SIN(ay) * BOR_COS(az);

    m->f[4] = BOR_COS(ay) * BOR_SIN(az);
    m->f[5]  = BOR_COS(ax) * BOR_COS(az);
    m->f[5] += BOR_SIN(ax) * BOR_SIN(ay) * BOR_SIN(az);
    m->f[6]  = -BOR_SIN(ax) * BOR_COS(az);
    m->f[6] += BOR_COS(ax) * BOR_SIN(ay) * BOR_SIN(az);

    m->f[8] = -BOR_SIN(ay);
    m->f[9] = BOR_SIN(ax) * BOR_COS(ay);
    m->f[10] = BOR_COS(ax) * BOR_COS(ay);
}

_bor_inline void borMat3TrScale(bor_mat3_t *m, bor_real_t s)
{
    bor_mat3_t scale;
    borMat3SetScale(&scale, s);
    borMat3Compose(m, &scale);
}

_bor_inline void borMat3Translate(bor_mat3_t *m, const bor_vec2_t *v)
{
    bor_mat3_t tr;
    borMat3SetTranslate(&tr, v);
    borMat3Compose(m, &tr);
}

_bor_inline void borMat3Rot(bor_mat3_t *m, bor_real_t angle)
{
    bor_mat3_t rot;
    borMat3SetRot(&rot, angle);
    borMat3Compose(m, &rot);
}


_bor_inline void borMat3Compose(bor_mat3_t *A, const bor_mat3_t *B)
{
    borMat3MulLeft(A, B);
}


_bor_inline void borMat3Add(bor_mat3_t *a, const bor_mat3_t *b)
{
    borVec3Add(a->v + 0, b->v + 0);
    borVec3Add(a->v + 1, b->v + 1);
    borVec3Add(a->v + 2, b->v + 2);
}

_bor_inline void borMat3Add2(bor_mat3_t *d, const bor_mat3_t *a,
                                            const bor_mat3_t *b)
{
    borVec3Add2(d->v + 0, a->v + 0, b->v + 0);
    borVec3Add2(d->v + 1, a->v + 1, b->v + 1);
    borVec3Add2(d->v + 2, a->v + 2, b->v + 2);
}

_bor_inline void borMat3Sub(bor_mat3_t *a, const bor_mat3_t *b)
{
    borVec3Sub(a->v + 0, b->v + 0);
    borVec3Sub(a->v + 1, b->v + 1);
    borVec3Sub(a->v + 2, b->v + 2);
}

_bor_inline void borMat3Sub2(bor_mat3_t *d, const bor_mat3_t *a,
                                            const bor_mat3_t *b)
{
    borVec3Sub2(d->v + 0, a->v + 0, b->v + 0);
    borVec3Sub2(d->v + 1, a->v + 1, b->v + 1);
    borVec3Sub2(d->v + 2, a->v + 2, b->v + 2);
}

_bor_inline void borMat3Scale(bor_mat3_t *d, bor_real_t s)
{
    borVec3Scale(d->v + 0, s);
    borVec3Scale(d->v + 1, s);
    borVec3Scale(d->v + 2, s);
}

_bor_inline void borMat3Scale2(bor_mat3_t *d, const bor_mat3_t *a, bor_real_t s)
{
    borVec3Scale2(d->v + 0, a->v + 0, s);
    borVec3Scale2(d->v + 1, a->v + 1, s);
    borVec3Scale2(d->v + 2, a->v + 2, s);
}

_bor_inline void borMat3AddConst(bor_mat3_t *d, bor_real_t c)
{
    borVec3AddConst(d->v + 0, c);
    borVec3AddConst(d->v + 1, c);
    borVec3AddConst(d->v + 2, c);
}

_bor_inline void borMat3AddConst2(bor_mat3_t *d, const bor_mat3_t *a, bor_real_t c)
{
    borVec3AddConst2(d->v + 0, a->v + 0, c);
    borVec3AddConst2(d->v + 1, a->v + 1, c);
    borVec3AddConst2(d->v + 2, a->v + 2, c);
}

_bor_inline void borMat3SubConst(bor_mat3_t *d, bor_real_t c)
{
    borVec3SubConst(d->v + 0, c);
    borVec3SubConst(d->v + 1, c);
    borVec3SubConst(d->v + 2, c);
}

_bor_inline void borMat3SubConst2(bor_mat3_t *d, const bor_mat3_t *a, bor_real_t c)
{
    borVec3SubConst2(d->v + 0, a->v + 0, c);
    borVec3SubConst2(d->v + 1, a->v + 1, c);
    borVec3SubConst2(d->v + 2, a->v + 2, c);
}

_bor_inline void borMat3Mul(bor_mat3_t *a, const bor_mat3_t *b)
{
    bor_vec3_t v;

    borVec3Copy(&v, a->v + 0);
    a->f[0]  = borMat3DotCol(b, 0, &v);
    a->f[1]  = borMat3DotCol(b, 1, &v);
    a->f[2]  = borMat3DotCol(b, 2, &v);

    borVec3Copy(&v, a->v + 1);
    a->f[4]  = borMat3DotCol(b, 0, &v);
    a->f[5]  = borMat3DotCol(b, 1, &v);
    a->f[6]  = borMat3DotCol(b, 2, &v);

    borVec3Copy(&v, a->v + 2);
    a->f[8]  = borMat3DotCol(b, 0, &v);
    a->f[9]  = borMat3DotCol(b, 1, &v);
    a->f[10] = borMat3DotCol(b, 2, &v);

}

_bor_inline void borMat3Mul2(bor_mat3_t *d, const bor_mat3_t *a,
                                            const bor_mat3_t *b)
{
    bor_vec3_t v;

    borMat3CopyCol(&v, b, 0);
    d->f[0]  = borMat3DotRow(a, 0, &v);
    d->f[4]  = borMat3DotRow(a, 1, &v);
    d->f[8]  = borMat3DotRow(a, 2, &v);

    borMat3CopyCol(&v, b, 1);
    d->f[1]  = borMat3DotRow(a, 0, &v);
    d->f[5]  = borMat3DotRow(a, 1, &v);
    d->f[9]  = borMat3DotRow(a, 2, &v);

    borMat3CopyCol(&v, b, 2);
    d->f[2]  = borMat3DotRow(a, 0, &v);
    d->f[6]  = borMat3DotRow(a, 1, &v);
    d->f[10] = borMat3DotRow(a, 2, &v);
}

_bor_inline void borMat3MulLeft(bor_mat3_t *a, const bor_mat3_t *b)
{
    bor_vec3_t v;

    borMat3CopyCol(&v, a, 0);
    a->f[0]  = borMat3DotRow(b, 0, &v);
    a->f[4]  = borMat3DotRow(b, 1, &v);
    a->f[8]  = borMat3DotRow(b, 2, &v);

    borMat3CopyCol(&v, a, 1);
    a->f[1]  = borMat3DotRow(b, 0, &v);
    a->f[5]  = borMat3DotRow(b, 1, &v);
    a->f[9]  = borMat3DotRow(b, 2, &v);

    borMat3CopyCol(&v, a, 2);
    a->f[2]  = borMat3DotRow(b, 0, &v);
    a->f[6]  = borMat3DotRow(b, 1, &v);
    a->f[10] = borMat3DotRow(b, 2, &v);

}

_bor_inline void borMat3MulLeftTrans(bor_mat3_t *a, const bor_mat3_t *b)
{
    bor_vec3_t v;

    borMat3CopyCol(&v, a, 0);
    a->f[0]  = borMat3DotCol(b, 0, &v);
    a->f[4]  = borMat3DotCol(b, 1, &v);
    a->f[8]  = borMat3DotCol(b, 2, &v);

    borMat3CopyCol(&v, a, 1);
    a->f[1]  = borMat3DotCol(b, 0, &v);
    a->f[5]  = borMat3DotCol(b, 1, &v);
    a->f[9]  = borMat3DotCol(b, 2, &v);

    borMat3CopyCol(&v, a, 2);
    a->f[2]  = borMat3DotCol(b, 0, &v);
    a->f[6]  = borMat3DotCol(b, 1, &v);
    a->f[10] = borMat3DotCol(b, 2, &v);
}

_bor_inline void borMat3MulTrans2(bor_mat3_t *d,
                                  const bor_mat3_t *a, const bor_mat3_t *b)
{
    bor_vec3_t v;

    borMat3CopyCol(&v, b, 0);
    d->f[0]  = borMat3DotCol(a, 0, &v);
    d->f[4]  = borMat3DotCol(a, 1, &v);
    d->f[8]  = borMat3DotCol(a, 2, &v);

    borMat3CopyCol(&v, b, 1);
    d->f[1]  = borMat3DotCol(a, 0, &v);
    d->f[5]  = borMat3DotCol(a, 1, &v);
    d->f[9]  = borMat3DotCol(a, 2, &v);

    borMat3CopyCol(&v, b, 2);
    d->f[2]  = borMat3DotCol(a, 0, &v);
    d->f[6]  = borMat3DotCol(a, 1, &v);
    d->f[10] = borMat3DotCol(a, 2, &v);
}

_bor_inline void borMat3MulColVecs2(bor_mat3_t *d, const bor_mat3_t *a,
                                    const bor_vec3_t *col1,
                                    const bor_vec3_t *col2,
                                    const bor_vec3_t *col3)
{
    d->f[0]  = borMat3DotRow(a, 0, col1);
    d->f[1]  = borMat3DotRow(a, 0, col2);
    d->f[2]  = borMat3DotRow(a, 0, col3);

    d->f[4]  = borMat3DotRow(a, 1, col1);
    d->f[5]  = borMat3DotRow(a, 1, col2);
    d->f[6]  = borMat3DotRow(a, 1, col3);

    d->f[8]  = borMat3DotRow(a, 2, col1);
    d->f[9]  = borMat3DotRow(a, 2, col2);
    d->f[10] = borMat3DotRow(a, 2, col3);
}

_bor_inline void borMat3MulLeftRowVecs2(bor_mat3_t *d, const bor_mat3_t *a,
                                        const bor_vec3_t *row1,
                                        const bor_vec3_t *row2,
                                        const bor_vec3_t *row3)
{
    d->f[0]  = borMat3DotCol(a, 0, row1);
    d->f[1]  = borMat3DotCol(a, 1, row1);
    d->f[2]  = borMat3DotCol(a, 2, row1);

    d->f[4]  = borMat3DotCol(a, 0, row2);
    d->f[5]  = borMat3DotCol(a, 1, row2);
    d->f[6]  = borMat3DotCol(a, 2, row2);

    d->f[8]  = borMat3DotCol(a, 0, row3);
    d->f[9]  = borMat3DotCol(a, 1, row3);
    d->f[10] = borMat3DotCol(a, 2, row3);
}

_bor_inline void borMat3MulComp(bor_mat3_t *a, const bor_mat3_t *b)
{
    borVec3MulComp(a->v + 0, b->v + 0);
    borVec3MulComp(a->v + 1, b->v + 1);
    borVec3MulComp(a->v + 2, b->v + 2);
}

_bor_inline void borMat3MulComp2(bor_mat3_t *d, const bor_mat3_t *a,
                                                const bor_mat3_t *b)
{
    borVec3MulComp2(d->v + 0, a->v + 0, b->v + 0);
    borVec3MulComp2(d->v + 1, a->v + 1, b->v + 1);
    borVec3MulComp2(d->v + 2, a->v + 2, b->v + 2);
}

_bor_inline void borMat3Trans(bor_mat3_t *d)
{
    bor_real_t a, b, c;

    a = d->f[1];
    b = d->f[2];
    c = d->f[6];
    borMat3Set(d, d->f[0], d->f[4], d->f[8],
                  a,       d->f[5], d->f[9],
                  b,       c,       d->f[10]);
}

_bor_inline void borMat3Trans2(bor_mat3_t *d, const bor_mat3_t *a)
{
    borMat3CopyCol(d->v + 0, a, 0);
    borMat3CopyCol(d->v + 1, a, 1);
    borMat3CopyCol(d->v + 2, a, 2);
}

_bor_inline int borMat3Regular(const bor_mat3_t *m)
{
    bor_real_t det;
    det = borMat3Det(m);
    return !borIsZero(det);
}

_bor_inline int borMat3Singular(const bor_mat3_t *m)
{
    return !borMat3Regular(m);
}

_bor_inline bor_real_t borMat3Det(const bor_mat3_t *m)
{
    bor_real_t det;

    det =  m->f[0] * m->f[5] * m->f[10];
    det += m->f[1] * m->f[6] * m->f[8];
    det += m->f[2] * m->f[4] * m->f[9];
    det -= m->f[0] * m->f[6] * m->f[9];
    det -= m->f[1] * m->f[4] * m->f[10];
    det -= m->f[2] * m->f[5] * m->f[8];

    return det;
}

_bor_inline int borMat3Inv(bor_mat3_t *m)
{
    bor_mat3_t n;
    borMat3Copy(&n, m);
    return borMat3Inv2(m, &n);
}

_bor_inline int borMat3Inv2(bor_mat3_t *m, const bor_mat3_t *a)
{
    bor_real_t det, invdet;

    det = borMat3Det(a);
    if (borIsZero(det))
        return -1;

    invdet = borRecp(det);

    m->f[0]  = a->f[5] * a->f[10] - a->f[6] * a->f[9];
    m->f[1]  = a->f[2] * a->f[9]  - a->f[1] * a->f[10];
    m->f[2]  = a->f[1] * a->f[6]  - a->f[2] * a->f[5];
    m->f[4]  = a->f[6] * a->f[8]  - a->f[4] * a->f[10];
    m->f[5]  = a->f[0] * a->f[10] - a->f[2] * a->f[8];
    m->f[6]  = a->f[2] * a->f[4]  - a->f[0] * a->f[6];
    m->f[8]  = a->f[4] * a->f[9]  - a->f[5] * a->f[8];
    m->f[9]  = a->f[1] * a->f[8]  - a->f[0] * a->f[9];
    m->f[10] = a->f[0] * a->f[5]  - a->f[1] * a->f[4];

    borMat3Scale(m, invdet);

    return 0;
}

_bor_inline void borMat3MulVec(bor_vec3_t *v, const bor_mat3_t *m,
                                              const bor_vec3_t *w)
{
    borVec3SetX(v, borMat3DotRow(m, 0, w));
    borVec3SetY(v, borMat3DotRow(m, 1, w));
    borVec3SetZ(v, borMat3DotRow(m, 2, w));
}

_bor_inline void borMat3MulVecTrans(bor_vec3_t *v, const bor_mat3_t *m,
                                                   const bor_vec3_t *w)
{
    borVec3SetX(v, borMat3DotCol(m, 0, w));
    borVec3SetY(v, borMat3DotCol(m, 1, w));
    borVec3SetZ(v, borMat3DotCol(m, 2, w));
}

_bor_inline void borMat3MulVec2(bor_vec2_t *v, const bor_mat3_t *m,
                                               const bor_vec2_t *_w)
{
    bor_real_t denom;
    bor_vec3_t w;

    borVec3Set(&w, borVec2X(_w), borVec2Y(_w), BOR_ONE);

    borVec2SetX(v, borMat3DotRow(m, 0, &w));
    borVec2SetY(v, borMat3DotRow(m, 1, &w));
    denom = borMat3DotRow(m, 2, &w);
    borVec2Scale(v, borRecp(denom));
}


_bor_inline void borMat3Abs2(bor_mat3_t *dst, const bor_mat3_t *src)
{
    size_t i, j;

    for (i = 0; i < 3 * 4;){
        for (j = 0; j < 3; j++, i++){
            dst->f[i] = BOR_FABS(src->f[i]);
        }
        i++;
    }
}



_bor_inline bor_real_t borMat3DotRowCol(const bor_mat3_t *m, size_t r,
                                        const bor_mat3_t *n, size_t c)
{
    bor_vec3_t col;
    borMat3CopyCol(&col, n, c);
    return borVec3Dot(&col, m->v + r);
}

_bor_inline void borMat3CopyCol(bor_vec3_t *col, const bor_mat3_t *m, size_t c)
{
    borVec3Set(col, m->f[c], m->f[c + 4], m->f[c + 8]);
}

_bor_inline void borMat3CopyRow(bor_vec3_t *row, const bor_mat3_t *m, size_t r)
{
    borVec3Copy(row, m->v + r);
}

_bor_inline bor_real_t borMat3DotCol(const bor_mat3_t *m, size_t c,
                                     const bor_vec3_t *v)
{
    bor_vec3_t col;
    borMat3CopyCol(&col, m, c);
    return borVec3Dot(&col, v);
}

_bor_inline bor_real_t borMat3DotRow(const bor_mat3_t *m, size_t row,
                                     const bor_vec3_t *v)
{
    return borVec3Dot(m->v + row, v);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_MAT3_H__ */
