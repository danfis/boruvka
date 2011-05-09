#ifndef TEST_CD_H
#define TEST_CD_H


TEST(cdOBBNew);
TEST(cdOBBNew2);
TEST(cdOBBNew3);
TEST(cdOBBCollide);

TEST(cdCollideTriMesh);
TEST(cdCollideTriMesh2);
TEST(cdCollideSphere);

TEST_SUITE(TSCD) {
    TEST_ADD(cdOBBNew),
    TEST_ADD(cdOBBNew2),
    TEST_ADD(cdOBBNew3),
    TEST_ADD(cdOBBCollide),

    TEST_ADD(cdCollideTriMesh),
    TEST_ADD(cdCollideTriMesh2),
    TEST_ADD(cdCollideSphere),

    TEST_SUITE_CLOSURE
};

#endif

