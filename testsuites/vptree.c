#include <cu/cu.h>
#include <boruvka/vptree.h>
#include <boruvka/rand-mt.h>
#include <boruvka/nearest-linear.h>
#include <boruvka/vec3.h>
#include <boruvka/dbg.h>

#define BUILD_ELS_LEN 10000
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
    el_t *el = FER_LIST_ENTRY(item2, el_t, list);

    return ferVec2Dist(p, &el->w);
}

static void build2Test(bor_rand_mt_t *rand,
                       bor_vptree_t *vp, bor_list_t *list, size_t num)
{
    bor_vptree_el_t *nn[10];
    bor_list_t *nn2[10];
    el_t *el, *el2;
    bor_vec2_t p;
    size_t len, len2, i;

    ferVec2Set(&p, ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3));
    len = ferVPTreeNearest(vp, (const bor_vec_t *)&p, num, nn);
    len2 = ferNearestLinear(list, (void *)&p, build2Dist, nn2, num, NULL);

    assertEquals(len, num);
    assertEquals(len2, num);

    for (i = 0; i < num; i++){
        el  = fer_container_of(nn[i], el_t, el);
        el2 = fer_container_of(nn2[i], el_t, list);

        if (el == el2){
            assertEquals(el, el2);
        }else{
            fprintf(stderr, "%.30f %.30f [%.30f] - %.30f %.30f [%.30f]\n",
                    ferVec2X(&el->w), ferVec2Y(&el->w), ferVec2Dist(&el->w, &p),
                    ferVec2X(&el2->w), ferVec2Y(&el2->w), ferVec2Dist(&el2->w, &p));
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

    rand = ferRandMTNewAuto();

    ferListInit(&els_list);
    for (i = 0; i < els_len; i++){
        ferVec2Set(&els[i].w, ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3));
        ferVPTreeElInit(&els[i].el, (const bor_vec_t *)&els[i].w);
        ferListAppend(&els_list, &els[i].list);
    }

    for (size = 1; size < BUILD_MAXSIZE; size++){
        ferVPTreeParamsInit(&params);
        params.dim = 2;
        params.maxsize = size;
        vp = ferVPTreeBuild(&params, &els[0].el, els_len, sizeof(el_t));

        //ferVPTreeDump(vp, stdout);
        for (i = 0; i < BUILD_NUM_TESTS; i++){
            for (j = 1; j <= BUILD_NUM_NNS; j++){
                build2Test(rand, vp, &els_list, j);
            }
        }
        ferVPTreeDel(vp);
    }


    ferRandMTDel(rand);
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
    el3_t *el = FER_LIST_ENTRY(item2, el3_t, list);

    return ferVec3Dist(p, &el->w);
}

static void build3Test(bor_rand_mt_t *rand,
                       bor_vptree_t *vp, bor_list_t *list, size_t num)
{
    bor_vptree_el_t *nn[10];
    bor_list_t *nn2[10];
    el3_t *el, *el2;
    bor_vec3_t p;
    size_t len, len2, i;

    ferVec3Set(&p, ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3));
    len = ferVPTreeNearest(vp, (const bor_vec_t *)&p, num, nn);
    len2 = ferNearestLinear(list, (void *)&p, build3Dist, nn2, num, NULL);

    assertEquals(len, num);
    assertEquals(len2, num);

    for (i = 0; i < num; i++){
        el  = fer_container_of(nn[i], el3_t, el);
        el2 = fer_container_of(nn2[i], el3_t, list);

        if (el == el2){
            assertEquals(el, el2);
        }else{
            fprintf(stderr, "%.30f %.30f %.30f [%.30f] - %.30f %.30f %.30f [%.30f]\n",
                    ferVec3X(&el->w), ferVec3Y(&el->w), ferVec3Z(&el->w), ferVec3Dist(&el->w, &p),
                    ferVec3X(&el2->w), ferVec3Y(&el2->w), ferVec3Z(&el2->w), ferVec3Dist(&el2->w, &p));
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

    rand = ferRandMTNewAuto();

    ferListInit(&els_list);
    for (i = 0; i < els_len; i++){
        ferVec3Set(&els[i].w, ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3));
        ferVPTreeElInit(&els[i].el, (const bor_vec_t *)&els[i].w);
        ferListAppend(&els_list, &els[i].list);
    }

    for (size = 1; size < BUILD_MAXSIZE; size++){
        ferVPTreeParamsInit(&params);
        params.dim = 3;
        params.maxsize = size;
        vp = ferVPTreeBuild(&params, &els[0].el, els_len, sizeof(el3_t));

        //ferVPTreeDump(vp, stdout);
        for (i = 0; i < BUILD_NUM_TESTS; i++){
            for (j = 1; j <= BUILD_NUM_NNS; j++){
                build3Test(rand, vp, &els_list, j);
            }
        }
        ferVPTreeDel(vp);
    }


    ferRandMTDel(rand);
}



#define ADD_NUM_TESTS 1000
#define ADD_ELS_LEN 10000
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

    rand = ferRandMTNewAuto();

    ferListInit(&els_list);
    for (i = 0; i < els_len; i++){
        ferVec2Set(&els[i].w, ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3));
        ferVPTreeElInit(&els[i].el, (const bor_vec_t *)&els[i].w);
        ferListAppend(&els_list, &els[i].list);
    }

    for (size = 1; size <= ADD_MAXSIZE; size++){
        ferVPTreeParamsInit(&params);
        params.dim = 2;
        params.maxsize = size;
        vp = ferVPTreeNew(&params);

        for (i = 0; i < ADD_ELS_LEN; i++){
            //fprintf(stdout, "%02d:\n", i);
            ferVPTreeAdd(vp, &els[i].el);
            //ferVPTreeDump(vp, stdout);
        }

        //ferVPTreeDump(vp, stdout);
        for (i = 0; i < ADD_NUM_TESTS; i++){
            for (j = 1; j <= ADD_NUM_NNS; j++){
                build2Test(rand, vp, &els_list, j);
            }
        }
        ferVPTreeDel(vp);
    }

    ferRandMTDel(rand);
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

    rand = ferRandMTNewAuto();

    ferListInit(&els_list);
    for (i = 0; i < els_len; i++){
        ferVec2Set(&els[i].w, ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3));
        ferVPTreeElInit(&els[i].el, (const bor_vec_t *)&els[i].w);
        ferListAppend(&els_list, &els[i].list);
        els[i].added = 0;
    }

    for (size = 1; size <= ADD_MAXSIZE; size++){
        ferVPTreeParamsInit(&params);
        params.dim = 2;
        params.maxsize = size;
        vp = ferVPTreeNew(&params);

        for (i = 0; i < ADD_ELS_LEN; i++){
            //fprintf(stdout, "%02d:\n", i);
            ferVPTreeAdd(vp, &els[i].el);
            //ferVPTreeDump(vp, stdout);
        }

        for (i = 0; i < ADD_ELS_LEN; i += 3){
            ferVPTreeRemove(vp, &els[i].el);
            ferListDel(&els[i].list);
        }

        //ferVPTreeDump(vp, stdout);
        for (i = 0; i < ADD_NUM_TESTS; i++){
            for (j = 1; j <= ADD_NUM_NNS; j++){
                build2Test(rand, vp, &els_list, j);
            }
        }
        ferVPTreeDel(vp);
    }

    ferRandMTDel(rand);
}
