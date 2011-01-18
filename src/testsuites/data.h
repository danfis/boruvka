#ifndef _DATA_H_
#define _DATA_H_

#include <fermat/vec2.h>
#include <fermat/vec3.h>
#include <fermat/vec4.h>
#include <fermat/quat.h>
#include <fermat/mat3.h>
#include <fermat/mat4.h>

extern fer_vec2_t vecs2[];
extern size_t vecs2_len;

extern fer_vec3_t vecs[];
extern size_t vecs_len;

extern fer_quat_t quats[];
extern size_t quats_len;

extern fer_vec4_t vecs4[];
extern size_t vecs4_len;

extern fer_mat3_t mat3s[];
extern size_t mat3s_len;

extern fer_mat4_t mat4s[];
extern size_t mat4s_len;
#endif
