#ifndef TEST_SORT_H
#define TEST_SORT_H


TEST(sortRadixPtr);
TEST(sortCount);
TEST(sortInsert);

TEST_SUITE(TSSort) {
    TEST_ADD(sortRadixPtr),
    TEST_ADD(sortCount),
    TEST_ADD(sortInsert),

    TEST_SUITE_CLOSURE
};

#endif


