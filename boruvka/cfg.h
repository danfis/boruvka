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

#ifndef __FER_CFG_H__
#define __FER_CFG_H__

#include <boruvka/hmap.h>
#include <boruvka/vec2.h>
#include <boruvka/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Config Files
 * =============
 *
 * See bor_cfg_t.
 *
 * Example config file:
 * ---------------------
 * ~~~~~
 * # comment
 * str = 'some string'  # comment
 * f   = 10.3e10
 *
 * # with specified types: float, string, 2-D vector and 3-D vector
 * # respectively
 * val1:f  = 1.2e-10
 * val2:s  = 'a123g'
 * vec2:v2 = 10 12.1
 * vec3:v3 = 10 12.1 11.1
 * vint:i  = 12
 *
 * # arrays
 * flts:f[4] = 1 2 3 4
 * vecs2:v2[2] =
 *      10 12.1
 *      11 13.1
 * vecs3:v3[3] =
 *      10 12.1 1.1
 *      1 1.1 2.e-2
 *      3 1.4 9.e3
 * ~~~~~
 *
 *
 */

struct _bor_cfg_t {
    bor_hmap_t *params;
};
typedef struct _bor_cfg_t bor_cfg_t;

/**
 * Parameter Types
 * ----------------
 */

/** vvvv */
#define FER_CFG_PARAM_NONE 0x00
#define FER_CFG_PARAM_STR 0x01
#define FER_CFG_PARAM_INT 0x02
#define FER_CFG_PARAM_FLT 0x03
#define FER_CFG_PARAM_V2  0x04
#define FER_CFG_PARAM_V3  0x05
#define FER_CFG_PARAM_ARR 0x80
/** ^^^^ */

/**
 * Functions
 * ----------
 */

/**
 * Reads config file
 */
bor_cfg_t *ferCfgRead(const char *filename);

/**
 * Free allocated memory
 */
void ferCfgDel(bor_cfg_t *c);

/**
 * Returns true if specified parameter is available
 */
int ferCfgHaveParam(const bor_cfg_t *c, const char *name);

/**
 * Returns type of param or FER_CFG_PARAM_NONE if no such parameter exists.
 */
uint8_t ferCfgParamType(const bor_cfg_t *c, const char *name);

/**
 * Returns true if the param is an array.
 */
int ferCfgParamIsArr(const bor_cfg_t *c, const char *name);

/**
 * Returns true if the param is a string.
 */
int ferCfgParamIsStr(const bor_cfg_t *c, const char *name);

/**
 * Returns true if the param is a floating point number.
 */
int ferCfgParamIsFlt(const bor_cfg_t *c, const char *name);

/**
 * Returns true if the param is a integer.
 */
int ferCfgParamIsInt(const bor_cfg_t *c, const char *name);

/**
 * Returns true if the param is 2-D vector (vec2).
 */
int ferCfgParamIsV2(const bor_cfg_t *c, const char *name);

/**
 * Returns true if the param is 3-D vector (vec3).
 */
int ferCfgParamIsV3(const bor_cfg_t *c, const char *name);

/**
 * Returns (via {val}) a value of speficied parameter.
 * Return 0 on success.
 */
int ferCfgParamStr(const bor_cfg_t *c, const char *name, const char **val);

/**
 * Returns (via {val}) a value of speficied parameter.
 * Return 0 on success.
 */
int ferCfgParamFlt(const bor_cfg_t *c, const char *name, bor_real_t *val);

/**
 * Returns (via {val}) a value of speficied parameter.
 * Return 0 on success.
 */
int ferCfgParamInt(const bor_cfg_t *c, const char *name, int *val);

/**
 * Returns (via {val}) a value of speficied parameter.
 * Return 0 on success.
 */
int ferCfgParamV2(const bor_cfg_t *c, const char *name, bor_vec2_t *val);

/**
 * Returns (via {val}) a value of speficied parameter.
 * Return 0 on success.
 */
int ferCfgParamV3(const bor_cfg_t *c, const char *name, bor_vec3_t *val);

/**
 * Returns (via {val} and {len}) a value of speficied parameter.
 * Return 0 on success.
 */
int ferCfgParamStrArr(const bor_cfg_t *c, const char *name,
                      char ***val, size_t *len);

/**
 * Returns (via {val} and {len}) a value of speficied parameter.
 * Return 0 on success.
 */
int ferCfgParamFltArr(const bor_cfg_t *c, const char *name,
                      const bor_real_t **val, size_t *len);

/**
 * Returns (via {val} and {len}) a value of speficied parameter.
 * Return 0 on success.
 */
int ferCfgParamIntArr(const bor_cfg_t *c, const char *name,
                      const int **val, size_t *len);

/**
 * Returns (via {val} and {len}) a value of speficied parameter.
 * Return 0 on success.
 */
int ferCfgParamV2Arr(const bor_cfg_t *c, const char *name,
                     const bor_vec2_t **val, size_t *len);

/**
 * Returns (via {val} and {len}) a value of speficied parameter.
 * Return 0 on success.
 */
int ferCfgParamV3Arr(const bor_cfg_t *c, const char *name,
                     const bor_vec3_t **val, size_t *len);


/**
 * Scans config file according to {format}, which consists of parameter
 * speficiers delimited by spaces.
 * Returns 0 on success.
 *
 * Parameter specifier has following form:
 *     name_of_param:type
 * where type can be:
 *     1. f, i, s, v2 or v3 for single value parameters
 *     2. f[], i[], s[], v2[] or v3[] for array pointers
 *     3. f#, i#, s#, v2# or v3# for array lengths of type size_t
 *
 * Example 1:
 * ~~~~~~~~
 * struct st_t {
 *     bor_real_t x, y;
 *     bor_vec2_t p;
 * };
 * ...
 * struct st_t data;
 * ferCfgScan(cfg, "xpos:f ypos:f point:v2", &data.x, &data.y, &data.p);
 * ~~~~~~~~~
 * 
 * Example 2:
 * ~~~~~~~~
 * struct st_t {
 *     bor_real_t x;
 *     const bor_real_t *fs;
 *     size_t fs_len;
 * };
 * ...
 * struct st_t data;
 * ferCfgScan(cfg, "xpos:f flts:f[] flts:f#", &data.x, &data.fs, &data.fs_len);
 * ~~~~~~~~~
 */
int ferCfgScan(const bor_cfg_t *c, const char *format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CFG_H__ */
