#include <unistd.h>
#include <mg/point_cloud_internal.h>
#include <mg/alloc.h>

static size_t __mg_page_size = 0;

mg_pc_mem_t *mgPCMemNew(size_t min_size)
{
    mg_pc_mem_t *m;
    void *mem;
    size_t memsize;

    if (mg_unlikely(__mg_page_size == 0))
        __mg_page_size = getpagesize();

    // Try to estimate how many memory do we need.
    // Estimation is based od assumption that one chunk will contain at
    // least min_size points.
    memsize  = min_size * sizeof(mg_vec3_t);
    memsize += sizeof(mg_pc_mem_t);
    memsize /= __mg_page_size;
    memsize  = (memsize + 1) * __mg_page_size;

    // allocated memory (TODO: use mmap??)
    mem = malloc(memsize);

    // set up structure .data will point _after_ struct in memory (lets
    // assume that allocated memory is always more than size of
    // mg_pc_mem_t struct) and .size must be set according to it
    m = (mg_pc_mem_t *)mem;
    m->data = (mg_vec3_t *)(((long)m) + sizeof(mg_pc_mem_t));
    m->size = ((long)memsize - sizeof(mg_pc_mem_t)) / sizeof(mg_vec3_t);
    m->len = 0;
    mgListInit(&m->list);

    //DBG("Alloc mem: %ld of size %d (%d)", (long)m, memsize, m->size);

    return m;
}

void mgPCMemDel(mg_pc_mem_t *m)

{
    mgListDel(&m->list);
    free(m);
}
