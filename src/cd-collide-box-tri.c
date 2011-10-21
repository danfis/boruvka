/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#include <fermat/cd.h>
#include <fermat/dbg.h>

static int overlap(const fer_cd_box_t *box,
                   const fer_vec3_t *tri);

int ferCDCollideBoxTri(struct _fer_cd_t *cd,
                       const fer_cd_box_t *b1,
                       const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                       const fer_cd_tri_t *t2,
                       const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    fer_vec3_t v[3];
    int i;
    fer_vec3_t tmpv;

    // compute tp[1,2,3] vertices of triangle in box's coordinate frame
    for (i = 0; i < 3; i++){
        ferMat3MulVec(&tmpv, rot2, t2->p[i]);
        ferVec3Add(&tmpv, tr2);
        ferVec3Sub(&tmpv, tr1);
        ferMat3MulVecTrans(&v[i], rot1, &tmpv);
        //ferVec3Sub(&v[i], tr1);
    }

    return overlap(b1, v);
}


/** Adapted code from
 *    http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox3.txt
 *  Thanks to Tomas Akenine-Moller
 */

#define FINDMINMAX(x0,x1,x2,min,max) \
    min = max = x0;   \
    if(x1<min) min=x1;\
    if(x1>max) max=x1;\
    if(x2<min) min=x2;\
    if(x2>max) max=x2;


/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)               \
    p0 = a*ferVec3Y(&tri[0]) - b*ferVec3Z(&tri[0]);                          \
    p2 = a*ferVec3Y(&tri[2]) - b*ferVec3Z(&tri[2]);                          \
    if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * ferVec3Y(box->half_extents) + fb * ferVec3Z(box->half_extents);   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_X2(a, b, fa, fb)               \
    p0 = a*ferVec3Y(&tri[0]) - b*ferVec3Z(&tri[0]);                       \
    p1 = a*ferVec3Y(&tri[1]) - b*ferVec3Z(&tri[1]);                          \
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * ferVec3Y(box->half_extents) + fb * ferVec3Z(box->half_extents);   \
    if(min>rad || max<-rad) return 0;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)               \
    p0 = -a*ferVec3X(&tri[0]) + b*ferVec3Z(&tri[0]);                     \
    p2 = -a*ferVec3X(&tri[2]) + b*ferVec3Z(&tri[2]);                             \
    if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * ferVec3X(box->half_extents) + fb * ferVec3Z(box->half_extents);   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb)               \
    p0 = -a*ferVec3X(&tri[0]) + b*ferVec3Z(&tri[0]);                     \
    p1 = -a*ferVec3X(&tri[1]) + b*ferVec3Z(&tri[1]);                           \
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * ferVec3X(box->half_extents) + fb * ferVec3Z(box->half_extents);   \
    if(min>rad || max<-rad) return 0;

/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)               \
    p1 = a*ferVec3X(&tri[1]) - b*ferVec3Y(&tri[1]);                       \
    p2 = a*ferVec3X(&tri[2]) - b*ferVec3Y(&tri[2]);                          \
    if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
    rad = fa * ferVec3X(box->half_extents) + fb * ferVec3Y(box->half_extents);   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)               \
    p0 = a*ferVec3X(&tri[0]) - b*ferVec3Y(&tri[0]);                   \
    p1 = a*ferVec3X(&tri[1]) - b*ferVec3Y(&tri[1]);                       \
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * ferVec3X(box->half_extents) + fb * ferVec3Y(box->half_extents);   \
    if(min>rad || max<-rad) return 0;


static int planeBoxOverlap(const fer_vec3_t *normal,
                           const fer_vec3_t *vert,
                           const fer_cd_box_t *box)
{
    int q;
    fer_real_t v;
    fer_vec3_t vmin, vmax;

    for (q = 0; q < 3; ++q){
        v = ferVec3Get(vert, q);
        if(ferVec3Get(normal, q) > FER_ZERO){
            ferVec3Set1(&vmin, q, -ferVec3Get(box->half_extents, q) - v);
            ferVec3Set1(&vmax, q,  ferVec3Get(box->half_extents, q) - v);
        }else{
            ferVec3Set1(&vmin, q,  ferVec3Get(box->half_extents, q) - v);
            ferVec3Set1(&vmax, q, -ferVec3Get(box->half_extents, q) - v);
        }
    }

    if (ferVec3Dot(normal, &vmin) > FER_ZERO)
        return 0;
    if (ferVec3Dot(normal, &vmax) >= FER_ZERO)
        return 1;

    return 0;
}

static int overlap(const fer_cd_box_t *box,
                   const fer_vec3_t *tri)
{
    /*    use separating axis theorem to test overlap between triangle and box */
    /*    need to test for overlap in these directions: */
    /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
    /*       we do not even need to test these) */
    /*    2) normal of the triangle */
    /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
    /*       this gives 3x3=9 more tests */
    fer_vec3_t e[3], normal;
    fer_real_t fex, fey, fez;
    fer_real_t min, max, p0, p1, p2, rad;

    /* compute triangle edges */
    ferVec3Sub2(&e[0], &tri[1], &tri[0]); /* tri edge 0 */
    ferVec3Sub2(&e[1], &tri[2], &tri[1]); /* tri edge 1 */
    ferVec3Sub2(&e[2], &tri[0], &tri[2]); /* tri edge 2 */

    /* Bullet 3:  */
    /*  test the 9 tests first (this was faster) */
    fex = FER_FABS(ferVec3X(&e[0]));
    fey = FER_FABS(ferVec3Y(&e[0]));
    fez = FER_FABS(ferVec3Z(&e[0]));
    AXISTEST_X01(ferVec3Z(&e[0]), ferVec3Y(&e[0]), fez, fey);
    AXISTEST_Y02(ferVec3Z(&e[0]), ferVec3X(&e[0]), fez, fex);
    AXISTEST_Z12(ferVec3Y(&e[0]), ferVec3X(&e[0]), fey, fex);

    fex = FER_FABS(ferVec3X(&e[1]));
    fey = FER_FABS(ferVec3Y(&e[1]));
    fez = FER_FABS(ferVec3Z(&e[1]));
    AXISTEST_X01(ferVec3Z(&e[1]), ferVec3Y(&e[1]), fez, fey);
    AXISTEST_Y02(ferVec3Z(&e[1]), ferVec3X(&e[1]), fez, fex);
    AXISTEST_Z0(ferVec3Y(&e[1]), ferVec3X(&e[1]), fey, fex);

    fex = FER_FABS(ferVec3X(&e[2]));
    fey = FER_FABS(ferVec3Y(&e[2]));
    fez = FER_FABS(ferVec3Z(&e[2]));
    AXISTEST_X2(ferVec3Z(&e[2]), ferVec3Y(&e[2]), fez, fey);
    AXISTEST_Y1(ferVec3Z(&e[2]), ferVec3X(&e[2]), fez, fex);
    AXISTEST_Z12(ferVec3Y(&e[2]), ferVec3X(&e[2]), fey, fex);

    /* Bullet 1: */
    /*  first test overlap in the {x,y,z}-directions */
    /*  find min, max of the triangle each direction, and test for overlap in */
    /*  that direction -- this is equivalent to testing a minimal AABB around */
    /*  the triangle against the AABB */

    /* test in X-direction */
    FINDMINMAX(ferVec3X(&tri[0]), ferVec3X(&tri[1]), ferVec3X(&tri[2]), min, max);
    if(min > ferVec3X(box->half_extents)
            || max < -ferVec3X(box->half_extents))
        return 0;

    /* test in Y-direction */
    FINDMINMAX(ferVec3Y(&tri[0]), ferVec3Y(&tri[1]), ferVec3Y(&tri[2]), min, max);
    if(min > ferVec3Y(box->half_extents)
            || max < -ferVec3Y(box->half_extents))
        return 0;

    /* test in Z-direction */
    FINDMINMAX(ferVec3Z(&tri[0]), ferVec3Z(&tri[1]), ferVec3Z(&tri[2]), min, max);
    if(min > ferVec3Z(box->half_extents)
            || max < -ferVec3Z(box->half_extents))
        return 0;

    /* Bullet 2: */
    /*  test if the box intersects the plane of the triangle */
    /*  compute plane equation of triangle: normal*x+d=0 */
    ferVec3Cross(&normal, &e[0], &e[1]);
    if(!planeBoxOverlap(&normal, &tri[0], box))
        return 0;

    return 1;   /* box and triangle overlaps */
}

