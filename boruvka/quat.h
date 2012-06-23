/***
 * Boruvka
 * --------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_QUAT_H__
#define __BOR_QUAT_H__

#include <boruvka/vec3.h>
#include <boruvka/vec4.h>
#include <boruvka/mat3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Quat - quaternions
 * ===================
 *
 *
 * .. c:type:: bor_quat_t
 */

/** vvvvv */
typedef bor_vec4_t bor_quat_t;
/** ^^^^^ */

/**
 * Functions
 * ----------
 */

/** vvvvv */
#define BOR_QUAT_STATIC BOR_VEC4_STATIC
#define BOR_QUAT(name, x, y, z, w) \
    bor_quat_t name = BOR_QUAT_STATIC((x), (y), (z), (w))

#define borQuatNew   borVec4New
#define borQuatDel   borVec4Del
#define borQuatCopy  borVec4Copy
#define borQuatClone borVec4Clone

#define borQuatX         borVec4X
#define borQuatY         borVec4Y
#define borQuatZ         borVec4Z
#define borQuatW         borVec4W
#define borQuatGet       borVec4Get
#define borQuatSet       borVec4Set
#define borQuatSetX      borVec4SetX
#define borQuatSetY      borVec4SetY
#define borQuatSetZ      borVec4SetZ
#define borQuatSetCoord  borVec4SetCoord
#define borQuatSetW      borVec4SetW
#define borQuatLen2      borVec4Len2
#define borQuatLen       borVec4Len
#define borQuatNormalize borVec4Normalize
#define borQuatScale     borVec4Scale

#define borQuatEq     borVec4Eq
#define borQuatNEq    borVec4NEq
#define borQuatEq2    borVec4Eq2
#define borQuatNEq2   borVec4NEq2
#define borQuatIsZero borVec4IsZero
/** ^^^^^ */


/**
 * Sets quaternion as rotation of given angle around given axis.
 */
_bor_inline void borQuatSetAngleAxis(bor_quat_t *q,
                                     bor_real_t angle, const bor_vec3_t *axis);

/**
 * Sets quaternion in euler angles (yaw, pitch, roll) or (heading,
 * elevation, bank) or rotation about (z, y, x)
 */
void borQuatSetEuler(bor_quat_t *q, bor_real_t yaw, bor_real_t pitch, bor_real_t roll);

/**
 * Composition of two quaternions.
 * q = q * q2
 */
_bor_inline void borQuatMul(bor_quat_t *q, const bor_quat_t *q2);

/**
 * q = a * b
 */
_bor_inline void borQuatMul2(bor_quat_t *q,
                           const bor_quat_t *a, const bor_quat_t *b);

/**
 * Inverts quaternion.
 * Returns 0 on success.
 */
_bor_inline int borQuatInvert(bor_quat_t *q);
_bor_inline int borQuatInvert2(bor_quat_t *dest, const bor_quat_t *src);


/**
 * Rotate vector v by quaternion q.
 */
_bor_inline void borQuatRotVec(bor_vec3_t *v, const bor_quat_t *q);

/**
 * Transforms quaternion into Mat3 rotation matrix.
 */
_bor_inline void borQuatToMat3(const bor_quat_t *q, bor_mat3_t *m);

/**
 * Creates quaternion from 3x3 matrix
 */
_bor_inline void borQuatFromMat3(bor_quat_t *q, const bor_mat3_t *m);


/**** INLINES ****/
_bor_inline void borQuatSetAngleAxis(bor_quat_t *q,
                                   bor_real_t angle, const bor_vec3_t *axis)
{
    bor_vec3_t xyz;
    bor_real_t a, s, n;

    borVec3Copy(&xyz, axis);
    n = borVec3Len(&xyz);

    if (bor_unlikely(borIsZero(n))){
        borQuatSet(q, BOR_ZERO, BOR_ZERO, BOR_ZERO, BOR_ONE);
    }else{
        a = angle * BOR_REAL(0.5);
        s = BOR_SIN(a) / n;

        borVec3Scale(&xyz, s);
        borQuatSet(q, borVec3X(&xyz), borVec3Y(&xyz), borVec3Z(&xyz), BOR_COS(a));
    }
}


_bor_inline void borQuatMul(bor_quat_t *q, const bor_quat_t *q2)
{
    bor_quat_t a;
    borQuatCopy(&a, q);
    borQuatMul2(q, &a, q2);
}

_bor_inline void borQuatMul2(bor_quat_t *q,
                           const bor_quat_t *a, const bor_quat_t *b)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    __m128 a1, a2, a3, a4, b1, b2, b3, b4, sign;

    /* shuffle quats */
    a1 = _mm_shuffle_ps(a->v, a->v, _MM_SHUFFLE(0, 3, 3, 3));
    a2 = _mm_shuffle_ps(a->v, a->v, _MM_SHUFFLE(1, 2, 1, 0));
    a3 = _mm_shuffle_ps(a->v, a->v, _MM_SHUFFLE(2, 0, 2, 1));
    a4 = _mm_shuffle_ps(a->v, a->v, _MM_SHUFFLE(3, 1, 0, 2));
    b1 = _mm_shuffle_ps(b->v, b->v, _MM_SHUFFLE(0, 2, 1, 0));
    b2 = _mm_shuffle_ps(b->v, b->v, _MM_SHUFFLE(1, 3, 3, 3));
    b3 = _mm_shuffle_ps(b->v, b->v, _MM_SHUFFLE(2, 1, 0, 2));
    b4 = _mm_shuffle_ps(b->v, b->v, _MM_SHUFFLE(3, 0, 2, 1));
    sign = _mm_set_ps(-BOR_ONE, BOR_ONE, BOR_ONE, BOR_ONE);

    /* mul into a* */
    a1 = _mm_mul_ps(a1, b1);
    a2 = _mm_mul_ps(a2, b2);
    a3 = _mm_mul_ps(a3, b3);
    a4 = _mm_mul_ps(a4, b4);

    /* add into a1  */
    a1 = _mm_add_ps(a1, a2);
    a1 = _mm_add_ps(a1, a3);
    a1 = _mm_sub_ps(a1, a4);

    /* change sign of w */
    q->v = _mm_mul_ps(a1, sign);
# else /* BOR_SSE_SINGLE */
    __m128d a33, a01, a12, a20, a21, a02, a30, a13;
    __m128d b33, b20, b12, b31, b03;
    __m128d sign;

    a33 = _mm_shuffle_pd(a->v[1], a->v[1], _MM_SHUFFLE2(1, 1));
    a01 = _mm_shuffle_pd(a->v[0], a->v[0], _MM_SHUFFLE2(1, 0));
    a12 = _mm_shuffle_pd(a->v[0], a->v[1], _MM_SHUFFLE2(0, 1));
    a20 = _mm_shuffle_pd(a->v[1], a->v[0], _MM_SHUFFLE2(0, 0));
    a21 = _mm_shuffle_pd(a->v[1], a->v[0], _MM_SHUFFLE2(1, 0));
    a02 = _mm_shuffle_pd(a->v[0], a->v[1], _MM_SHUFFLE2(0, 0));
    a30 = _mm_shuffle_pd(a->v[1], a->v[0], _MM_SHUFFLE2(0, 1));
    a13 = _mm_shuffle_pd(a->v[0], a->v[1], _MM_SHUFFLE2(1, 1));
    b33 = _mm_shuffle_pd(b->v[1], b->v[1], _MM_SHUFFLE2(1, 1));
    b20 = _mm_shuffle_pd(b->v[1], b->v[0], _MM_SHUFFLE2(0, 0));
    b12 = _mm_shuffle_pd(b->v[0], b->v[1], _MM_SHUFFLE2(0, 1));
    b31 = _mm_shuffle_pd(b->v[1], b->v[0], _MM_SHUFFLE2(1, 1));
    b03 = _mm_shuffle_pd(b->v[0], b->v[1], _MM_SHUFFLE2(1, 0));

    a33 = _mm_mul_pd(a33, b->v[0]);
    a21 = _mm_mul_pd(a21, b31);
    a01 = _mm_mul_pd(a01, b33);
    a02 = _mm_mul_pd(a02, b12);
    a12 = _mm_mul_pd(a12, b20);
    a30 = _mm_mul_pd(a30, b20);
    a20 = _mm_mul_pd(a20, b12);
    a13 = _mm_mul_pd(a13, b03);

    a33 = _mm_add_pd(a33, a01);
    a33 = _mm_add_pd(a33, a12);
    a33 = _mm_sub_pd(a33, a20);
    a21 = _mm_add_pd(a21, a02);
    a21 = _mm_add_pd(a21, a30);
    a21 = _mm_sub_pd(a21, a13);

    q->v[0] = a33;
    sign = _mm_set_pd(-BOR_ONE, BOR_ONE);
    q->v[1] = _mm_mul_pd(a21, sign);
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    q->f[0] = a->f[3] * b->f[0]
                + a->f[0] * b->f[3]
                + a->f[1] * b->f[2]
                - a->f[2] * b->f[1];
    q->f[1] = a->f[3] * b->f[1]
                + a->f[1] * b->f[3]
                - a->f[0] * b->f[2]
                + a->f[2] * b->f[0];
    q->f[2] = a->f[3] * b->f[2]
                + a->f[2] * b->f[3]
                + a->f[0] * b->f[1]
                - a->f[1] * b->f[0];
    q->f[3] = a->f[3] * b->f[3]
                - a->f[0] * b->f[0]
                - a->f[1] * b->f[1]
                - a->f[2] * b->f[2];
#endif /* BOR_SSE */
}

_bor_inline int borQuatInvert(bor_quat_t *q)
{
    bor_real_t len2 = borQuatLen2(q);
    if (len2 < BOR_EPS)
        return -1;

#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    __m128 k;
    k = _mm_set_ps(len2, -len2, -len2, -len2);
    q->v = _mm_div_ps(q->v, k);
# else /* BOR_SSE_SINGLE */
    __m128d k1, k2;
    k1 = _mm_set_pd(-len2, -len2);
    k2 = _mm_set_pd(len2, -len2);
    q->v[0] = _mm_div_pd(q->v[0], k1);
    q->v[1] = _mm_div_pd(q->v[1], k2);
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    len2 = borRecp(len2);

    q->f[0] = -q->f[0] * len2;
    q->f[1] = -q->f[1] * len2;
    q->f[2] = -q->f[2] * len2;
    q->f[3] = q->f[3] * len2;
#endif /* BOR_SSE */

    return 0;
}
_bor_inline int borQuatInvert2(bor_quat_t *dest, const bor_quat_t *src)
{
    borQuatCopy(dest, src);
    return borQuatInvert(dest);
}

_bor_inline void borQuatRotVec(bor_vec3_t *v, const bor_quat_t *q)
{
    bor_real_t w, x, y, z, ww, xx, yy, zz, wx, wy, wz, xy, xz, yz;
    bor_real_t vx, vy, vz;

    w = q->f[3];
    x = q->f[0];
    y = q->f[1];
    z = q->f[2];
    ww = w*w;
    xx = x*x;
    yy = y*y;
    zz = z*z;
    wx = w*x;
    wy = w*y;
    wz = w*z;
    xy = x*y;
    xz = x*z;
    yz = y*z;

    vx = ww * borVec3X(v)
            + xx * borVec3X(v)
            - yy * borVec3X(v)
            - zz * borVec3X(v)
            + 2 * ((xy - wz) * borVec3Y(v)
            + (xz + wy) * borVec3Z(v));
    vy = ww * borVec3Y(v)
            - xx * borVec3Y(v)
            + yy * borVec3Y(v)
            - zz * borVec3Y(v)
            + 2 * ((xy + wz) * borVec3X(v)
            + (yz - wx) * borVec3Z(v));
    vz = ww * borVec3Z(v)
            - xx * borVec3Z(v)
            - yy * borVec3Z(v)
            + zz * borVec3Z(v)
            + 2 * ((xz - wy) * borVec3X(v)
            + (yz + wx) * borVec3Y(v));
    borVec3Set(v, vx, vy, vz);
}

_bor_inline void borQuatToMat3(const bor_quat_t *q, bor_mat3_t *m)
{
    bor_real_t x, y, z, w;
    bor_real_t xx, yy, zz, xy, zw, xz, yw, yz, xw;

    x = borQuatX(q);
    y = borQuatY(q);
    z = borQuatZ(q);
    w = borQuatW(q);
    xx = BOR_REAL(2.) * x * x;
    yy = BOR_REAL(2.) * y * y;
    zz = BOR_REAL(2.) * z * z;
    xy = BOR_REAL(2.) * x * y;
    zw = BOR_REAL(2.) * z * w;
    xz = BOR_REAL(2.) * x * z;
    yw = BOR_REAL(2.) * y * w;
    yz = BOR_REAL(2.) * y * z;
    xw = BOR_REAL(2.) * x * w;

    borMat3Set(m, 1.0-yy-zz, xy-zw, xz+yw,
                  xy+zw, 1.0-xx-zz, yz-xw,
                  xz-yw, yz+xw, 1.0-xx-yy);
}

_bor_inline void borQuatFromMat3(bor_quat_t *q, const bor_mat3_t *m)
{
    bor_real_t w, w4;

    w  = 1 + borMat3Get(m, 0, 0) + borMat3Get(m, 1, 1) + borMat3Get(m, 2, 2);
    w  = BOR_SQRT(w);
    w4 = borRecp(BOR_REAL(2.) * w);
    w /= BOR_REAL(2.);

    borQuatSetX(q, (borMat3Get(m, 2, 1) - borMat3Get(m, 1, 2)) * w4);
    borQuatSetX(q, (borMat3Get(m, 0, 2) - borMat3Get(m, 2, 0)) * w4);
    borQuatSetX(q, (borMat3Get(m, 1, 0) - borMat3Get(m, 0, 1)) * w4);
    borQuatSetW(q, w);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_QUAT_H__ */
