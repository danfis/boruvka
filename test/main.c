#include <boruvka/config.h>
#include "vec4.h"
#include "vec3.h"
#include "vec2.h"
#include "vec.h"
#include "quat.h"
#include "poly2.h"
#include "pc3.h"
#include "pc.h"
#include "mat3.h"
#include "mat4.h"
#include "gug.h"
#include "mesh3.h"
#include "nearest.h"
#include "fibo.h"
#include "pairheap.h"
#include "rbtree.h"
#include "rbtree_int.h"
#include "splaytree.h"
#include "splaytree_int.h"
#include "bucketheap.h"
#include "dij.h"
#include "chull3.h"
#include "tasks.h"
#include "task-pool.h"
#include "vptree.h"
#include "vptree-hamming.h"
#include "nn.h"
#include "sort.h"
#include "htable.h"
#include "hfunc.h"
#include "segmarr.h"
#include "multimap.h"
#include "fifo.h"
#include "lifo.h"
#ifdef BOR_HDF5
#ifdef BOR_GSL
# include "thdf5.h"
#endif /* BOR_GSL */
#endif /* BOR_HDF5 */
#include "scc.h"
#include "msg-schema.h"
#include "lp.h"
#include "set.h"
#include "queue.h"

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
    TEST_SUITE_ADD(TSPoly2),
    TEST_SUITE_ADD(TSGUG),
    TEST_SUITE_ADD(TSVPTree),
    TEST_SUITE_ADD(TSVPTreeHamming),
    TEST_SUITE_ADD(TSNN),
    TEST_SUITE_ADD(TSMesh3),
    TEST_SUITE_ADD(TSNearest),
    TEST_SUITE_ADD(TSFibo),
    TEST_SUITE_ADD(TSPairHeap),
    TEST_SUITE_ADD(TSRBTree),
    TEST_SUITE_ADD(TSRBTreeInt),
    TEST_SUITE_ADD(TSSplayTree),
    TEST_SUITE_ADD(TSSplayTreeInt),
    TEST_SUITE_ADD(TSBucketHeap),
    TEST_SUITE_ADD(TSDij),
    TEST_SUITE_ADD(TSCHull3),
    TEST_SUITE_ADD(TSTasks),
    TEST_SUITE_ADD(TSTaskPool),
    TEST_SUITE_ADD(TSSort),
    TEST_SUITE_ADD(TSHTable),
    TEST_SUITE_ADD(TSHFunc),
    TEST_SUITE_ADD(TSSegmArr),
    TEST_SUITE_ADD(TSMultiMap),
    TEST_SUITE_ADD(TSFifo),
    TEST_SUITE_ADD(TSLifo),
#ifdef BOR_HDF5
#ifdef BOR_GSL
    TEST_SUITE_ADD(TSHDF5),
#endif /* BOR_GSL */
#endif /* BOR_HDF5 */
    TEST_SUITE_ADD(TSSCC),
    TEST_SUITE_ADD(TSMsgSchema),
    TEST_SUITE_ADD(TSLP),
    TEST_SUITE_ADD(TSSet),
    TEST_SUITE_ADD(TSQueue),

    TEST_SUITES_CLOSURE
};
int main(int argc, char *argv[])
{
    CU_SET_OUT_PREFIX("reg/");
    CU_SET_OUT_PER_TEST(1);
    return CU_RUN(argc, argv);
}
