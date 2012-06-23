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

static size_t __fer_page_size = 0;

fer_pc_mem_t *ferPCMemNew(size_t min_size, size_t elsize, int align)
{
    fer_pc_mem_t *m;
    void *mem, *datamem;
    size_t memsize;

    if (fer_unlikely(__fer_page_size == 0))
        __fer_page_size = getpagesize();

    // Try to estimate how many memory do we need.
    // Estimation is based od assumption that one chunk will contain at
    // least min_size points.
    memsize  = min_size * elsize;
    memsize += sizeof(fer_pc_mem_t);
    memsize /= __fer_page_size;
    memsize  = (memsize + 1) * __fer_page_size;

    // allocated memory
    mem = ferRealloc(NULL, memsize);

    // set up structure, .data will point _after_ struct in memory (lets
    // assume that allocated memory is always more than size of
    // fer_pc_mem_t struct) and .size must be set according to it
    m = (fer_pc_mem_t *)mem;
    m->len = 0;
    ferListInit(&m->list);

    datamem = (void *)((long)m + sizeof(fer_pc_mem_t));
    datamem = ferAlign(datamem, align);
    m->data = datamem;
    m->size = ((long)mem + (long)memsize - (long)datamem) / elsize;

    //DBG("Alloc mem: %ld of size %d (%d)", (long)m, memsize, m->size);

    return m;
}

void ferPCMemDel(fer_pc_mem_t *m)

{
    ferListDel(&m->list);
    FER_FREE(m);
}
