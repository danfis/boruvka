/***
 * fermat
 * -------
 * Copyright (c)2010,2011 Daniel Fiser <danfis@danfis.cz>
 *
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

#include <fermat/vec3.h>
#include <fermat/ccd.h>
#include <fermat/ccd-simplex.h>
#include <fermat/ccd-polytope.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>


/** Performs GJK algorithm. Returns 0 if intersection was found and simplex
 *  is filled with resulting polytope. */
static int __ferGJK(const void *obj1, const void *obj2,
                    const fer_ccd_t *ccd, fer_ccd_simplex_t *simplex);

/** Performs GJK+EPA algorithm. Returns 0 if intersection was found and
 *  pt is filled with resulting polytope and nearest with pointer to
 *  nearest element (vertex, edge, face) of polytope to origin. */
static int __ferGJKEPA(const void *obj1, const void *obj2,
                       const fer_ccd_t *ccd,
                       fer_ccd_pt_t *pt, fer_ccd_pt_el_t **nearest);


/** Returns true if simplex contains origin.
 *  This function also alteres simplex and dir according to further
 *  processing of GJK algorithm. */
static int doSimplex(fer_ccd_simplex_t *simplex, fer_vec3_t *dir);
static int doSimplex2(fer_ccd_simplex_t *simplex, fer_vec3_t *dir);
static int doSimplex3(fer_ccd_simplex_t *simplex, fer_vec3_t *dir);
static int doSimplex4(fer_ccd_simplex_t *simplex, fer_vec3_t *dir);

/** d = a x b x c */
_fer_inline void tripleCross(const fer_vec3_t *a, const fer_vec3_t *b,
                             const fer_vec3_t *c, fer_vec3_t *d);


/** Transforms simplex to polytope. It is assumed that simplex has 4
 *  vertices. */
static int simplexToPolytope4(const void *obj1, const void *obj2,
                              const fer_ccd_t *ccd,
                              fer_ccd_simplex_t *simplex,
                              fer_ccd_pt_t *pt, fer_ccd_pt_el_t **nearest);

/** Transforms simplex to polytope, three vertices required */
static int simplexToPolytope3(const void *obj1, const void *obj2,
                              const fer_ccd_t *ccd,
                              const fer_ccd_simplex_t *simplex,
                              fer_ccd_pt_t *pt, fer_ccd_pt_el_t **nearest);

/** Transforms simplex to polytope, two vertices required */
static int simplexToPolytope2(const void *obj1, const void *obj2,
                              const fer_ccd_t *ccd,
                              const fer_ccd_simplex_t *simplex,
                              fer_ccd_pt_t *pt, fer_ccd_pt_el_t **nearest);

/** Expands polytope using new vertex v. */
static void expandPolytope(fer_ccd_pt_t *pt, fer_ccd_pt_el_t *el,
                           const fer_ccd_support_t *newv);

/** Finds next support point (at stores it in out argument).
 *  Returns 0 on success, -1 otherwise */
static int nextSupport(const void *obj1, const void *obj2, const fer_ccd_t *ccd,
                       const fer_ccd_pt_el_t *el,
                       fer_ccd_support_t *out);



void ferCCDFirstDirDefault(const void *o1, const void *o2, fer_vec3_t *dir)
{
    ferVec3Set(dir, FER_ONE, FER_ZERO, FER_ZERO);
}

void ferCCDInit(fer_ccd_t *ccd)
{
    ccd->first_dir = ferCCDFirstDirDefault;
    ccd->support1 = NULL;
    ccd->support2 = NULL;
    ccd->center1  = NULL;
    ccd->center2  = NULL;

    ccd->max_iterations = (unsigned long)-1;
    ccd->epa_tolerance = FER_REAL(0.0001);
    ccd->mpr_tolerance = FER_REAL(0.0001);
}

int ferCCDGJKCollide(const fer_ccd_t *ccd, const void *obj1, const void *obj2)
{
    fer_ccd_simplex_t simplex;
    return __ferGJK(obj1, obj2, ccd, &simplex) == 0;
}

int ferCCDGJKSeparate(const fer_ccd_t *ccd,
                      const void *obj1, const void *obj2,
                      fer_vec3_t *sep)
{
    fer_ccd_pt_t polytope;
    fer_ccd_pt_el_t *nearest;
    int ret;

    ferCCDPtInit(&polytope);

    ret = __ferGJKEPA(obj1, obj2, ccd, &polytope, &nearest);

    // set separation vector
    if (nearest)
        ferVec3Copy(sep, &nearest->witness);

    ferCCDPtDestroy(&polytope);

    return ret;
}


static int penEPAPosCmp(const void *a, const void *b)
{
    fer_ccd_pt_vertex_t *v1, *v2;
    v1 = *(fer_ccd_pt_vertex_t **)a;
    v2 = *(fer_ccd_pt_vertex_t **)b;

    if (ferEq(v1->dist, v2->dist)){
        return 0;
    }else if (v1->dist < v2->dist){
        return -1;
    }else{
        return 1;
    }
}

static void penEPAPos(const fer_ccd_pt_t *pt, const fer_ccd_pt_el_t *nearest,
                      fer_vec3_t *pos)
{
    fer_ccd_pt_vertex_t *v;
    fer_ccd_pt_vertex_t **vs;
    size_t i, len;
    fer_real_t scale;

    // compute median
    len = 0;
    FER_LIST_FOR_EACH_ENTRY(&pt->vertices, fer_ccd_pt_vertex_t, v, list){
        len++;
    }

    vs = FER_ALLOC_ARR(fer_ccd_pt_vertex_t *, len);
    i = 0;
    FER_LIST_FOR_EACH_ENTRY(&pt->vertices, fer_ccd_pt_vertex_t, v, list){
        vs[i++] = v;
    }

    qsort(vs, len, sizeof(fer_ccd_pt_vertex_t *), penEPAPosCmp);

    ferVec3Set(pos, FER_ZERO, FER_ZERO, FER_ZERO);
    scale = FER_ZERO;
    if (len % 2 == 1)
        len++;

    for (i = 0; i < len / 2; i++){
        ferVec3Add(pos, &vs[i]->v.v1);
        ferVec3Add(pos, &vs[i]->v.v2);
        scale += FER_REAL(2.);
    }
    ferVec3Scale(pos, FER_ONE / scale);

    free(vs);
}

int ferCCDGJKPenetration(const fer_ccd_t *ccd,
                         const void *obj1, const void *obj2,
                         fer_real_t *depth, fer_vec3_t *dir, fer_vec3_t *pos)
{
    fer_ccd_pt_t polytope;
    fer_ccd_pt_el_t *nearest;
    int ret;

    ferCCDPtInit(&polytope);

    ret = __ferGJKEPA(obj1, obj2, ccd, &polytope, &nearest);

    // set separation vector
    if (ret == 0 && nearest){
        // compute depth of penetration
        *depth = FER_SQRT(nearest->dist);

        // store normalized direction vector
        ferVec3Copy(dir, &nearest->witness);
        ferVec3Normalize(dir);

        // compute position
        penEPAPos(&polytope, nearest, pos);
    }

    ferCCDPtDestroy(&polytope);

    return ret;
}




static int __ferGJK(const void *obj1, const void *obj2,
                    const fer_ccd_t *ccd, fer_ccd_simplex_t *simplex)
{
    unsigned long iterations;
    fer_vec3_t dir; // direction vector
    fer_ccd_support_t last; // last support point
    int do_simplex_res;

    // initialize simplex struct
    ferCCDSimplexInit(simplex);

    // get first direction
    ccd->first_dir(obj1, obj2, &dir);
    // get first support point
    __ferCCDSupport(obj1, obj2, &dir, ccd, &last);
    // and add this point to simplex as last one
    ferCCDSimplexAdd(simplex, &last);

    // set up direction vector to as (O - last) which is exactly -last
    ferVec3Copy(&dir, &last.v);
    ferVec3Scale(&dir, -FER_ONE);

    // start iterations
    for (iterations = 0UL; iterations < ccd->max_iterations; ++iterations) {
        // obtain support point
        __ferCCDSupport(obj1, obj2, &dir, ccd, &last);

        // check if farthest point in Minkowski difference in direction dir
        // isn't somewhere before origin (the test on negative dot product)
        // - because if it is, objects are not intersecting at all.
        if (ferVec3Dot(&last.v, &dir) < FER_ZERO){
            return -1; // intersection not found
        }

        // add last support vector to simplex
        ferCCDSimplexAdd(simplex, &last);

        // if doSimplex returns 1 if objects intersect, -1 if objects don't
        // intersect and 0 if algorithm should continue
        do_simplex_res = doSimplex(simplex, &dir);
        if (do_simplex_res == 1){
            return 0; // intersection found
        }else if (do_simplex_res == -1){
            return -1; // intersection not found
        }

        if (ferIsZero(ferVec3Len2(&dir))){
            return -1; // intersection not found
        }
    }

    // intersection wasn't found
    return -1;
}

static int __ferGJKEPA(const void *obj1, const void *obj2,
                       const fer_ccd_t *ccd,
                       fer_ccd_pt_t *polytope, fer_ccd_pt_el_t **nearest)
{
    fer_ccd_simplex_t simplex;
    fer_ccd_support_t supp; // support point
    int ret, size;

    *nearest = NULL;

    // run GJK and obtain terminal simplex
    ret = __ferGJK(obj1, obj2, ccd, &simplex);
    if (ret != 0)
        return -1;

    // transform simplex to polytope - simplex won't be used anymore
    size = ferCCDSimplexSize(&simplex);
    if (size == 4){
        if (simplexToPolytope4(obj1, obj2, ccd, &simplex, polytope, nearest) != 0){
            return 0;// touch contact
        }
    }else if (size == 3){
        if (simplexToPolytope3(obj1, obj2, ccd, &simplex, polytope, nearest) != 0){
            return 0; // touch contact
        }
    }else{ // size == 2
        if (simplexToPolytope2(obj1, obj2, ccd, &simplex, polytope, nearest) != 0){
            return 0; // touch contact
        }
    }

    while (1){
        // get triangle nearest to origin
        *nearest = ferCCDPtNearest(polytope);

        // get next support point
        if (nextSupport(obj1, obj2, ccd, *nearest, &supp) != 0)
            break;

        // expand nearest triangle using new point - supp
        expandPolytope(polytope, *nearest, &supp);
    }

    return 0;
}



static int doSimplex2(fer_ccd_simplex_t *simplex, fer_vec3_t *dir)
{
    const fer_ccd_support_t *A, *B;
    fer_vec3_t AB, AO, tmp;
    fer_real_t dot;

    // get last added as A
    A = ferCCDSimplexLast(simplex);
    // get the other point
    B = ferCCDSimplexPoint(simplex, 0);
    // compute AB oriented segment
    ferVec3Sub2(&AB, &B->v, &A->v);
    // compute AO vector
    ferVec3Copy(&AO, &A->v);
    ferVec3Scale(&AO, -FER_ONE);

    // dot product AB . AO
    dot = ferVec3Dot(&AB, &AO);

    // check if origin doesn't lie on AB segment
    ferVec3Cross(&tmp, &AB, &AO);
    if (ferIsZero(ferVec3Len2(&tmp)) && dot > FER_ZERO){
        return 1;
    }

    // check if origin is in area where AB segment is
    if (ferIsZero(dot) || dot < FER_ZERO){
        // origin is in outside are of A
        ferCCDSimplexSet(simplex, 0, A);
        ferCCDSimplexSetSize(simplex, 1);
        ferVec3Copy(dir, &AO);
    }else{
        // origin is in area where AB segment is

        // keep simplex untouched and set direction to
        // AB x AO x AB
        tripleCross(&AB, &AO, &AB, dir);
    }

    return 0;
}

static int doSimplex3(fer_ccd_simplex_t *simplex, fer_vec3_t *dir)
{
    const fer_ccd_support_t *A, *B, *C;
    fer_vec3_t AO, AB, AC, ABC, tmp;
    fer_real_t dot, dist;

    // get last added as A
    A = ferCCDSimplexLast(simplex);
    // get the other points
    B = ferCCDSimplexPoint(simplex, 1);
    C = ferCCDSimplexPoint(simplex, 0);

    // check touching contact
    dist = ferVec3PointTriDist2(fer_vec3_origin, &A->v, &B->v, &C->v, NULL);
    if (ferIsZero(dist)){
        return 1;
    }

    // check if triangle is really triangle (has area > 0)
    // if not simplex can't be expanded and thus no itersection is found
    if (ferVec3Eq(&A->v, &B->v) || ferVec3Eq(&A->v, &C->v)){
        return -1;
    }

    // compute AO vector
    ferVec3Copy(&AO, &A->v);
    ferVec3Scale(&AO, -FER_ONE);

    // compute AB and AC segments and ABC vector (perpendircular to triangle)
    ferVec3Sub2(&AB, &B->v, &A->v);
    ferVec3Sub2(&AC, &C->v, &A->v);
    ferVec3Cross(&ABC, &AB, &AC);

    ferVec3Cross(&tmp, &ABC, &AC);
    dot = ferVec3Dot(&tmp, &AO);
    if (ferIsZero(dot) || dot > FER_ZERO){
        dot = ferVec3Dot(&AC, &AO);
        if (ferIsZero(dot) || dot > FER_ZERO){
            // C is already in place
            ferCCDSimplexSet(simplex, 1, A);
            ferCCDSimplexSetSize(simplex, 2);
            tripleCross(&AC, &AO, &AC, dir);
        }else{
fer_do_simplex3_45:
            dot = ferVec3Dot(&AB, &AO);
            if (ferIsZero(dot) || dot > FER_ZERO){
                ferCCDSimplexSet(simplex, 0, B);
                ferCCDSimplexSet(simplex, 1, A);
                ferCCDSimplexSetSize(simplex, 2);
                tripleCross(&AB, &AO, &AB, dir);
            }else{
                ferCCDSimplexSet(simplex, 0, A);
                ferCCDSimplexSetSize(simplex, 1);
                ferVec3Copy(dir, &AO);
            }
        }
    }else{
        ferVec3Cross(&tmp, &AB, &ABC);
        dot = ferVec3Dot(&tmp, &AO);
        if (ferIsZero(dot) || dot > FER_ZERO){
            goto fer_do_simplex3_45;
        }else{
            dot = ferVec3Dot(&ABC, &AO);
            if (ferIsZero(dot) || dot > FER_ZERO){
                ferVec3Copy(dir, &ABC);
            }else{
                fer_ccd_support_t Ctmp;
                ferCCDSupportCopy(&Ctmp, C);
                ferCCDSimplexSet(simplex, 0, B);
                ferCCDSimplexSet(simplex, 1, &Ctmp);

                ferVec3Copy(dir, &ABC);
                ferVec3Scale(dir, -FER_ONE);
            }
        }
    }

    return 0;
}

static int doSimplex4(fer_ccd_simplex_t *simplex, fer_vec3_t *dir)
{
    const fer_ccd_support_t *A, *B, *C, *D;
    fer_vec3_t AO, AB, AC, AD, ABC, ACD, ADB;
    int B_on_ACD, C_on_ADB, D_on_ABC;
    int AB_O, AC_O, AD_O;
    fer_real_t dist;

    // get last added as A
    A = ferCCDSimplexLast(simplex);
    // get the other points
    B = ferCCDSimplexPoint(simplex, 2);
    C = ferCCDSimplexPoint(simplex, 1);
    D = ferCCDSimplexPoint(simplex, 0);

    // check if tetrahedron is really tetrahedron (has volume > 0)
    // if it is not simplex can't be expanded and thus no intersection is
    // found
    dist = ferVec3PointTriDist2(&A->v, &B->v, &C->v, &D->v, NULL);
    if (ferIsZero(dist)){
        return -1;
    }

    // check if origin lies on some of tetrahedron's face - if so objects
    // intersect
    dist = ferVec3PointTriDist2(fer_vec3_origin, &A->v, &B->v, &C->v, NULL);
    if (ferIsZero(dist))
        return 1;
    dist = ferVec3PointTriDist2(fer_vec3_origin, &A->v, &C->v, &D->v, NULL);
    if (ferIsZero(dist))
        return 1;
    dist = ferVec3PointTriDist2(fer_vec3_origin, &A->v, &B->v, &D->v, NULL);
    if (ferIsZero(dist))
        return 1;
    dist = ferVec3PointTriDist2(fer_vec3_origin, &B->v, &C->v, &D->v, NULL);
    if (ferIsZero(dist))
        return 1;

    // compute AO, AB, AC, AD segments and ABC, ACD, ADB normal vectors
    ferVec3Copy(&AO, &A->v);
    ferVec3Scale(&AO, -FER_ONE);
    ferVec3Sub2(&AB, &B->v, &A->v);
    ferVec3Sub2(&AC, &C->v, &A->v);
    ferVec3Sub2(&AD, &D->v, &A->v);
    ferVec3Cross(&ABC, &AB, &AC);
    ferVec3Cross(&ACD, &AC, &AD);
    ferVec3Cross(&ADB, &AD, &AB);

    // side (positive or negative) of B, C, D relative to planes ACD, ADB
    // and ABC respectively
    B_on_ACD = ferSign(ferVec3Dot(&ACD, &AB));
    C_on_ADB = ferSign(ferVec3Dot(&ADB, &AC));
    D_on_ABC = ferSign(ferVec3Dot(&ABC, &AD));

    // whether origin is on same side of ACD, ADB, ABC as B, C, D
    // respectively
    AB_O = ferSign(ferVec3Dot(&ACD, &AO)) == B_on_ACD;
    AC_O = ferSign(ferVec3Dot(&ADB, &AO)) == C_on_ADB;
    AD_O = ferSign(ferVec3Dot(&ABC, &AO)) == D_on_ABC;

    if (AB_O && AC_O && AD_O){
        // origin is in tetrahedron
        return 1;

    // rearrange simplex to triangle and call doSimplex3()
    }else if (!AB_O){
        // B is farthest from the origin among all of the tetrahedron's
        // points, so remove it from the list and go on with the triangle
        // case

        // D and C are in place
        ferCCDSimplexSet(simplex, 2, A);
        ferCCDSimplexSetSize(simplex, 3);
    }else if (!AC_O){
        // C is farthest
        ferCCDSimplexSet(simplex, 1, D);
        ferCCDSimplexSet(simplex, 0, B);
        ferCCDSimplexSet(simplex, 2, A);
        ferCCDSimplexSetSize(simplex, 3);
    }else{ // (!AD_O)
        ferCCDSimplexSet(simplex, 0, C);
        ferCCDSimplexSet(simplex, 1, B);
        ferCCDSimplexSet(simplex, 2, A);
        ferCCDSimplexSetSize(simplex, 3);
    }

    return doSimplex3(simplex, dir);
}

static int doSimplex(fer_ccd_simplex_t *simplex, fer_vec3_t *dir)
{
    if (ferCCDSimplexSize(simplex) == 2){
        // simplex contains segment only one segment
        return doSimplex2(simplex, dir);
    }else if (ferCCDSimplexSize(simplex) == 3){
        // simplex contains triangle
        return doSimplex3(simplex, dir);
    }else{ // ferCCDSimplexSize(simplex) == 4
        // tetrahedron - this is the only shape which can encapsule origin
        // so doSimplex4() also contains test on it
        return doSimplex4(simplex, dir);
    }
}

_fer_inline void tripleCross(const fer_vec3_t *a, const fer_vec3_t *b,
                             const fer_vec3_t *c, fer_vec3_t *d)
{
    fer_vec3_t e;
    ferVec3Cross(&e, a, b);
    ferVec3Cross(d, &e, c);
}



/** Transforms simplex to polytope. It is assumed that simplex has 4
 *  vertices! */
static int simplexToPolytope4(const void *obj1, const void *obj2,
                              const fer_ccd_t *ccd,
                              fer_ccd_simplex_t *simplex,
                              fer_ccd_pt_t *pt, fer_ccd_pt_el_t **nearest)
{
    const fer_ccd_support_t *a, *b, *c, *d;
    int use_polytope3;
    fer_real_t dist;
    fer_ccd_pt_vertex_t *v[4];
    fer_ccd_pt_edge_t *e[6];
    size_t i;

    a = ferCCDSimplexPoint(simplex, 0);
    b = ferCCDSimplexPoint(simplex, 1);
    c = ferCCDSimplexPoint(simplex, 2);
    d = ferCCDSimplexPoint(simplex, 3);

    // check if origin lies on some of tetrahedron's face - if so use
    // simplexToPolytope3()
    use_polytope3 = 0;
    dist = ferVec3PointTriDist2(fer_vec3_origin, &a->v, &b->v, &c->v, NULL);
    if (ferIsZero(dist)){
        use_polytope3 = 1;
    }
    dist = ferVec3PointTriDist2(fer_vec3_origin, &a->v, &c->v, &d->v, NULL);
    if (ferIsZero(dist)){
        use_polytope3 = 1;
        ferCCDSimplexSet(simplex, 1, c);
        ferCCDSimplexSet(simplex, 2, d);
    }
    dist = ferVec3PointTriDist2(fer_vec3_origin, &a->v, &b->v, &d->v, NULL);
    if (ferIsZero(dist)){
        use_polytope3 = 1;
        ferCCDSimplexSet(simplex, 2, d);
    }
    dist = ferVec3PointTriDist2(fer_vec3_origin, &b->v, &c->v, &d->v, NULL);
    if (ferIsZero(dist)){
        use_polytope3 = 1;
        ferCCDSimplexSet(simplex, 0, b);
        ferCCDSimplexSet(simplex, 1, c);
        ferCCDSimplexSet(simplex, 2, d);
    }

    if (use_polytope3){
        ferCCDSimplexSetSize(simplex, 3);
        return simplexToPolytope3(obj1, obj2, ccd, simplex, pt, nearest);
    }

    // no touching contact - simply create tetrahedron
    for (i = 0; i < 4; i++){
        v[i] = ferCCDPtAddVertex(pt, ferCCDSimplexPoint(simplex, i));
    }
    
    e[0] = ferCCDPtAddEdge(pt, v[0], v[1]);
    e[1] = ferCCDPtAddEdge(pt, v[1], v[2]);
    e[2] = ferCCDPtAddEdge(pt, v[2], v[0]);
    e[3] = ferCCDPtAddEdge(pt, v[3], v[0]);
    e[4] = ferCCDPtAddEdge(pt, v[3], v[1]);
    e[5] = ferCCDPtAddEdge(pt, v[3], v[2]);

    ferCCDPtAddFace(pt, e[0], e[1], e[2]);
    ferCCDPtAddFace(pt, e[3], e[4], e[0]);
    ferCCDPtAddFace(pt, e[4], e[5], e[1]);
    ferCCDPtAddFace(pt, e[5], e[3], e[2]);

    return 0;
}

/** Transforms simplex to polytope, three vertices required */
static int simplexToPolytope3(const void *obj1, const void *obj2,
                              const fer_ccd_t *ccd,
                              const fer_ccd_simplex_t *simplex,
                              fer_ccd_pt_t *pt, fer_ccd_pt_el_t **nearest)
{
    const fer_ccd_support_t *a, *b, *c;
    fer_ccd_support_t d, d2;
    fer_vec3_t ab, ac, dir;
    fer_ccd_pt_vertex_t *v[5];
    fer_ccd_pt_edge_t *e[9];
    fer_real_t dist, dist2;

    *nearest = NULL;

    a = ferCCDSimplexPoint(simplex, 0);
    b = ferCCDSimplexPoint(simplex, 1);
    c = ferCCDSimplexPoint(simplex, 2);

    // If only one triangle left from previous GJK run origin lies on this
    // triangle. So it is necessary to expand triangle into two
    // tetrahedrons connected with base (which is exactly abc triangle).

    // get next support point in direction of normal of triangle
    ferVec3Sub2(&ab, &b->v, &a->v);
    ferVec3Sub2(&ac, &c->v, &a->v);
    ferVec3Cross(&dir, &ab, &ac);
    __ferCCDSupport(obj1, obj2, &dir, ccd, &d);
    dist = ferVec3PointTriDist2(&d.v, &a->v, &b->v, &c->v, NULL);

    // and second one take in opposite direction
    ferVec3Scale(&dir, -FER_ONE);
    __ferCCDSupport(obj1, obj2, &dir, ccd, &d2);
    dist2 = ferVec3PointTriDist2(&d2.v, &a->v, &b->v, &c->v, NULL);

    // check if face isn't already on edge of minkowski sum and thus we
    // have touching contact
    if (ferIsZero(dist) || ferIsZero(dist2)){
        v[0] = ferCCDPtAddVertex(pt, a);
        v[1] = ferCCDPtAddVertex(pt, b);
        v[2] = ferCCDPtAddVertex(pt, c);
        e[0] = ferCCDPtAddEdge(pt, v[0], v[1]);
        e[1] = ferCCDPtAddEdge(pt, v[1], v[2]);
        e[2] = ferCCDPtAddEdge(pt, v[2], v[0]);
        *nearest = (fer_ccd_pt_el_t *)ferCCDPtAddFace(pt, e[0], e[1], e[2]);

        return -1;
    }

    // form polyhedron
    v[0] = ferCCDPtAddVertex(pt, a);
    v[1] = ferCCDPtAddVertex(pt, b);
    v[2] = ferCCDPtAddVertex(pt, c);
    v[3] = ferCCDPtAddVertex(pt, &d);
    v[4] = ferCCDPtAddVertex(pt, &d2);

    e[0] = ferCCDPtAddEdge(pt, v[0], v[1]);
    e[1] = ferCCDPtAddEdge(pt, v[1], v[2]);
    e[2] = ferCCDPtAddEdge(pt, v[2], v[0]);

    e[3] = ferCCDPtAddEdge(pt, v[3], v[0]);
    e[4] = ferCCDPtAddEdge(pt, v[3], v[1]);
    e[5] = ferCCDPtAddEdge(pt, v[3], v[2]);

    e[6] = ferCCDPtAddEdge(pt, v[4], v[0]);
    e[7] = ferCCDPtAddEdge(pt, v[4], v[1]);
    e[8] = ferCCDPtAddEdge(pt, v[4], v[2]);

    ferCCDPtAddFace(pt, e[3], e[4], e[0]);
    ferCCDPtAddFace(pt, e[4], e[5], e[1]);
    ferCCDPtAddFace(pt, e[5], e[3], e[2]);

    ferCCDPtAddFace(pt, e[6], e[7], e[0]);
    ferCCDPtAddFace(pt, e[7], e[8], e[1]);
    ferCCDPtAddFace(pt, e[8], e[6], e[2]);

    return 0;
}

/** Transforms simplex to polytope, two vertices required */
static int simplexToPolytope2(const void *obj1, const void *obj2,
                              const fer_ccd_t *ccd,
                              const fer_ccd_simplex_t *simplex,
                              fer_ccd_pt_t *pt, fer_ccd_pt_el_t **nearest)
{
    const fer_ccd_support_t *a, *b;
    fer_vec3_t ab, ac, dir;
    fer_ccd_support_t supp[4];
    fer_ccd_pt_vertex_t *v[6];
    fer_ccd_pt_edge_t *e[12];
    size_t i;
    int found;

    a = ferCCDSimplexPoint(simplex, 0);
    b = ferCCDSimplexPoint(simplex, 1);

    // This situation is a bit tricky. If only one segment comes from
    // previous run of GJK - it means that either this segment is on
    // minkowski edge (and thus we have touch contact) or it it isn't and
    // therefore segment is somewhere *inside* minkowski sum and it *must*
    // be possible to fully enclose this segment with polyhedron formed by
    // at least 8 triangle faces.

    // get first support point (any)
    found = 0;
    for (i = 0; i < fer_points_on_sphere_len; i++){
        __ferCCDSupport(obj1, obj2, &fer_points_on_sphere[i], ccd, &supp[0]);
        if (!ferVec3Eq(&a->v, &supp[0].v) && !ferVec3Eq(&b->v, &supp[0].v)){
            found = 1;
            break;
        }
    }
    if (!found)
        goto simplexToPolytope2_touching_contact;

    // get second support point in opposite direction than supp[0]
    ferVec3Copy(&dir, &supp[0].v);
    ferVec3Scale(&dir, -FER_ONE);
    __ferCCDSupport(obj1, obj2, &dir, ccd, &supp[1]);
    if (ferVec3Eq(&a->v, &supp[1].v) || ferVec3Eq(&b->v, &supp[1].v))
        goto simplexToPolytope2_touching_contact;

    // next will be in direction of normal of triangle a,supp[0],supp[1]
    ferVec3Sub2(&ab, &supp[0].v, &a->v);
    ferVec3Sub2(&ac, &supp[1].v, &a->v);
    ferVec3Cross(&dir, &ab, &ac);
    __ferCCDSupport(obj1, obj2, &dir, ccd, &supp[2]);
    if (ferVec3Eq(&a->v, &supp[2].v) || ferVec3Eq(&b->v, &supp[2].v))
        goto simplexToPolytope2_touching_contact;

    // and last one will be in opposite direction
    ferVec3Scale(&dir, -FER_ONE);
    __ferCCDSupport(obj1, obj2, &dir, ccd, &supp[3]);
    if (ferVec3Eq(&a->v, &supp[3].v) || ferVec3Eq(&b->v, &supp[3].v))
        goto simplexToPolytope2_touching_contact;

    goto simplexToPolytope2_not_touching_contact;
simplexToPolytope2_touching_contact:
    v[0] = ferCCDPtAddVertex(pt, a);
    v[1] = ferCCDPtAddVertex(pt, b);
    *nearest = (fer_ccd_pt_el_t *)ferCCDPtAddEdge(pt, v[0], v[1]);
    return -1;

simplexToPolytope2_not_touching_contact:
    // form polyhedron
    v[0] = ferCCDPtAddVertex(pt, a);
    v[1] = ferCCDPtAddVertex(pt, &supp[0]);
    v[2] = ferCCDPtAddVertex(pt, b);
    v[3] = ferCCDPtAddVertex(pt, &supp[1]);
    v[4] = ferCCDPtAddVertex(pt, &supp[2]);
    v[5] = ferCCDPtAddVertex(pt, &supp[3]);

    e[0] = ferCCDPtAddEdge(pt, v[0], v[1]);
    e[1] = ferCCDPtAddEdge(pt, v[1], v[2]);
    e[2] = ferCCDPtAddEdge(pt, v[2], v[3]);
    e[3] = ferCCDPtAddEdge(pt, v[3], v[0]);

    e[4] = ferCCDPtAddEdge(pt, v[4], v[0]);
    e[5] = ferCCDPtAddEdge(pt, v[4], v[1]);
    e[6] = ferCCDPtAddEdge(pt, v[4], v[2]);
    e[7] = ferCCDPtAddEdge(pt, v[4], v[3]);

    e[8]  = ferCCDPtAddEdge(pt, v[5], v[0]);
    e[9]  = ferCCDPtAddEdge(pt, v[5], v[1]);
    e[10] = ferCCDPtAddEdge(pt, v[5], v[2]);
    e[11] = ferCCDPtAddEdge(pt, v[5], v[3]);

    ferCCDPtAddFace(pt, e[4], e[5], e[0]);
    ferCCDPtAddFace(pt, e[5], e[6], e[1]);
    ferCCDPtAddFace(pt, e[6], e[7], e[2]);
    ferCCDPtAddFace(pt, e[7], e[4], e[3]);

    ferCCDPtAddFace(pt, e[8],  e[9],  e[0]);
    ferCCDPtAddFace(pt, e[9],  e[10], e[1]);
    ferCCDPtAddFace(pt, e[10], e[11], e[2]);
    ferCCDPtAddFace(pt, e[11], e[8],  e[3]);

    return 0;
}

/** Expands polytope's tri by new vertex v. Triangle tri is replaced by
 *  three triangles each with one vertex in v. */
static void expandPolytope(fer_ccd_pt_t *pt, fer_ccd_pt_el_t *el,
                           const fer_ccd_support_t *newv)
{
    fer_ccd_pt_vertex_t *v[5];
    fer_ccd_pt_edge_t *e[8];
    fer_ccd_pt_face_t *f[2];


    // element can be either segment or triangle
    if (el->type == FER_CCD_PT_EDGE){
        // In this case, segment should be replaced by new point.
        // Simpliest case is when segment stands alone and in this case
        // this segment is replaced by two other segments both connected to
        // newv.
        // Segment can be also connected to max two faces and in that case
        // each face must be replaced by two other faces. To do this
        // correctly it is necessary to have correctly ordered edges and
        // vertices which is exactly what is done in following code.
        //

        ferCCDPtEdgeVertices((const fer_ccd_pt_edge_t *)el, &v[0], &v[2]);

        ferCCDPtEdgeFaces((fer_ccd_pt_edge_t *)el, &f[0], &f[1]);

        if (f[0]){
            e[3] = NULL;
            ferCCDPtFaceEdges(f[0], &e[0], &e[1], &e[2]);
            if (e[0] == (fer_ccd_pt_edge_t *)el){
                e[0] = e[2];
            }else if (e[1] == (fer_ccd_pt_edge_t *)el){
                e[1] = e[2];
            }
            ferCCDPtEdgeVertices(e[0], &v[1], &v[3]);
            if (v[1] != v[0] && v[3] != v[0]){
                e[2] = e[0];
                e[0] = e[1];
                e[1] = e[2];
                if (v[1] == v[2])
                    v[1] = v[3];
            }else{
                if (v[1] == v[0])
                    v[1] = v[3];
            }

            if (f[1]){
                ferCCDPtFaceEdges(f[1], &e[2], &e[3], &e[4]);
                if (e[2] == (fer_ccd_pt_edge_t *)el){
                    e[2] = e[4];
                }else if (e[3] == (fer_ccd_pt_edge_t *)el){
                    e[3] = e[4];
                }
                ferCCDPtEdgeVertices(e[2], &v[3], &v[4]);
                if (v[3] != v[2] && v[4] != v[2]){
                    e[4] = e[2];
                    e[2] = e[3];
                    e[3] = e[4];
                    if (v[3] == v[0])
                        v[3] = v[4];
                }else{
                    if (v[3] == v[2])
                        v[3] = v[4];
                }
            }


            v[4] = ferCCDPtAddVertex(pt, newv);

            ferCCDPtDelFace(pt, f[0]);
            if (f[1]){
                ferCCDPtDelFace(pt, f[1]);
                ferCCDPtDelEdge(pt, (fer_ccd_pt_edge_t *)el);
            }

            e[4] = ferCCDPtAddEdge(pt, v[4], v[2]);
            e[5] = ferCCDPtAddEdge(pt, v[4], v[0]);
            e[6] = ferCCDPtAddEdge(pt, v[4], v[1]);
            if (f[1])
                e[7] = ferCCDPtAddEdge(pt, v[4], v[3]);

            ferCCDPtAddFace(pt, e[1], e[4], e[6]);
            ferCCDPtAddFace(pt, e[0], e[6], e[5]);
            if (f[1]){
                ferCCDPtAddFace(pt, e[3], e[5], e[7]);
                ferCCDPtAddFace(pt, e[4], e[7], e[2]);
            }else{
                ferCCDPtAddFace(pt, e[4], e[5], (fer_ccd_pt_edge_t *)el);
            }
        }
    }else{ // el->type == FER_CCD_PT_FACE
        // replace triangle by tetrahedron without base (base would be the
        // triangle that will be removed)

        // get triplet of surrounding edges and vertices of triangle face
        ferCCDPtFaceEdges((const fer_ccd_pt_face_t *)el, &e[0], &e[1], &e[2]);
        ferCCDPtEdgeVertices(e[0], &v[0], &v[1]);
        ferCCDPtEdgeVertices(e[1], &v[2], &v[3]);

        // following code sorts edges to have e[0] between vertices 0-1,
        // e[1] between 1-2 and e[2] between 2-0
        if (v[2] != v[1] && v[3] != v[1]){
            // swap e[1] and e[2] 
            e[3] = e[1];
            e[1] = e[2];
            e[2] = e[3];
        }
        if (v[3] != v[0] && v[3] != v[1])
            v[2] = v[3];

        // remove triangle face
        ferCCDPtDelFace(pt, (fer_ccd_pt_face_t *)el);

        // expand triangle to tetrahedron
        v[3] = ferCCDPtAddVertex(pt, newv);
        e[3] = ferCCDPtAddEdge(pt, v[3], v[0]);
        e[4] = ferCCDPtAddEdge(pt, v[3], v[1]);
        e[5] = ferCCDPtAddEdge(pt, v[3], v[2]);

        ferCCDPtAddFace(pt, e[3], e[4], e[0]);
        ferCCDPtAddFace(pt, e[4], e[5], e[1]);
        ferCCDPtAddFace(pt, e[5], e[3], e[2]);
    }
}

/** Finds next support point (at stores it in out argument).
 *  Returns 0 on success, -1 otherwise */
static int nextSupport(const void *obj1, const void *obj2, const fer_ccd_t *ccd,
                       const fer_ccd_pt_el_t *el,
                       fer_ccd_support_t *out)
{
    fer_vec3_t *a, *b, *c;
    fer_real_t dist;

    if (el->type == FER_CCD_PT_VERTEX)
        return -1;

    // touch contact
    if (ferIsZero(el->dist))
        return -1;

    __ferCCDSupport(obj1, obj2, &el->witness, ccd, out);

    if (el->type == FER_CCD_PT_EDGE){
        // fetch end points of edge
        ferCCDPtEdgeVec3((fer_ccd_pt_edge_t *)el, &a, &b);

        // get distance from segment
        dist = ferVec3PointSegmentDist2(&out->v, a, b, NULL);
    }else{ // el->type == FER_CCD_PT_FACE
        // fetch vertices of triangle face
        ferCCDPtFaceVec3((fer_ccd_pt_face_t *)el, &a, &b, &c);

        // check if new point can significantly expand polytope
        dist = ferVec3PointTriDist2(&out->v, a, b, c, NULL);
    }

    if (dist < ccd->epa_tolerance)
        return -1;

    return 0;
}
