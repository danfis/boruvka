#include <cu/cu.h>
#include <fermat/vptree.h>
#include <fermat/rand-mt.h>
#include <fermat/nearest-linear.h>
#include <fermat/vec3.h>
#include <fermat/dbg.h>

#define BUILD_ELS_LEN 10000
#define BUILD_NUM_TESTS 1000
#define BUILD_NUM_NNS 5
#define BUILD_MAXSIZE 5

struct _el_t {
    fer_vec2_t w;
    fer_vptree_el_t el;
    fer_list_t list;
};
typedef struct _el_t el_t;

static fer_real_t build2Dist(void *item1, fer_list_t *item2,
                             void *data)
{
    fer_vec2_t *p = (fer_vec2_t *)item1;
    el_t *el = FER_LIST_ENTRY(item2, el_t, list);

    return ferVec2Dist(p, &el->w);
}

static void build2Test(fer_rand_mt_t *rand,
                       fer_vptree_t *vp, fer_list_t *list, size_t num)
{
    fer_vptree_el_t *nn[10];
    fer_list_t *nn2[10];
    el_t *el, *el2;
    fer_vec2_t p;
    size_t len, len2, i;

    ferVec2Set(&p, ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3));
    len = ferVPTreeNearest(vp, (const fer_vec_t *)&p, num, nn);
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
    fer_rand_mt_t *rand;
    fer_vptree_t *vp;
    fer_vptree_params_t params;
    static fer_list_t els_list;
    static int els_len = BUILD_ELS_LEN;
    static el_t els[BUILD_ELS_LEN];
    int i, j, size;

    rand = ferRandMTNewAuto();

    ferListInit(&els_list);
    for (i = 0; i < els_len; i++){
        ferVec2Set(&els[i].w, ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3));
        ferVPTreeElInit(&els[i].el, (const fer_vec_t *)&els[i].w);
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
    fer_vec3_t w;
    fer_vptree_el_t el;
    fer_list_t list;
};
typedef struct _el3_t el3_t;

static fer_real_t build3Dist(void *item1, fer_list_t *item2,
                             void *data)
{
    fer_vec3_t *p = (fer_vec3_t *)item1;
    el3_t *el = FER_LIST_ENTRY(item2, el3_t, list);

    return ferVec3Dist(p, &el->w);
}

static void build3Test(fer_rand_mt_t *rand,
                       fer_vptree_t *vp, fer_list_t *list, size_t num)
{
    fer_vptree_el_t *nn[10];
    fer_list_t *nn2[10];
    el3_t *el, *el2;
    fer_vec3_t p;
    size_t len, len2, i;

    ferVec3Set(&p, ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3));
    len = ferVPTreeNearest(vp, (const fer_vec_t *)&p, num, nn);
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
    fer_rand_mt_t *rand;
    fer_vptree_t *vp;
    fer_vptree_params_t params;
    static fer_list_t els_list;
    static int els_len = BUILD_ELS_LEN;
    static el3_t els[BUILD_ELS_LEN];
    int i, j, size;

    rand = ferRandMTNewAuto();

    ferListInit(&els_list);
    for (i = 0; i < els_len; i++){
        ferVec3Set(&els[i].w, ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3));
        ferVPTreeElInit(&els[i].el, (const fer_vec_t *)&els[i].w);
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
