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

#include <math.h>

#include <mg/alloc.h>
#include <mg/vec2.h>

MG_VEC2(__mg_vec2_origin, MG_ZERO, MG_ZERO);
const mg_vec2_t *mg_vec2_origin = &__mg_vec2_origin;

mg_vec2_t *mgVec2New(mg_real_t x, mg_real_t y)
{
    mg_vec2_t *v = MG_ALLOC(mg_vec2_t);
    mgVec2Set(v, x, y);
    return v;
}

void mgVec2Del(mg_vec2_t *v)
{
    free(v);
}

/*
mg_real_t __mgVec2Area2(const mg_vec2_t *a, const mg_vec2_t *b, const mg_vec2_t *c)
{
    * Area2 can be computed as determinant:
     * | a.x a.y 1 |
     * | b.x b.y 1 |
     * | c.x c.y 1 |
     *

    mg_real_t ax, ay, bx, by, cx, cy;

    ax = mgVec2X(a);
    ay = mgVec2Y(a);
    bx = mgVec2X(b);
    by = mgVec2Y(b);
    cx = mgVec2X(c);
    cy = mgVec2Y(c);
    return ax * by - ay * bx +
           ay * cx - ax * cy +
           bx * cy - by * cx;
}
*/


mg_real_t mgVec2Angle(const mg_vec2_t *a, const mg_vec2_t *b, const mg_vec2_t *c)
{
    mg_real_t dist_ab, dist_bc, dist_ca;
    mg_real_t x, y;

    dist_ab = MG_SQRT(mgVec2Dist2(a, b));
    dist_bc = MG_SQRT(mgVec2Dist2(b, c));
    dist_ca = MG_SQRT(mgVec2Dist2(c, a));

    if (mgEq(dist_ab, MG_ZERO) ||
        mgEq(dist_bc, MG_ZERO))
        return MG_ZERO;

    x = (dist_ab*dist_ab + dist_bc*dist_bc - dist_ca*dist_ca);
    y = (2 * dist_ab * dist_bc);

    if (mgEq(x, y))
        return MG_ZERO;

    return MG_ACOS(x / y);
}


/** Returns true if given number is between 0 and 1 inclusive. */
_mg_inline int num01(mg_real_t n)
{
    if (mgEq(n, MG_ZERO) || mgEq(n, MG_ONE)
        || (n > MG_ZERO && n < MG_ONE))
        return 1;
    return 0;
}

int mgVec2IntersectPoint(const mg_vec2_t *a, const mg_vec2_t *b,
                         const mg_vec2_t *c, const mg_vec2_t *d,
                         mg_vec2_t *p)
{
    mg_real_t x4x3, x1x3, x2x1;
    mg_real_t y1y3, y4y3, y2y1;
    mg_real_t ua, ub, factor;

    x4x3 = mgVec2X(d) - mgVec2X(c);
    x1x3 = mgVec2X(a) - mgVec2X(c);
    x2x1 = mgVec2X(b) - mgVec2X(a);
    y1y3 = mgVec2Y(a) - mgVec2Y(c);
    y4y3 = mgVec2Y(d) - mgVec2Y(c);
    y2y1 = mgVec2Y(b) - mgVec2Y(a);

    factor = y4y3 * x2x1 - x4x3 * y2y1;
    if (mgEq(factor, MG_ZERO))
        return -1; // segments are parallel

    ua = (x4x3 * y1y3 - y4y3 * x1x3) / factor;
    ub = (x2x1 * y1y3 - y2y1 * x1x3) / factor;

    if (!num01(ua) || !num01(ub))
        return -1;

    mgVec2Set(p, mgVec2X(a) + ua * (mgVec2X(b) - mgVec2X(a)),
                mgVec2Y(a) + ua * (mgVec2Y(b) - mgVec2Y(a)));
    return 0;
}



int mgVec2ProjectionPointOntoSegment(const mg_vec2_t *a, const mg_vec2_t *b,
                                     const mg_vec2_t *c,
                                     mg_vec2_t *x)
{
    mg_real_t ux, uy, cxax, cyay, k, xx, xy;

    // directional vector (ux, uy) of segment ab
    ux = mgVec2X(b) - mgVec2X(a);
    uy = mgVec2Y(b) - mgVec2Y(a);

    cxax = mgVec2X(c) - mgVec2X(a);
    cyay = mgVec2Y(c) - mgVec2Y(a);

    if (mgEq(ux, MG_ZERO) && mgEq(uy, MG_ZERO)){
        return -1;
    }else if (mgEq(ux, MG_ZERO)){
        xx = mgVec2X(a);
        xy = mgVec2Y(c);
        k = (xy - mgVec2Y(a)) / uy;
    }else if (mgEq(uy, MG_ZERO)){
        xx = mgVec2X(c);
        xy = mgVec2Y(a);
        k = (xx - mgVec2X(a)) / ux;
    }else{
        k = uy * cyay;
        k += ux * cxax;
        k /= uy * uy + ux * ux;

        xx = mgVec2X(a) + k * ux;
        xy = mgVec2Y(a) + k * uy;
    }

    if (mgVec2Eq2(a, xx, xy)){
        mgVec2Copy(x, a);
    }else if (mgVec2Eq2(b, xx, xy)){
        mgVec2Copy(x, b);
    }else if (k > MG_ZERO && k < MG_ONE){
        mgVec2Set(x, xx, xy);
    }else{
        return -1;
    }

    return 0;
}


int mgVec2InCircle(const mg_vec2_t *a, const mg_vec2_t *b, const mg_vec2_t *c,
                   const mg_vec2_t *d)
{
    // | ax  ay  ax2+ay2  1 |
    // | bx  by  bx2+by2  1 |
    // | cx  cy  cx2+cy2  1 | =
    // | dx  dy  dx2+dy2  1 |  
    // 
    //    | (ax - dx)  (ay - dy)  (ax - dx)2 + (ay - dy)2 |
    //  = | (bx - dx)  (by - dy)  (bx - dx)2 + (by - dy)2 | > 0
    //    | (cx - dx)  (cy - dy)  (cx - dx)2 + (cy - dy)2 |

    mg_real_t adx, ady, bdx, bdy, cdx, cdy;
    mg_real_t adx2ady2, bdx2bdy2, cdx2cdy2;
    mg_real_t det;

    adx = mgVec2X(a) - mgVec2X(d);
    ady = mgVec2Y(a) - mgVec2Y(d);
    bdx = mgVec2X(b) - mgVec2X(d);
    bdy = mgVec2Y(b) - mgVec2Y(d);
    cdx = mgVec2X(c) - mgVec2X(d);
    cdy = mgVec2Y(c) - mgVec2Y(d);
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

int mgVec2LiesOn(const mg_vec2_t *a, const mg_vec2_t *b, const mg_vec2_t *c)
{
    // if a lies directly on b or c, return true
    if (mgVec2Eq(a, b) || mgVec2Eq(a, c))
        return 1;

    // if points are not collinear poin can't lie on bc
    if (!mgVec2Collinear(a, b, c))
        return 0;

    // here it's certain that a, b and c are collinear
    if (!mgEq(mgVec2X(b), mgVec2X(c))){
        return (mgVec2X(b) < mgVec2X(a) && mgVec2X(c) > mgVec2X(a)) 
               || (mgVec2X(b) > mgVec2X(a) && mgVec2X(c) < mgVec2X(a));
    }else{
        return (mgVec2Y(b) < mgVec2Y(a) && mgVec2Y(c) > mgVec2Y(a)) 
               || (mgVec2Y(b) > mgVec2Y(a) && mgVec2Y(c) < mgVec2Y(a));
    }
}


int mgVec2InCone(const mg_vec2_t *v,
                 const mg_vec2_t *p1, const mg_vec2_t *c, const mg_vec2_t *p2)
{
    // point is concave
    if (mgVec2Area2(p1, c, p2) < 0){
        if (mgVec2Area2(c, v, p2) < 0 ||
            mgVec2Area2(v, c, p1) < 0)
            return 1;
        return 0;
    }else{ // point is not concave (is convex)
        if (mgVec2Area2(c, v, p1) > 0 &&
            mgVec2Area2(v, c, p2) > 0)
            return 1;
        return 0;
    }
}




/** Returns 1 if x lies inside convex polygon abcd */
_mg_inline int liesIn(const mg_vec2_t *a, const mg_vec2_t *b,
                     const mg_vec2_t *c, const mg_vec2_t *d,
                     const mg_vec2_t *x)
{
    mg_real_t areaab, areabc, areacd, areada;
    int liesin = 0;

    areaab = mgVec2Area2(a, b, x);
    areabc = mgVec2Area2(b, c, x);
    areacd = mgVec2Area2(c, d, x);
    areada = mgVec2Area2(d, a, x);
    if (mgNEq(areaab, MG_ZERO) && areaab > MG_ZERO
        && mgNEq(areabc, MG_ZERO) && areabc > MG_ZERO
        && mgNEq(areacd, MG_ZERO) && areacd > MG_ZERO
        && mgNEq(areada, MG_ZERO) && areada > MG_ZERO){
        liesin = 1;
    }

    return liesin;
}

/** Finds intersection between segment xy and ab, bc, cd and da.
 * Intersection points are returned via r1 and r2 and returned value is
 * number of found intersections. */
_mg_inline int intersections(const mg_vec2_t *a, const mg_vec2_t *b,
                            const mg_vec2_t *c, const mg_vec2_t *d,
                            const mg_vec2_t *x, const mg_vec2_t *y,
                            mg_vec2_t *r1, mg_vec2_t *r2)
{
    int num = 0;
    mg_vec2_t *v[2];

    v[0] = r1;
    v[1] = r2;

    if (mgVec2IntersectPoint(a, b, x, y, v[0]) == 0)
        num++;

    if (mgVec2IntersectPoint(b, c, x, y, v[num]) == 0)
        if (num == 0 || mgVec2NEq(v[0], v[1]))
            num++;

    if (num != 2 && mgVec2IntersectPoint(c, d, x, y, v[num]) == 0)
        if (num == 0 || mgVec2NEq(v[0], v[1]))
            num++;
    if (num != 2 && mgVec2IntersectPoint(d, a, x, y, v[num]) == 0)
        if (num == 0 || mgVec2NEq(v[0], v[1]))
            num++;

    return num;
}

int mgVec2SegmentInRect(const mg_vec2_t *a, const mg_vec2_t *b,
                        const mg_vec2_t *c, const mg_vec2_t *d,
                        const mg_vec2_t *x, const mg_vec2_t *y,
                        mg_vec2_t *s1, mg_vec2_t *s2)
{
    int xliesin, yliesin;
    int num;

    xliesin = liesIn(a, b, c, d, x);
    yliesin = liesIn(a, b, c, d, y);

    if (xliesin && yliesin){
        mgVec2Copy(s1, x);
        mgVec2Copy(s2, y);

        return 0;
    }

    num = intersections(a, b, c, d, x, y, s1, s2);

    if (num == 2){
        return 0;
    }else if (num == 1){
        if (xliesin){
            mgVec2Copy(s2, x);
        }else if (yliesin){
            mgVec2Copy(s2, y);
        }else{
            return -1;
        }
    }else{ // num == 0
        return -1;
    }

    return 0;
}

mg_real_t mgVec2AngleSameDir(const mg_vec2_t *a, const mg_vec2_t *b)
{
    mg_vec2_t *origin = mgVec2New(0., 0.);
    mg_real_t angle;

    if (mgVec2Collinear(origin, a, b)){
        if (mgVec2LiesOn(a, origin, b) || mgVec2LiesOn(b, origin, a)){
            angle = MG_ZERO;
        }else{
            angle = M_PI;
        }
    }else{
        angle = mgVec2SignedAngle(b, origin, a);
    }

    mgVec2Del(origin);

    return angle;
}

mg_real_t mgVec2AngleSegsSameDir(const mg_vec2_t *A, const mg_vec2_t *B,
                               const mg_vec2_t *C, const mg_vec2_t *D)
{
    mg_vec2_t *a, *c;
    mg_real_t angle;

    a = mgVec2New(0., 0.);
    c = mgVec2New(0., 0.);
    mgVec2Sub2(a, B, A);
    mgVec2Sub2(c, D, C);

    angle = mgVec2AngleSameDir(a, c);

    mgVec2Del(a);
    mgVec2Del(c);

    return angle;
}
