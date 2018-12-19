#include <stdio.h>
#include <boruvka/rand-mt.h>

void test(bor_rand_mt_t *r)
{
    size_t i;

    printf("    Generate 10 numbers in [0,1):\n       ");
    for (i = 0; i < 10; i++)
        printf(" %g", borRandMT01(r));
    printf("\n");

    printf("    Generate 10 numbers in [0,1]:\n       ");
    for (i = 0; i < 10; i++)
        printf(" %g", borRandMT01Closed(r));
    printf("\n");

    printf("    Generate 10 numbers in (0,1):\n       ");
    for (i = 0; i < 10; i++)
        printf(" %g", borRandMT01Open(r));
    printf("\n");

    printf("    Generate 5 integers in:\n       ");
    for (i = 0; i < 5; i++)
        printf(" %u", borRandMTInt(r));
    printf("\n");

    printf("    Generate 10 numbers in [0, 1.5):\n       ");
    for (i = 0; i < 10; i++)
        printf(" %g", borRandMT(r, 0., 1.5));
    printf("\n");

    printf("    Generate 10 numbers in [1.1, 1.2):\n       ");
    for (i = 0; i < 10; i++)
        printf(" %g", borRandMT(r, 1.1, 1.2));
    printf("\n");

    printf("    Generate 10 numbers in [-.5, .2):\n       ");
    for (i = 0; i < 10; i++)
        printf(" %g", borRandMT(r, -.5, .2));
    printf("\n");
}

int main(int argc, char *argv[])
{
    bor_rand_mt_t *r;

    printf("Auto:\n");
    r = borRandMTNewAuto();
    test(r);
    borRandMTDel(r);

    printf("7654:\n");
    r = borRandMTNew(7654);
    test(r);
    borRandMTDel(r);

    printf("1111:\n");
    r = borRandMTNew(1111);
    test(r);

    printf("1111 -> 2222:\n");
    borRandMTReseed(r, 2222);
    test(r);

    printf("2222 -> auto:\n");
    borRandMTReseedAuto(r);
    test(r);
    borRandMTDel(r);

    return 0;
}
