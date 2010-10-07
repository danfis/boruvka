#include "vec3.h"

TEST_SUITES {
    TEST_SUITE_ADD(TSVec3),

    TEST_SUITES_CLOSURE
};
int main(int argc, char *argv[])
{
    CU_SET_OUT_PREFIX("regressions/");
    CU_RUN(argc, argv);

    return 0;
}
