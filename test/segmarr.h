#ifndef TEST_SEGM_ARR_H
#define TEST_SEGM_ARR_H

TEST(segmarrTest);

TEST_SUITE(TSSegmArr) {
    TEST_ADD(segmarrTest),
    TEST_SUITE_CLOSURE
};

#endif /* TEST_SEGM_ARR_H */
