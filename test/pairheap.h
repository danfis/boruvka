#ifndef TEST_PAIRHEAP_H
#define TEST_PAIRHEAP_H


TEST(pairheap1);
TEST(pairheapClear);
TEST(pairheapNonIntrusiveInt);

TEST_SUITE(TSPairHeap) {
    TEST_ADD(pairheap1),
    TEST_ADD(pairheapClear),
    TEST_ADD(pairheapNonIntrusiveInt),

    TEST_SUITE_CLOSURE
};

#endif
