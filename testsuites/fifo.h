#ifndef TEST_FIFO_H
#define TEST_FIFO_H

TEST(fifo1);

TEST_SUITE(TSFifo) {
    TEST_ADD(fifo1),
    TEST_SUITE_CLOSURE
};

#endif
