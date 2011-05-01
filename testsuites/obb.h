#ifndef TEST_OBB_H
#define TEST_OBB_H


TEST(obbNew);
TEST(obbNew2);
TEST(obbNew3);
TEST(obbCollide);
TEST(obbTriMesh);
TEST(obbSphere);
TEST(obbPairs1);

TEST_SUITE(TSOBB) {
    TEST_ADD(obbNew),
    TEST_ADD(obbNew2),
    TEST_ADD(obbNew3),
    TEST_ADD(obbCollide),
    //TEST_ADD(obbTriMesh),
    //TEST_ADD(obbSphere),
    //TEST_ADD(obbPairs1),

    TEST_SUITE_CLOSURE
};

#endif


