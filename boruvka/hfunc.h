/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_HFUNC_H__
#define __BOR_HFUNC_H__

#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * HFunc - Hash Functions
 * =======================
 */


/**
 * Robert Jenkins hash function.
 *
 * Taken from http://burtleburtle.net/bob/c/lookup3.c.
 */
uint32_t borHashJenkins(const uint32_t *k, size_t length, uint32_t initval);

/**
 * Hash function by Dan Bernstein
 */
uint32_t borHashDJB2(const char *str);

/*
 * This algorithm was created for sdbm (a public-domain reimplementation of
 * ndbm) database library. it was found to do well in scrambling bits,
 * causing better distribution of the keys and fewer splits.
 * It also happens to be a good general hashing function with good distribution.
 * The actual function is hash(i) = hash(i - 1) * 65599 + str[i]; what is
 * included below is the faster version used in gawk. [there is even a
 * faster, duff-device version]
 * The magic constant 65599 was picked out of thin air while experimenting
 * with different constants, and turns out to be a prime. This is one of
 * the algorithms used in berkeley db (see sleepycat) and elsewhere.
 */
uint32_t borHashSDBM(const char *str);


/**
 * Computes fnv-1a 32-bit hash for the buffer of the given size.
 * Taken from www.isthe.com/chongo/src/fnv/hash_32a.c.
 */
uint32_t borFnv1a_32(const void *buf, size_t size);

/**
 * Computes fnv-1a 64-bit hash for the buffer of the given size.
 * Taken from www.isthe.com/chongo/src/fnv/hash_64a.c.
 */
uint64_t borFnv1a_64(const void *buf, size_t size);

/**
 * Murmur3 32-bit hash function.
 * Taken from https://github.com/PeterScott/murmur3.
 */
uint32_t borMurmur3_32(const void *buf, size_t size);

/**
 * CityHash 32-bit hash function.
 * Taken from https://code.google.com/p/cityhash.
 */
uint32_t borCityHash_32(const void *buf, size_t size);

/**
 * CityHash 64-bit hash function.
 * Taken from https://code.google.com/p/cityhash.
 */
uint64_t borCityHash_64(const void *buf, size_t size);

/**
 * FastHash hash function.
 * Taken from https://code.google.com/p/fast-hash.
 */
uint64_t borFastHash_64(const void *buf, size_t size, uint64_t seed);

/**
 * FastHash hash function.
 * Taken from https://code.google.com/p/fast-hash.
 */
uint32_t borFastHash_32(const void *buf, size_t size, uint32_t seed);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_HFUNC_H__ */

