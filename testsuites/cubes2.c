#include <stdio.h>
#include "cu.h"
#include <fermat/cubes2.h>
#include <fermat/rand.h>
#include <fermat/dbg.h>
#include <fermat/alloc.h>
#include <fermat/nearest-linear.h>

static fer_rand_t r;

TEST(cubes2SetUp)
{
    ferRandInit(&r);
}

TEST(cubes2TearDown)
{
}

TEST(cubes2New)
{
    fer_real_t range[4] = { -1., 5.,
                            -1., 10. };
    size_t num = 1000;
    const size_t *dim;

    printf("cubesNew:\n");

    fer_cubes2_t *cs = ferCubes2New(range, num);

    printf("cube size: %f\n", (float)ferCubes2Size(cs));
    printf("cubes len: %d\n", (int)ferCubes2Len(cs));
    dim = ferCubes2Dim(cs);
    printf("cubes dim: %d %d\n", dim[0], dim[1]);

    ferCubes2Del(cs);

    printf("------ cubesNew\n\n");
    fflush(stdout);
}

struct _el_t {
    fer_vec2_t v;
    fer_cubes2_el_t c;
    fer_list_t list;
};
typedef struct _el_t el_t;

TEST(cubes2El)
{
    el_t n;
    fer_cubes2_t *cs;
    fer_real_t range[6] = { -1., 1.,
                            -2., 2. };
    size_t num = 16;
    const size_t *dim;

    printf("cubesNode:\n");

    ferVec2Set(&n.v, 0., 0.);
    ferCubes2ElInit(&n.c, &n.v);
    n.c.coords = &n.v;

    cs = ferCubes2New(range, num);

    ferCubes2Add(cs, &n.c);

    printf("cube size: %f\n", (float)ferCubes2Size(cs));
    printf("cubes len: %d\n", (int)ferCubes2Len(cs));
    dim = ferCubes2Dim(cs);
    printf("cubes dim: %d %d\n", dim[0], dim[1]);

    //assertEquals(ferCubes2CubeId(cs, n), 22);

    ferVec2Set(&n.v, 0.8, 0.2);
    ferCubes2Update(cs, &n.c);
    assertEquals(__ferCubes2IDCoords(cs, n.c.coords), 11);
    assertEquals(n.c.cube_id, 11);

    ferVec2Set(&n.v, 0.8, -0.7);
    ferCubes2Update(cs, &n.c);
    assertEquals(__ferCubes2IDCoords(cs, n.c.coords), 5);
    assertEquals(n.c.cube_id, 5);

    ferVec2Set(&n.v, -0.2, -1.1);
    ferCubes2Update(cs, &n.c);
    assertEquals(__ferCubes2IDCoords(cs, n.c.coords), 4);
    assertEquals(n.c.cube_id, 4);

    ferVec2Set(&n.v, -10., -10.);
    ferCubes2Update(cs, &n.c);
    assertEquals(__ferCubes2IDCoords(cs, n.c.coords), 0);
    assertEquals(n.c.cube_id, 0);

    ferVec2Set(&n.v, 10., 10.);
    ferCubes2Update(cs, &n.c);
    assertEquals(__ferCubes2IDCoords(cs, n.c.coords), 17);
    assertEquals(n.c.cube_id, 17);

    ferVec2Set(&n.v, 10., 1.2);
    ferCubes2Update(cs, &n.c);
    assertEquals(__ferCubes2IDCoords(cs, n.c.coords), 14);
    assertEquals(n.c.cube_id, 14);

    ferCubes2Del(cs);

    printf("------ cubesNode\n\n");
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
        ferCubes2ElInit(&ns[i].c, &ns[i].v);

        ferListAppend(head, &ns[i].list);
    }
}

static void elAdd(fer_cubes2_t *cs, el_t *ns, size_t len)
{
    size_t i;

    for (i = 0; i < len; i++){
        ferCubes2Add(cs, &ns[i].c);
    }
}

static fer_real_t dist2(void *item1, fer_list_t *item2)
{
    el_t *el1, *el2;

    el1 = item1;
    el2 = fer_container_of(item2, el_t, list);
    return ferVec2Dist2(&el1->v, &el2->v);
}

#define N_LEN 200
#define N_LOOPS 2000
TEST(cubes2Nearest)
{
    fer_vec2_t v;
    fer_list_t head;
    el_t ns[N_LEN];
    fer_cubes2_el_t *nsc[5];
    fer_list_t *nsl[5];
    el_t *near[10];
    fer_cubes2_t *cs;
    fer_real_t range[4] = { -9., 9., -11., 7. };
    size_t num = 40, i, j, k;
    const size_t *dim;

    printf("cubesNearest:\n");

    cs = ferCubes2New(range, num);
    ferVec2Set(&v, 0., 0.1);
    elNew(ns, N_LEN, &head);
    elAdd(cs, ns, N_LEN);


    printf("cube size: %f\n", (float)ferCubes2Size(cs));
    printf("cubes len: %d\n", (int)ferCubes2Len(cs));
    dim = ferCubes2Dim(cs);
    printf("cubes dim: %d %d\n", dim[0], dim[1]);

    for (k = 0; k < 5; k++){
        for (i=0; i < N_LOOPS; i++){
            ferVec2Set(&v, ferRand(&r, -10., 10.), ferRand(&r, -10, 10));

            ferCubes2Nearest(cs, &v, k + 1, nsc);
            ferNearestLinear(&head, &v, dist2, nsl, k + 1);

            for (j = 0; j < k + 1; j++){
                near[0] = fer_container_of(nsc[j], el_t, c);
                near[1] = FER_LIST_ENTRY(nsl[j], el_t, list);
                assertEquals(near[0], near[1]);
            }
        }
    }

    ferCubes2Del(cs);

    printf("------ cubesNearest\n\n");
}
