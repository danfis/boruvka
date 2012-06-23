#ifndef _DATA_H_
#define _DATA_H_

#include <boruvka/vec2.h>
#include <boruvka/vec3.h>
#include <boruvka/vec4.h>
#include <boruvka/quat.h>
#include <boruvka/mat3.h>
#include <boruvka/mat4.h>

extern bor_vec2_t vecs2[];
extern size_t vecs2_len;

extern bor_vec3_t vecs[];
extern size_t vecs_len;

extern bor_quat_t quats[];
extern size_t quats_len;

extern bor_vec4_t vecs4[];
extern size_t vecs4_len;

extern bor_mat3_t mat3s[];
extern size_t mat3s_len;

extern bor_mat4_t mat4s[];
extern size_t mat4s_len;

extern bor_vec3_t *bunny_coords;
extern size_t bunny_coords_len;
extern unsigned int *bunny_ids;
extern size_t bunny_ids_len;
extern size_t bunny_tri_len;

void testBunnyDumpSVT(FILE *out, const char *name);

#endif
