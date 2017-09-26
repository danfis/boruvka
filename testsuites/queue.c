#include <cu/cu.h>
#include <boruvka/ibucketq.h>
#include <boruvka/lbucketq.h>

TEST(testBucketQ)
{
    bor_ibucketq_t iq;
    bor_lbucketq_t lq;
    int key, ival;
    long lval;

    borIBucketQInit(&iq);
    assertTrue(borIBucketQIsEmpty(&iq));
    borIBucketQPush(&iq, 10, 1234);
    borIBucketQPush(&iq, 2, 89);
    borIBucketQPush(&iq, 8, 91);
    borIBucketQPush(&iq, 80, 240);
    assertFalse(borIBucketQIsEmpty(&iq));

    ival = borIBucketQPop(&iq, &key);
    assertEquals(ival, 89);
    assertEquals(key, 2);

    ival = borIBucketQPop(&iq, &key);
    assertEquals(ival, 91);
    assertEquals(key, 8);

    borIBucketQPush(&iq, 125, 111);
    borIBucketQPush(&iq, 0, 99);

    assertFalse(borIBucketQIsEmpty(&iq));
    ival = borIBucketQPop(&iq, &key);
    assertEquals(ival, 99);
    assertEquals(key, 0);

    assertFalse(borIBucketQIsEmpty(&iq));
    ival = borIBucketQPop(&iq, &key);
    assertEquals(ival, 1234);
    assertEquals(key, 10);

    assertFalse(borIBucketQIsEmpty(&iq));
    ival = borIBucketQPop(&iq, &key);
    assertEquals(ival, 240);
    assertEquals(key, 80);

    assertFalse(borIBucketQIsEmpty(&iq));
    ival = borIBucketQPop(&iq, &key);
    assertEquals(ival, 111);
    assertEquals(key, 125);

    assertTrue(borIBucketQIsEmpty(&iq));
    borIBucketQFree(&iq);


    borLBucketQInit(&lq);
    assertTrue(borLBucketQIsEmpty(&lq));
    borLBucketQPush(&lq, 10, 1234);
    borLBucketQPush(&lq, 2, 89);
    borLBucketQPush(&lq, 8, 91);
    borLBucketQPush(&lq, 80, 240);
    assertFalse(borLBucketQIsEmpty(&lq));

    lval = borLBucketQPop(&lq, &key);
    assertEquals(lval, 89);
    assertEquals(key, 2);

    lval = borLBucketQPop(&lq, &key);
    assertEquals(lval, 91);
    assertEquals(key, 8);

    borLBucketQPush(&lq, 125, 111);
    borLBucketQPush(&lq, 0, 99);

    assertFalse(borLBucketQIsEmpty(&lq));
    lval = borLBucketQPop(&lq, &key);
    assertEquals(lval, 99);
    assertEquals(key, 0);

    assertFalse(borLBucketQIsEmpty(&lq));
    lval = borLBucketQPop(&lq, &key);
    assertEquals(lval, 1234L);
    assertEquals(key, 10);

    assertFalse(borLBucketQIsEmpty(&lq));
    lval = borLBucketQPop(&lq, &key);
    assertEquals(lval, 240L);
    assertEquals(key, 80);

    assertFalse(borLBucketQIsEmpty(&lq));
    lval = borLBucketQPop(&lq, &key);
    assertEquals(lval, 111L);
    assertEquals(key, 125);

    assertTrue(borLBucketQIsEmpty(&lq));
    borLBucketQFree(&lq);
}
