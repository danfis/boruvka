#include <fermat/config.h>
#include "vec4.h"
#include "vec3.h"
#include "vec2.h"
#include "vec.h"
#include "quat.h"
#include "pc3.h"
#include "pc.h"
#include "mat3.h"
#include "mat4.h"
#include "cubes3.h"
#include "cubes2.h"
#include "gug.h"
#include "mesh3.h"
#include "nearest.h"
#include "fibo.h"
#include "pairheap.h"
#include "dij.h"
#include "cd.h"
#include "cd-sphere-grid.h"
#include "chull3.h"
#include "tasks.h"
#include "task-pool.h"
#include "vptree.h"

TEST_SUITES {
    TEST_SUITE_ADD(TSVec4),
    TEST_SUITE_ADD(TSVec3),
    TEST_SUITE_ADD(TSVec2),
    TEST_SUITE_ADD(TSVec),
    TEST_SUITE_ADD(TSQuat),
    TEST_SUITE_ADD(TSPC3),
    TEST_SUITE_ADD(TSPC),
    TEST_SUITE_ADD(TSMat3),
    TEST_SUITE_ADD(TSMat4),

    TEST_SUITE_ADD(TSCubes3),
    TEST_SUITE_ADD(TSCubes2),
    TEST_SUITE_ADD(TSGUG),
    TEST_SUITE_ADD(TSVPTree),
    TEST_SUITE_ADD(TSMesh3),
    TEST_SUITE_ADD(TSNearest),

    TEST_SUITE_ADD(TSFibo),
    TEST_SUITE_ADD(TSPairHeap),

    TEST_SUITE_ADD(TSDij),

    TEST_SUITE_ADD(TSCD),
    TEST_SUITE_ADD(TSCDSphereGrid),

    TEST_SUITE_ADD(TSCHull3),

    TEST_SUITE_ADD(TSTasks),
    TEST_SUITE_ADD(TSTaskPool),

    TEST_SUITES_CLOSURE
};
int main(int argc, char *argv[])
{
    CU_SET_OUT_PREFIX("regressions/");
    CU_RUN(argc, argv);

    return 0;
}
