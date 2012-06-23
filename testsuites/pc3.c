#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/vec3.h>
#include <boruvka/pc.h>
#include <boruvka/dbg.h>

TEST(pcSetUp)
{
}

TEST(pcTearDown)
{
}

static int pcContains(fer_pc_t *pc, const fer_vec3_t *v)
{
    fer_pc_it_t it;
    const fer_vec3_t *w;

    ferPCItInit(&it, pc);
    while (!ferPCItEnd(&it)){
        w = (fer_vec3_t *)ferPCItGet(&it);
        if (ferVec3Eq(w, v))
            return 1;

        ferPCItNext(&it);
    }
    return 0;
}

void pcPrint(fer_pc_t *pc)
{
    size_t i, len = ferPCLen(pc);
    const fer_vec3_t *w;

    for (i = 0; i < len; i++){
        w = (fer_vec3_t *)ferPCGet(pc, i);
        fprintf(stdout, "%g %g %g\n",
                ferVec3X(w), ferVec3Y(w), ferVec3Z(w));
    }
}

TEST(pcPermutate)
{
    fer_vec3_t v[1000], w;
    fer_pc_t *pc;
    size_t i;

    for (i = 0; i < 1000; i++){
        ferVec3Set(&v[i], i, 0., 0.);
    }

    pc = ferPCNew2(3, 2);

    ferPCAdd(pc, (fer_vec_t *)&v[0]);
    ferPCPermutate(pc);
    assertEquals(1, ferPCLen(pc));
    assertTrue(pcContains(pc, &v[0]));

    ferPCAdd(pc, (fer_vec_t *)&v[1]);
    ferPCAdd(pc, (fer_vec_t *)&v[2]);
    assertEquals(3, ferPCLen(pc));
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));
    ferPCPermutate(pc);
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));

    ferPCAdd(pc, (fer_vec_t *)&v[3]);
    assertEquals(4, ferPCLen(pc));
    ferPCPermutate(pc);
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));
    assertTrue(pcContains(pc, &v[3]));


    for (i = 4; i < 169; i++){
        ferPCAdd(pc, (fer_vec_t *)&v[i]);
    }
    assertEquals(ferPCLen(pc), 169);
    ferPCPermutate(pc);
    for (i = 0; i < 169; i++){
        assertTrue(pcContains(pc, &v[i]));
    }


    for (i = 169; i < 1000; i++){
        ferPCAdd(pc, (fer_vec_t *)&v[i]);
    }
    assertEquals(ferPCLen(pc), 1000);
    ferPCPermutate(pc);
    for (i = 0; i < 1000; i++){
        assertTrue(pcContains(pc, &v[i]));
    }

    // test some basic operation on the points
    for (i = 0; i < 10; i++){
        ferVec3Add2(&w, (const fer_vec3_t *)ferPCGet(pc, 0),
                        (const fer_vec3_t *)ferPCGet(pc, 1));
    }

    ferPCDel(pc);
}

TEST(pcFromFile)
{
    fer_pc_t *pc;
    size_t added;

    pc = ferPCNew(3);

    added = ferPCAddFromFile(pc, "asdfg");
    assertEquals(added, 0);

    added = ferPCAddFromFile(pc, "cube.txt");
    assertEquals(added, 2402);
    printf("\n----- pcFromFile ----\n");
    pcPrint(pc);
    printf("\n----- pcFromFile END ----\n");

    ferPCDel(pc);
}
