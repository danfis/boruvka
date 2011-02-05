#include <stdio.h>
#include "cu.h"
#include <fermat/cubes3.h>
#include <fermat/rand.h>
#include <fermat/dbg.h>
#include <fermat/alloc.h>

static fer_rand_t r;

TEST(cubesSetUp)
{
    ferRandInit(&r);
}

TEST(cubesTearDown)
{
}

TEST(cubesNew)
{
    fer_real_t range[6] = { -1., 5.,
                            -1., 10.,
                            -1., 4. };
    size_t num = 1000;
    const size_t *dim;

    printf("cubesNew:\n");

    fer_cubes3_t *cs = ferCubes3New(range, num);

    printf("cube size: %f\n", (float)ferCubes3Size(cs));
    printf("cubes len: %d\n", (int)ferCubes3Len(cs));
    dim = ferCubes3Dim(cs);
    printf("cubes dim: %d %d %d\n", dim[0], dim[1], dim[2]);

    ferCubes3Del(cs);

    printf("------ cubesNew\n\n");
    fflush(stdout);
}

struct _el_t {
    fer_vec3_t v;
    fer_cubes3_el_t c;
};
typedef struct _el_t el_t;

TEST(cubesEl)
{
    el_t n;
    fer_cubes3_t *cs;
    fer_real_t range[6] = { -1., 1.,
                            -2., 2.,
                            -1., 1. };
    size_t num = 27;
    const size_t *dim;

    printf("cubesNode:\n");

    ferVec3Set(&n.v, 0., 0., 0.);
    ferCubes3ElInit(&n.c, &n.v);
    n.c.coords = &n.v;

    cs = ferCubes3New(range, num);

    ferCubes3Add(cs, &n.c);

    printf("cube size: %f\n", (float)ferCubes3Size(cs));
    printf("cubes len: %d\n", (int)ferCubes3Len(cs));
    dim = ferCubes3Dim(cs);
    printf("cubes dim: %d %d %d\n", dim[0], dim[1], dim[2]);

    //assertEquals(ferCubes3CubeId(cs, n), 22);

    ferVec3Set(&n.v, 0.8, 0., 0.);
    ferCubes3Update(cs, &n.c);
    assertEquals(__ferCubes3IDCoords(cs, n.c.coords), 23);
    assertEquals(n.c.cube_id, 23);

    ferVec3Set(&n.v, 0.8, -0.6, -0.8);
    ferCubes3Update(cs, &n.c);
    assertEquals(__ferCubes3IDCoords(cs, n.c.coords), 5);
    assertEquals(n.c.cube_id, 5);

    ferVec3Set(&n.v, -10., -10., -10.);
    ferCubes3Update(cs, &n.c);
    assertEquals(__ferCubes3IDCoords(cs, n.c.coords), 0);
    assertEquals(n.c.cube_id, 0);

    ferVec3Set(&n.v, 10., 10., 10.);
    ferCubes3Update(cs, &n.c);
    assertEquals(__ferCubes3IDCoords(cs, n.c.coords), 44);
    assertEquals(n.c.cube_id, 44);

    ferVec3Set(&n.v, 10., 1.8, 0.);
    ferCubes3Update(cs, &n.c);
    assertEquals(__ferCubes3IDCoords(cs, n.c.coords), 29);
    assertEquals(n.c.cube_id, 29);

    ferCubes3Del(cs);

    printf("------ cubesNode\n\n");
}



static void elNew(el_t *ns, size_t len)
{
    size_t i;
    fer_real_t x, y, z;

    for (i = 0; i < len; i++){
        x = ferRand(&r, -10., 10.);
        y = ferRand(&r, -10., 10.);
        z = ferRand(&r, -10., 10.);

        ferVec3Set(&ns[i].v, x, y, z);
        ferCubes3ElInit(&ns[i].c, &ns[i].v);
    }
}

static void elAdd(fer_cubes3_t *cs, el_t *ns, size_t len)
{
    size_t i;

    for (i = 0; i < len; i++){
        ferCubes3Add(cs, &ns[i].c);
    }
}

/*
static void pNodes(fer_node_t **ns, size_t len, const fer_vec_t *v)
{
    size_t i;

    printf("%lx: ", (long)ns);

    for (i=0; i < len; i++){
        printf("%ld(%lf %lf %lf) - %lf, ",
               (long)ns[i], ferVecX(ns[i]), ferVecY(ns[i]), ferVecZ(ns[i]),
               ferVecDistance(v, ns[i]));

    }
    printf("\n");
}

static void pVec(const fer_vec_t *v, const char *p)
{
    printf("%s(%lf %lf %lf)\n", p, ferVecX(v), ferVecY(v), ferVecZ(v));
}
*/

static size_t nearest(el_t *ns, size_t nslen,
                    const fer_vec3_t *v,
                    fer_cubes3_el_t **n, size_t len)
{
    size_t i, j, nlen, curi;
    fer_real_t dist, curdist;
    //el_t *tmp;
    int skip;

    len = FER_MIN(len, nslen);

    nlen = 0;
    curi = 0;
    while (curi != len){
        curdist = FER_REAL_MAX;
        for (i = 0; i < nslen; i++){
            dist = ferVec3Dist2(v, ns[i].c.coords);
            if (dist < curdist){
                skip = 0;
                for (j = 0; j < curi; j++){
                    if (n[j] == &ns[i].c){
                        skip = 1;
                        break;
                    }
                }

                if (!skip){
                    n[curi] = &ns[i].c;
                    curdist = dist;
                }
            }
        }

        curi++;
    }

    return curi;
}

static void vInit(fer_vec3_t *v)
{
    fer_real_t x, y, z;

    x = ferRand(&r, -10., 10.);
    y = ferRand(&r, -10., 10.);
    z = ferRand(&r, -10., 10.);

    ferVec3Set(v, x, y, z);
}

static int cmp(fer_cubes3_el_t **n1, fer_cubes3_el_t **n2, size_t len)
{
    size_t i;

    for (i=0; i < len; i++){
        if (n1[i] != n2[i]){
            return 0;
        }
    }

    return 1;
}

static void nearRun(fer_cubes3_t *cs,
                    fer_cubes3_el_t **near, fer_cubes3_el_t **near2, size_t len,
                    fer_vec3_t *v,
                    el_t *n, size_t nlen)

{
    size_t i, p, reslen;
    size_t howmany = ferRand(&r, 0, nlen);

    // change some nodes
    for (i=0; i < howmany; i++){
        p = ferRand(&r, 0, nlen);
        vInit(&n[p].v);
        ferCubes3Update(cs, &n[p].c);
    }

    vInit(v);

    for (i=0; i < len; i++){
        near[i] = near2[i] = NULL;
    }

    reslen = ferCubes3Nearest(cs, v, len, near);
    nearest(n, nlen, v, near2, len);

    assertTrue(cmp(near, near2, len));
}

#define N_LEN 300
#define N_LOOPS 3000
TEST(cubesNearest)
{
    fer_vec3_t v;
    el_t ns[N_LEN];
    fer_cubes3_el_t *near[5], *near2[5];
    fer_cubes3_t *cs;
    fer_real_t range[6] = { -9., 9., -11., 7., -10., 11. };
    size_t num = 40, i;
    const size_t *dim;

    printf("cubesNearest:\n");

    cs = ferCubes3New(range, num);
    ferVec3Set(&v, 0., 0.1, 0.1);
    elNew(ns, N_LEN);
    elAdd(cs, ns, N_LEN);


    printf("cube size: %f\n", (float)ferCubes3Size(cs));
    printf("cubes len: %d\n", (int)ferCubes3Len(cs));
    dim = ferCubes3Dim(cs);
    printf("cubes dim: %d %d %d\n", dim[0], dim[1], dim[2]);

    for (i=0; i < N_LOOPS; i++)
        nearRun(cs, near, near2, 1, &v, ns, N_LEN);
    for (i=0; i < N_LOOPS; i++)
        nearRun(cs, near, near2, 2, &v, ns, N_LEN);
    for (i=0; i < N_LOOPS; i++)
        nearRun(cs, near, near2, 3, &v, ns, N_LEN);
    for (i=0; i < N_LOOPS; i++)
        nearRun(cs, near, near2, 4, &v, ns, N_LEN);
    for (i=0; i < N_LOOPS; i++)
        nearRun(cs, near, near2, 5, &v, ns, N_LEN);

    ferCubes3Del(cs);

    printf("------ cubesNearest\n\n");
}

TEST(cubesNearest2)
{
    fer_vec3_t v;
    el_t ns[3];
    fer_cubes3_el_t *near[5], *near2[5];
    fer_cubes3_t *cs;
    fer_real_t range[6] = { -9., 9., -11., 7., -10., 11. };
    size_t num = 40, i;
    const size_t *dim;

    printf("cubesNearest:\n");

    cs = ferCubes3New(range, num);
    ferVec3Set(&v, 0., 0.1, 0.1);
    elNew(ns, 3);
    elAdd(cs, ns, 3);


    printf("cube size: %f\n", (float)ferCubes3Size(cs));
    printf("cubes len: %d\n", (int)ferCubes3Len(cs));
    dim = ferCubes3Dim(cs);
    printf("cubes dim: %d %d %d\n", dim[0], dim[1], dim[2]);

    for (i=0; i < N_LOOPS; i++)
        nearRun(cs, near, near2, 5, &v, ns, 3);

    ferCubes3Del(cs);

    printf("------ cubesNearest\n\n");
}
