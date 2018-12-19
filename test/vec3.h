#ifndef TEST_VEC3_H
#define TEST_VEC3_H

#include <cu/cu.h>

TEST(vec3SetUp);
TEST(vec3TearDown);
TEST(vec3Alloc);

TEST(vec3Angle);

TEST(vec3Core);
TEST(vec3PointSegmentDist);
TEST(vec3PointTriDist);
TEST(vec3PointInTri);
TEST(vec3ProjToPlane);
TEST(vec3Centroid);

TEST(vec3TriTriOverlap);

TEST_SUITE(TSVec3) {
    TEST_ADD(vec3SetUp),

    TEST_ADD(vec3Alloc),

    TEST_ADD(vec3Angle),

    TEST_ADD(vec3Core),
    TEST_ADD(vec3PointSegmentDist),
    TEST_ADD(vec3PointTriDist),
    TEST_ADD(vec3PointInTri),
    TEST_ADD(vec3ProjToPlane),
    TEST_ADD(vec3Centroid),

    TEST_ADD(vec3TriTriOverlap),

    TEST_ADD(vec3TearDown),
    TEST_SUITE_CLOSURE
};
#endif
