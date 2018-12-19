#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/fifo.h>
#include <boruvka/alloc.h>
#include <boruvka/rand.h>

TEST(fifo1)
{
    bor_fifo_t *fifo;
    int i;
    int nums[10000], *num;
    bor_rand_t rnd;

    borRandInit(&rnd);
    for (i = 0; i < 10000; ++i){
        nums[i] = borRand(&rnd, -5000, 5000);
    }

    fifo = borFifoNew(sizeof(int));
    for (i = 0; i < 10000; ++i){
        borFifoPush(fifo, nums + i);
    }

    for (i = 0; !borFifoEmpty(fifo); ++i){
        num = borFifoFront(fifo);
        assertEquals((*num), nums[i]);
        borFifoPop(fifo);
    }

    for (i = 0; i < 1000; ++i){
        borFifoPush(fifo, nums + i);
    }

    for (i = 0; !borFifoEmpty(fifo); ++i){
        num = borFifoFront(fifo);
        assertEquals((*num), nums[i]);
        borFifoPop(fifo);
    }
    assertEquals(i, 1000);

    for (i = 1000; i < 2000; ++i){
        borFifoPush(fifo, nums + i);
    }

    for (i = 0; !borFifoEmpty(fifo); ++i){
        num = borFifoFront(fifo);
        assertEquals((*num), nums[i + 1000]);
        borFifoPop(fifo);
    }
    assertEquals(i, 1000);

    for (i = 1000; i < 2000; ++i){
        borFifoPush(fifo, nums + i);
    }
    borFifoClear(fifo);
    assertTrue(borFifoEmpty(fifo));

    borFifoDel(fifo);
}
