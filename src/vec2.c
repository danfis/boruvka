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

#include <math.h>

#include <boruvka/alloc.h>
#include <boruvka/vec2.h>
#include <boruvka/mat3.h>
#include <boruvka/dbg.h>

BOR_VEC2(__bor_vec2_origin, BOR_ZERO, BOR_ZERO);
const bor_vec2_t *bor_vec2_origin = &__bor_vec2_origin;
BOR_VEC2(__bor_vec2_01, BOR_ZERO, BOR_ONE);
const bor_vec2_t *bor_vec2_01 = &__bor_vec2_01;
BOR_VEC2(__bor_vec2_10, BOR_ONE, BOR_ZERO);
const bor_vec2_t *bor_vec2_10 = &__bor_vec2_10;
BOR_VEC2(__bor_vec2_11, BOR_ONE, BOR_ONE);
const bor_vec2_t *bor_vec2_11 = &__bor_vec2_11;

bor_vec2_t *borVec2New(bor_real_t x, bor_real_t y)
{
    bor_vec2_t *v;

#ifdef BOR_SSE
    v = BOR_ALLOC_ALIGN(bor_vec2_t, 16);
#else /* BOR_SSE */
    v = BOR_ALLOC(bor_vec2_t);
#endif /* BOR_SSE */
    borVec2Set(v, x, y);
    return v;
}

void borVec2Del(bor_vec2_t *v)
{
    BOR_FREE(v);
}

_bor_inline void _borVec2Rot(const bor_vec2_t *v, bor_real_t angle,
                             bor_real_t *x, bor_real_t *y)
{
    bor_real_t cosa, sina;
    cosa = BOR_COS(angle);
    sina = BOR_SIN(angle);

    *x = borVec2X(v) * cosa - borVec2Y(v) * sina;
    *y = borVec2X(v) * sina + borVec2Y(v) * cosa;
}

void borVec2Rot(bor_vec2_t *v, bor_real_t angle)
{
    bor_real_t x, y;
    _borVec2Rot(v, angle, &x, &y);
    borVec2Set(v, x, y);
}

void borVec2Rot2(bor_vec2_t *w, const bor_vec2_t *v, bor_real_t angle)
{
    bor_real_t x, y;
    _borVec2Rot(v, angle, &x, &y);
    borVec2Set(w, x, y);
}

/*
bor_real_t __borVec2Area2(const bor_vec2_t *a, const bor_vec2_t *b, const bor_vec2_t *c)
{
    * Area2 can be computed as determinant:
     * | a.x a.y 1 |
     * | b.x b.y 1 |
     * | c.x c.y 1 |
     *

    bor_real_t ax, ay, bx, by, cx, cy;

    ax = borVec2X(a);
    ay = borVec2Y(a);
    bx = borVec2X(b);
    by = borVec2Y(b);
    cx = borVec2X(c);
    cy = borVec2Y(c);
    return ax * by - ay * bx +
           ay * cx - ax * cy +
           bx * cy - by * cx;
}
*/


bor_real_t borVec2Angle(const bor_vec2_t *a, const bor_vec2_t *b, const bor_vec2_t *c)
{
    bor_vec2_t v, w;
    bor_real_t ang, num, denom;

    borVec2Sub2(&v, a, b);
    borVec2Sub2(&w, c, b);

    num   = borVec2Dot(&v, &w);
    denom = borVec2Len2(&v) * borVec2Len2(&w);
    denom = BOR_SQRT(denom);
    ang   = num / denom;

    if (ang > BOR_ONE)
        ang = BOR_ONE;
    if (ang < -BOR_ONE)
        ang = -BOR_ONE;

    return BOR_ACOS(ang);
}


/** Returns true if given number is between 0 and 1 inclusive. */
_bor_inline int num01(bor_real_t n)
{
    if (borEq(n, BOR_ZERO) || borEq(n, BOR_ONE)
        || (n > BOR_ZERO && n < BOR_ONE))
        return 1;
    return 0;
}

int borVec2IntersectPoint(const bor_vec2_t *a, const bor_vec2_t *b,
                         const bor_vec2_t *c, const bor_vec2_t *d,
                         bor_vec2_t *p)
{
    bor_real_t x4x3, x1x3, x2x1;
    bor_real_t y1y3, y4y3, y2y1;
    bor_real_t ua, ub, factor;

    x4x3 = borVec2X(d) - borVec2X(c);
    x1x3 = borVec2X(a) - borVec2X(c);
    x2x1 = borVec2X(b) - borVec2X(a);
    y1y3 = borVec2Y(a) - borVec2Y(c);
    y4y3 = borVec2Y(d) - borVec2Y(c);
    y2y1 = borVec2Y(b) - borVec2Y(a);

    factor = y4y3 * x2x1 - x4x3 * y2y1;
    if (borEq(factor, BOR_ZERO))
        return -1; // segments are parallel

    ua = (x4x3 * y1y3 - y4y3 * x1x3) / factor;
    ub = (x2x1 * y1y3 - y2y1 * x1x3) / factor;

    if (!num01(ua) || !num01(ub))
        return -1;

    borVec2Set(p, borVec2X(a) + ua * (borVec2X(b) - borVec2X(a)),
                  borVec2Y(a) + ua * (borVec2Y(b) - borVec2Y(a)));
    return 0;
}



int borVec2ProjectionPointOntoSegment(const bor_vec2_t *a, const bor_vec2_t *b,
                                     const bor_vec2_t *c,
                                     bor_vec2_t *x)
{
    bor_real_t ux, uy, cxax, cyay, k, xx, xy;

    // directional vector (ux, uy) of segment ab
    ux = borVec2X(b) - borVec2X(a);
    uy = borVec2Y(b) - borVec2Y(a);

    cxax = borVec2X(c) - borVec2X(a);
    cyay = borVec2Y(c) - borVec2Y(a);

    if (borEq(ux, BOR_ZERO) && borEq(uy, BOR_ZERO)){
        return -1;
    }else if (borEq(ux, BOR_ZERO)){
        xx = borVec2X(a);
        xy = borVec2Y(c);
        k = (xy - borVec2Y(a)) / uy;
    }else if (borEq(uy, BOR_ZERO)){
        xx = borVec2X(c);
        xy = borVec2Y(a);
        k = (xx - borVec2X(a)) / ux;
    }else{
        k = uy * cyay;
        k += ux * cxax;
        k /= uy * uy + ux * ux;

        xx = borVec2X(a) + k * ux;
        xy = borVec2Y(a) + k * uy;
    }

    if (borVec2Eq2(a, xx, xy)){
        borVec2Copy(x, a);
    }else if (borVec2Eq2(b, xx, xy)){
        borVec2Copy(x, b);
    }else if (k > BOR_ZERO && k < BOR_ONE){
        borVec2Set(x, xx, xy);
    }else{
        return -1;
    }

    return 0;
}


int borVec2InCircle(const bor_vec2_t *a, const bor_vec2_t *b, const bor_vec2_t *c,
                    const bor_vec2_t *d)
{
    // | ax  ay  ax2+ay2  1 |
    // | bx  by  bx2+by2  1 |
    // | cx  cy  cx2+cy2  1 | =
    // | dx  dy  dx2+dy2  1 |  
    // 
    //    | (ax - dx)  (ay - dy)  (ax - dx)2 + (ay - dy)2 |
    //  = | (bx - dx)  (by - dy)  (bx - dx)2 + (by - dy)2 | > 0
    //    | (cx - dx)  (cy - dy)  (cx - dx)2 + (cy - dy)2 |

    bor_real_t adx, ady, bdx, bdy, cdx, cdy;
    bor_real_t adx2ady2, bdx2bdy2, cdx2cdy2;
    bor_real_t det;

    adx = borVec2X(a) - borVec2X(d);
    ady = borVec2Y(a) - borVec2Y(d);
    bdx = borVec2X(b) - borVec2X(d);
    bdy = borVec2Y(b) - borVec2Y(d);
    cdx = borVec2X(c) - borVec2X(d);
    cdy = borVec2Y(c) - borVec2Y(d);
    adx2ady2 = adx * adx + ady * ady;
    bdx2bdy2 = bdx * bdx + bdy * bdy;
    cdx2cdy2 = cdx * cdx + cdy * cdy;

    det  = adx * bdy * cdx2cdy2;
    det += ady * bdx2bdy2 * cdx;
    det += adx2ady2 * bdx * cdy;
    det -= adx2ady2 * bdy * cdx;
    det -= bdx2bdy2 * cdy * adx;
    det -= cdx2cdy2 * ady * bdx;

    return det > 0.;
}

int borVec2LiesOn(const bor_vec2_t *a, const bor_vec2_t *b, const bor_vec2_t *c)
{
    // if a lies directly on b or c, return true
    if (borVec2Eq(a, b) || borVec2Eq(a, c))
        return 1;

    // if points are not collinear poin can't lie on bc
    if (!borVec2Collinear(a, b, c))
        return 0;

    // here it's certain that a, b and c are collinear
    if (!borEq(borVec2X(b), borVec2X(c))){
        return (borVec2X(b) < borVec2X(a) && borVec2X(c) > borVec2X(a)) 
                || (borVec2X(b) > borVec2X(a) && borVec2X(c) < borVec2X(a));
    }else{
        return (borVec2Y(b) < borVec2Y(a) && borVec2Y(c) > borVec2Y(a)) 
                || (borVec2Y(b) > borVec2Y(a) && borVec2Y(c) < borVec2Y(a));
    }
}


int borVec2InCone(const bor_vec2_t *v,
                 const bor_vec2_t *p1, const bor_vec2_t *c, const bor_vec2_t *p2)
{
    // point is concave
    if (borVec2Area2(p1, c, p2) < 0){
        if (borVec2Area2(c, v, p2) < 0 ||
            borVec2Area2(v, c, p1) < 0)
            return 1;
        return 0;
    }else{ // point is not concave (is convex)
        if (borVec2Area2(c, v, p1) > 0 &&
            borVec2Area2(v, c, p2) > 0)
            return 1;
        return 0;
    }
}




/** Returns 1 if x lies inside convex polygon abcd */
_bor_inline int liesIn(const bor_vec2_t *a, const bor_vec2_t *b,
                     const bor_vec2_t *c, const bor_vec2_t *d,
                     const bor_vec2_t *x)
{
    bor_real_t areaab, areabc, areacd, areada;
    int liesin = 0;

    areaab = borVec2Area2(a, b, x);
    areabc = borVec2Area2(b, c, x);
    areacd = borVec2Area2(c, d, x);
    areada = borVec2Area2(d, a, x);
    if (borNEq(areaab, BOR_ZERO) && areaab > BOR_ZERO
        && borNEq(areabc, BOR_ZERO) && areabc > BOR_ZERO
        && borNEq(areacd, BOR_ZERO) && areacd > BOR_ZERO
        && borNEq(areada, BOR_ZERO) && areada > BOR_ZERO){
        liesin = 1;
    }

    return liesin;
}

/** Finds intersection between segment xy and ab, bc, cd and da.
 * Intersection points are returned via r1 and r2 and returned value is
 * number of found intersections. */
_bor_inline int intersections(const bor_vec2_t *a, const bor_vec2_t *b,
                            const bor_vec2_t *c, const bor_vec2_t *d,
                            const bor_vec2_t *x, const bor_vec2_t *y,
                            bor_vec2_t *r1, bor_vec2_t *r2)
{
    int num = 0;
    bor_vec2_t *v[2];

    v[0] = r1;
    v[1] = r2;

    if (borVec2IntersectPoint(a, b, x, y, v[0]) == 0)
        num++;

    if (borVec2IntersectPoint(b, c, x, y, v[num]) == 0)
        if (num == 0 || borVec2NEq(v[0], v[1]))
            num++;

    if (num != 2 && borVec2IntersectPoint(c, d, x, y, v[num]) == 0)
        if (num == 0 || borVec2NEq(v[0], v[1]))
            num++;
    if (num != 2 && borVec2IntersectPoint(d, a, x, y, v[num]) == 0)
        if (num == 0 || borVec2NEq(v[0], v[1]))
            num++;

    return num;
}

int borVec2SegmentInRect(const bor_vec2_t *a, const bor_vec2_t *b,
                        const bor_vec2_t *c, const bor_vec2_t *d,
                        const bor_vec2_t *x, const bor_vec2_t *y,
                        bor_vec2_t *s1, bor_vec2_t *s2)
{
    int xliesin, yliesin;
    int num;

    xliesin = liesIn(a, b, c, d, x);
    yliesin = liesIn(a, b, c, d, y);

    if (xliesin && yliesin){
        borVec2Copy(s1, x);
        borVec2Copy(s2, y);

        return 0;
    }

    num = intersections(a, b, c, d, x, y, s1, s2);

    if (num == 2){
        return 0;
    }else if (num == 1){
        if (xliesin){
            borVec2Copy(s2, x);
        }else if (yliesin){
            borVec2Copy(s2, y);
        }else{
            return -1;
        }
    }else{ // num == 0
        return -1;
    }

    return 0;
}

bor_real_t borVec2AngleSameDir(const bor_vec2_t *a, const bor_vec2_t *b)
{
    bor_vec2_t *origin = borVec2New(0., 0.);
    bor_real_t angle;

    if (borVec2Collinear(origin, a, b)){
        if (borVec2LiesOn(a, origin, b) || borVec2LiesOn(b, origin, a)){
            angle = BOR_ZERO;
        }else{
            angle = M_PI;
        }
    }else{
        angle = borVec2SignedAngle(b, origin, a);
    }

    borVec2Del(origin);

    return angle;
}

bor_real_t borVec2AngleSegsSameDir(const bor_vec2_t *A, const bor_vec2_t *B,
                                   const bor_vec2_t *C, const bor_vec2_t *D)
{
    bor_vec2_t *a, *c;
    bor_real_t angle;

    a = borVec2New(0., 0.);
    c = borVec2New(0., 0.);
    borVec2Sub2(a, B, A);
    borVec2Sub2(c, D, C);

    angle = borVec2AngleSameDir(a, c);

    borVec2Del(a);
    borVec2Del(c);

    return angle;
}


int borVec2TriTriOverlap(const bor_vec2_t *p1, const bor_vec2_t *q1,
                         const bor_vec2_t *r1,
                         const bor_vec2_t *p2, const bor_vec2_t *q2,
                         const bor_vec2_t *r2)
{
    // TODO: this can be done effeciently
    if (borVec2Intersect(p1, q1, p2, q2)
            || borVec2Intersect(p1, q1, p2, r2)
            || borVec2Intersect(p1, q1, q2, r2)
            || borVec2Intersect(p1, r1, p2, q2)
            || borVec2Intersect(p1, r1, p2, r2)
            || borVec2Intersect(p1, r1, q2, r2)
            || borVec2Intersect(q1, r1, p2, q2)
            || borVec2Intersect(q1, r1, p2, r2)
            || borVec2Intersect(q1, r1, q2, r2)
            || borVec2PointInTri(p1, p2, q2, r2)
            || borVec2PointInTri(p2, p1, q1, r1))
        return 1;

    return 0;
}


static int __borVec2BoxBoxOverlapAxis(const bor_vec2_t *axis,
                                      const bor_vec2_t *p,
                                      const bor_vec2_t *pts)
{
    bor_vec2_t s;
    bor_real_t dot;
    int i, pos = 0, neg = 0;

    for (i = 0; i < 4; i++){
        borVec2Sub2(&s, &pts[i], p);
        dot = borVec2Dot(axis, &s);
        if (dot > BOR_ZERO){
            ++pos;
        }else{
            ++neg;
        }

        if (pos && neg)
            return 0;
    }

    return (pos ? 1 : -1);
}

static int __borVec2BoxBoxOverlap(const bor_vec2_t *c1,
                                  const bor_vec2_t *c2)
{
    bor_vec2_t axis;
    int i;

    for (i = 0; i < 4; i++){
        borVec2Sub2(&axis, &c1[(i + 1) % 4], &c1[i]);
        borVec2Set(&axis, borVec2Y(&axis), -borVec2X(&axis));
        if (__borVec2BoxBoxOverlapAxis(&axis, &c1[(i + 1) % 4], c2) > 0)
            return 0;
    }

    for (i = 0; i < 4; i++){
        borVec2Sub2(&axis, &c2[(i + 1) % 4], &c2[i]);
        borVec2Set(&axis, borVec2Y(&axis), -borVec2X(&axis));
        if (__borVec2BoxBoxOverlapAxis(&axis, &c2[(i + 1) % 4], c1) > 0)
            return 0;
    }

    return 1;
}

int borVec2BoxBoxOverlap(const bor_vec2_t *half_edges1,
                         const bor_vec2_t *pos1, bor_real_t rot1,
                         const bor_vec2_t *half_edges2,
                         const bor_vec2_t *pos2, bor_real_t rot2)
{
    bor_vec2_t c1[4], c2[4], tmp;
    bor_mat3_t tr;

    // Box1:
    // transformation matrix
    borMat3SetRot(&tr, rot1);
    borMat3Set1(&tr, 0, 2, borVec2X(pos1));
    borMat3Set1(&tr, 1, 2, borVec2Y(pos1));

    // corner points
    borMat3MulVec2(&c1[0], &tr, half_edges1);
    borVec2Set(&tmp, -borVec2X(half_edges1), borVec2Y(half_edges1));
    borMat3MulVec2(&c1[1], &tr, &tmp);
    borVec2Set(&tmp, -borVec2X(half_edges1), -borVec2Y(half_edges1));
    borMat3MulVec2(&c1[2], &tr, &tmp);
    borVec2Set(&tmp, borVec2X(half_edges1), -borVec2Y(half_edges1));
    borMat3MulVec2(&c1[3], &tr, &tmp);

    // Box2:
    // transformation matrix
    borMat3SetRot(&tr, rot2);
    borMat3Set1(&tr, 0, 2, borVec2X(pos2));
    borMat3Set1(&tr, 1, 2, borVec2Y(pos2));

    // corner points
    borMat3MulVec2(&c2[0], &tr, half_edges2);
    borVec2Set(&tmp, -borVec2X(half_edges2), borVec2Y(half_edges2));
    borMat3MulVec2(&c2[1], &tr, &tmp);
    borVec2Set(&tmp, -borVec2X(half_edges2), -borVec2Y(half_edges2));
    borMat3MulVec2(&c2[2], &tr, &tmp);
    borVec2Set(&tmp, borVec2X(half_edges2), -borVec2Y(half_edges2));
    borMat3MulVec2(&c2[3], &tr, &tmp);

    return __borVec2BoxBoxOverlap(c1, c2);
}
