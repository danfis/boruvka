#ifndef TEST_VEC3_H
#define TEST_VEC3_H

#include <cu/cu.h>

TEST(vec3SetUp);
TEST(vec3TearDown);
TEST(vec3Alloc);

TEST(vec3Add);
TEST(vec3Sub);
TEST(vec3Scale);
TEST(vec3Normalize);
TEST(vec3Dot);
TEST(vec3Cross);
TEST(vec3Len2);
TEST(vec3Len);
TEST(vec3Dist2);
TEST(vec3Dist);
TEST(vec3SegmentDist);
TEST(vec3TriDist);

TEST(vec3Angle);

TEST(vec3Core);
TEST(vec3PointSegmentDist);
TEST(vec3PointTriDist);
TEST(vec3PointInTri);
TEST(vec3ProjToPlane);
TEST(vec3Centroid);

TEST_SUITE(TSVec3) {
    TEST_ADD(vec3SetUp),

    TEST_ADD(vec3Alloc),

    TEST_ADD(vec3Add),
    TEST_ADD(vec3Sub),
    TEST_ADD(vec3Scale),
    TEST_ADD(vec3Normalize),
    TEST_ADD(vec3Dot),
    TEST_ADD(vec3Cross),
    TEST_ADD(vec3Len2),
    TEST_ADD(vec3Len),
    TEST_ADD(vec3Dist2),
    TEST_ADD(vec3Dist),
    TEST_ADD(vec3SegmentDist),
    TEST_ADD(vec3TriDist),

    TEST_ADD(vec3Angle),

    TEST_ADD(vec3Core),
    TEST_ADD(vec3PointSegmentDist),
    TEST_ADD(vec3PointTriDist),
    TEST_ADD(vec3PointInTri),
    TEST_ADD(vec3ProjToPlane),
    TEST_ADD(vec3Centroid),

    TEST_ADD(vec3TearDown),
    TEST_SUITE_CLOSURE
};
#endif
