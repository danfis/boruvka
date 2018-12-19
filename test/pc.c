#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/pc.h>
#include <boruvka/dbg.h>

TEST(ppcSetUp)
{
}

TEST(ppcTearDown)
{
}

static int pcContains(bor_pc_t *pc, const bor_vec2_t *v)
{
    bor_pc_it_t it;
    const bor_vec_t *w;

    borPCItInit(&it, pc);
    while (!borPCItEnd(&it)){
        w = borPCItGet(&it);
        if (borVec2Eq((const bor_vec2_t *)w, v))
            return 1;

        borPCItNext(&it);
    }
    return 0;
}

static void pcPrint(bor_pc_t *pc)
{
    size_t i, j, len = borPCLen(pc);
    const bor_vec_t *w;

    for (i = 0; i < len; i++){
        w = borPCGet(pc, i);

        for (j = 0; j < pc->dim; j++){
            fprintf(stdout, "%f ", (float)borVecGet(w, j));
        }
        fprintf(stdout, "\n");
    }
}

TEST(ppcPermutate)
{
    bor_vec2_t v[1000];
    bor_pc_t *pc;
    size_t i;

    for (i = 0; i < 1000; i++){
        borVec2Set(&v[i], i, 0.);
    }

    pc = borPCNew2(2, 12);

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
    bor_vec2_t w;
    // test some basic operation on the points
    for (i = 0; i < 10; i++){
        borVec2Add2(&w, (bor_vec2_t *)borPCGet(pc, 0), (bor_vec2_t *)borPCGet(pc, 1));
    }
#endif /* BOR_SSE */

    borPCDel(pc);
}

TEST(ppcFromFile)
{
    bor_pc_t *pc;
    size_t added;

    pc = borPCNew(7);

    added = borPCAddFromFile(pc, "asdfg");
    assertEquals(added, 0);

    added = borPCAddFromFile(pc, "data-test-cd-spheres.trans.txt");
    assertEquals(added, 5000);
    printf("\n----- pcFromFile ----\n");
    pcPrint(pc);
    printf("\n----- pcFromFile END ----\n");

    borPCDel(pc);
}
