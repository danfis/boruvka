#ifndef TEST_MESH3_H
#define TEST_MESH3_H

TEST(testMesh3SetUp);
TEST(testMesh3TearDown);

TEST(testMesh);
TEST(testMesh2);

TEST_SUITE(TSMesh3){
    TEST_ADD(testMesh3SetUp),

    TEST_ADD(testMesh),
    TEST_ADD(testMesh2),

    TEST_ADD(testMesh3TearDown),
    TEST_SUITE_CLOSURE
};

#endif
