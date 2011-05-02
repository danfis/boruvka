#ifndef TEST_CD_H
#define TEST_CD_H


TEST(cdOBBNew);
TEST(cdOBBNew2);
TEST(cdOBBNew3);
TEST(cdOBBCollide);

TEST(cdTriMesh);
TEST(cdSphere);
TEST(cdPairs1);

TEST_SUITE(TSCD) {
    TEST_ADD(cdOBBNew),
    TEST_ADD(cdOBBNew2),
    TEST_ADD(cdOBBNew3),
    TEST_ADD(cdOBBCollide),

    //TEST_ADD(cdTriMesh),
    //TEST_ADD(cdSphere),
    //TEST_ADD(cdPairs1),

    TEST_SUITE_CLOSURE
};

#endif


