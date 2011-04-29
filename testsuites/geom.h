#ifndef TEST_GEOM_H
#define TEST_GEOM_H


TEST(geomCollideTriMesh);
TEST(geomSphere);

TEST_SUITE(TSGeom) {
    TEST_ADD(geomCollideTriMesh),
    TEST_ADD(geomSphere),

    TEST_SUITE_CLOSURE
};

#endif
