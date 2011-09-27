/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *
 *  This file is part of fermat.
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

#include <fermat/hmap.h>
#include <fermat/vec2.h>
#include <fermat/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Config Files
 * =============
 *
 * Example config file:
 * ---------------------
 * ~~~~~
 * # comment
 * str = some string  # comment
 * f   = 10.3e10
 * vec2 = v2(10 12.1)
 * vec3 = v3(10 12.1 11.1)
 *
 * vecs2 = v2[2](
 *      10 12.1
 *      11 13.1)
 * vecs3 = v3[2](
 *      10 12.1 1.1
 *      1 1.1 2.e-2
 *      )
 * ~~~~~
 *
 *
 * See fer_cfg_t.
 */

struct _fer_cfg_t {
    fer_hmap_t *params;
};
typedef struct _fer_cfg_t fer_cfg_t;



#define FER_CFG_PARAM \
    char *name; \
    uint8_t type; \
    fer_list_t hmap
struct _fer_cfg_param_t {
    FER_CFG_PARAM;
};
typedef struct _fer_cfg_param_t fer_cfg_param_t;

struct _fer_cfg_param_str_t {
    FER_CFG_PARAM;
    char *val;
};
typedef struct _fer_cfg_param_str_t fer_cfg_param_str_t;

struct _fer_cfg_param_flt_t {
    FER_CFG_PARAM;
    fer_real_t val;
};
typedef struct _fer_cfg_param_flt_t fer_cfg_param_flt_t;

struct _fer_cfg_param_v2_t {
    FER_CFG_PARAM;
    fer_vec2_t val;
};
typedef struct _fer_cfg_param_v2_t fer_cfg_param_v2_t;

struct _fer_cfg_param_v3_t {
    FER_CFG_PARAM;
    fer_vec3_t val;
};
typedef struct _fer_cfg_param_v3_t fer_cfg_param_v3_t;


struct _fer_cfg_param_arr_t {
    FER_CFG_PARAM;
    void *val;
    size_t len;
};
typedef struct _fer_cfg_param_arr_t fer_cfg_param_arr_t;

struct _fer_cfg_param_str_arr_t {
    FER_CFG_PARAM;
    char **val;
    size_t len;
};
typedef struct _fer_cfg_param_str_arr_t fer_cfg_param_str_arr_t;

struct _fer_cfg_param_flt_arr_t {
    FER_CFG_PARAM;
    fer_real_t *val;
    size_t len;
};
typedef struct _fer_cfg_param_flt_arr_t fer_cfg_param_flt_arr_t;

struct _fer_cfg_param_v2_arr_t {
    FER_CFG_PARAM;
    fer_vec2_t *val;
    size_t len;
};
typedef struct _fer_cfg_param_v2_arr_t fer_cfg_param_v2_arr_t;

struct _fer_cfg_param_v3_arr_t {
    FER_CFG_PARAM;
    fer_vec3_t *val;
    size_t len;
};
typedef struct _fer_cfg_param_v3_arr_t fer_cfg_param_v3_arr_t;

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
fer_cfg_t *ferCfgRead(const char *filename);

/**
 * Free allocated memory
 */
void ferCfgDel(fer_cfg_t *c);

/**
 * Returns true if specified parameter is available
 */
int ferCfgHaveParam(const fer_cfg_t *c, const char *name);

/**
 * Returns type of param or FER_CFG_NONE if no such parameter exists.
 */
uint8_t ferCfgParamType(const fer_cfg_t *c, const char *name);

/**
 * Returns true if the param is an array.
 */
int ferCfgParamIsArr(const fer_cfg_t *c, const char *name);

/**
 * Returns true if the param is a string.
 */
int ferCfgParamIsStr(const fer_cfg_t *c, const char *name);

/**
 * Returns true if the param is a floating point number.
 */
int ferCfgParamIsFlt(const fer_cfg_t *c, const char *name);

/**
 * Returns true if the param is 2-D vector (vec2).
 */
int ferCfgParamIsV2(const fer_cfg_t *c, const char *name);

/**
 * Returns true if the param is 3-D vector (vec3).
 */
int ferCfgParamIsV3(const fer_cfg_t *c, const char *name);

/**
 * Returns (via {val}) a value of speficied parameter.
 * Return 0 on success.
 */
int ferCfgParamStr(const fer_cfg_t *c, const char *name, const char **val);

/**
 * Returns (via {val}) a value of speficied parameter.
 * Return 0 on success.
 */
int ferCfgParamFlt(const fer_cfg_t *c, const char *name, fer_real_t *val);

/**
 * Returns (via {val}) a value of speficied parameter.
 * Return 0 on success.
 */
int ferCfgParamV2(const fer_cfg_t *c, const char *name, fer_vec2_t *val);

/**
 * Returns (via {val}) a value of speficied parameter.
 * Return 0 on success.
 */
int ferCfgParamV3(const fer_cfg_t *c, const char *name, fer_vec3_t *val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CFG_H__ */
