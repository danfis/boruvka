#ifndef TEST_CFG_H
#define TEST_CFG_H

TEST(cfg1);
TEST(cfg1format);

TEST_SUITE(TSCfg) {
    TEST_ADD(cfg1),
    TEST_ADD(cfg1format),

    TEST_SUITE_CLOSURE
};

#endif
