#ifndef __MG_QUAT_H__
#define __MG_QUAT_H__

#include <mg/core.h>
#include <mg/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _mg_quat_t {
    mg_real_t q[4]; //!< x, y, z, w
};
typedef struct _mg_quat_t mg_quat_t;

#define MG_QUAT(name, x, y, z, w) \
    mg_quat_t name = { {x, y, z, w} }

mg_quat_t *mgQuatNew(mg_real_t x, mg_real_t y, mg_real_t z, mg_real_t w);
void mgQuatDel(mg_quat_t *q);
_mg_inline void mgQuatCopy(mg_quat_t *dest, const mg_quat_t *src);
_mg_inline mg_quat_t *mgQuatClone(const mg_quat_t *src);

_mg_inline mg_real_t mgQuatLen2(const mg_quat_t *q);
_mg_inline mg_real_t mgQuatLen(const mg_quat_t *q);

_mg_inline mg_real_t mgQuatX(const mg_quat_t *q);
_mg_inline mg_real_t mgQuatY(const mg_quat_t *q);
_mg_inline mg_real_t mgQuatZ(const mg_quat_t *q);
_mg_inline mg_real_t mgQuatW(const mg_quat_t *q);

_mg_inline void mgQuatSet(mg_quat_t *q, mg_real_t x, mg_real_t y, mg_real_t z, mg_real_t w);
_mg_inline void mgQuatSetX(mg_quat_t *q, mg_real_t v);
_mg_inline void mgQuatSetY(mg_quat_t *q, mg_real_t v);
_mg_inline void mgQuatSetZ(mg_quat_t *q, mg_real_t v);
_mg_inline void mgQuatSetW(mg_quat_t *q, mg_real_t v);

_mg_inline int mgQuatNormalize(mg_quat_t *q);

_mg_inline void mgQuatSetAngleAxis(mg_quat_t *q,
                                     mg_real_t angle, const mg_vec3_t *axis);

_mg_inline void mgQuatScale(mg_quat_t *q, mg_real_t k);

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
_mg_inline mg_real_t mgQuatLen2(const mg_quat_t *q)
{
    mg_real_t len;

    len  = q->q[0] * q->q[0];
    len += q->q[1] * q->q[1];
    len += q->q[2] * q->q[2];
    len += q->q[3] * q->q[3];

    return len;
}

_mg_inline mg_real_t mgQuatLen(const mg_quat_t *q)
{
    return MG_SQRT(mgQuatLen2(q));
}

_mg_inline mg_real_t mgQuatX(const mg_quat_t *q)
{
    return q->q[0];
}
_mg_inline mg_real_t mgQuatY(const mg_quat_t *q)
{
    return q->q[1];
}
_mg_inline mg_real_t mgQuatZ(const mg_quat_t *q)
{
    return q->q[2];
}
_mg_inline mg_real_t mgQuatW(const mg_quat_t *q)
{
    return q->q[3];
}

_mg_inline void mgQuatSet(mg_quat_t *q, mg_real_t x, mg_real_t y, mg_real_t z, mg_real_t w)
{
    q->q[0] = x;
    q->q[1] = y;
    q->q[2] = z;
    q->q[3] = w;
}

_mg_inline void mgQuatSetX(mg_quat_t *q, mg_real_t v)
{
    q->q[0] = v;
}
_mg_inline void mgQuatSetY(mg_quat_t *q, mg_real_t v)
{
    q->q[1] = v;
}
_mg_inline void mgQuatSetZ(mg_quat_t *q, mg_real_t v)
{
    q->q[2] = v;
}
_mg_inline void mgQuatSetW(mg_quat_t *q, mg_real_t v)
{
    q->q[3] = v;
}

_mg_inline void mgQuatCopy(mg_quat_t *dest, const mg_quat_t *src)
{
    *dest = *src;
}

_mg_inline mg_quat_t *mgQuatClone(const mg_quat_t *s)
{
    return mgQuatNew(mgQuatX(s), mgQuatY(s), mgQuatZ(s), mgQuatW(s));
}

_mg_inline int mgQuatNormalize(mg_quat_t *q)
{
    mg_real_t len = mgQuatLen(q);
    if (len < MG_EPS)
        return 0;

    mgQuatScale(q, MG_ONE / len);
    return 1;
}

_mg_inline void mgQuatSetAngleAxis(mg_quat_t *q,
                                     mg_real_t angle, const mg_vec3_t *axis)
{
    mg_real_t a, x, y, z, n, s;

    a = angle/2;
    x = mgVec3X(axis);
    y = mgVec3Y(axis);
    z = mgVec3Z(axis);
    n = MG_SQRT(x*x + y*y + z*z);

    // axis==0? (treat this the same as angle==0 with an arbitrary axis)
    if (n < MG_EPS){
        q->q[0] = q->q[1] = q->q[2] = MG_ZERO;
        q->q[3] = MG_ONE;
    }else{
        s = sin(a)/n;

        q->q[3] = cos(a);
        q->q[0] = x*s;
        q->q[1] = y*s;
        q->q[2] = z*s;

        mgQuatNormalize(q);
    }
}


_mg_inline void mgQuatScale(mg_quat_t *q, mg_real_t k)
{
    size_t i;
    for (i = 0; i < 4; i++)
        q->q[i] *= k;
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
    q->q[0] = a->q[3] * b->q[0]
                + a->q[0] * b->q[3]
                + a->q[1] * b->q[2]
                - a->q[2] * b->q[1];
    q->q[1] = a->q[3] * b->q[1]
                + a->q[1] * b->q[3]
                - a->q[0] * b->q[2]
                + a->q[2] * b->q[0];
    q->q[2] = a->q[3] * b->q[2]
                + a->q[2] * b->q[3]
                + a->q[0] * b->q[1]
                - a->q[1] * b->q[0];
    q->q[3] = a->q[3] * b->q[3]
                - a->q[0] * b->q[0]
                - a->q[1] * b->q[1]
                - a->q[2] * b->q[2];
}

_mg_inline int mgQuatInvert(mg_quat_t *q)
{
    mg_real_t len2 = mgQuatLen2(q);
    if (len2 < MG_EPS)
        return -1;

    len2 = MG_ONE / len2;

    q->q[0] = -q->q[0] * len2;
    q->q[1] = -q->q[1] * len2;
    q->q[2] = -q->q[2] * len2;
    q->q[3] = q->q[3] * len2;

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

    w = q->q[3];
    x = q->q[0];
    y = q->q[1];
    z = q->q[2];
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
