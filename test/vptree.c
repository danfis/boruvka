#include <cu/cu.h>
#include <boruvka/vptree.h>
#include <boruvka/rand-mt.h>
#include <boruvka/nearest-linear.h>
#include <boruvka/vec3.h>
#include <boruvka/dbg.h>

#define BUILD_ELS_LEN 3000
#define BUILD_NUM_TESTS 1000
#define BUILD_NUM_NNS 5
#define BUILD_MAXSIZE 5

struct _el_t {
    bor_vec2_t w;
    bor_vptree_el_t el;
    bor_list_t list;
    int added;
};
typedef struct _el_t el_t;

static bor_real_t build2Dist(void *item1, bor_list_t *item2,
                             void *data)
{
    bor_vec2_t *p = (bor_vec2_t *)item1;
    el_t *el = BOR_LIST_ENTRY(item2, el_t, list);

    return borVec2Dist(p, &el->w);
}

static void build2Test(bor_rand_mt_t *rand,
                       bor_vptree_t *vp, bor_list_t *list, size_t num)
{
    bor_vptree_el_t *nn[10];
    bor_list_t *nn2[10];
    el_t *el, *el2;
    bor_vec2_t p;
    size_t len, len2, i;

    borVec2Set(&p, borRandMT(rand, -3, 3), borRandMT(rand, -3, 3));
    len = borVPTreeNearest(vp, (const bor_vec_t *)&p, num, nn);
    len2 = borNearestLinear(list, (void *)&p, build2Dist, nn2, num, NULL);

    assertEquals(len, num);
    assertEquals(len2, num);

    for (i = 0; i < num; i++){
        el  = bor_container_of(nn[i], el_t, el);
        el2 = bor_container_of(nn2[i], el_t, list);

        assertTrue(el == el2
                        || borEq(borVec2Dist(&el->w, &p),
                                 borVec2Dist(&el2->w, &p)));
        if (el != el2 && !borEq(borVec2Dist(&el->w, &p),
                                borVec2Dist(&el2->w, &p))){
            fprintf(stderr, "%.30f %.30f [%.30f] - %.30f %.30f [%.30f]\n",
                    borVec2X(&el->w), borVec2Y(&el->w), borVec2Dist(&el->w, &p),
                    borVec2X(&el2->w), borVec2Y(&el2->w), borVec2Dist(&el2->w, &p));
        }
    }
}

TEST(vptreeBuild2)
{
    bor_rand_mt_t *rand;
    bor_vptree_t *vp;
    bor_vptree_params_t params;
    static bor_list_t els_list;
    static int els_len = BUILD_ELS_LEN;
    static el_t els[BUILD_ELS_LEN];
    int i, j, size;

    rand = borRandMTNewAuto();

    borListInit(&els_list);
    for (i = 0; i < els_len; i++){
        borVec2Set(&els[i].w, borRandMT(rand, -3, 3), borRandMT(rand, -3, 3));
        borVPTreeElInit(&els[i].el, (const bor_vec_t *)&els[i].w);
        borListAppend(&els_list, &els[i].list);
    }

    for (size = 1; size < BUILD_MAXSIZE; size++){
        borVPTreeParamsInit(&params);
        params.dim = 2;
        params.maxsize = size;
        vp = borVPTreeBuild(&params, &els[0].el, els_len, sizeof(el_t));

        //borVPTreeDump(vp, stdout);
        for (i = 0; i < BUILD_NUM_TESTS; i++){
            for (j = 1; j <= BUILD_NUM_NNS; j++){
                build2Test(rand, vp, &els_list, j);
            }
        }
        borVPTreeDel(vp);
    }


    borRandMTDel(rand);
}


struct _el3_t {
    bor_vec3_t w;
    bor_vptree_el_t el;
    bor_list_t list;
};
typedef struct _el3_t el3_t;

static bor_real_t build3Dist(void *item1, bor_list_t *item2,
                             void *data)
{
    bor_vec3_t *p = (bor_vec3_t *)item1;
    el3_t *el = BOR_LIST_ENTRY(item2, el3_t, list);

    return borVec3Dist(p, &el->w);
}

static void build3Test(bor_rand_mt_t *rand,
                       bor_vptree_t *vp, bor_list_t *list, size_t num)
{
    bor_vptree_el_t *nn[10];
    bor_list_t *nn2[10];
    el3_t *el, *el2;
    bor_vec3_t p;
    size_t len, len2, i;

    borVec3Set(&p, borRandMT(rand, -3, 3), borRandMT(rand, -3, 3), borRandMT(rand, -3, 3));
    len = borVPTreeNearest(vp, (const bor_vec_t *)&p, num, nn);
    len2 = borNearestLinear(list, (void *)&p, build3Dist, nn2, num, NULL);

    assertEquals(len, num);
    assertEquals(len2, num);

    for (i = 0; i < num; i++){
        el  = bor_container_of(nn[i], el3_t, el);
        el2 = bor_container_of(nn2[i], el3_t, list);

        assertTrue(el == el2
                        || borEq(borVec3Dist(&el->w, &p),
                                 borVec3Dist(&el2->w, &p)));
        if (el != el2 && !borEq(borVec3Dist(&el->w, &p),
                                borVec3Dist(&el2->w, &p))){
            fprintf(stderr, "%.30f %.30f %.30f [%.30f] - %.30f %.30f %.30f [%.30f]\n",
                    borVec3X(&el->w), borVec3Y(&el->w), borVec3Z(&el->w), borVec3Dist(&el->w, &p),
                    borVec3X(&el2->w), borVec3Y(&el2->w), borVec3Z(&el2->w), borVec3Dist(&el2->w, &p));
        }
    }
}

TEST(vptreeBuild3)
{
    bor_rand_mt_t *rand;
    bor_vptree_t *vp;
    bor_vptree_params_t params;
    static bor_list_t els_list;
    static int els_len = BUILD_ELS_LEN;
    static el3_t els[BUILD_ELS_LEN];
    int i, j, size;

    rand = borRandMTNewAuto();

    borListInit(&els_list);
    for (i = 0; i < els_len; i++){
        borVec3Set(&els[i].w, borRandMT(rand, -3, 3), borRandMT(rand, -3, 3), borRandMT(rand, -3, 3));
        borVPTreeElInit(&els[i].el, (const bor_vec_t *)&els[i].w);
        borListAppend(&els_list, &els[i].list);
    }

    for (size = 1; size < BUILD_MAXSIZE; size++){
        borVPTreeParamsInit(&params);
        params.dim = 3;
        params.maxsize = size;
        vp = borVPTreeBuild(&params, &els[0].el, els_len, sizeof(el3_t));

        //borVPTreeDump(vp, stdout);
        for (i = 0; i < BUILD_NUM_TESTS; i++){
            for (j = 1; j <= BUILD_NUM_NNS; j++){
                build3Test(rand, vp, &els_list, j);
            }
        }
        borVPTreeDel(vp);
    }


    borRandMTDel(rand);
}



#define ADD_NUM_TESTS 1000
#define ADD_ELS_LEN 3000
#define ADD_NUM_NNS 5
#define ADD_MAXSIZE 5
TEST(vptreeAdd)
{
    bor_rand_mt_t *rand;
    bor_vptree_t *vp;
    bor_vptree_params_t params;
    static bor_list_t els_list;
    static int els_len = ADD_ELS_LEN;
    static el_t els[ADD_ELS_LEN];
    int i, j, size;

    rand = borRandMTNewAuto();

    borListInit(&els_list);
    for (i = 0; i < els_len; i++){
        borVec2Set(&els[i].w, borRandMT(rand, -3, 3), borRandMT(rand, -3, 3));
        borVPTreeElInit(&els[i].el, (const bor_vec_t *)&els[i].w);
        borListAppend(&els_list, &els[i].list);
    }

    for (size = 1; size <= ADD_MAXSIZE; size++){
        borVPTreeParamsInit(&params);
        params.dim = 2;
        params.maxsize = size;
        vp = borVPTreeNew(&params);

        for (i = 0; i < ADD_ELS_LEN; i++){
            //fprintf(stdout, "%02d:\n", i);
            borVPTreeAdd(vp, &els[i].el);
            //borVPTreeDump(vp, stdout);
        }

        //borVPTreeDump(vp, stdout);
        for (i = 0; i < ADD_NUM_TESTS; i++){
            for (j = 1; j <= ADD_NUM_NNS; j++){
                build2Test(rand, vp, &els_list, j);
            }
        }
        borVPTreeDel(vp);
    }

    borRandMTDel(rand);
}

TEST(vptreeAddRm)
{
    bor_rand_mt_t *rand;
    bor_vptree_t *vp;
    bor_vptree_params_t params;
    static bor_list_t els_list;
    static int els_len = ADD_ELS_LEN;
    static el_t els[ADD_ELS_LEN];
    int i, j, size;

    rand = borRandMTNewAuto();

    borListInit(&els_list);
    for (i = 0; i < els_len; i++){
        borVec2Set(&els[i].w, borRandMT(rand, -3, 3), borRandMT(rand, -3, 3));
        borVPTreeElInit(&els[i].el, (const bor_vec_t *)&els[i].w);
        borListAppend(&els_list, &els[i].list);
        els[i].added = 0;
    }

    for (size = 1; size <= ADD_MAXSIZE; size++){
        borVPTreeParamsInit(&params);
        params.dim = 2;
        params.maxsize = size;
        vp = borVPTreeNew(&params);

        for (i = 0; i < ADD_ELS_LEN; i++){
            //fprintf(stdout, "%02d:\n", i);
            borVPTreeAdd(vp, &els[i].el);
            //borVPTreeDump(vp, stdout);
        }

        for (i = 0; i < ADD_ELS_LEN; i += 3){
            borVPTreeRemove(vp, &els[i].el);
            borListDel(&els[i].list);
        }

        //borVPTreeDump(vp, stdout);
        for (i = 0; i < ADD_NUM_TESTS; i++){
            for (j = 1; j <= ADD_NUM_NNS; j++){
                build2Test(rand, vp, &els_list, j);
            }
        }
        borVPTreeDel(vp);
    }

    borRandMTDel(rand);
}
