#ifndef TEST_TASKPOOL2_H
#define TEST_TASKPOOL2_H

TEST(taskpool1);
TEST(taskpool2);

TEST_SUITE(TSTaskPool) {
    TEST_ADD(taskpool1),
    //TEST_ADD(taskpool2),
    TEST_SUITE_CLOSURE
};

#endif
