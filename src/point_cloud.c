#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <mg/point_cloud.h>
#include <mg/rand.h>
#include <mg/parse.h>
#include <mg/alloc.h>
#include <mg/dbg.h>

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

    pc->min_chunk_size = MG_PC_MIN_CHUNK_SIZE;
    

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
        mem = mgPCMemNew(pc->min_chunk_size);
        mgListAppend(&pc->head, &mem->list);
    }

    mgPCMemAddCoords(mem, x, y, z);
    mgPCAABBUpdate(pc->aabb, x, y, z);
    pc->len++;
}

mg_vec3_t *mgPCGet(mg_pc_t *pc, size_t n)
{
    mg_list_t *item;
    mg_pc_mem_t *mem;
    size_t pos;
    mg_vec3_t *p = NULL;

    if (n >= pc->len)
        return NULL;

    pos = 0;
    mgListForEach(&pc->head, item){
        mem = mgListEntry(item, mg_pc_mem_t, list);
        if (pos + mem->len > n){
            p = mgPCMemGet(mem, n - pos);
            break;
        }
        pos += mem->len;
    }

    return p;
}

/** Returns (via other and other_pos) memory chunk and position within it
 *  randomly chosen from point cloud from range starting at position from
 *  of mem chunk mem_from. */
static void mgPCPermutateOther(mg_pc_mem_t *mem_from, size_t from,
                               size_t len, mg_rand_t *rand,
                               mg_pc_mem_t **other, size_t *other_pos)
{
    size_t pos;
    mg_pc_mem_t *mem;
    mg_list_t *item;

    mem = mem_from;

    // choose position
    pos = mgRand(rand, (mg_real_t)from, (mg_real_t)len);

    // find correct mem chunk
    while (pos >= mem->len){
        pos -= mem->len;
        item = mgListNext(&mem->list);
        mem = mgListEntry(item, mg_pc_mem_t, list);
    }

    *other = mem;
    *other_pos = pos;
}

void mgPCPermutate(mg_pc_t *pc)
{
    mg_list_t *item;
    mg_pc_mem_t *cur_mem, *other_mem;
    size_t cur_pos, pc_len, other_pos;
    mg_rand_t rand;
    mg_vec3_t v, *cur, *other;

    mgRandInit(&rand);
    pc_len = pc->len;

    // iterate over all positions in all chunks consequently from beginning
    // and choose some point from rest of point cloud (from positions _after_
    // the current one) and swap points
    mgListForEach(&pc->head, item){
        cur_mem = mgListEntry(item, mg_pc_mem_t, list);
        for (cur_pos = 0; cur_pos < cur_mem->len && pc_len - cur_pos > 1; cur_pos++){
            // choose other point for swapping
            mgPCPermutateOther(cur_mem, cur_pos + 1, pc_len, &rand,
                               &other_mem, &other_pos);

            // swap points
            cur   = mgPCMemGet(cur_mem, cur_pos);
            other = mgPCMemGet(other_mem, other_pos);
            mgVec3Copy(&v, other);
            mgVec3Copy(other, cur);
            mgVec3Copy(cur, &v);
        }

        // length must be decreased because mgPCPermutateOther takes
        // length which is relative to first mem chunk
        pc_len -= cur_mem->len;
    }
}

size_t mgPCAddFromFile(mg_pc_t *pc, const char *filename)
{

    int fd;
    size_t size;
    struct stat st;
    void *file;
    char *fstr, *fend, *fnext;
    mg_vec3_t v;
    size_t added = 0;

    // open file
    if ((fd = open(filename, O_RDONLY)) == -1){
        ERR("Can't open file '%s'", filename);
        return added;
    }

    // get stats (mainly size of file)
    if (fstat(fd, &st) == -1){
        close(fd);
        ERR("Can't get file info of '%s'", filename);
        return added;
    }

    // pick up size of file
    size = st.st_size;

    // mmap whole file into memory, we need only read from it and don't need
    // to share anything
    file = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file == MAP_FAILED){
        close(fd);
        ERR("Can't map file '%s' into memory: %s", filename, strerror(errno));

        // Fall to stdio method if it's not possible to map whole file into
        // memory
        // TODO gsrmInsigAddFromFileSTDIO(is, filename);
        return added;
    }

    // set up char pointers to current char (fstr) and to end of memory (fend)
    fstr = (char *)file;
    fend = (char *)file + size;
    while (mgParseVec3(fstr, fend, &v, &fnext) == 0){
        mgPCAdd(pc, &v);
        added++;
        fstr = fnext;
    }

    // unmap mapped memory
    munmap(file, size);

    // close file
    close(fd);

    return added;
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
