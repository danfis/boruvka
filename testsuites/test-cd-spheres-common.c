#include <stdio.h>

static FILE *transin = NULL;

static int nextTrans(fer_mat3_t *rot, fer_vec3_t *tr, int *ret)
{
    float x, y, z, w, p, r;
    static int c = 0;
    fer_vec3_t v;

    c++;
    if (!transin)
        transin = fopen("data-test-cd-spheres.trans.txt", "r");

    if (fscanf(transin, "%f %f %f %f %f %f %d", &x, &y, &z, &w, &p, &r, ret) == 7){
        ferMat3SetRot3D(rot, w, p, r);
        ferVec3Set(&v, 8.72557, 2.69257, 21.6929);
        ferMat3MulVec(tr, rot, &v);
        ferVec3Set(&v, x, y, z);
        ferVec3Add(tr, &v);
        return 0;
    }else{
        fclose(transin);
        transin = NULL;
        return -1;
    }
}
