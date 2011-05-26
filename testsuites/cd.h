#ifndef TEST_CD_H
#define TEST_CD_H


TEST(cdOBBNew);
TEST(cdOBBNew2);
TEST(cdOBBNew3);
TEST(cdGeomNew);
TEST(cdOBBCollide);

TEST(cdCollideTriMesh);
TEST(cdCollideTriMesh2);
TEST(cdCollideSphere);
TEST(cdCollide1);
TEST(cdCollide2);
TEST(cdCollide3);
TEST(cdCollide4);
TEST(cdCollide5);
TEST(cdCollide6);
TEST(cdCollide7);
TEST(cdCollide8);
TEST(cdCollide9);

TEST_SUITE(TSCD) {
    TEST_ADD(cdOBBNew),
    TEST_ADD(cdOBBNew2),
    TEST_ADD(cdOBBNew3),
    TEST_ADD(cdGeomNew),
    TEST_ADD(cdOBBCollide),

    TEST_ADD(cdCollideTriMesh),
    TEST_ADD(cdCollideTriMesh2),
    TEST_ADD(cdCollideSphere),
    TEST_ADD(cdCollide1),
    TEST_ADD(cdCollide2),
    TEST_ADD(cdCollide3),
    TEST_ADD(cdCollide4),
    TEST_ADD(cdCollide5),
    TEST_ADD(cdCollide6),
    TEST_ADD(cdCollide7),
    TEST_ADD(cdCollide8),
    TEST_ADD(cdCollide9),

    TEST_SUITE_CLOSURE
};

#endif

