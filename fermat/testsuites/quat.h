#ifndef TEST_QUAT_H
#define TEST_QUAT_H

#include <cu/cu.h>

TEST(quatCore);

TEST(quatLen2);
TEST(quatLen);
TEST(quatNormalize);
TEST(quatScale);
TEST(quatMul);
TEST(quatInvert);
TEST(quatRotVec);
TEST(quatAngleAxis);

TEST_SUITE(TSQuat) {
    TEST_ADD(quatCore),

    TEST_ADD(quatLen2),
    TEST_ADD(quatLen),
    TEST_ADD(quatNormalize),
    TEST_ADD(quatScale),
    TEST_ADD(quatMul),
    TEST_ADD(quatInvert),
    TEST_ADD(quatRotVec),
    TEST_ADD(quatAngleAxis),

    TEST_SUITE_CLOSURE
};

#endif
