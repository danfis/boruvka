#include <stdio.h>
#include <boruvka/rand-mt.h>

void test(fer_rand_mt_t *r)
{
    size_t i;

    printf("    Generate 10 numbers in [0,1):\n       ");
    for (i = 0; i < 10; i++)
        printf(" %g", ferRandMT01(r));
    printf("\n");

    printf("    Generate 10 numbers in [0,1]:\n       ");
    for (i = 0; i < 10; i++)
        printf(" %g", ferRandMT01Closed(r));
    printf("\n");

    printf("    Generate 10 numbers in (0,1):\n       ");
    for (i = 0; i < 10; i++)
        printf(" %g", ferRandMT01Open(r));
    printf("\n");

    printf("    Generate 5 integers in:\n       ");
    for (i = 0; i < 5; i++)
        printf(" %u", ferRandMTInt(r));
    printf("\n");

    printf("    Generate 10 numbers in [0, 1.5):\n       ");
    for (i = 0; i < 10; i++)
        printf(" %g", ferRandMT(r, 0., 1.5));
    printf("\n");

    printf("    Generate 10 numbers in [1.1, 1.2):\n       ");
    for (i = 0; i < 10; i++)
        printf(" %g", ferRandMT(r, 1.1, 1.2));
    printf("\n");

    printf("    Generate 10 numbers in [-.5, .2):\n       ");
    for (i = 0; i < 10; i++)
        printf(" %g", ferRandMT(r, -.5, .2));
    printf("\n");
}

int main(int argc, char *argv[])
{
    fer_rand_mt_t *r;

    printf("Auto:\n");
    r = ferRandMTNewAuto();
    test(r);
    ferRandMTDel(r);

    printf("7654:\n");
    r = ferRandMTNew(7654);
    test(r);
    ferRandMTDel(r);

    printf("1111:\n");
    r = ferRandMTNew(1111);
    test(r);

    printf("1111 -> 2222:\n");
    ferRandMTReseed(r, 2222);
    test(r);

    printf("2222 -> auto:\n");
    ferRandMTReseedAuto(r);
    test(r);
    ferRandMTDel(r);

    return 0;
}
