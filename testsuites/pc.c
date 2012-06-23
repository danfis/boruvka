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

    ferPCItInit(&it, pc);
    while (!ferPCItEnd(&it)){
        w = ferPCItGet(&it);
        if (ferVec2Eq((const bor_vec2_t *)w, v))
            return 1;

        ferPCItNext(&it);
    }
    return 0;
}

static void pcPrint(bor_pc_t *pc)
{
    size_t i, j, len = ferPCLen(pc);
    const bor_vec_t *w;

    for (i = 0; i < len; i++){
        w = ferPCGet(pc, i);

        for (j = 0; j < pc->dim; j++){
            fprintf(stdout, "%f ", (float)ferVecGet(w, j));
        }
        fprintf(stdout, "\n");
    }
}

TEST(ppcPermutate)
{
    bor_vec2_t v[1000], w;
    bor_pc_t *pc;
    size_t i;

    for (i = 0; i < 1000; i++){
        ferVec2Set(&v[i], i, 0.);
    }

    pc = ferPCNew2(2, 12);

    ferPCAdd(pc, (bor_vec_t *)&v[0]);
    ferPCPermutate(pc);
    assertEquals(1, ferPCLen(pc));
    assertTrue(pcContains(pc, &v[0]));

    ferPCAdd(pc, (bor_vec_t *)&v[1]);
    ferPCAdd(pc, (bor_vec_t *)&v[2]);
    assertEquals(3, ferPCLen(pc));
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));
    ferPCPermutate(pc);
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));

    ferPCAdd(pc, (bor_vec_t *)&v[3]);
    assertEquals(4, ferPCLen(pc));
    ferPCPermutate(pc);
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));
    assertTrue(pcContains(pc, &v[3]));


    for (i = 4; i < 169; i++){
        ferPCAdd(pc, (bor_vec_t *)&v[i]);
    }
    assertEquals(ferPCLen(pc), 169);
    ferPCPermutate(pc);
    for (i = 0; i < 169; i++){
        assertTrue(pcContains(pc, &v[i]));
    }


    for (i = 169; i < 1000; i++){
        ferPCAdd(pc, (bor_vec_t *)&v[i]);
    }
    assertEquals(ferPCLen(pc), 1000);
    ferPCPermutate(pc);
    for (i = 0; i < 1000; i++){
        assertTrue(pcContains(pc, &v[i]));
    }

    // test some basic operation on the points
    for (i = 0; i < 10; i++){
        ferVec2Add2(&w, (bor_vec2_t *)ferPCGet(pc, 0), (bor_vec2_t *)ferPCGet(pc, 1));
    }

    ferPCDel(pc);
}

TEST(ppcFromFile)
{
    bor_pc_t *pc;
    size_t added;

    pc = ferPCNew(7);

    added = ferPCAddFromFile(pc, "asdfg");
    assertEquals(added, 0);

    added = ferPCAddFromFile(pc, "data-test-cd-spheres.trans.txt");
    assertEquals(added, 5000);
    printf("\n----- pcFromFile ----\n");
    pcPrint(pc);
    printf("\n----- pcFromFile END ----\n");

    ferPCDel(pc);
}
