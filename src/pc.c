/***
 * fermat
 * -------
 * Copyright (c)2010-2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <fermat/pc.h>
#include <fermat/parse.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>


fer_pc_t *ferPCNew(size_t dim)
{
    return ferPCNew2(dim, FER_PC_MIN_CHUNK_SIZE);
}

fer_pc_t *ferPCNew2(size_t dim, size_t min_chunk_size)
{
    fer_pc_t *pc;

    pc = FER_ALLOC(fer_pc_t);
    pc->dim = dim;
    ferListInit(&pc->head);
    pc->len = 0;

    pc->min_chunk_size = min_chunk_size;

    pc->rand = NULL;

    return pc;

}

void ferPCDel(fer_pc_t *pc)
{
    fer_list_t *item, *tmp;
    fer_pc_mem_t *mem;

    FER_LIST_FOR_EACH_SAFE(&pc->head, item, tmp){
        mem = FER_LIST_ENTRY(item, fer_pc_mem_t, list);
        ferPCMemDel(mem);
    }

    if (pc->rand)
        ferRandMTDel(pc->rand);

    FER_FREE(pc);
}

void ferPCAdd(fer_pc_t *pc, const fer_vec_t *v)
{
    fer_list_t *item;
    fer_pc_mem_t *mem;

    item = ferListPrev(&pc->head);
    mem = FER_LIST_ENTRY(item, fer_pc_mem_t, list);
    if (ferListEmpty(&pc->head) || ferPCMemFull(mem)){
#ifdef FER_SSE
        mem = ferPCMemNew(pc->min_chunk_size, sizeof(fer_vec_t) * pc->dim, 16);
#else /* FER_SSE */
        mem = ferPCMemNew(pc->min_chunk_size, sizeof(fer_vec_t) * pc->dim, 0);
#endif /* FER_SSE */
        ferListAppend(&pc->head, &mem->list);
    }

    ferPCMemAdd2Memcpy(mem, v, fer_vec_t, sizeof(fer_vec_t) * pc->dim);
    // TODO: AABBUpdate(pc->aabb, v);
    pc->len++;
}

fer_vec_t *ferPCGet(fer_pc_t *pc, size_t n)
{
    fer_list_t *item;
    fer_pc_mem_t *mem;
    size_t pos;
    fer_vec_t *p = NULL;

    if (n >= pc->len)
        return NULL;

    pos = 0;
    FER_LIST_FOR_EACH(&pc->head, item){
        mem = FER_LIST_ENTRY(item, fer_pc_mem_t, list);
        if (pos + mem->len > n){
            p = ferPCMemGet2(mem, n - pos, fer_vec_t, sizeof(fer_vec_t) * pc->dim);
            break;
        }
        pos += mem->len;
    }

    return p;
}

/** Returns (via other and other_pos) memory chunk and position within it
 *  randomly chosen from point cloud from range starting at position from
 *  of mem chunk mem_from. */
static void ferPCPermutateOther(fer_pc_mem_t *mem_from, size_t from,
                                size_t len, fer_rand_mt_t *rand,
                                fer_pc_mem_t **other, size_t *other_pos)
{
    size_t pos;
    fer_pc_mem_t *mem;
    fer_list_t *item;

    mem = mem_from;

    // choose position
    pos = ferRandMT(rand, (fer_real_t)from, (fer_real_t)len);

    // find correct mem chunk
    while (pos >= mem->len){
        pos -= mem->len;
        item = ferListNext(&mem->list);
        mem = FER_LIST_ENTRY(item, fer_pc_mem_t, list);
    }

    *other = mem;
    *other_pos = pos;
}

void ferPCPermutate(fer_pc_t *pc)
{
    fer_list_t *item;
    fer_pc_mem_t *cur_mem, *other_mem;
    size_t cur_pos, pc_len, other_pos;
    fer_vec_t *v, *cur, *other;

    if (!pc->rand){
        pc->rand = ferRandMTNewAuto();
    }

    pc_len = pc->len;

    v = ferVecNew(pc->dim);

    // iterate over all positions in all chunks consequently from beginning
    // and choose some point from rest of point cloud (from positions _after_
    // the current one) and swap points
    FER_LIST_FOR_EACH(&pc->head, item){
        cur_mem = FER_LIST_ENTRY(item, fer_pc_mem_t, list);
        for (cur_pos = 0; cur_pos < cur_mem->len && pc_len - cur_pos > 1; cur_pos++){
            // choose other point for swapping
            ferPCPermutateOther(cur_mem, cur_pos + 1, pc_len, pc->rand,
                                &other_mem, &other_pos);

            // swap points
            cur   = ferPCMemGet2(cur_mem, cur_pos, fer_vec_t, sizeof(fer_vec_t) * pc->dim);
            other = ferPCMemGet2(other_mem, other_pos, fer_vec_t, sizeof(fer_vec_t) * pc->dim);
            ferVecCopy(pc->dim, v, other);
            ferVecCopy(pc->dim, other, cur);
            ferVecCopy(pc->dim, cur, v);
        }

        // length must be decreased because ferPC`N`PermutateOther takes
        // length which is relative to first mem chunk
        pc_len -= cur_mem->len;
    }

    ferVecDel(v);
}


size_t ferPCAddFromFile(fer_pc_t *pc, const char *filename)
{
    int fd;
    size_t size;
    struct stat st;
    void *file;
    char *fstr, *fend, *fnext;
    fer_vec_t *v;
    fer_real_t val;
    size_t i, added = 0;

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

    v = ferVecNew(pc->dim);

    // set up char pointers to current char (fstr) and to end of memory (fend)
    i = 0;
    fstr = (char *)file;
    fend = (char *)file + size;
    while (ferParseReal(fstr, fend, &val, &fnext) == 0){
        fstr = fnext;

        ferVecSet(v, i, val);
        i++;

        if (i == pc->dim){
            ferPCAdd(pc, v);
            added++;
            i = 0;
        }
    }

    ferVecDel(v);

    // unmap mapped memory
    munmap(file, size);

    // close file
    close(fd);

    return added;
}

void ferPCAABB(const fer_pc_t *pc, fer_real_t *aabb)
{
    size_t i;
    fer_pc_it_t it;
    const fer_vec_t *v;

    for (i = 0; i < pc->dim; i++){
        aabb[2 * i] = FER_REAL_MAX;
        aabb[2 * i + 1] = -FER_REAL_MAX;
    }

    ferPCItInit(&it, (fer_pc_t *)pc);
    while (!ferPCItEnd(&it)){
        v = ferPCItGet(&it);
        for (i = 0; i < pc->dim; i++){
            if (ferVecGet(v, i) < aabb[2 * i])
                aabb[2 * i] = ferVecGet(v, i);
            if (ferVecGet(v, i) > aabb[2 * i + 1])
                aabb[2 * i + 1] = ferVecGet(v, i);
        }

        ferPCItNext(&it);
    }
}
