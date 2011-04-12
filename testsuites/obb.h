#ifndef TEST_OBB_H
#define TEST_OBB_H


TEST(obbNew);
TEST(obbCollide);
TEST(obbTriMesh);

TEST_SUITE(TSOBB) {
    TEST_ADD(obbNew),
    TEST_ADD(obbCollide),
    TEST_ADD(obbTriMesh),

    TEST_SUITE_CLOSURE
};

#endif


