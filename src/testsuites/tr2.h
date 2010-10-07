#ifndef TEST_TR2
#define TEST_TR2

#include "cu.h"

TEST(tr2SetUp);
TEST(tr2TearDown);

TEST(tr2Translation);
TEST(tr2Rotation);
TEST(tr2TransformVector);
TEST(tr2Clone);
TEST(tr2Copy);

TEST_SUITE(TSTr2) {
    TEST_ADD(tr2SetUp),

    TEST_ADD(tr2Translation),
    TEST_ADD(tr2Rotation),
    TEST_ADD(tr2TransformVector),
    TEST_ADD(tr2Clone),
    TEST_ADD(tr2Copy),

    TEST_ADD(tr2TearDown),
    TEST_SUITE_CLOSURE
};

#endif
