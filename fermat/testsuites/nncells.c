#include <cu/cu.h>
#include <fermat/nncells.h>
#include <fermat/vec2.h>
#include <fermat/rand.h>
#include <fermat/nearest-linear.h>
#include <fermat/dbg.h>

static fer_rand_t r;

TEST(nncellsSetUp)
{
    ferRandInit(&r);
}

TEST(nncellsTearDown)
{
}

TEST(nncellsNew2)
{
    fer_nncells_t *cs;
    fer_real_t bound[4] = { -3, 2, 1, 4 };

    cs = ferNNCellsNew(2, bound, 10);
    assertEquals(ferNNCellsD(cs), 2);
    assertEquals(ferNNCellsSize(cs), 0);
    ferNNCellsDel(cs);
}

struct _el_t {
    fer_vec2_t v;
    fer_nncells_el_t c;
    fer_list_t list;
};
typedef struct _el_t el_t;

TEST(nncellsEl2)
{
    el_t n;
    fer_nncells_t *cs;
    fer_real_t range[6] = { -1., 1.,
                            -2., 2. };
    size_t num = 16;
    const size_t *dim;

    printf("nncellsNode:\n");

    ferVec2Set(&n.v, 0., 0.);
    ferNNCellsElInit(&n.c, (const fer_vec_t *)&n.v);

    cs = ferNNCellsNew(2, range, num);

    ferNNCellsAdd(cs, &n.c);

    printf("cube size: %f\n", (float)ferNNCellsCellSize(cs));
    printf("nncells2 len: %d\n", (int)ferNNCellsCellsLen(cs));
    dim = ferNNCellsDim(cs);
    printf("nncells2 dim: %d %d\n", dim[0], dim[1]);

    //assertEquals(ferNNCellsCubeId(cs, n), 22);

    ferVec2Set(&n.v, 0.8, 0.2);
    ferNNCellsUpdate(cs, &n.c);
    assertEquals(__ferNNCellsCoordsToID(cs, n.c.coords), 11);
    assertEquals(n.c.cell_id, 11);

    ferVec2Set(&n.v, 0.8, -0.7);
    ferNNCellsUpdate(cs, &n.c);
    assertEquals(__ferNNCellsCoordsToID(cs, n.c.coords), 5);
    assertEquals(n.c.cell_id, 5);

    ferVec2Set(&n.v, -0.2, -1.1);
    ferNNCellsUpdate(cs, &n.c);
    assertEquals(__ferNNCellsCoordsToID(cs, n.c.coords), 4);
    assertEquals(n.c.cell_id, 4);

    ferVec2Set(&n.v, -10., -10.);
    ferNNCellsUpdate(cs, &n.c);
    assertEquals(__ferNNCellsCoordsToID(cs, n.c.coords), 0);
    assertEquals(n.c.cell_id, 0);

    ferVec2Set(&n.v, 10., 10.);
    ferNNCellsUpdate(cs, &n.c);
    assertEquals(__ferNNCellsCoordsToID(cs, n.c.coords), 17);
    assertEquals(n.c.cell_id, 17);

    ferVec2Set(&n.v, 10., 1.2);
    ferNNCellsUpdate(cs, &n.c);
    assertEquals(__ferNNCellsCoordsToID(cs, n.c.coords), 14);
    assertEquals(n.c.cell_id, 14);

    ferNNCellsDel(cs);

    printf("------ nncellsNode\n\n");
}

static void elNew(el_t *ns, size_t len, fer_list_t *head)
{
    size_t i;
    fer_real_t x, y;

    ferListInit(head);

    for (i = 0; i < len; i++){
        x = ferRand(&r, -10., 10.);
        y = ferRand(&r, -10., 10.);

        ferVec2Set(&ns[i].v, x, y);
        ferNNCellsElInit(&ns[i].c, (const fer_vec_t *)&ns[i].v);

        ferListAppend(head, &ns[i].list);
    }
}

static void elAdd(fer_nncells_t *cs, el_t *ns, size_t len)
{
    size_t i;

    for (i = 0; i < len; i++){
        ferNNCellsAdd(cs, &ns[i].c);
    }
}

static fer_real_t dist2(void *item1, fer_list_t *item2)
{
    el_t *el2;
    fer_vec2_t *v;

    v   = (fer_vec2_t *)item1;
    el2 = fer_container_of(item2, el_t, list);
    return ferVec2Dist2(v, &el2->v);
}

#define N_LEN 500
#define N_LOOPS 2000
TEST(nncellsNearest2)
{
    fer_vec2_t v;
    fer_list_t head;
    el_t ns[N_LEN];
    fer_nncells_el_t *nsc[5];
    fer_list_t *nsl[5];
    el_t *near[10];
    fer_nncells_t *cs;
    fer_real_t range[4] = { -9., 9., -11., 7. };
    size_t num = 40, i, j, k;
    const size_t *dim;

    printf("nncells2Nearest:\n");

    cs = ferNNCellsNew(2, range, num);
    ferVec2Set(&v, 0., 0.1);
    elNew(ns, N_LEN, &head);
    elAdd(cs, ns, N_LEN);


    printf("cube size: %f\n", (float)ferNNCellsCellSize(cs));
    printf("nncells2 len: %d\n", (int)ferNNCellsCellsLen(cs));
    dim = ferNNCellsDim(cs);
    printf("nncells2 dim: %d %d\n", dim[0], dim[1]);

    for (k = 0; k < 5; k++){
        for (i=0; i < N_LOOPS; i++){
            ferVec2Set(&v, ferRand(&r, -10., 10.), ferRand(&r, -10, 10));

            ferNNCellsNearest(cs, (const fer_vec_t *)&v, k + 1, nsc);
            ferNearestLinear(&head, &v, dist2, nsl, k + 1);

            for (j = 0; j < k + 1; j++){
                near[0] = fer_container_of(nsc[j], el_t, c);
                near[1] = ferListEntry(nsl[j], el_t, list);
                assertEquals(near[0], near[1]);
            }
        }
    }

    ferNNCellsDel(cs);

    printf("------ nncellsNearest\n\n");
}
