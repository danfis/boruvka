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

#include <math.h>

#include <fermat/alloc.h>
#include <fermat/vec2.h>

FER_VEC2(__fer_vec2_origin, FER_ZERO, FER_ZERO);
const fer_vec2_t *fer_vec2_origin = &__fer_vec2_origin;

fer_vec2_t *ferVec2New(fer_real_t x, fer_real_t y)
{
    fer_vec2_t *v;

#ifdef FER_SSE
    v = FER_ALLOC_ALIGN(fer_vec2_t, sizeof(fer_vec2_t));
#else /* FER_SSE */
    v = FER_ALLOC(fer_vec2_t);
#endif /* FER_SSE */
    ferVec2Set(v, x, y);
    return v;
}

void ferVec2Del(fer_vec2_t *v)
{
    free(v);
}

/*
fer_real_t __ferVec2Area2(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c)
{
    * Area2 can be computed as determinant:
     * | a.x a.y 1 |
     * | b.x b.y 1 |
     * | c.x c.y 1 |
     *

    fer_real_t ax, ay, bx, by, cx, cy;

    ax = ferVec2X(a);
    ay = ferVec2Y(a);
    bx = ferVec2X(b);
    by = ferVec2Y(b);
    cx = ferVec2X(c);
    cy = ferVec2Y(c);
    return ax * by - ay * bx +
           ay * cx - ax * cy +
           bx * cy - by * cx;
}
*/


fer_real_t ferVec2Angle(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c)
{
    fer_vec2_t v, w;
    fer_real_t ang, num, denom;

    ferVec2Sub2(&v, a, b);
    ferVec2Sub2(&w, c, b);

    num   = ferVec2Dot(&v, &w);
    denom = ferVec2Len2(&v) * ferVec2Len2(&w);
    denom = FER_SQRT(denom);
    ang   = num / denom;

    if (ang > FER_ONE)
        ang = FER_ONE;
    if (ang < -FER_ONE)
        ang = -FER_ONE;

    return FER_ACOS(ang);
}


/** Returns true if given number is between 0 and 1 inclusive. */
_fer_inline int num01(fer_real_t n)
{
    if (ferEq(n, FER_ZERO) || ferEq(n, FER_ONE)
        || (n > FER_ZERO && n < FER_ONE))
        return 1;
    return 0;
}

int ferVec2IntersectPoint(const fer_vec2_t *a, const fer_vec2_t *b,
                         const fer_vec2_t *c, const fer_vec2_t *d,
                         fer_vec2_t *p)
{
    fer_real_t x4x3, x1x3, x2x1;
    fer_real_t y1y3, y4y3, y2y1;
    fer_real_t ua, ub, factor;

    x4x3 = ferVec2X(d) - ferVec2X(c);
    x1x3 = ferVec2X(a) - ferVec2X(c);
    x2x1 = ferVec2X(b) - ferVec2X(a);
    y1y3 = ferVec2Y(a) - ferVec2Y(c);
    y4y3 = ferVec2Y(d) - ferVec2Y(c);
    y2y1 = ferVec2Y(b) - ferVec2Y(a);

    factor = y4y3 * x2x1 - x4x3 * y2y1;
    if (ferEq(factor, FER_ZERO))
        return -1; // segments are parallel

    ua = (x4x3 * y1y3 - y4y3 * x1x3) / factor;
    ub = (x2x1 * y1y3 - y2y1 * x1x3) / factor;

    if (!num01(ua) || !num01(ub))
        return -1;

    ferVec2Set(p, ferVec2X(a) + ua * (ferVec2X(b) - ferVec2X(a)),
                  ferVec2Y(a) + ua * (ferVec2Y(b) - ferVec2Y(a)));
    return 0;
}



int ferVec2ProjectionPointOntoSegment(const fer_vec2_t *a, const fer_vec2_t *b,
                                     const fer_vec2_t *c,
                                     fer_vec2_t *x)
{
    fer_real_t ux, uy, cxax, cyay, k, xx, xy;

    // directional vector (ux, uy) of segment ab
    ux = ferVec2X(b) - ferVec2X(a);
    uy = ferVec2Y(b) - ferVec2Y(a);

    cxax = ferVec2X(c) - ferVec2X(a);
    cyay = ferVec2Y(c) - ferVec2Y(a);

    if (ferEq(ux, FER_ZERO) && ferEq(uy, FER_ZERO)){
        return -1;
    }else if (ferEq(ux, FER_ZERO)){
        xx = ferVec2X(a);
        xy = ferVec2Y(c);
        k = (xy - ferVec2Y(a)) / uy;
    }else if (ferEq(uy, FER_ZERO)){
        xx = ferVec2X(c);
        xy = ferVec2Y(a);
        k = (xx - ferVec2X(a)) / ux;
    }else{
        k = uy * cyay;
        k += ux * cxax;
        k /= uy * uy + ux * ux;

        xx = ferVec2X(a) + k * ux;
        xy = ferVec2Y(a) + k * uy;
    }

    if (ferVec2Eq2(a, xx, xy)){
        ferVec2Copy(x, a);
    }else if (ferVec2Eq2(b, xx, xy)){
        ferVec2Copy(x, b);
    }else if (k > FER_ZERO && k < FER_ONE){
        ferVec2Set(x, xx, xy);
    }else{
        return -1;
    }

    return 0;
}


int ferVec2InCircle(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c,
                    const fer_vec2_t *d)
{
    // | ax  ay  ax2+ay2  1 |
    // | bx  by  bx2+by2  1 |
    // | cx  cy  cx2+cy2  1 | =
    // | dx  dy  dx2+dy2  1 |  
    // 
    //    | (ax - dx)  (ay - dy)  (ax - dx)2 + (ay - dy)2 |
    //  = | (bx - dx)  (by - dy)  (bx - dx)2 + (by - dy)2 | > 0
    //    | (cx - dx)  (cy - dy)  (cx - dx)2 + (cy - dy)2 |

    fer_real_t adx, ady, bdx, bdy, cdx, cdy;
    fer_real_t adx2ady2, bdx2bdy2, cdx2cdy2;
    fer_real_t det;

    adx = ferVec2X(a) - ferVec2X(d);
    ady = ferVec2Y(a) - ferVec2Y(d);
    bdx = ferVec2X(b) - ferVec2X(d);
    bdy = ferVec2Y(b) - ferVec2Y(d);
    cdx = ferVec2X(c) - ferVec2X(d);
    cdy = ferVec2Y(c) - ferVec2Y(d);
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

int ferVec2LiesOn(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c)
{
    // if a lies directly on b or c, return true
    if (ferVec2Eq(a, b) || ferVec2Eq(a, c))
        return 1;

    // if points are not collinear poin can't lie on bc
    if (!ferVec2Collinear(a, b, c))
        return 0;

    // here it's certain that a, b and c are collinear
    if (!ferEq(ferVec2X(b), ferVec2X(c))){
        return (ferVec2X(b) < ferVec2X(a) && ferVec2X(c) > ferVec2X(a)) 
                || (ferVec2X(b) > ferVec2X(a) && ferVec2X(c) < ferVec2X(a));
    }else{
        return (ferVec2Y(b) < ferVec2Y(a) && ferVec2Y(c) > ferVec2Y(a)) 
                || (ferVec2Y(b) > ferVec2Y(a) && ferVec2Y(c) < ferVec2Y(a));
    }
}


int ferVec2InCone(const fer_vec2_t *v,
                 const fer_vec2_t *p1, const fer_vec2_t *c, const fer_vec2_t *p2)
{
    // point is concave
    if (ferVec2Area2(p1, c, p2) < 0){
        if (ferVec2Area2(c, v, p2) < 0 ||
            ferVec2Area2(v, c, p1) < 0)
            return 1;
        return 0;
    }else{ // point is not concave (is convex)
        if (ferVec2Area2(c, v, p1) > 0 &&
            ferVec2Area2(v, c, p2) > 0)
            return 1;
        return 0;
    }
}




/** Returns 1 if x lies inside convex polygon abcd */
_fer_inline int liesIn(const fer_vec2_t *a, const fer_vec2_t *b,
                     const fer_vec2_t *c, const fer_vec2_t *d,
                     const fer_vec2_t *x)
{
    fer_real_t areaab, areabc, areacd, areada;
    int liesin = 0;

    areaab = ferVec2Area2(a, b, x);
    areabc = ferVec2Area2(b, c, x);
    areacd = ferVec2Area2(c, d, x);
    areada = ferVec2Area2(d, a, x);
    if (ferNEq(areaab, FER_ZERO) && areaab > FER_ZERO
        && ferNEq(areabc, FER_ZERO) && areabc > FER_ZERO
        && ferNEq(areacd, FER_ZERO) && areacd > FER_ZERO
        && ferNEq(areada, FER_ZERO) && areada > FER_ZERO){
        liesin = 1;
    }

    return liesin;
}

/** Finds intersection between segment xy and ab, bc, cd and da.
 * Intersection points are returned via r1 and r2 and returned value is
 * number of found intersections. */
_fer_inline int intersections(const fer_vec2_t *a, const fer_vec2_t *b,
                            const fer_vec2_t *c, const fer_vec2_t *d,
                            const fer_vec2_t *x, const fer_vec2_t *y,
                            fer_vec2_t *r1, fer_vec2_t *r2)
{
    int num = 0;
    fer_vec2_t *v[2];

    v[0] = r1;
    v[1] = r2;

    if (ferVec2IntersectPoint(a, b, x, y, v[0]) == 0)
        num++;

    if (ferVec2IntersectPoint(b, c, x, y, v[num]) == 0)
        if (num == 0 || ferVec2NEq(v[0], v[1]))
            num++;

    if (num != 2 && ferVec2IntersectPoint(c, d, x, y, v[num]) == 0)
        if (num == 0 || ferVec2NEq(v[0], v[1]))
            num++;
    if (num != 2 && ferVec2IntersectPoint(d, a, x, y, v[num]) == 0)
        if (num == 0 || ferVec2NEq(v[0], v[1]))
            num++;

    return num;
}

int ferVec2SegmentInRect(const fer_vec2_t *a, const fer_vec2_t *b,
                        const fer_vec2_t *c, const fer_vec2_t *d,
                        const fer_vec2_t *x, const fer_vec2_t *y,
                        fer_vec2_t *s1, fer_vec2_t *s2)
{
    int xliesin, yliesin;
    int num;

    xliesin = liesIn(a, b, c, d, x);
    yliesin = liesIn(a, b, c, d, y);

    if (xliesin && yliesin){
        ferVec2Copy(s1, x);
        ferVec2Copy(s2, y);

        return 0;
    }

    num = intersections(a, b, c, d, x, y, s1, s2);

    if (num == 2){
        return 0;
    }else if (num == 1){
        if (xliesin){
            ferVec2Copy(s2, x);
        }else if (yliesin){
            ferVec2Copy(s2, y);
        }else{
            return -1;
        }
    }else{ // num == 0
        return -1;
    }

    return 0;
}

fer_real_t ferVec2AngleSameDir(const fer_vec2_t *a, const fer_vec2_t *b)
{
    fer_vec2_t *origin = ferVec2New(0., 0.);
    fer_real_t angle;

    if (ferVec2Collinear(origin, a, b)){
        if (ferVec2LiesOn(a, origin, b) || ferVec2LiesOn(b, origin, a)){
            angle = FER_ZERO;
        }else{
            angle = M_PI;
        }
    }else{
        angle = ferVec2SignedAngle(b, origin, a);
    }

    ferVec2Del(origin);

    return angle;
}

fer_real_t ferVec2AngleSegsSameDir(const fer_vec2_t *A, const fer_vec2_t *B,
                                   const fer_vec2_t *C, const fer_vec2_t *D)
{
    fer_vec2_t *a, *c;
    fer_real_t angle;

    a = ferVec2New(0., 0.);
    c = ferVec2New(0., 0.);
    ferVec2Sub2(a, B, A);
    ferVec2Sub2(c, D, C);

    angle = ferVec2AngleSameDir(a, c);

    ferVec2Del(a);
    ferVec2Del(c);

    return angle;
}
