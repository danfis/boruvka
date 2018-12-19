#include <cu/cu.h>
#include <boruvka/lifo.h>
#include <boruvka/alloc.h>
#include <boruvka/rand.h>

TEST(lifo1)
{
    bor_lifo_t *lifo;
    int i;
    int nums[10000], *num;
    bor_rand_t rnd;

    borRandInit(&rnd);
    for (i = 0; i < 10000; ++i){
        nums[i] = borRand(&rnd, -5000, 5000);
    }

    lifo = borLifoNew(sizeof(int));
    for (i = 0; i < 10000; ++i){
        borLifoPush(lifo, nums + i);
    }

    for (i = 0; !borLifoEmpty(lifo); ++i){
        num = borLifoBack(lifo);
        assertEquals((*num), nums[10000 - i - 1]);
        borLifoPop(lifo);
    }

    for (i = 0; i < 1000; ++i){
        borLifoPush(lifo, nums + i);
    }

    for (i = 0; !borLifoEmpty(lifo); ++i){
        num = borLifoBack(lifo);
        assertEquals((*num), nums[1000 - i - 1]);
        borLifoPop(lifo);
    }
    assertEquals(i, 1000);

    for (i = 1000; i < 2000; ++i){
        borLifoPush(lifo, nums + i);
    }

    for (i = 0; !borLifoEmpty(lifo); ++i){
        num = borLifoBack(lifo);
        assertEquals((*num), nums[2000 - i - 1]);
        borLifoPop(lifo);
    }
    assertEquals(i, 1000);

    for (i = 1000; i < 2000; ++i){
        borLifoPush(lifo, nums + i);
    }
    borLifoClear(lifo);
    assertTrue(borLifoEmpty(lifo));

    borLifoDel(lifo);
}

