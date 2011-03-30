#include <fermat/opencl.h>
#include <fermat/timer.h>
#include <fermat/surf-matching.h>

#include "surfs.c"


int main(int argc, char *argv[])
{
    size_t i;
    fer_surf_match_t *sm, *sml;
    fer_timer_t timer;
    int n1, n2, nl1, nl2, len1, len2;
    fer_real_t d1, d2, dl1, dl2;

    sm = ferSurfMatchNew(3000, 3000);
    sml = ferSurfMatchNew(3000, 3000);

    for (i = 0; i < sizeof(surf1) / sizeof(float) / 64; i++){
        ferSurfMatchSet1(sm, i, surf1 + (i * 64));
        ferSurfMatchSet1(sml, i, surf1 + (i * 64));
    }
    for (i = 0; i < sizeof(surf3) / sizeof(float) / 64; i++){
        ferSurfMatchSet2(sm, i, surf3 + (i * 64));
        ferSurfMatchSet2(sml, i, surf3 + (i * 64));
    }

    len1 = sizeof(surf1) / sizeof(float) / 64;
    len2 = sizeof(surf2) / sizeof(float) / 64;
    //len1 = 20;
    //len2 = 20;

    ferTimerStart(&timer);
    ferSurfMatchLinear(sml, len1, len2);
    ferTimerStop(&timer);
    printf("Linear: %lu us\n", ferTimerElapsedInUs(&timer));

    /*
    for (i = 0; i < len1; i++){
        ferSurfMatchNearest(sml, i, &n1, &n2, &d1, &d2);
        printf("[%04d] - % 4d (%2.3f), % 4d (%2.3f)\n", i, n1, d1, n2, d2);
    }
    */

    ferTimerStart(&timer);
    ferSurfMatch(sm, len1, len2);
    ferTimerStop(&timer);
    printf("OpenCL: %lu us\n", ferTimerElapsedInUs(&timer));

    /*
    for (i = 0; i < len1; i++){
        ferSurfMatchNearest(sm, i, &n1, &n2, &d1, &d2);
        printf("[%04d] - % 4d (%2.3f), % 4d (%2.3f)\n", i, n1, d1, n2, d2);
    }
    */

    // check
    for (i = 0; i < len1; i++){
        ferSurfMatchNearest(sm, i, &n1, &n2, &d1, &d2);
        ferSurfMatchNearest(sml, i, &nl1, &nl2, &dl1, &dl2);

        if (n1 != nl1 || n2 != nl2){
            printf("diff opencl [%04d] - % 4d (%2.3f), % 4d (%2.3f)\n", i, n1, d1, n2, d2);
            printf("diff linear [%04d] - % 4d (%2.3f), % 4d (%2.3f)\n", i, nl1, dl1, nl2, dl2);
        }
    }

    ferSurfMatchDel(sm);
    ferSurfMatchDel(sml);

    return 0;
}
