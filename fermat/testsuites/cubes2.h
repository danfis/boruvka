#ifndef TEST_CUBES2_H
#define TEST_CUBES2_H

TEST(cubes2SetUp);
TEST(cubes2TearDown);

TEST(cubes2New);

TEST(cubes2El);
TEST(cubes2Nearest);
/*
TEST(cubesNearest2);
*/

TEST_SUITE(TSCubes2) {
    TEST_ADD(cubes2SetUp),

    TEST_ADD(cubes2New),
    TEST_ADD(cubes2El),
    TEST_ADD(cubes2Nearest),
    /*
    TEST_ADD(cubesNearest2),
    */

    TEST_ADD(cubes2TearDown),
    TEST_SUITE_CLOSURE
};

#endif
