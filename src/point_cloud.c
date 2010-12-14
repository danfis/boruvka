#include <mg/point_cloud.h>
#include <mg/alloc.h>

/**
 * Updates given AABB using given point.
 */
_mg_inline void mgPCAABBUpdate(mg_real_t *aabb, mg_real_t x, mg_real_t y, mg_real_t z);

mg_pc_t *mgPCNew(void)
{
    mg_pc_t *pc;

    pc = MG_ALLOC(mg_pc_t);
    mgListInit(&pc->head);
    pc->len = 0;
    pc->aabb[0] = pc->aabb[2] = pc->aabb[4] = MG_REAL_MAX;
    pc->aabb[1] = pc->aabb[3] = pc->aabb[5] = MG_REAL_MIN;
    

    return pc;
}

void mgPCDel(mg_pc_t *pc)
{
    mg_list_t *item, *tmp;
    mg_pc_mem_t *mem;

    mgListForEachSafe(&pc->head, item, tmp){
        mem = mgListEntry(item, mg_pc_mem_t, list);
        mgPCMemDel(mem);
    }

    free(pc);
}

void mgPCAddCoords(mg_pc_t *pc, mg_real_t x, mg_real_t y, mg_real_t z)
{
    mg_list_t *item;
    mg_pc_mem_t *mem;

    item = mgListPrev(&pc->head);
    mem = mgListEntry(item, mg_pc_mem_t, list);
    if (mgListEmpty(&pc->head) || mgPCMemFull(mem)){
        mem = mgPCMemNew(10000); // TODO
        mgListAppend(&pc->head, &mem->list);
    }

    mgPCMemAddCoords(mem, x, y, z);
    mgPCAABBUpdate(pc->aabb, x, y, z);
    pc->len++;
}

_mg_inline void mgPCAABBUpdate(mg_real_t *aabb, mg_real_t x, mg_real_t y, mg_real_t z)
{
    if (aabb[0] > x)
        aabb[0] = x;
    if (aabb[1] < x)
        aabb[1] = x;
    if (aabb[2] > y)
        aabb[2] = y;
    if (aabb[3] < y)
        aabb[3] = y;
    if (aabb[4] > z)
        aabb[4] = z;
    if (aabb[5] < z)
        aabb[5] = z;
}
