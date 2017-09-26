#ifndef TEST_QUEUE_H
#define TEST_QUEUE_H

TEST(testBucketQ);
TEST_SUITE(TSQueue){
    TEST_ADD(testBucketQ),
    TEST_SUITE_CLOSURE
};
#endif
