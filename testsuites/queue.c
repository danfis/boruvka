#include <cu/cu.h>
#include <boruvka/ibucketq.h>
#include <boruvka/lbucketq.h>
#include <boruvka/pbucketq.h>
#include <boruvka/iadaq.h>
#include <boruvka/padaq.h>

TEST(testIBucketQ)
{
    bor_ibucketq_t iq;
    int key, ival;

    borIBucketQInit(&iq);
    assertTrue(borIBucketQIsEmpty(&iq));
    borIBucketQPush(&iq, 10, 1234);
    borIBucketQPush(&iq, 2, -89);
    borIBucketQPush(&iq, 8, 91);
    borIBucketQPush(&iq, 80, -240);
    assertFalse(borIBucketQIsEmpty(&iq));

    ival = borIBucketQPop(&iq, &key);
    assertEquals(ival, -89);
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
    assertEquals(ival, -240);
    assertEquals(key, 80);

    assertFalse(borIBucketQIsEmpty(&iq));
    ival = borIBucketQPop(&iq, &key);
    assertEquals(ival, 111);
    assertEquals(key, 125);

    assertTrue(borIBucketQIsEmpty(&iq));
    borIBucketQFree(&iq);
}

TEST(testLBucketQ)
{
    bor_lbucketq_t lq;
    int key;
    long lval;

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

TEST(testPBucketQ)
{
    bor_pbucketq_t q;
    int key;
    int vals[6] = {1234, 89, 91, 240, 111, 99 };
    void *val;

    borPBucketQInit(&q);
    assertTrue(borPBucketQIsEmpty(&q));
    borPBucketQPush(&q, 10, vals + 0);
    borPBucketQPush(&q, 2, vals + 1);
    borPBucketQPush(&q, 8, vals + 2);
    borPBucketQPush(&q, 80, vals + 3);
    assertFalse(borPBucketQIsEmpty(&q));

    val = borPBucketQPop(&q, &key);
    assertEquals(val, (void *)(vals + 1));
    assertEquals(*(int *)val, 89);
    assertEquals(key, 2);

    val = borPBucketQPop(&q, &key);
    assertEquals(val, (void *)(vals + 2));
    assertEquals(*(int *)val, 91);
    assertEquals(key, 8);

    borPBucketQPush(&q, 125, vals + 4);
    borPBucketQPush(&q, 0, vals + 5);

    assertFalse(borPBucketQIsEmpty(&q));
    val = borPBucketQPop(&q, &key);
    assertEquals(val, (void *)(vals + 5));
    assertEquals(*(int *)val, 99);
    assertEquals(key, 0);

    assertFalse(borPBucketQIsEmpty(&q));
    val = borPBucketQPop(&q, &key);
    assertEquals(val, (void *)(vals + 0));
    assertEquals(*(int *)val, 1234);
    assertEquals(key, 10);

    assertFalse(borPBucketQIsEmpty(&q));
    val = borPBucketQPop(&q, &key);
    assertEquals(val, (void *)(vals + 3));
    assertEquals(*(int *)val, 240);
    assertEquals(key, 80);

    assertFalse(borPBucketQIsEmpty(&q));
    val = borPBucketQPop(&q, &key);
    assertEquals(val, (void *)(vals + 4));
    assertEquals(*(int *)val, 111);
    assertEquals(key, 125);

    assertTrue(borPBucketQIsEmpty(&q));
    borPBucketQFree(&q);
}

TEST(testIAdaQ)
{
    bor_iadaq_t iq;
    int key, ival;

    borIAdaQInit(&iq);
    assertTrue(borIAdaQIsEmpty(&iq));
    borIAdaQPush(&iq, 10, 1234);
    borIAdaQPush(&iq, 2, 89);
    borIAdaQPush(&iq, 8, 91);
    borIAdaQPush(&iq, 80, 240);
    assertFalse(borIAdaQIsEmpty(&iq));

    ival = borIAdaQPop(&iq, &key);
    assertEquals(ival, 89);
    assertEquals(key, 2);

    ival = borIAdaQPop(&iq, &key);
    assertEquals(ival, 91);
    assertEquals(key, 8);

    borIAdaQPush(&iq, 125, 111);
    borIAdaQPush(&iq, 0, 99);

    assertFalse(borIAdaQIsEmpty(&iq));
    ival = borIAdaQPop(&iq, &key);
    assertEquals(ival, 99);
    assertEquals(key, 0);

    assertFalse(borIAdaQIsEmpty(&iq));
    ival = borIAdaQPop(&iq, &key);
    assertEquals(ival, 1234);
    assertEquals(key, 10);

    assertFalse(borIAdaQIsEmpty(&iq));
    ival = borIAdaQPop(&iq, &key);
    assertEquals(ival, 240);
    assertEquals(key, 80);

    assertFalse(borIAdaQIsEmpty(&iq));
    ival = borIAdaQPop(&iq, &key);
    assertEquals(ival, 111);
    assertEquals(key, 125);

    assertTrue(borIAdaQIsEmpty(&iq));
    borIAdaQPush(&iq, 10, 1234);
    borIAdaQPush(&iq, 2, 89);
    borIAdaQPush(&iq, 8, 91);
    assertFalse(borIAdaQIsEmpty(&iq));

    ival = borIAdaQPop(&iq, &key);
    assertEquals(ival, 89);
    assertEquals(key, 2);

    ival = borIAdaQPop(&iq, &key);
    assertEquals(ival, 91);
    assertEquals(key, 8);

    borIAdaQPush(&iq, 125, 111);
    borIAdaQPush(&iq, 80000, 240);
    borIAdaQPush(&iq, 0, 99);

    assertFalse(borIAdaQIsEmpty(&iq));
    ival = borIAdaQPop(&iq, &key);
    assertEquals(ival, 99);
    assertEquals(key, 0);

    assertFalse(borIAdaQIsEmpty(&iq));
    ival = borIAdaQPop(&iq, &key);
    assertEquals(ival, 1234);
    assertEquals(key, 10);

    assertFalse(borIAdaQIsEmpty(&iq));
    ival = borIAdaQPop(&iq, &key);
    assertEquals(ival, 111);
    assertEquals(key, 125);

    assertFalse(borIAdaQIsEmpty(&iq));
    ival = borIAdaQPop(&iq, &key);
    assertEquals(ival, 240);
    assertEquals(key, 80000);
    borIAdaQFree(&iq);
}


TEST(testPAdaQ)
{
    bor_padaq_t q;
    int key;
    int vals[6] = {1234, 89, 91, 240, 111, 99 };
    void *val;

    borPAdaQInit(&q);
    assertTrue(borPAdaQIsEmpty(&q));
    borPAdaQPush(&q, 10, vals + 0);
    borPAdaQPush(&q, 2, vals + 1);
    borPAdaQPush(&q, 8, vals + 2);
    borPAdaQPush(&q, 80, vals + 3);
    assertFalse(borPAdaQIsEmpty(&q));

    val = borPAdaQPop(&q, &key);
    assertEquals(val, (void *)(vals + 1));
    assertEquals(*(int *)val, 89);
    assertEquals(key, 2);

    val = borPAdaQPop(&q, &key);
    assertEquals(val, (void *)(vals + 2));
    assertEquals(*(int *)val, 91);
    assertEquals(key, 8);

    borPAdaQPush(&q, 125, vals + 4);
    borPAdaQPush(&q, 0, vals + 5);

    assertFalse(borPAdaQIsEmpty(&q));
    val = borPAdaQPop(&q, &key);
    assertEquals(val, (void *)(vals + 5));
    assertEquals(*(int *)val, 99);
    assertEquals(key, 0);

    borPAdaQPush(&q, 80000, vals + 0);

    assertFalse(borPAdaQIsEmpty(&q));
    val = borPAdaQPop(&q, &key);
    assertEquals(val, (void *)(vals + 0));
    assertEquals(*(int *)val, 1234);
    assertEquals(key, 10);

    assertFalse(borPAdaQIsEmpty(&q));
    val = borPAdaQPop(&q, &key);
    assertEquals(val, (void *)(vals + 3));
    assertEquals(*(int *)val, 240);
    assertEquals(key, 80);

    assertFalse(borPAdaQIsEmpty(&q));
    val = borPAdaQPop(&q, &key);
    assertEquals(val, (void *)(vals + 4));
    assertEquals(*(int *)val, 111);
    assertEquals(key, 125);

    assertFalse(borPAdaQIsEmpty(&q));
    val = borPAdaQPop(&q, &key);
    assertEquals(val, (void *)(vals + 0));
    assertEquals(*(int *)val, 1234);
    assertEquals(key, 80000);

    assertTrue(borPAdaQIsEmpty(&q));
    borPAdaQFree(&q);
}
