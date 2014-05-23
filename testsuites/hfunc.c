#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/hfunc.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

#include "data.h"

TEST(hfuncFnv)
{
    size_t i;
    uint32_t val32;
    uint64_t val64;

    for (i = 0; i < 1000; ++i){
        val32 = borFnv1a32(&vecs[i], sizeof(bor_vec2_t));
        printf("Fnv1a32: %lu %llu\n",
               (unsigned long)i,
               (unsigned long long)val32);

        val64 = borFnv1a64(&vecs[i], sizeof(bor_vec2_t));
        printf("Fnv1a64: %lu %llu\n",
               (unsigned long)i,
               (unsigned long long)val64);
    }
}
