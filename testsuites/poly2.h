#ifndef TEST_POLY2_H
#define TEST_POLY2_H

#include "cu.h"

TEST(poly2SetUp);
TEST(poly2TearDown);

TEST(poly2Test);

TEST_SUITE(TSPoly2)
{
    TEST_ADD(poly2SetUp),

    TEST_ADD(poly2Test),

    TEST_ADD(poly2TearDown),
    TEST_SUITE_CLOSURE
};
#endif


