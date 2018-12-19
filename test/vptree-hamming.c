#include <cu/cu.h>
#include <boruvka/vptree-hamming.h>
#include <boruvka/rand-mt.h>
#include <boruvka/nearest-linear.h>
#include <boruvka/vec3.h>
#include <boruvka/dbg.h>

struct _el_t {
    uint32_t w;
    bor_vptree_hamming_el_t el;
    bor_list_t list;
    int added;
};
typedef struct _el_t el_t;


static char bin_buf[33];
static const char *binUInt32(uint32_t v)
{
    int i;
    for (i = 0; i < 32; i++){
        bin_buf[31 - i] = '0' + (v & 0x1u);
        v >>= 1;
    }
    bin_buf[32] = 0x0;
    return bin_buf;
}

static int hammingDist(uint32_t a, uint32_t b)
{
    int dist = 0;
    uint32_t val = a ^ b;

    while(val){
        ++dist; 
        val &= val - 1;
    }

    return dist;
}

static bor_real_t distCB(void *item1, bor_list_t *item2, void *data)
{
    uint32_t p = *(uint32_t *)item1;
    el_t *el = BOR_LIST_ENTRY(item2, el_t, list);

    return hammingDist(p, el->w);
}

static void test(bor_rand_mt_t *rand,
                 bor_vptree_hamming_t *vp,
                 bor_list_t *list, size_t num)
{
    bor_vptree_hamming_el_t *nn[100];
    bor_list_t *nn2[100];
    el_t *el, *el2;
    uint32_t p;
    size_t len, len2, i;
    int d1, d2;

    p = borRandMTInt(rand);
    len = borVPTreeHammingNearest(vp, (const unsigned char *)&p, num, nn);
    len2 = borNearestLinear(list, (void *)&p, distCB, nn2, num, NULL);

    assertEquals(len, num);
    assertEquals(len2, num);

    if (len != num || len2 != num)
        return;

    for (i = 0; i < num; i++){
        el  = bor_container_of(nn[i], el_t, el);
        el2 = bor_container_of(nn2[i], el_t, list);

        d1 = hammingDist(p, el->w);
        d2 = hammingDist(p, el2->w);
        assertEquals(d1, d2);
        if (d1 != d2){
            assertEquals(el, el2);
            fprintf(stderr, "ref   : %s\n", binUInt32(p));
            /*
            fprintf(stderr, "vptree: %s %lu [%u]\n", binUInt32(el->w),
                            (unsigned long)el->w, hammingDist(el->w, p));
            fprintf(stderr, "linear: %s %lu [%u]\n", binUInt32(el2->w),
                            (unsigned long)el2->w, hammingDist(el2->w, p));
            */
        }
    }
}


#define ADD_NUM_TESTS 300
#define ADD_ELS_LEN 1000
#define ADD_NUM_NNS 19
#define ADD_MAXSIZE 5
TEST(vptreeHammingAdd)
{
    bor_rand_mt_t *rand;
    bor_vptree_hamming_t *vp;
    bor_vptree_hamming_params_t params;
    static bor_list_t els_list;
    static int els_len = ADD_ELS_LEN;
    static el_t els[ADD_ELS_LEN];
    int i, j, size;

    rand = borRandMTNewAuto();

    borListInit(&els_list);
    for (i = 0; i < els_len; i++){
        els[i].w = borRandMTInt(rand);
        borVPTreeHammingElInit(&els[i].el, (const unsigned char *)&els[i].w);
        borListAppend(&els_list, &els[i].list);
    }

    for (size = 1; size <= ADD_MAXSIZE; size++){
        borVPTreeHammingParamsInit(&params);
        params.size = 4;
        params.maxsize = size;

        vp = borVPTreeHammingNew(&params);

        for (i = 0; i < ADD_ELS_LEN; i++){
            borVPTreeHammingAdd(vp, &els[i].el);
        }

        for (i = 0; i < ADD_NUM_TESTS; i++){
            for (j = 3; j <= ADD_NUM_NNS; j++){
                test(rand, vp, &els_list, j);
            }
        }

        borVPTreeHammingDel(vp);
    }

    borRandMTDel(rand);
}

TEST(vptreeHammingAddRm)
{
    bor_rand_mt_t *rand;
    bor_vptree_hamming_t *vp;
    bor_vptree_hamming_params_t params;
    static bor_list_t els_list;
    static int els_len = ADD_ELS_LEN;
    static el_t els[ADD_ELS_LEN];
    int i, j, size;

    rand = borRandMTNewAuto();

    borListInit(&els_list);
    for (i = 0; i < els_len; i++){
        els[i].w = borRandMTInt(rand);
        borVPTreeHammingElInit(&els[i].el, (const unsigned char *)&els[i].w);
        borListAppend(&els_list, &els[i].list);
        els[i].added = 0;
    }

    for (size = 1; size <= ADD_MAXSIZE; size++){
        borVPTreeHammingParamsInit(&params);
        params.size = 4;
        params.maxsize = size;
        vp = borVPTreeHammingNew(&params);

        for (i = 0; i < ADD_ELS_LEN; i++){
            borVPTreeHammingAdd(vp, &els[i].el);
        }

        for (i = 0; i < ADD_ELS_LEN; i += 3){
            borVPTreeHammingRemove(vp, &els[i].el);
            borListDel(&els[i].list);
        }

        for (i = 0; i < ADD_NUM_TESTS; i++){
            for (j = 1; j <= ADD_NUM_NNS; j++){
                test(rand, vp, &els_list, j);
            }
        }
        borVPTreeHammingDel(vp);
    }

    borRandMTDel(rand);
}

