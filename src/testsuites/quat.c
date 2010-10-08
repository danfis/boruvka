#include <cu/cu.h>
#include <mg/quat.h>
#include <mg/dbg.h>

TEST(quatCore)
{
    mg_quat_t *a, b;
    mg_vec3_t v1, v2;

    a = mgQuatNew(0., 1., 3., 1.);
    assertTrue(mgEq(mgQuatX(a), 0.));
    assertTrue(mgEq(mgQuatY(a), 1.));
    assertTrue(mgEq(mgQuatZ(a), 3.));
    assertTrue(mgEq(mgQuatW(a), 1.));

    mgQuatSet(a, 1., 3., 4., 5.);
    assertTrue(mgEq(mgQuatX(a), 1.));
    assertTrue(mgEq(mgQuatY(a), 3.));
    assertTrue(mgEq(mgQuatZ(a), 4.));
    assertTrue(mgEq(mgQuatW(a), 5.));

    mgQuatSetX(a, 2.);
    assertTrue(mgEq(mgQuatX(a), 2.));
    assertTrue(mgEq(mgQuatY(a), 3.));
    assertTrue(mgEq(mgQuatZ(a), 4.));
    assertTrue(mgEq(mgQuatW(a), 5.));

    mgQuatSetY(a, 2.);
    assertTrue(mgEq(mgQuatX(a), 2.));
    assertTrue(mgEq(mgQuatY(a), 2.));
    assertTrue(mgEq(mgQuatZ(a), 4.));
    assertTrue(mgEq(mgQuatW(a), 5.));

    mgQuatSetZ(a, 2.);
    assertTrue(mgEq(mgQuatX(a), 2.));
    assertTrue(mgEq(mgQuatY(a), 2.));
    assertTrue(mgEq(mgQuatZ(a), 2.));
    assertTrue(mgEq(mgQuatW(a), 5.));

    mgQuatSetW(a, 2.);
    assertTrue(mgEq(mgQuatX(a), 2.));
    assertTrue(mgEq(mgQuatY(a), 2.));
    assertTrue(mgEq(mgQuatZ(a), 2.));
    assertTrue(mgEq(mgQuatW(a), 2.));

    assertTrue(mgEq(mgQuatLen2(a), 4. * 4.));

    mgQuatScale(a, 0.5);
    assertTrue(mgEq(mgQuatX(a), 1.));
    assertTrue(mgEq(mgQuatY(a), 1.));
    assertTrue(mgEq(mgQuatZ(a), 1.));
    assertTrue(mgEq(mgQuatW(a), 1.));

    mgQuatNormalize(a);
    assertTrue(mgEq(mgQuatX(a), .5));
    assertTrue(mgEq(mgQuatY(a), .5));
    assertTrue(mgEq(mgQuatZ(a), .5));
    assertTrue(mgEq(mgQuatW(a), .5));

    mgQuatSet(&b, 2., 3., 4., 5.);
    mgQuatMul(a, &b);
    assertTrue(mgEq(mgQuatX(a), 4.));
    assertTrue(mgEq(mgQuatY(a), 3.));
    assertTrue(mgEq(mgQuatZ(a), 5.));
    assertTrue(mgEq(mgQuatW(a), -2.));

    mgQuatSet(a, 1., 2., 1., 2.);
    mgQuatInvert(a);
    assertTrue(mgEq(mgQuatX(a), -.1));
    assertTrue(mgEq(mgQuatY(a), -.2));
    assertTrue(mgEq(mgQuatZ(a), -.1));
    assertTrue(mgEq(mgQuatW(a),  .2));

    mgVec3Set(&v1, 0, 0, 1);
    mgQuatSetAngleAxis(a, M_PI_2, &v1);
    assertTrue(mgEq(mgQuatX(a), 0.));
    assertTrue(mgEq(mgQuatY(a), 0.));
    assertTrue(mgEq(mgQuatZ(a), MG_SQRT(2.)/2.));
    assertTrue(mgEq(mgQuatW(a), MG_SQRT(2.)/2.));

    mgVec3Set(&v2, 1., 0., 1.);
    mgQuatRotVec(&v2, a);
    assertTrue(mgEq(mgQuatX(a), 0.));
    assertTrue(mgEq(mgQuatY(a), 0.));
    assertTrue(mgEq(mgQuatZ(a), MG_SQRT(2.)/2.));
    assertTrue(mgEq(mgQuatW(a), MG_SQRT(2.)/2.));
    assertTrue(mgEq(mgVec3Y(&v2), 1.));
    assertTrue(mgEq(mgVec3Z(&v2), 1.));

    mgQuatDel(a);
}

