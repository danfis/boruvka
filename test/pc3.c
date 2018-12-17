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

static int pcContains(bor_pc_t *pc, const bor_vec3_t *v)
{
    bor_pc_it_t it;
    const bor_vec3_t *w;

    borPCItInit(&it, pc);
    while (!borPCItEnd(&it)){
        w = (bor_vec3_t *)borPCItGet(&it);
        if (borVec3Eq(w, v))
            return 1;

        borPCItNext(&it);
    }
    return 0;
}

void pcPrint(bor_pc_t *pc)
{
    size_t i, len = borPCLen(pc);
    const bor_vec3_t *w;

    for (i = 0; i < len; i++){
        w = (bor_vec3_t *)borPCGet(pc, i);
        fprintf(stdout, "%g %g %g\n",
                borVec3X(w), borVec3Y(w), borVec3Z(w));
    }
}

TEST(pcPermutate)
{
    bor_vec3_t v[1000];
    bor_pc_t *pc;
    size_t i;

    for (i = 0; i < 1000; i++){
        borVec3Set(&v[i], i, 0., 0.);
    }

    pc = borPCNew2(3, 2);

    borPCAdd(pc, (bor_vec_t *)&v[0]);
    borPCPermutate(pc);
    assertEquals(1, borPCLen(pc));
    assertTrue(pcContains(pc, &v[0]));

    borPCAdd(pc, (bor_vec_t *)&v[1]);
    borPCAdd(pc, (bor_vec_t *)&v[2]);
    assertEquals(3, borPCLen(pc));
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));
    borPCPermutate(pc);
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));

    borPCAdd(pc, (bor_vec_t *)&v[3]);
    assertEquals(4, borPCLen(pc));
    borPCPermutate(pc);
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));
    assertTrue(pcContains(pc, &v[3]));


    for (i = 4; i < 169; i++){
        borPCAdd(pc, (bor_vec_t *)&v[i]);
    }
    assertEquals(borPCLen(pc), 169);
    borPCPermutate(pc);
    for (i = 0; i < 169; i++){
        assertTrue(pcContains(pc, &v[i]));
    }


    for (i = 169; i < 1000; i++){
        borPCAdd(pc, (bor_vec_t *)&v[i]);
    }
    assertEquals(borPCLen(pc), 1000);
    borPCPermutate(pc);
    for (i = 0; i < 1000; i++){
        assertTrue(pcContains(pc, &v[i]));
    }

#ifndef BOR_SSE
    bor_vec3_t w;
    // test some basic operation on the points
    for (i = 0; i < 10; i++){
        borVec3Add2(&w, (const bor_vec3_t *)borPCGet(pc, 0),
                        (const bor_vec3_t *)borPCGet(pc, 1));
    }
#endif /* BOR_SSE */

    borPCDel(pc);
}

TEST(pcFromFile)
{
    bor_pc_t *pc;
    size_t added;

    pc = borPCNew(3);

    added = borPCAddFromFile(pc, "asdfg");
    assertEquals(added, 0);

    added = borPCAddFromFile(pc, "cube.txt");
    assertEquals(added, 2402);
    printf("\n----- pcFromFile ----\n");
    pcPrint(pc);
    printf("\n----- pcFromFile END ----\n");

    borPCDel(pc);
}
