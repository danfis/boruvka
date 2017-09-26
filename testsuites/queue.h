#ifndef TEST_QUEUE_H
#define TEST_QUEUE_H

TEST(testIBucketQ);
TEST(testLBucketQ);
TEST(testPBucketQ);
TEST(testIAdaQ);
TEST(testPAdaQ);
TEST(testAPQ);
TEST_SUITE(TSQueue){
    TEST_ADD(testIBucketQ),
    TEST_ADD(testLBucketQ),
    TEST_ADD(testPBucketQ),
    TEST_ADD(testIAdaQ),
    TEST_ADD(testPAdaQ),
    TEST_ADD(testAPQ),
    TEST_SUITE_CLOSURE
};
#endif
