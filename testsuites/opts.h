#ifndef TEST_OPTS_H
#define TEST_OPTS_H

TEST(opts1);

TEST_SUITE(TSOpts) {
    TEST_ADD(opts1),

    TEST_SUITE_CLOSURE
};

#endif
