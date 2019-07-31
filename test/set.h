#ifndef TEST_SET_H
#define TEST_SET_H

TEST(testISet);
TEST(testHashSet);
TEST_SUITE(TSSet){
    TEST_ADD(testISet),
    TEST_ADD(testHashSet),
    TEST_SUITE_CLOSURE
};
#endif
