#include <stdio.h>
#include <cu/cu.h>
#include <fermat/pc3.h>
#include <fermat/dbg.h>

TEST(pcSetUp)
{
}

TEST(pcTearDown)
{
}

static int pcContains(fer_pc3_t *pc, const fer_vec3_t *v)
{
    fer_pc3_it_t it;
    const fer_vec3_t *w;

    ferPC3ItInit(&it, pc);
    while (!ferPC3ItEnd(&it)){
        w = ferPC3ItGet(&it);
        if (ferVec3Eq(w, v))
            return 1;

        ferPC3ItNext(&it);
    }
    return 0;
}

void pcPrint(fer_pc3_t *pc)
{
    size_t i, len = ferPC3Len(pc);
    const fer_vec3_t *w;

    for (i = 0; i < len; i++){
        w = ferPC3Get(pc, i);
        fprintf(stdout, "%g %g %g\n",
                ferVec3X(w), ferVec3Y(w), ferVec3Z(w));
    }
}

TEST(pcPermutate)
{
    fer_vec3_t v[1000], w;
    fer_pc3_t *pc;
    size_t i;

    for (i = 0; i < 1000; i++){
        ferVec3Set(&v[i], i, 0., 0.);
    }

    pc = ferPC3New();
    ferPC3SetMinChunkSize(pc, 2);

    ferPC3Add(pc, &v[0]);
    ferPC3Permutate(pc);
    assertEquals(1, ferPC3Len(pc));
    assertTrue(pcContains(pc, &v[0]));

    ferPC3Add(pc, &v[1]);
    ferPC3Add(pc, &v[2]);
    assertEquals(3, ferPC3Len(pc));
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));
    ferPC3Permutate(pc);
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));

    ferPC3Add(pc, &v[3]);
    assertEquals(4, ferPC3Len(pc));
    ferPC3Permutate(pc);
    assertTrue(pcContains(pc, &v[0]));
    assertTrue(pcContains(pc, &v[1]));
    assertTrue(pcContains(pc, &v[2]));
    assertTrue(pcContains(pc, &v[3]));


    for (i = 4; i < 169; i++){
        ferPC3Add(pc, &v[i]);
    }
    assertEquals(ferPC3Len(pc), 169);
    ferPC3Permutate(pc);
    for (i = 0; i < 169; i++){
        assertTrue(pcContains(pc, &v[i]));
    }


    for (i = 169; i < 1000; i++){
        ferPC3Add(pc, &v[i]);
    }
    assertEquals(ferPC3Len(pc), 1000);
    ferPC3Permutate(pc);
    for (i = 0; i < 1000; i++){
        assertTrue(pcContains(pc, &v[i]));
    }

    // test some basic operation on the points
    for (i = 0; i < 10; i++){
        ferVec3Add2(&w, ferPC3Get(pc, 0), ferPC3Get(pc, 1));
    }

    ferPC3Del(pc);
}

TEST(pcFromFile)
{
    fer_pc3_t *pc;
    size_t added;

    pc = ferPC3New();

    added = ferPC3AddFromFile(pc, "asdfg");
    assertEquals(added, 0);

    added = ferPC3AddFromFile(pc, "cube.txt");
    assertEquals(added, 2402);
    printf("\n----- pcFromFile ----\n");
    pcPrint(pc);
    printf("\n----- pcFromFile END ----\n");

    ferPC3Del(pc);
}
