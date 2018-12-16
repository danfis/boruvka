#include <cu/cu.h>
#include <boruvka/quat.h>
#include <boruvka/dbg.h>

#include "data.h"

TEST(quatCore)
{
    bor_quat_t *a, b;
    bor_vec3_t v1, v2;

    a = borQuatNew(0., 1., 3., 1.);
    assertTrue(borEq(borQuatX(a), 0.));
    assertTrue(borEq(borQuatY(a), 1.));
    assertTrue(borEq(borQuatZ(a), 3.));
    assertTrue(borEq(borQuatW(a), 1.));

    borQuatSet(a, 1., 3., 4., 5.);
    assertTrue(borEq(borQuatX(a), 1.));
    assertTrue(borEq(borQuatY(a), 3.));
    assertTrue(borEq(borQuatZ(a), 4.));
    assertTrue(borEq(borQuatW(a), 5.));

    borQuatSetX(a, 2.);
    assertTrue(borEq(borQuatX(a), 2.));
    assertTrue(borEq(borQuatY(a), 3.));
    assertTrue(borEq(borQuatZ(a), 4.));
    assertTrue(borEq(borQuatW(a), 5.));

    borQuatSetY(a, 2.);
    assertTrue(borEq(borQuatX(a), 2.));
    assertTrue(borEq(borQuatY(a), 2.));
    assertTrue(borEq(borQuatZ(a), 4.));
    assertTrue(borEq(borQuatW(a), 5.));

    borQuatSetZ(a, 2.);
    assertTrue(borEq(borQuatX(a), 2.));
    assertTrue(borEq(borQuatY(a), 2.));
    assertTrue(borEq(borQuatZ(a), 2.));
    assertTrue(borEq(borQuatW(a), 5.));

    borQuatSetW(a, 2.);
    assertTrue(borEq(borQuatX(a), 2.));
    assertTrue(borEq(borQuatY(a), 2.));
    assertTrue(borEq(borQuatZ(a), 2.));
    assertTrue(borEq(borQuatW(a), 2.));

    assertTrue(borEq(borQuatLen2(a), 4. * 4.));

    borQuatScale(a, 0.5);
    assertTrue(borEq(borQuatX(a), 1.));
    assertTrue(borEq(borQuatY(a), 1.));
    assertTrue(borEq(borQuatZ(a), 1.));
    assertTrue(borEq(borQuatW(a), 1.));

    borQuatNormalize(a);
    assertTrue(borEq(borQuatX(a), .5));
    assertTrue(borEq(borQuatY(a), .5));
    assertTrue(borEq(borQuatZ(a), .5));
    assertTrue(borEq(borQuatW(a), .5));

    borQuatSet(&b, 2., 3., 4., 5.);
    borQuatMul(a, &b);
    assertTrue(borEq(borQuatX(a), 4.));
    assertTrue(borEq(borQuatY(a), 3.));
    assertTrue(borEq(borQuatZ(a), 5.));
    assertTrue(borEq(borQuatW(a), -2.));

    borQuatSet(a, 1., 2., 1., 2.);
    borQuatInvert(a);
    assertTrue(borEq(borQuatX(a), -.1));
    assertTrue(borEq(borQuatY(a), -.2));
    assertTrue(borEq(borQuatZ(a), -.1));
    assertTrue(borEq(borQuatW(a),  .2));

    borVec3Set(&v1, 0, 0, 1);
    borQuatSetAngleAxis(a, M_PI_2, &v1);
    assertTrue(borEq(borQuatX(a), 0.));
    assertTrue(borEq(borQuatY(a), 0.));
    assertTrue(borEq(borQuatZ(a), BOR_SQRT(2.)/2.));
    assertTrue(borEq(borQuatW(a), BOR_SQRT(2.)/2.));

    borVec3Set(&v2, 1., 0., 1.);
    borQuatRotVec(&v2, a);
    assertTrue(borEq(borQuatX(a), 0.));
    assertTrue(borEq(borQuatY(a), 0.));
    assertTrue(borEq(borQuatZ(a), BOR_SQRT(2.)/2.));
    assertTrue(borEq(borQuatW(a), BOR_SQRT(2.)/2.));
    assertTrue(borEq(borVec3Y(&v2), 1.));
    assertTrue(borEq(borVec3Z(&v2), 1.));

    borQuatDel(a);
}
