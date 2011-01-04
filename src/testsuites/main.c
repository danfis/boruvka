#include "vec4.h"
#include "vec3.h"
#include "vec2.h"
#include "tr2.h"
#include "quat.h"
#include "point_cloud.h"
#include "mat3.h"

TEST_SUITES {
    TEST_SUITE_ADD(TSVec4),
    TEST_SUITE_ADD(TSVec3),
    TEST_SUITE_ADD(TSVec2),
    TEST_SUITE_ADD(TSTr2),
    TEST_SUITE_ADD(TSQuat),
    TEST_SUITE_ADD(TSPC),
    TEST_SUITE_ADD(TSMat3),

    TEST_SUITES_CLOSURE
};
int main(int argc, char *argv[])
{
    CU_SET_OUT_PREFIX("regressions/");
    CU_RUN(argc, argv);

    return 0;
}
