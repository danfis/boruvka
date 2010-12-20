#include <stdio.h>
#include <cu/cu.h>
#include <mg/point_cloud.h>
#include <mg/dbg.h>

TEST(pcSetUp)
{
}

TEST(pcTearDown)
{
}

static int pcContains(mg_pc_t *pc, const mg_vec3_t *v)
{
    mg_pc_it_t it;
    const mg_vec3_t *w;

    mgPCItInit(&it, pc);
    while (!mgPCItEnd(&it)){
        w = mgPCItGet(&it);
        if (mgVec3Eq(w, v))
            return 1;

        mgPCItNext(&it);
    }
    return 0;
}

void pcPrint(mg_pc_t *pc)
{
    size_t i, len = mgPCLen(pc);
    const mg_vec3_t *w;

    for (i = 0; i < len; i++){
        w = mgPCGet(pc, i);
        fprintf(stdout, "%g %g %g\n",
                mgVec3X(w), mgVec3Y(w), mgVec3Z(w));
    }
}

TEST(pcPermutate)
{
    mg_vec3_t v[1000], w;
    mg_pc_t *pc;
    size_t i;

    for (i = 0; i < 1000; i++){
        mgVec3Set(&v[i], i, 0., 0.);
    }

    pc = mgPCNew();
    mgPCSetMinChunkSize(pc, 2);

    mgPCAdd(pc, &v[0]);
    mgPCPermutate(pc);
    assertEquals(1, mgPCLen(pc));
    assertTrue(pcContains(pc, &v[0]));

    mgPCAdd(pc, &v[1]);
    mgPCAdd(pc, &v[2]);
    assertEquals(3, mgPCLen(pc));
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));
    mgPCPermutate(pc);
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));

    mgPCAdd(pc, &v[3]);
    assertEquals(4, mgPCLen(pc));
    mgPCPermutate(pc);
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));
    assertTrue(pcContains(pc, &v[3]));


    for (i = 4; i < 169; i++){
        mgPCAdd(pc, &v[i]);
    }
    assertEquals(mgPCLen(pc), 169);
    mgPCPermutate(pc);
    for (i = 0; i < 169; i++){
        assertTrue(pcContains(pc, &v[i]));
    }


    for (i = 169; i < 1000; i++){
        mgPCAdd(pc, &v[i]);
    }
    assertEquals(mgPCLen(pc), 1000);
    mgPCPermutate(pc);
    for (i = 0; i < 1000; i++){
        assertTrue(pcContains(pc, &v[i]));
    }

    // test some basic operation on the points
    for (i = 0; i < 10; i++){
        mgVec3Add2(&w, mgPCGet(pc, 0), mgPCGet(pc, 1));
    }

    mgPCDel(pc);
}

TEST(pcFromFile)
{
    mg_pc_t *pc;
    size_t added;

    pc = mgPCNew();

    added = mgPCAddFromFile(pc, "asdfg");
    assertEquals(added, 0);

    added = mgPCAddFromFile(pc, "cube.txt");
    assertEquals(added, 2402);
    printf("\n----- pcFromFile ----\n");
    pcPrint(pc);
    printf("\n----- pcFromFile END ----\n");

    mgPCDel(pc);
}
