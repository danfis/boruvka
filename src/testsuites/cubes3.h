#ifndef TEST_CUBES_H
#define TEST_CUBES_H

TEST(cubesSetUp);
TEST(cubesTearDown);

TEST(cubesNew);

TEST(cubesEl);
TEST(cubesNearest);
TEST(cubesNearest2);

TEST_SUITE(TSCubes3) {
    TEST_ADD(cubesSetUp),

    TEST_ADD(cubesNew),
    TEST_ADD(cubesEl),
    TEST_ADD(cubesNearest),
    TEST_ADD(cubesNearest2),

    TEST_ADD(cubesTearDown),
    TEST_SUITE_CLOSURE
};

#endif
