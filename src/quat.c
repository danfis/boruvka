#include <mg/quat.h>
#include <mg/alloc.h>

mg_quat_t *mgQuatNew(mg_real_t x, mg_real_t y, mg_real_t z, mg_real_t w)
{
    mg_quat_t *q;

    q = MG_ALLOC(mg_quat_t);
    mgQuatSet(q, x, y, z, w);

    return q;
}

void mgQuatDel(mg_quat_t *q)
{
    free(q);
}
