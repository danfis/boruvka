#ifndef TEST_GEOM_H
#define TEST_GEOM_H


TEST(geomCollideTriMesh);

TEST_SUITE(TSGeom) {
    TEST_ADD(geomCollideTriMesh),

    TEST_SUITE_CLOSURE
};

#endif
