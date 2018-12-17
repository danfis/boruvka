#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/hfunc.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

#include "data.h"

static void loadVec3(float *v, int i)
{
    v[0] = borVec3X(&vecs[i]);
    v[1] = borVec3Y(&vecs[i]);
    v[2] = borVec3Z(&vecs[i]);
}

static uint32_t hash32(int vi, uint32_t (*hfunc)(const void *, size_t))
{
    float v[3];
    loadVec3(v, vi);
    return hfunc(v, sizeof(float) * 3);
}

static uint64_t hash64(int vi, uint64_t (*hfunc)(const void *, size_t))
{
    float v[3];
    loadVec3(v, vi);
    return hfunc(v, sizeof(float) * 3);
}

TEST(hfuncFnv)
{
    size_t i;
    uint32_t val32;
    uint64_t val64;

    for (i = 0; i < 1000; ++i){
        val32 = hash32(i, borFnv1a_32);
        printf("Fnv1a32: %lu %llu\n",
               (unsigned long)i,
               (unsigned long long)val32);

        val64 = hash64(i, borFnv1a_64);
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
        val32 = hash32(i, borMurmur3_32);
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
        val32 = hash32(i, borCityHash_32);
        printf("CityHash32: %lu %llu\n",
               (unsigned long)i,
               (unsigned long long)val32);

        val64 = hash64(i, borCityHash_64);
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
    float v[3];

    for (i = 0; i < 1000; ++i){
        loadVec3(v, i);
        val32 = borFastHash_32(v, sizeof(float) * 3, 111);
        printf("FastHash32: %lu %llu\n",
               (unsigned long)i,
               (unsigned long long)val32);

        val64 = borFastHash_64(v, sizeof(float) * 3, 111);
        printf("FastHash64: %lu %llu\n",
               (unsigned long)i,
               (unsigned long long)val64);
    }
}
