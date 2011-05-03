#ifndef TEST_CD_SPHERE_GRID_H
#define TEST_CD_SPHERE_GRID_H


TEST(cdspheregridSimple);
TEST(cdspheregridRandom);

TEST_SUITE(TSCDSphereGrid) {
    TEST_ADD(cdspheregridSimple),
    TEST_ADD(cdspheregridRandom),

    TEST_SUITE_CLOSURE
};

#endif


