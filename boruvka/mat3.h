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

#ifndef __FER_MAT3_H__
#define __FER_MAT3_H__

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
union _fer_mat3_t {
    fer_vec3_t v[3];
    fer_real_t f[3 * 4];
};
typedef union _fer_mat3_t fer_mat3_t;

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
 * Functions
 * ----------
 */

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
 * Sets rows of matrix
 */
_fer_inline void ferMat3SetRows(fer_mat3_t *m,
                                const fer_vec3_t *row1,
                                const fer_vec3_t *row2,
                                const fer_vec3_t *row3);

/**
 * Set identity matrix.
 * ~~~~
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
 * ~~~~
 *     | s 0 0 |
 * m = | 0 s 0 |
 *     | 0 0 1 |
 */
_fer_inline void ferMat3SetScale(fer_mat3_t *m, fer_real_t s);

/**
 * Set diagonal.
 * ~~~~
 *     | s 0 0 |
 * m = | 0 s 0 |
 *     | 0 0 s |
 */
_fer_inline void ferMat3SetDiag(fer_mat3_t *m, fer_real_t s);

/**
 * Set translation matrix (translation along v vector).
 * ~~~~
 *     | 1 0 x |
 * m = | 0 1 y |
 *     | 0 0 1 |
 */
_fer_inline void ferMat3SetTranslate(fer_mat3_t *m, const fer_vec2_t *v);

/**
 * Set rotation matrix (2D).
 * ~~~~
 *     | cos(a) -sin(a) 0 |
 * m = | sin(a)  cos(a) 0 |
 *     |   0       0    1 |
 */
_fer_inline void ferMat3SetRot(fer_mat3_t *m, fer_real_t angle);

/**
 * Set rotation matrix (3D) defined by rotation about x, y, z axis
 * respectively.
 */
_fer_inline void ferMat3SetRot3D(fer_mat3_t *m,
                                 fer_real_t ax, fer_real_t ay, fer_real_t az);

/**
 * Apply scale transfomation.
 * ~~~~
 *     | s 0 0 |
 * m = | 0 s 0 | . m
 *     | 0 0 1 |
 */
_fer_inline void ferMat3TrScale(fer_mat3_t *m, fer_real_t s);

/**
 * Translate transformation matrix towards vector v.
 * ~~~~
 *     | 1 0 x |
 * m = | 0 1 y | . m
 *     | 0 0 1 |
 */
_fer_inline void ferMat3Translate(fer_mat3_t *m, const fer_vec2_t *v);

/**
 * Rotate transformation matrix.
 * ~~~~
 *     | cos(a) -sin(a) 0 |
 * m = | sin(a)  cos(a) 0 | . m
 *     |   0       0    1 |
 */
_fer_inline void ferMat3Rot(fer_mat3_t *m, fer_real_t angle);


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
 * d = d - c
 */
_fer_inline void ferMat3SubConst(fer_mat3_t *d, fer_real_t c);

/**
 * d = a - c
 */
_fer_inline void ferMat3SubConst2(fer_mat3_t *d, const fer_mat3_t *a, fer_real_t c);


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
 * a = b^t * a
 */
_fer_inline void ferMat3MulLeftTrans(fer_mat3_t *a, const fer_mat3_t *b);

/**
 * d = a^t * b
 */
_fer_inline void ferMat3MulTrans2(fer_mat3_t *d,
                                  const fer_mat3_t *a, const fer_mat3_t *b);

/**
 * Multiplies {a} by matrix defined by three column vectors.
 */
_fer_inline void ferMat3MulColVecs2(fer_mat3_t *d, const fer_mat3_t *a,
                                    const fer_vec3_t *col1,
                                    const fer_vec3_t *col2,
                                    const fer_vec3_t *col3);

/**
 * Multiplies {a} from left by matrix defined by three row vectors.
 * ~~~~~
 *     |row1|
 * d = |row2| . a
 *     |row3|
 */
_fer_inline void ferMat3MulLeftRowVecs2(fer_mat3_t *d, const fer_mat3_t *a,
                                        const fer_vec3_t *row1,
                                        const fer_vec3_t *row2,
                                        const fer_vec3_t *row3);


/**
 * Multiplies two matrices by components.
 * ~~~~
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
 * m = inv(a)
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
 * Multiplies 3D vector by transposed matrix (vectors are considered to be
 * colunmal).
 * v = m^t * w
 */
_fer_inline void ferMat3MulVecTrans(fer_vec3_t *v, const fer_mat3_t *m,
                                                   const fer_vec3_t *w);

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
_fer_inline void ferMat3MulVec2(fer_vec2_t *v, const fer_mat3_t *m,
                                               const fer_vec2_t *w);

/**
 * dst = ABS(src)
 */
_fer_inline void ferMat3Abs2(fer_mat3_t *dst, const fer_mat3_t *src);

/**
 * Compute eigenvectors of a given matrix.
 * If eigenvals is non-NULL it is filled with eigen values.
 *
 * Jacobi Rotation Algorithm is used.
 */
int ferMat3Eigen(const fer_mat3_t *_m, fer_mat3_t *eigen,
                 fer_vec3_t *eigenvals);

/**
 * Computes dot product of r'th row of matrix m and c'th column of matrix n.
 */
_fer_inline fer_real_t ferMat3DotRowCol(const fer_mat3_t *m, size_t r,
                                        const fer_mat3_t *n, size_t c);


_fer_inline void ferMat3CopyCol(fer_vec3_t *col, const fer_mat3_t *m, size_t c);
_fer_inline void ferMat3CopyRow(fer_vec3_t *row, const fer_mat3_t *m, size_t r);

/**
 * Returns dot product of col'th column of matrix m with vector (a, b, c).
 */
_fer_inline fer_real_t ferMat3DotCol(const fer_mat3_t *m, size_t col,
                                     const fer_vec3_t *v);

/**
 * Returns dot product of row'th row of matrix m with vector (a, b, c).
 */
_fer_inline fer_real_t ferMat3DotRow(const fer_mat3_t *m, size_t row,
                                     const fer_vec3_t *v);

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
    size_t i;
    for (i = 0; i < 3; i++){
        ferVec3Set(m->v + i, val, val, val);
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

_fer_inline void ferMat3SetRows(fer_mat3_t *m,
                                const fer_vec3_t *row1,
                                const fer_vec3_t *row2,
                                const fer_vec3_t *row3)
{
    ferVec3Copy(&m->v[0], row1);
    ferVec3Copy(&m->v[1], row2);
    ferVec3Copy(&m->v[2], row3);
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
                  FER_ZERO, FER_ZERO, FER_ONE);
}

_fer_inline void ferMat3SetDiag(fer_mat3_t *m, fer_real_t s)
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

_fer_inline void ferMat3SetRot3D(fer_mat3_t *m,
                                 fer_real_t ax, fer_real_t ay, fer_real_t az)
{
    m->f[0] = FER_COS(ay) * FER_COS(az);
    m->f[1]  = -FER_COS(ax) * FER_SIN(az);
    m->f[1] += FER_SIN(ax) * FER_SIN(ay) * FER_COS(az);
    m->f[2]  = FER_SIN(ax) * FER_SIN(az);
    m->f[2] += FER_COS(ax) * FER_SIN(ay) * FER_COS(az);

    m->f[4] = FER_COS(ay) * FER_SIN(az);
    m->f[5]  = FER_COS(ax) * FER_COS(az);
    m->f[5] += FER_SIN(ax) * FER_SIN(ay) * FER_SIN(az);
    m->f[6]  = -FER_SIN(ax) * FER_COS(az);
    m->f[6] += FER_COS(ax) * FER_SIN(ay) * FER_SIN(az);

    m->f[8] = -FER_SIN(ay);
    m->f[9] = FER_SIN(ax) * FER_COS(ay);
    m->f[10] = FER_COS(ax) * FER_COS(ay);
}

_fer_inline void ferMat3TrScale(fer_mat3_t *m, fer_real_t s)
{
    fer_mat3_t scale;
    ferMat3SetScale(&scale, s);
    ferMat3Compose(m, &scale);
}

_fer_inline void ferMat3Translate(fer_mat3_t *m, const fer_vec2_t *v)
{
    fer_mat3_t tr;
    ferMat3SetTranslate(&tr, v);
    ferMat3Compose(m, &tr);
}

_fer_inline void ferMat3Rot(fer_mat3_t *m, fer_real_t angle)
{
    fer_mat3_t rot;
    ferMat3SetRot(&rot, angle);
    ferMat3Compose(m, &rot);
}


_fer_inline void ferMat3Compose(fer_mat3_t *A, const fer_mat3_t *B)
{
    ferMat3MulLeft(A, B);
}


_fer_inline void ferMat3Add(fer_mat3_t *a, const fer_mat3_t *b)
{
    ferVec3Add(a->v + 0, b->v + 0);
    ferVec3Add(a->v + 1, b->v + 1);
    ferVec3Add(a->v + 2, b->v + 2);
}

_fer_inline void ferMat3Add2(fer_mat3_t *d, const fer_mat3_t *a,
                                            const fer_mat3_t *b)
{
    ferVec3Add2(d->v + 0, a->v + 0, b->v + 0);
    ferVec3Add2(d->v + 1, a->v + 1, b->v + 1);
    ferVec3Add2(d->v + 2, a->v + 2, b->v + 2);
}

_fer_inline void ferMat3Sub(fer_mat3_t *a, const fer_mat3_t *b)
{
    ferVec3Sub(a->v + 0, b->v + 0);
    ferVec3Sub(a->v + 1, b->v + 1);
    ferVec3Sub(a->v + 2, b->v + 2);
}

_fer_inline void ferMat3Sub2(fer_mat3_t *d, const fer_mat3_t *a,
                                            const fer_mat3_t *b)
{
    ferVec3Sub2(d->v + 0, a->v + 0, b->v + 0);
    ferVec3Sub2(d->v + 1, a->v + 1, b->v + 1);
    ferVec3Sub2(d->v + 2, a->v + 2, b->v + 2);
}

_fer_inline void ferMat3Scale(fer_mat3_t *d, fer_real_t s)
{
    ferVec3Scale(d->v + 0, s);
    ferVec3Scale(d->v + 1, s);
    ferVec3Scale(d->v + 2, s);
}

_fer_inline void ferMat3Scale2(fer_mat3_t *d, const fer_mat3_t *a, fer_real_t s)
{
    ferVec3Scale2(d->v + 0, a->v + 0, s);
    ferVec3Scale2(d->v + 1, a->v + 1, s);
    ferVec3Scale2(d->v + 2, a->v + 2, s);
}

_fer_inline void ferMat3AddConst(fer_mat3_t *d, fer_real_t c)
{
    ferVec3AddConst(d->v + 0, c);
    ferVec3AddConst(d->v + 1, c);
    ferVec3AddConst(d->v + 2, c);
}

_fer_inline void ferMat3AddConst2(fer_mat3_t *d, const fer_mat3_t *a, fer_real_t c)
{
    ferVec3AddConst2(d->v + 0, a->v + 0, c);
    ferVec3AddConst2(d->v + 1, a->v + 1, c);
    ferVec3AddConst2(d->v + 2, a->v + 2, c);
}

_fer_inline void ferMat3SubConst(fer_mat3_t *d, fer_real_t c)
{
    ferVec3SubConst(d->v + 0, c);
    ferVec3SubConst(d->v + 1, c);
    ferVec3SubConst(d->v + 2, c);
}

_fer_inline void ferMat3SubConst2(fer_mat3_t *d, const fer_mat3_t *a, fer_real_t c)
{
    ferVec3SubConst2(d->v + 0, a->v + 0, c);
    ferVec3SubConst2(d->v + 1, a->v + 1, c);
    ferVec3SubConst2(d->v + 2, a->v + 2, c);
}

_fer_inline void ferMat3Mul(fer_mat3_t *a, const fer_mat3_t *b)
{
    fer_vec3_t v;

    ferVec3Copy(&v, a->v + 0);
    a->f[0]  = ferMat3DotCol(b, 0, &v);
    a->f[1]  = ferMat3DotCol(b, 1, &v);
    a->f[2]  = ferMat3DotCol(b, 2, &v);

    ferVec3Copy(&v, a->v + 1);
    a->f[4]  = ferMat3DotCol(b, 0, &v);
    a->f[5]  = ferMat3DotCol(b, 1, &v);
    a->f[6]  = ferMat3DotCol(b, 2, &v);

    ferVec3Copy(&v, a->v + 2);
    a->f[8]  = ferMat3DotCol(b, 0, &v);
    a->f[9]  = ferMat3DotCol(b, 1, &v);
    a->f[10] = ferMat3DotCol(b, 2, &v);

}

_fer_inline void ferMat3Mul2(fer_mat3_t *d, const fer_mat3_t *a,
                                            const fer_mat3_t *b)
{
    fer_vec3_t v;

    ferMat3CopyCol(&v, b, 0);
    d->f[0]  = ferMat3DotRow(a, 0, &v);
    d->f[4]  = ferMat3DotRow(a, 1, &v);
    d->f[8]  = ferMat3DotRow(a, 2, &v);

    ferMat3CopyCol(&v, b, 1);
    d->f[1]  = ferMat3DotRow(a, 0, &v);
    d->f[5]  = ferMat3DotRow(a, 1, &v);
    d->f[9]  = ferMat3DotRow(a, 2, &v);

    ferMat3CopyCol(&v, b, 2);
    d->f[2]  = ferMat3DotRow(a, 0, &v);
    d->f[6]  = ferMat3DotRow(a, 1, &v);
    d->f[10] = ferMat3DotRow(a, 2, &v);
}

_fer_inline void ferMat3MulLeft(fer_mat3_t *a, const fer_mat3_t *b)
{
    fer_vec3_t v;

    ferMat3CopyCol(&v, a, 0);
    a->f[0]  = ferMat3DotRow(b, 0, &v);
    a->f[4]  = ferMat3DotRow(b, 1, &v);
    a->f[8]  = ferMat3DotRow(b, 2, &v);

    ferMat3CopyCol(&v, a, 1);
    a->f[1]  = ferMat3DotRow(b, 0, &v);
    a->f[5]  = ferMat3DotRow(b, 1, &v);
    a->f[9]  = ferMat3DotRow(b, 2, &v);

    ferMat3CopyCol(&v, a, 2);
    a->f[2]  = ferMat3DotRow(b, 0, &v);
    a->f[6]  = ferMat3DotRow(b, 1, &v);
    a->f[10] = ferMat3DotRow(b, 2, &v);

}

_fer_inline void ferMat3MulLeftTrans(fer_mat3_t *a, const fer_mat3_t *b)
{
    fer_vec3_t v;

    ferMat3CopyCol(&v, a, 0);
    a->f[0]  = ferMat3DotCol(b, 0, &v);
    a->f[4]  = ferMat3DotCol(b, 1, &v);
    a->f[8]  = ferMat3DotCol(b, 2, &v);

    ferMat3CopyCol(&v, a, 1);
    a->f[1]  = ferMat3DotCol(b, 0, &v);
    a->f[5]  = ferMat3DotCol(b, 1, &v);
    a->f[9]  = ferMat3DotCol(b, 2, &v);

    ferMat3CopyCol(&v, a, 2);
    a->f[2]  = ferMat3DotCol(b, 0, &v);
    a->f[6]  = ferMat3DotCol(b, 1, &v);
    a->f[10] = ferMat3DotCol(b, 2, &v);
}

_fer_inline void ferMat3MulTrans2(fer_mat3_t *d,
                                  const fer_mat3_t *a, const fer_mat3_t *b)
{
    fer_vec3_t v;

    ferMat3CopyCol(&v, b, 0);
    d->f[0]  = ferMat3DotCol(a, 0, &v);
    d->f[4]  = ferMat3DotCol(a, 1, &v);
    d->f[8]  = ferMat3DotCol(a, 2, &v);

    ferMat3CopyCol(&v, b, 1);
    d->f[1]  = ferMat3DotCol(a, 0, &v);
    d->f[5]  = ferMat3DotCol(a, 1, &v);
    d->f[9]  = ferMat3DotCol(a, 2, &v);

    ferMat3CopyCol(&v, b, 2);
    d->f[2]  = ferMat3DotCol(a, 0, &v);
    d->f[6]  = ferMat3DotCol(a, 1, &v);
    d->f[10] = ferMat3DotCol(a, 2, &v);
}

_fer_inline void ferMat3MulColVecs2(fer_mat3_t *d, const fer_mat3_t *a,
                                    const fer_vec3_t *col1,
                                    const fer_vec3_t *col2,
                                    const fer_vec3_t *col3)
{
    d->f[0]  = ferMat3DotRow(a, 0, col1);
    d->f[1]  = ferMat3DotRow(a, 0, col2);
    d->f[2]  = ferMat3DotRow(a, 0, col3);

    d->f[4]  = ferMat3DotRow(a, 1, col1);
    d->f[5]  = ferMat3DotRow(a, 1, col2);
    d->f[6]  = ferMat3DotRow(a, 1, col3);

    d->f[8]  = ferMat3DotRow(a, 2, col1);
    d->f[9]  = ferMat3DotRow(a, 2, col2);
    d->f[10] = ferMat3DotRow(a, 2, col3);
}

_fer_inline void ferMat3MulLeftRowVecs2(fer_mat3_t *d, const fer_mat3_t *a,
                                        const fer_vec3_t *row1,
                                        const fer_vec3_t *row2,
                                        const fer_vec3_t *row3)
{
    d->f[0]  = ferMat3DotCol(a, 0, row1);
    d->f[1]  = ferMat3DotCol(a, 1, row1);
    d->f[2]  = ferMat3DotCol(a, 2, row1);

    d->f[4]  = ferMat3DotCol(a, 0, row2);
    d->f[5]  = ferMat3DotCol(a, 1, row2);
    d->f[6]  = ferMat3DotCol(a, 2, row2);

    d->f[8]  = ferMat3DotCol(a, 0, row3);
    d->f[9]  = ferMat3DotCol(a, 1, row3);
    d->f[10] = ferMat3DotCol(a, 2, row3);
}

_fer_inline void ferMat3MulComp(fer_mat3_t *a, const fer_mat3_t *b)
{
    ferVec3MulComp(a->v + 0, b->v + 0);
    ferVec3MulComp(a->v + 1, b->v + 1);
    ferVec3MulComp(a->v + 2, b->v + 2);
}

_fer_inline void ferMat3MulComp2(fer_mat3_t *d, const fer_mat3_t *a,
                                                const fer_mat3_t *b)
{
    ferVec3MulComp2(d->v + 0, a->v + 0, b->v + 0);
    ferVec3MulComp2(d->v + 1, a->v + 1, b->v + 1);
    ferVec3MulComp2(d->v + 2, a->v + 2, b->v + 2);
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
    ferMat3CopyCol(d->v + 0, a, 0);
    ferMat3CopyCol(d->v + 1, a, 1);
    ferMat3CopyCol(d->v + 2, a, 2);
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
    ferVec3SetX(v, ferMat3DotRow(m, 0, w));
    ferVec3SetY(v, ferMat3DotRow(m, 1, w));
    ferVec3SetZ(v, ferMat3DotRow(m, 2, w));
}

_fer_inline void ferMat3MulVecTrans(fer_vec3_t *v, const fer_mat3_t *m,
                                                   const fer_vec3_t *w)
{
    ferVec3SetX(v, ferMat3DotCol(m, 0, w));
    ferVec3SetY(v, ferMat3DotCol(m, 1, w));
    ferVec3SetZ(v, ferMat3DotCol(m, 2, w));
}

_fer_inline void ferMat3MulVec2(fer_vec2_t *v, const fer_mat3_t *m,
                                               const fer_vec2_t *_w)
{
    fer_real_t denom;
    fer_vec3_t w;

    ferVec3Set(&w, ferVec2X(_w), ferVec2Y(_w), FER_ONE);

    ferVec2SetX(v, ferMat3DotRow(m, 0, &w));
    ferVec2SetY(v, ferMat3DotRow(m, 1, &w));
    denom = ferMat3DotRow(m, 2, &w);
    ferVec2Scale(v, ferRecp(denom));
}


_fer_inline void ferMat3Abs2(fer_mat3_t *dst, const fer_mat3_t *src)
{
    size_t i, j;

    for (i = 0; i < 3 * 4;){
        for (j = 0; j < 3; j++, i++){
            dst->f[i] = FER_FABS(src->f[i]);
        }
        i++;
    }
}



_fer_inline fer_real_t ferMat3DotRowCol(const fer_mat3_t *m, size_t r,
                                        const fer_mat3_t *n, size_t c)
{
    fer_vec3_t col;
    ferMat3CopyCol(&col, n, c);
    return ferVec3Dot(&col, m->v + r);
}

_fer_inline void ferMat3CopyCol(fer_vec3_t *col, const fer_mat3_t *m, size_t c)
{
    ferVec3Set(col, m->f[c], m->f[c + 4], m->f[c + 8]);
}

_fer_inline void ferMat3CopyRow(fer_vec3_t *row, const fer_mat3_t *m, size_t r)
{
    ferVec3Copy(row, m->v + r);
}

_fer_inline fer_real_t ferMat3DotCol(const fer_mat3_t *m, size_t c,
                                     const fer_vec3_t *v)
{
    fer_vec3_t col;
    ferMat3CopyCol(&col, m, c);
    return ferVec3Dot(&col, v);
}

_fer_inline fer_real_t ferMat3DotRow(const fer_mat3_t *m, size_t row,
                                     const fer_vec3_t *v)
{
    return ferVec3Dot(m->v + row, v);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_MAT3_H__ */
