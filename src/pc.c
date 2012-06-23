/***
 * Boruvka
 * --------
 * Copyright (c)2010-2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
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
#include <boruvka/pc.h>
#include <boruvka/parse.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>


bor_pc_t *borPCNew(size_t dim)
{
    return borPCNew2(dim, BOR_PC_MIN_CHUNK_SIZE);
}

bor_pc_t *borPCNew2(size_t dim, size_t min_chunk_size)
{
    bor_pc_t *pc;

    pc = BOR_ALLOC(bor_pc_t);
    pc->dim = dim;
    borListInit(&pc->head);
    pc->len = 0;

    pc->min_chunk_size = min_chunk_size;

    pc->rand = NULL;

    return pc;

}

void borPCDel(bor_pc_t *pc)
{
    bor_list_t *item, *tmp;
    bor_pc_mem_t *mem;

    BOR_LIST_FOR_EACH_SAFE(&pc->head, item, tmp){
        mem = BOR_LIST_ENTRY(item, bor_pc_mem_t, list);
        borPCMemDel(mem);
    }

    if (pc->rand)
        borRandMTDel(pc->rand);

    BOR_FREE(pc);
}

void borPCAdd(bor_pc_t *pc, const bor_vec_t *v)
{
    bor_list_t *item;
    bor_pc_mem_t *mem;

    item = borListPrev(&pc->head);
    mem = BOR_LIST_ENTRY(item, bor_pc_mem_t, list);
    if (borListEmpty(&pc->head) || borPCMemFull(mem)){
#ifdef BOR_SSE
        mem = borPCMemNew(pc->min_chunk_size, sizeof(bor_vec_t) * pc->dim, 16);
#else /* BOR_SSE */
        mem = borPCMemNew(pc->min_chunk_size, sizeof(bor_vec_t) * pc->dim, 0);
#endif /* BOR_SSE */
        borListAppend(&pc->head, &mem->list);
    }

    borPCMemAdd2Memcpy(mem, v, bor_vec_t, sizeof(bor_vec_t) * pc->dim);
    // TODO: AABBUpdate(pc->aabb, v);
    pc->len++;
}

bor_vec_t *borPCGet(bor_pc_t *pc, size_t n)
{
    bor_list_t *item;
    bor_pc_mem_t *mem;
    size_t pos;
    bor_vec_t *p = NULL;

    if (n >= pc->len)
        return NULL;

    pos = 0;
    BOR_LIST_FOR_EACH(&pc->head, item){
        mem = BOR_LIST_ENTRY(item, bor_pc_mem_t, list);
        if (pos + mem->len > n){
            p = borPCMemGet2(mem, n - pos, bor_vec_t, sizeof(bor_vec_t) * pc->dim);
            break;
        }
        pos += mem->len;
    }

    return p;
}

/** Returns (via other and other_pos) memory chunk and position within it
 *  randomly chosen from point cloud from range starting at position from
 *  of mem chunk mem_from. */
static void borPCPermutateOther(bor_pc_mem_t *mem_from, size_t from,
                                size_t len, bor_rand_mt_t *rand,
                                bor_pc_mem_t **other, size_t *other_pos)
{
    size_t pos;
    bor_pc_mem_t *mem;
    bor_list_t *item;

    mem = mem_from;

    // choose position
    pos = borRandMT(rand, (bor_real_t)from, (bor_real_t)len);

    // find correct mem chunk
    while (pos >= mem->len){
        pos -= mem->len;
        item = borListNext(&mem->list);
        mem = BOR_LIST_ENTRY(item, bor_pc_mem_t, list);
    }

    *other = mem;
    *other_pos = pos;
}

void borPCPermutate(bor_pc_t *pc)
{
    bor_list_t *item;
    bor_pc_mem_t *cur_mem, *other_mem;
    size_t cur_pos, pc_len, other_pos;
    bor_vec_t *v, *cur, *other;

    if (!pc->rand){
        pc->rand = borRandMTNewAuto();
    }

    pc_len = pc->len;

    v = borVecNew(pc->dim);

    // iterate over all positions in all chunks consequently from beginning
    // and choose some point from rest of point cloud (from positions _after_
    // the current one) and swap points
    BOR_LIST_FOR_EACH(&pc->head, item){
        cur_mem = BOR_LIST_ENTRY(item, bor_pc_mem_t, list);
        for (cur_pos = 0; cur_pos < cur_mem->len && pc_len - cur_pos > 1; cur_pos++){
            // choose other point for swapping
            borPCPermutateOther(cur_mem, cur_pos + 1, pc_len, pc->rand,
                                &other_mem, &other_pos);

            // swap points
            cur   = borPCMemGet2(cur_mem, cur_pos, bor_vec_t, sizeof(bor_vec_t) * pc->dim);
            other = borPCMemGet2(other_mem, other_pos, bor_vec_t, sizeof(bor_vec_t) * pc->dim);
            borVecCopy(pc->dim, v, other);
            borVecCopy(pc->dim, other, cur);
            borVecCopy(pc->dim, cur, v);
        }

        // length must be decreased because borPC`N`PermutateOther takes
        // length which is relative to first mem chunk
        pc_len -= cur_mem->len;
    }

    borVecDel(v);
}


size_t borPCAddFromFile(bor_pc_t *pc, const char *filename)
{
    int fd;
    size_t size;
    struct stat st;
    void *file;
    char *fstr, *fend, *fnext;
    bor_vec_t *v;
    bor_real_t val;
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

    v = borVecNew(pc->dim);

    // set up char pointers to current char (fstr) and to end of memory (fend)
    i = 0;
    fstr = (char *)file;
    fend = (char *)file + size;
    while (borParseReal(fstr, fend, &val, &fnext) == 0){
        fstr = fnext;

        borVecSet(v, i, val);
        i++;

        if (i == pc->dim){
            borPCAdd(pc, v);
            added++;
            i = 0;
        }
    }

    borVecDel(v);

    // unmap mapped memory
    munmap(file, size);

    // close file
    close(fd);

    return added;
}

void borPCAABB(const bor_pc_t *pc, bor_real_t *aabb)
{
    size_t i;
    bor_pc_it_t it;
    const bor_vec_t *v;

    for (i = 0; i < pc->dim; i++){
        aabb[2 * i] = BOR_REAL_MAX;
        aabb[2 * i + 1] = -BOR_REAL_MAX;
    }

    borPCItInit(&it, (bor_pc_t *)pc);
    while (!borPCItEnd(&it)){
        v = borPCItGet(&it);
        for (i = 0; i < pc->dim; i++){
            if (borVecGet(v, i) < aabb[2 * i])
                aabb[2 * i] = borVecGet(v, i);
            if (borVecGet(v, i) > aabb[2 * i + 1])
                aabb[2 * i + 1] = borVecGet(v, i);
        }

        borPCItNext(&it);
    }
}
