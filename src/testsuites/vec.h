#ifndef TEST_VEC_H
#define TEST_VEC_H

#include "cu.h"

TEST(vecSetUp);
TEST(vecTearDown);

TEST(vecInit);
TEST(vecOperators);
/*
TEST(vecLength);
TEST(vecAngleArea);
TEST(vecIntersect);
TEST(vecSegmentInRect);
TEST(vecProjection);
TEST(vecAngleSameDir);

TEST(vecAdd);
TEST(vecSub);
TEST(vecScale);
TEST(vecNormalize);
TEST(vecDot);
TEST(vecMul);
TEST(vecLen);
TEST(vecDist);
TEST(vecArea);
TEST(vecAngle);
TEST(vecProjSeg);
TEST(vecInCircle);
TEST(vecLiesOn);
TEST(vecCollinear);
TEST(vecInCone);
TEST(vecInter);
TEST(vecOnLeft);
TEST(vecSegInRect);
*/

TEST_SUITE(TSVec)
{
    TEST_ADD(vecSetUp),

    TEST_ADD(vecInit),
    TEST_ADD(vecOperators),
    /*
    TEST_ADD(vecLength),
    TEST_ADD(vecAngleArea),
    TEST_ADD(vecIntersect),
    TEST_ADD(vecSegmentInRect),
    TEST_ADD(vecProjection),
    TEST_ADD(vecAngleSameDir),

    TEST_ADD(vecAdd),
    TEST_ADD(vecSub),
    TEST_ADD(vecScale),
    TEST_ADD(vecNormalize),
    TEST_ADD(vecDot),
    TEST_ADD(vecMul),
    TEST_ADD(vecLen),
    TEST_ADD(vecDist),
    TEST_ADD(vecArea),
    TEST_ADD(vecAngle),
    TEST_ADD(vecProjSeg),
    TEST_ADD(vecInCircle),
    TEST_ADD(vecLiesOn),
    TEST_ADD(vecCollinear),
    TEST_ADD(vecInCone),
    TEST_ADD(vecInter),
    TEST_ADD(vecOnLeft),
    TEST_ADD(vecSegInRect),
    */

    TEST_ADD(vecTearDown),
    TEST_SUITE_CLOSURE
};
#endif

