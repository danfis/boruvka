#ifndef TEST_SET_H
#define TEST_SET_H

TEST(testISet);
TEST_SUITE(TSSet){
    TEST_ADD(testISet),
    TEST_SUITE_CLOSURE
};
#endif
