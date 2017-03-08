#ifndef TEST_SORT_H
#define TEST_SORT_H


TEST(sortRadixPtr);
TEST(sortCount);
TEST(sortInsert);
TEST(sortHeap);
TEST(sortMerge);
TEST(sortQuick);
TEST(sortTim);
TEST(sort);
TEST(sortByIntKey);
TEST(sortByLongKey);
TEST(sortListInsert);
TEST(sortList);

TEST_SUITE(TSSort) {
    TEST_ADD(sortRadixPtr),
    TEST_ADD(sortCount),
    TEST_ADD(sortInsert),
    TEST_ADD(sortHeap),
    TEST_ADD(sortMerge),
    TEST_ADD(sortQuick),
    TEST_ADD(sortTim),
    TEST_ADD(sort),
    TEST_ADD(sortByIntKey),
    TEST_ADD(sortByLongKey),
    TEST_ADD(sortListInsert),
    TEST_ADD(sortList),

    TEST_SUITE_CLOSURE
};

#endif
