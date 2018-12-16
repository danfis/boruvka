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
        val32 = borFnv1a_32(&vecs[i], sizeof(bor_vec2_t));
        printf("Fnv1a32: %lu %llu\n",
               (unsigned long)i,
               (unsigned long long)val32);

        val64 = borFnv1a_64(&vecs[i], sizeof(bor_vec2_t));
        printf("Fnv1a64: %lu %llu\n",
               (unsigned long)i,
               (unsigned long long)val64);
    }
}

TEST(hfuncMurmur3)
{
    size_t i;
    uint32_t val32;

    for (i = 0; i < 1000; ++i){
        val32 = borMurmur3_32(&vecs[i], sizeof(bor_vec2_t));
        printf("Murmur3_32: %lu %llu\n",
               (unsigned long)i,
               (unsigned long long)val32);
    }
}

TEST(hfuncCityHash)
{
    size_t i;
    uint32_t val32;
    uint64_t val64;

    for (i = 0; i < 1000; ++i){
        val32 = borCityHash_32(&vecs[i], sizeof(bor_vec2_t));
        printf("CityHash32: %lu %llu\n",
               (unsigned long)i,
               (unsigned long long)val32);

        val64 = borCityHash_64(&vecs[i], sizeof(bor_vec2_t));
        printf("CityHash64: %lu %llu\n",
               (unsigned long)i,
               (unsigned long long)val64);
    }
}

TEST(hfuncFastHash)
{
    size_t i;
    uint32_t val32;
    uint64_t val64;

    for (i = 0; i < 1000; ++i){
        val32 = borFastHash_32(&vecs[i], sizeof(bor_vec2_t), 111);
        printf("FastHash32: %lu %llu\n",
               (unsigned long)i,
               (unsigned long long)val32);

        val64 = borFastHash_64(&vecs[i], sizeof(bor_vec2_t), 111);
        printf("FastHash64: %lu %llu\n",
               (unsigned long)i,
               (unsigned long long)val64);
    }
}
