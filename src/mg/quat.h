/***
 * mg
 * ---
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of mg.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __MG_QUAT_H__
#define __MG_QUAT_H__

#include <mg/vec3.h>
#include <mg/vec4.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef mg_vec4_t mg_quat_t;

#define MG_QUAT_STATIC MG_VEC4_STATIC
#define MG_QUAT(name, x, y, z, w) \
    mg_quat_t name = MG_QUAT_STATIC((x), (y), (z), (w))

#define mgQuatNew   mgVec4New
#define mgQuatDel   mgVec4Del
#define mgQuatCopy  mgVec4Copy
#define mgQuatClone mgVec4Clone

#define mgQuatX         mgVec4X
#define mgQuatY         mgVec4Y
#define mgQuatZ         mgVec4Z
#define mgQuatW         mgVec4W
#define mgQuatGet       mgVec4Get
#define mgQuatSet       mgVec4Set
#define mgQuatSetX      mgVec4SetX
#define mgQuatSetY      mgVec4SetY
#define mgQuatSetZ      mgVec4SetZ
#define mgQuatSetCoord  mgVec4SetCoord
#define mgQuatSetW      mgVec4SetW
#define mgQuatLen2      mgVec4Len2
#define mgQuatLen       mgVec4Len
#define mgQuatNormalize mgVec4Normalize
#define mgQuatScale     mgVec4Scale

#define mgQuatEq     mgVec4Eq
#define mgQuatNEq    mgVec4NEq
#define mgQuatEq2    mgVec4Eq2
#define mgQuatNEq2   mgVec4NEq2
#define mgQuatIsZero mgVec4IsZero


_mg_inline void mgQuatSetAngleAxis(mg_quat_t *q,
                                     mg_real_t angle, const mg_vec3_t *axis);

/**
 * q = q * q2
 */
_mg_inline void mgQuatMul(mg_quat_t *q, const mg_quat_t *q2);

/**
 * q = a * b
 */
_mg_inline void mgQuatMul2(mg_quat_t *q,
                           const mg_quat_t *a, const mg_quat_t *b);

/**
 * Inverts quaternion.
 * Returns 0 on success.
 */
_mg_inline int mgQuatInvert(mg_quat_t *q);
_mg_inline int mgQuatInvert2(mg_quat_t *dest, const mg_quat_t *src);


/**
 * Rotate vector v by quaternion q.
 */
_mg_inline void mgQuatRotVec(mg_vec3_t *v, const mg_quat_t *q);


/**** INLINES ****/
_mg_inline void mgQuatSetAngleAxis(mg_quat_t *q,
                                   mg_real_t angle, const mg_vec3_t *axis)
{
    mg_vec3_t xyz;
    mg_real_t a, s, n;

    mgVec3Copy(&xyz, axis);
    n = mgVec3Len(&xyz);

    if (mg_unlikely(mgIsZero(n))){
        mgQuatSet(q, MG_ZERO, MG_ZERO, MG_ZERO, MG_ONE);
    }else{
        a = angle * MG_REAL(0.5);
        s = MG_SIN(a) / n;

        mgVec3Scale(&xyz, s);
        mgQuatSet(q, mgVec3X(&xyz), mgVec3Y(&xyz), mgVec3Z(&xyz), MG_COS(a));
    }
}


_mg_inline void mgQuatMul(mg_quat_t *q, const mg_quat_t *q2)
{
    mg_quat_t a;
    mgQuatCopy(&a, q);
    mgQuatMul2(q, &a, q2);
}

_mg_inline void mgQuatMul2(mg_quat_t *q,
                           const mg_quat_t *a, const mg_quat_t *b)
{
#ifdef MG_SSE
# ifdef MG_SSE_SINGLE
    __m128 a1, a2, a3, a4, b1, b2, b3, b4, sign;

    // shuffle quats
    a1 = _mm_shuffle_ps(a->q, a->q, _MM_SHUFFLE(0, 3, 3, 3));
    a2 = _mm_shuffle_ps(a->q, a->q, _MM_SHUFFLE(1, 2, 1, 0));
    a3 = _mm_shuffle_ps(a->q, a->q, _MM_SHUFFLE(2, 0, 2, 1));
    a4 = _mm_shuffle_ps(a->q, a->q, _MM_SHUFFLE(3, 1, 0, 2));
    b1 = _mm_shuffle_ps(b->q, b->q, _MM_SHUFFLE(0, 2, 1, 0));
    b2 = _mm_shuffle_ps(b->q, b->q, _MM_SHUFFLE(1, 3, 3, 3));
    b3 = _mm_shuffle_ps(b->q, b->q, _MM_SHUFFLE(2, 1, 0, 2));
    b4 = _mm_shuffle_ps(b->q, b->q, _MM_SHUFFLE(3, 0, 2, 1));
    sign = _mm_set_ps(-MG_ONE, MG_ONE, MG_ONE, MG_ONE);

    // mul into a*
    a1 = _mm_mul_ps(a1, b1);
    a2 = _mm_mul_ps(a2, b2);
    a3 = _mm_mul_ps(a3, b3);
    a4 = _mm_mul_ps(a4, b4);

    // add into a1
    a1 = _mm_add_ps(a1, a2);
    a1 = _mm_add_ps(a1, a3);
    a1 = _mm_sub_ps(a1, a4);

    // change sign of w
    q->q = _mm_mul_ps(a1, sign);
# else /* MG_SSE_SINGLE */
    __m128d a33, a01, a12, a20, a21, a02, a30, a13;
    __m128d b33, b20, b12, b31, b03;
    __m128d sign;

    a33 = _mm_shuffle_pd(a->q[1], a->q[1], _MM_SHUFFLE2(1, 1));
    a01 = _mm_shuffle_pd(a->q[0], a->q[0], _MM_SHUFFLE2(1, 0));
    a12 = _mm_shuffle_pd(a->q[0], a->q[1], _MM_SHUFFLE2(0, 1));
    a20 = _mm_shuffle_pd(a->q[1], a->q[0], _MM_SHUFFLE2(0, 0));
    a21 = _mm_shuffle_pd(a->q[1], a->q[0], _MM_SHUFFLE2(1, 0));
    a02 = _mm_shuffle_pd(a->q[0], a->q[1], _MM_SHUFFLE2(0, 0));
    a30 = _mm_shuffle_pd(a->q[1], a->q[0], _MM_SHUFFLE2(0, 1));
    a13 = _mm_shuffle_pd(a->q[0], a->q[1], _MM_SHUFFLE2(1, 1));
    b33 = _mm_shuffle_pd(b->q[1], b->q[1], _MM_SHUFFLE2(1, 1));
    b20 = _mm_shuffle_pd(b->q[1], b->q[0], _MM_SHUFFLE2(0, 0));
    b12 = _mm_shuffle_pd(b->q[0], b->q[1], _MM_SHUFFLE2(0, 1));
    b31 = _mm_shuffle_pd(b->q[1], b->q[0], _MM_SHUFFLE2(1, 1));
    b03 = _mm_shuffle_pd(b->q[0], b->q[1], _MM_SHUFFLE2(1, 0));

    a33 = _mm_mul_pd(a33, b->q[0]);
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

    q->q[0] = a33;
    sign = _mm_set_pd(-MG_ONE, MG_ONE);
    q->q[1] = _mm_mul_pd(a21, sign);
# endif /* MG_SSE_SINGLE */
#else /* MG_SSE */
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
#endif /* MG_SSE */
}

_mg_inline int mgQuatInvert(mg_quat_t *q)
{
    mg_real_t len2 = mgQuatLen2(q);
    if (len2 < MG_EPS)
        return -1;

#ifdef MG_SSE
# ifdef MG_SSE_SINGLE
    __m128 k;
    k = _mm_set_ps(len2, -len2, -len2, -len2);
    q->q = _mm_div_ps(q->q, k);
# else /* MG_SSE_SINGLE */
    __m128d k1, k2;
    k1 = _mm_set_pd(-len2, -len2);
    k2 = _mm_set_pd(len2, -len2);
    q->q[0] = _mm_div_pd(q->q[0], k1);
    q->q[1] = _mm_div_pd(q->q[1], k2);
# endif /* MG_SSE_SINGLE */
#else /* MG_SSE */
    len2 = mgRecp(len2);

    q->f[0] = -q->f[0] * len2;
    q->f[1] = -q->f[1] * len2;
    q->f[2] = -q->f[2] * len2;
    q->f[3] = q->f[3] * len2;
#endif /* MG_SSE */

    return 0;
}
_mg_inline int mgQuatInvert2(mg_quat_t *dest, const mg_quat_t *src)
{
    mgQuatCopy(dest, src);
    return mgQuatInvert(dest);
}

_mg_inline void mgQuatRotVec(mg_vec3_t *v, const mg_quat_t *q)
{
    mg_real_t w, x, y, z, ww, xx, yy, zz, wx, wy, wz, xy, xz, yz;
    mg_real_t vx, vy, vz;

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

    vx = ww * mgVec3X(v)
            + xx * mgVec3X(v)
            - yy * mgVec3X(v)
            - zz * mgVec3X(v)
            + 2 * ((xy - wz) * mgVec3Y(v)
            + (xz + wy) * mgVec3Z(v));
    vy = ww * mgVec3Y(v)
            - xx * mgVec3Y(v)
            + yy * mgVec3Y(v)
            - zz * mgVec3Y(v)
            + 2 * ((xy + wz) * mgVec3X(v)
            + (yz - wx) * mgVec3Z(v));
    vz = ww * mgVec3Z(v)
            - xx * mgVec3Z(v)
            - yy * mgVec3Z(v)
            + zz * mgVec3Z(v)
            + 2 * ((xz - wy) * mgVec3X(v)
            + (yz + wx) * mgVec3Y(v));
    mgVec3Set(v, vx, vy, vz);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __MG_QUAT_H__ */
