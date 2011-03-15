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

#ifndef __FER_QUAT_H__
#define __FER_QUAT_H__

#include <fermat/vec3.h>
#include <fermat/vec4.h>
#include <fermat/mat3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Quat - quaternions
 * ===================
 *
 *
 * .. c:type:: fer_quat_t
 */

/** vvvvv */
typedef fer_vec4_t fer_quat_t;
/** ^^^^^ */

/**
 * Functions
 * ----------
 */

/** vvvvv */
#define FER_QUAT_STATIC FER_VEC4_STATIC
#define FER_QUAT(name, x, y, z, w) \
    fer_quat_t name = FER_QUAT_STATIC((x), (y), (z), (w))

#define ferQuatNew   ferVec4New
#define ferQuatDel   ferVec4Del
#define ferQuatCopy  ferVec4Copy
#define ferQuatClone ferVec4Clone

#define ferQuatX         ferVec4X
#define ferQuatY         ferVec4Y
#define ferQuatZ         ferVec4Z
#define ferQuatW         ferVec4W
#define ferQuatGet       ferVec4Get
#define ferQuatSet       ferVec4Set
#define ferQuatSetX      ferVec4SetX
#define ferQuatSetY      ferVec4SetY
#define ferQuatSetZ      ferVec4SetZ
#define ferQuatSetCoord  ferVec4SetCoord
#define ferQuatSetW      ferVec4SetW
#define ferQuatLen2      ferVec4Len2
#define ferQuatLen       ferVec4Len
#define ferQuatNormalize ferVec4Normalize
#define ferQuatScale     ferVec4Scale

#define ferQuatEq     ferVec4Eq
#define ferQuatNEq    ferVec4NEq
#define ferQuatEq2    ferVec4Eq2
#define ferQuatNEq2   ferVec4NEq2
#define ferQuatIsZero ferVec4IsZero
/** ^^^^^ */


/**
 * Sets quaternion as rotation of given angle around given axis.
 */
_fer_inline void ferQuatSetAngleAxis(fer_quat_t *q,
                                     fer_real_t angle, const fer_vec3_t *axis);

/**
 * Composition of two quaternions.
 * q = q * q2
 */
_fer_inline void ferQuatMul(fer_quat_t *q, const fer_quat_t *q2);

/**
 * q = a * b
 */
_fer_inline void ferQuatMul2(fer_quat_t *q,
                           const fer_quat_t *a, const fer_quat_t *b);

/**
 * Inverts quaternion.
 * Returns 0 on success.
 */
_fer_inline int ferQuatInvert(fer_quat_t *q);
_fer_inline int ferQuatInvert2(fer_quat_t *dest, const fer_quat_t *src);


/**
 * Rotate vector v by quaternion q.
 */
_fer_inline void ferQuatRotVec(fer_vec3_t *v, const fer_quat_t *q);

/**
 * Transforms quaternion into Mat3 rotation matrix.
 */
_fer_inline void ferQuatToMat3(const fer_quat_t *q, fer_mat3_t *m);


/**** INLINES ****/
_fer_inline void ferQuatSetAngleAxis(fer_quat_t *q,
                                   fer_real_t angle, const fer_vec3_t *axis)
{
    fer_vec3_t xyz;
    fer_real_t a, s, n;

    ferVec3Copy(&xyz, axis);
    n = ferVec3Len(&xyz);

    if (fer_unlikely(ferIsZero(n))){
        ferQuatSet(q, FER_ZERO, FER_ZERO, FER_ZERO, FER_ONE);
    }else{
        a = angle * FER_REAL(0.5);
        s = FER_SIN(a) / n;

        ferVec3Scale(&xyz, s);
        ferQuatSet(q, ferVec3X(&xyz), ferVec3Y(&xyz), ferVec3Z(&xyz), FER_COS(a));
    }
}


_fer_inline void ferQuatMul(fer_quat_t *q, const fer_quat_t *q2)
{
    fer_quat_t a;
    ferQuatCopy(&a, q);
    ferQuatMul2(q, &a, q2);
}

_fer_inline void ferQuatMul2(fer_quat_t *q,
                           const fer_quat_t *a, const fer_quat_t *b)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    __m128 a1, a2, a3, a4, b1, b2, b3, b4, sign;

    // shuffle quats
    a1 = _mm_shuffle_ps(a->v, a->v, _MM_SHUFFLE(0, 3, 3, 3));
    a2 = _mm_shuffle_ps(a->v, a->v, _MM_SHUFFLE(1, 2, 1, 0));
    a3 = _mm_shuffle_ps(a->v, a->v, _MM_SHUFFLE(2, 0, 2, 1));
    a4 = _mm_shuffle_ps(a->v, a->v, _MM_SHUFFLE(3, 1, 0, 2));
    b1 = _mm_shuffle_ps(b->v, b->v, _MM_SHUFFLE(0, 2, 1, 0));
    b2 = _mm_shuffle_ps(b->v, b->v, _MM_SHUFFLE(1, 3, 3, 3));
    b3 = _mm_shuffle_ps(b->v, b->v, _MM_SHUFFLE(2, 1, 0, 2));
    b4 = _mm_shuffle_ps(b->v, b->v, _MM_SHUFFLE(3, 0, 2, 1));
    sign = _mm_set_ps(-FER_ONE, FER_ONE, FER_ONE, FER_ONE);

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
    q->v = _mm_mul_ps(a1, sign);
# else /* FER_SSE_SINGLE */
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
    sign = _mm_set_pd(-FER_ONE, FER_ONE);
    q->v[1] = _mm_mul_pd(a21, sign);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
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
#endif /* FER_SSE */
}

_fer_inline int ferQuatInvert(fer_quat_t *q)
{
    fer_real_t len2 = ferQuatLen2(q);
    if (len2 < FER_EPS)
        return -1;

#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    __m128 k;
    k = _mm_set_ps(len2, -len2, -len2, -len2);
    q->v = _mm_div_ps(q->v, k);
# else /* FER_SSE_SINGLE */
    __m128d k1, k2;
    k1 = _mm_set_pd(-len2, -len2);
    k2 = _mm_set_pd(len2, -len2);
    q->v[0] = _mm_div_pd(q->v[0], k1);
    q->v[1] = _mm_div_pd(q->v[1], k2);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    len2 = ferRecp(len2);

    q->f[0] = -q->f[0] * len2;
    q->f[1] = -q->f[1] * len2;
    q->f[2] = -q->f[2] * len2;
    q->f[3] = q->f[3] * len2;
#endif /* FER_SSE */

    return 0;
}
_fer_inline int ferQuatInvert2(fer_quat_t *dest, const fer_quat_t *src)
{
    ferQuatCopy(dest, src);
    return ferQuatInvert(dest);
}

_fer_inline void ferQuatRotVec(fer_vec3_t *v, const fer_quat_t *q)
{
    fer_real_t w, x, y, z, ww, xx, yy, zz, wx, wy, wz, xy, xz, yz;
    fer_real_t vx, vy, vz;

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

    vx = ww * ferVec3X(v)
            + xx * ferVec3X(v)
            - yy * ferVec3X(v)
            - zz * ferVec3X(v)
            + 2 * ((xy - wz) * ferVec3Y(v)
            + (xz + wy) * ferVec3Z(v));
    vy = ww * ferVec3Y(v)
            - xx * ferVec3Y(v)
            + yy * ferVec3Y(v)
            - zz * ferVec3Y(v)
            + 2 * ((xy + wz) * ferVec3X(v)
            + (yz - wx) * ferVec3Z(v));
    vz = ww * ferVec3Z(v)
            - xx * ferVec3Z(v)
            - yy * ferVec3Z(v)
            + zz * ferVec3Z(v)
            + 2 * ((xz - wy) * ferVec3X(v)
            + (yz + wx) * ferVec3Y(v));
    ferVec3Set(v, vx, vy, vz);
}

_fer_inline void ferQuatToMat3(const fer_quat_t *q, fer_mat3_t *m)
{
    fer_real_t x, y, z, w;
    fer_real_t xx, yy, zz, xy, zw, xz, yw, yz, xw;

    x = ferQuatX(q);
    y = ferQuatY(q);
    z = ferQuatZ(q);
    w = ferQuatW(q);
    xx = FER_REAL(2.) * x * x;
    yy = FER_REAL(2.) * y * y;
    zz = FER_REAL(2.) * z * z;
    xy = FER_REAL(2.) * x * y;
    zw = FER_REAL(2.) * z * w;
    xz = FER_REAL(2.) * x * z;
    yw = FER_REAL(2.) * y * w;
    yz = FER_REAL(2.) * y * z;
    xw = FER_REAL(2.) * x * w;

    ferMat3Set(m, 1.0-yy-zz, xy-zw, xz+yw,
                  xy+zw, 1.0-xx-zz, yz-xw,
                  xz-yw, yz+xw, 1.0-xx-yy);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_QUAT_H__ */
