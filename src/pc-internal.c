/***
 * Boruvka
 * --------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
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

#include <unistd.h>
#include <sys/mman.h>
#include <boruvka/pc-internal.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>
#include <boruvka/alloc.h>

static size_t __bor_page_size = 0;

bor_pc_mem_t *borPCMemNew(size_t min_size, size_t elsize, int align)
{
    bor_pc_mem_t *m;
    void *mem, *datamem;
    size_t memsize;

    if (bor_unlikely(__bor_page_size == 0))
        __bor_page_size = getpagesize();

    // Try to estimate how many memory do we need.
    // Estimation is based od assumption that one chunk will contain at
    // least min_size points.
    memsize  = min_size * elsize;
    memsize += sizeof(bor_pc_mem_t);
    memsize /= __bor_page_size;
    memsize  = (memsize + 1) * __bor_page_size;

    // allocated memory
    mem = BOR_MALLOC(memsize);

    // set up structure, .data will point _after_ struct in memory (lets
    // assume that allocated memory is always more than size of
    // bor_pc_mem_t struct) and .size must be set according to it
    m = (bor_pc_mem_t *)mem;
    m->len = 0;
    borListInit(&m->list);

    datamem = (void *)((long)m + sizeof(bor_pc_mem_t));
    datamem = borAlign(datamem, align);
    m->data = datamem;
    m->size = ((long)mem + (long)memsize - (long)datamem) / elsize;

    //DBG("Alloc mem: %ld of size %d (%d)", (long)m, memsize, m->size);

    return m;
}

void borPCMemDel(bor_pc_mem_t *m)

{
    borListDel(&m->list);
    BOR_FREE(m);
}
