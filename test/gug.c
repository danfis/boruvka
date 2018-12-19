#include <cu/cu.h>
#include <boruvka/gug.h>
#include <boruvka/vec2.h>
#include <boruvka/rand.h>
#include <boruvka/nearest-linear.h>
#include <boruvka/dbg.h>

static bor_rand_t r;

TEST(gugSetUp)
{
    borRandInit(&r);
}

TEST(gugTearDown)
{
}

TEST(gugNew2)
{
    bor_gug_t *cs;
    bor_gug_params_t params;
    bor_real_t bound[4] = { -3, 2, 1, 4 };

    borGUGParamsInit(&params);
    params.dim = 2;
    params.aabb = bound;
    params.num_cells = 10;
    cs = borGUGNew(&params);
    assertEquals(borGUGD(cs), 2);
    assertEquals(borGUGSize(cs), 0);
    borGUGDel(cs);
}

struct _el_t {
    bor_vec2_t v;
    bor_gug_el_t c;
    bor_list_t list;
};
typedef struct _el_t el_t;

TEST(gugEl2)
{
    el_t n;
    bor_gug_t *cs;
    bor_gug_params_t params;
    bor_real_t range[6] = { -1., 1.,
                            -2., 2. };
    size_t num = 16;

    borVec2Set(&n.v, 0., 0.);
    borGUGElInit(&n.c, (const bor_vec_t *)&n.v);

    borGUGParamsInit(&params);
    params.dim = 2;
    params.aabb = range;
    params.num_cells = num;
    cs = borGUGNew(&params);

    borGUGAdd(cs, &n.c);

    //assertEquals(borGUGCubeId(cs, n), 22);

    borVec2Set(&n.v, 0.8, 0.2);
    borGUGUpdate(cs, &n.c);
    assertEquals(__borGUGCoordsToID(cs, n.c.p), 11);
    assertEquals(n.c.cell_id, 11);

    borVec2Set(&n.v, 0.8, -0.7);
    borGUGUpdate(cs, &n.c);
    assertEquals(__borGUGCoordsToID(cs, n.c.p), 5);
    assertEquals(n.c.cell_id, 5);

    borVec2Set(&n.v, -0.2, -1.1);
    borGUGUpdate(cs, &n.c);
    assertEquals(__borGUGCoordsToID(cs, n.c.p), 4);
    assertEquals(n.c.cell_id, 4);

    borVec2Set(&n.v, -10., -10.);
    borGUGUpdate(cs, &n.c);
    assertEquals(__borGUGCoordsToID(cs, n.c.p), 0);
    assertEquals(n.c.cell_id, 0);

    borVec2Set(&n.v, 10., 10.);
    borGUGUpdate(cs, &n.c);
    assertEquals(__borGUGCoordsToID(cs, n.c.p), 17);
    assertEquals(n.c.cell_id, 17);

    borVec2Set(&n.v, 10., 1.2);
    borGUGUpdate(cs, &n.c);
    assertEquals(__borGUGCoordsToID(cs, n.c.p), 14);
    assertEquals(n.c.cell_id, 14);

    borGUGDel(cs);
}

static void elNew(el_t *ns, size_t len, bor_list_t *head)
{
    size_t i;
    bor_real_t x, y;

    borListInit(head);

    for (i = 0; i < len; i++){
        x = borRand(&r, -10., 10.);
        y = borRand(&r, -10., 10.);

        borVec2Set(&ns[i].v, x, y);
        borGUGElInit(&ns[i].c, (const bor_vec_t *)&ns[i].v);

        borListAppend(head, &ns[i].list);
    }
}

static void elAdd(bor_gug_t *cs, el_t *ns, size_t len)
{
    size_t i;

    for (i = 0; i < len; i++){
        borGUGAdd(cs, &ns[i].c);
    }
}

static bor_real_t dist2(void *item1, bor_list_t *item2, void *_)
{
    el_t *el2;
    bor_vec2_t *v;

    v   = (bor_vec2_t *)item1;
    el2 = bor_container_of(item2, el_t, list);
    return borVec2Dist2(v, &el2->v);
}

#define N_LEN 500
#define N_LOOPS 2000
TEST(gugNearest2)
{
    bor_vec2_t v;
    bor_list_t head;
    el_t ns[N_LEN];
    bor_gug_el_t *nsc[5];
    bor_list_t *nsl[5];
    el_t *near[10];
    bor_gug_t *cs;
    bor_gug_params_t params;
    bor_real_t range[4] = { -9., 9., -11., 7. };
    size_t i, j, k;

    borGUGParamsInit(&params);
    params.dim = 2;
    params.num_cells = 0;
    params.max_dens = 1;
    params.expand_rate = 2.;
    params.aabb = range;
    cs = borGUGNew(&params);
    borVec2Set(&v, 0., 0.1);
    elNew(ns, N_LEN, &head);
    elAdd(cs, ns, N_LEN);


    for (k = 0; k < 5; k++){
        for (i=0; i < N_LOOPS; i++){
            borVec2Set(&v, borRand(&r, -10., 10.), borRand(&r, -10, 10));

            borGUGNearest(cs, (const bor_vec_t *)&v, k + 1, nsc);
            borNearestLinear(&head, &v, dist2, nsl, k + 1, NULL);

            for (j = 0; j < k + 1; j++){
                near[0] = bor_container_of(nsc[j], el_t, c);
                near[1] = BOR_LIST_ENTRY(nsl[j], el_t, list);
                assertEquals(near[0], near[1]);
            }
        }
    }

    borGUGDel(cs);
}


struct _el6_t {
    BOR_VEC(v, 6);
    bor_gug_el_t c;
    bor_list_t list;
};
typedef struct _el6_t el6_t;

static void el6New(el6_t *ns, size_t len, bor_list_t *head)
{
    size_t i, j;

    borListInit(head);

    for (i = 0; i < len; i++){
        for (j = 0; j < 6; j++){
            borVecSet(ns[i].v, j, borRand(&r, -10., 10.));
        }

        borGUGElInit(&ns[i].c, ns[i].v);

        borListAppend(head, &ns[i].list);
    }
}

static void el6Add(bor_gug_t *cs, el6_t *ns, size_t len)
{
    size_t i;

    for (i = 0; i < len; i++){
        borGUGAdd(cs, &ns[i].c);
    }
}

static bor_real_t dist62(void *item1, bor_list_t *item2, void *_)
{
    el6_t *el2;
    bor_vec_t *v;

    v   = (bor_vec_t *)item1;
    el2 = bor_container_of(item2, el6_t, list);
    return borVecDist2(6, v, el2->v);
}

TEST(gugNearest6)
{
    BOR_VEC(v, 6);
    bor_list_t head;
    el6_t ns[N_LEN];
    bor_gug_el_t *nsc[5];
    bor_list_t *nsl[5];
    el6_t *near[10];
    bor_gug_t *cs;
    bor_gug_params_t params;
    bor_real_t range[12] = { -9., 9., -11., 7., -10, 7, -10, 10, -9, 12, -16, 12 };
    size_t i, j, k;

    borGUGParamsInit(&params);
    params.dim = 6;
    params.num_cells = 0;
    params.max_dens = 1;
    params.expand_rate = 2.;
    params.aabb = range;
    cs = borGUGNew(&params);
    el6New(ns, N_LEN, &head);
    el6Add(cs, ns, N_LEN);


    for (k = 0; k < 5; k++){
        for (i=0; i < N_LOOPS; i++){
            for (j = 0; j < 6; j++){
                borVecSet(v, j, borRand(&r, -10., 10.));
            }

            borGUGNearest(cs, v, k + 1, nsc);
            borNearestLinear(&head, v, dist62, nsl, k + 1, NULL);

            for (j = 0; j < k + 1; j++){
                near[0] = bor_container_of(nsc[j], el6_t, c);
                near[1] = BOR_LIST_ENTRY(nsl[j], el6_t, list);
                assertEquals(near[0], near[1]);
            }
        }
    }

    borGUGDel(cs);
}
