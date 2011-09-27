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

#include <fermat/cfg.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>
#include <fermat/hfunc.h>

static uint32_t hmapHash(fer_list_t *key, void *data);
static int hmapEq(const fer_list_t *k1, const fer_list_t *k2, void *data);

static void ferCfgParamDel(fer_cfg_param_t *p);
static fer_cfg_param_t *ferCfgParamByType(const fer_cfg_t *c, const char *name,
                                          uint8_t type);


fer_cfg_t *ferCfgRead(const char *filename)
{
    fer_cfg_t *c;

    c = FER_ALLOC(fer_cfg_t);
    c->params = ferHMapNew(1023, hmapHash, hmapEq, c);

    return c;
}


void ferCfgDel(fer_cfg_t *c)
{
    fer_list_t list, *item;
    fer_cfg_param_t *p;

    ferListInit(&list);
    ferHMapGather(c->params, &list);

    while (!ferListEmpty(&list)){
        item = ferListNext(&list);
        p = FER_LIST_ENTRY(item, fer_cfg_param_t, hmap);
        ferListDel(item);
        ferCfgParamDel(p);
    }

    ferHMapDel(c->params);
    FER_FREE(c);
}

int ferCfgHaveParam(const fer_cfg_t *c, const char *name)
{
    fer_list_t *item;

    item = ferHMapGet(c->params, (fer_list_t *)name);
    return item != NULL;
}

uint8_t ferCfgParamType(const fer_cfg_t *c, const char *name)
{
    fer_list_t *item;
    fer_cfg_param_t *p;

    item = ferHMapGet(c->params, (fer_list_t *)name);
    if (!item)
        return FER_CFG_PARAM_NONE;

    p = FER_LIST_ENTRY(item, fer_cfg_param_t, hmap);
    return p->type;
}

int ferCfgParamIsArr(const fer_cfg_t *c, const char *name)
{
    return (ferCfgParamType(c, name) & FER_CFG_PARAM_ARR) == FER_CFG_PARAM_ARR;
}

int ferCfgParamIsStr(const fer_cfg_t *c, const char *name)
{
    return (ferCfgParamType(c, name) & (0xff >> 1)) == FER_CFG_PARAM_STR;
}

int ferCfgParamIsFlt(const fer_cfg_t *c, const char *name)
{
    return (ferCfgParamType(c, name) & (0xff >> 1)) == FER_CFG_PARAM_FLT;
}

int ferCfgParamIsV2(const fer_cfg_t *c, const char *name)
{
    return (ferCfgParamType(c, name) & (0xff >> 1)) == FER_CFG_PARAM_V2;
}

int ferCfgParamIsV3(const fer_cfg_t *c, const char *name)
{
    return (ferCfgParamType(c, name) & (0xff >> 1)) == FER_CFG_PARAM_V3;
}

int ferCfgParamStr(const fer_cfg_t *c, const char *name, const char **val)
{
    fer_cfg_param_str_t *v;

    v = (fer_cfg_param_str_t *)ferCfgParamByType(c, name, FER_CFG_PARAM_STR);
    if (!v)
        return -1;

    *val = v->val;
    return 0;
}

int ferCfgParamFlt(const fer_cfg_t *c, const char *name, fer_real_t *val)
{
    fer_cfg_param_flt_t *v;

    v = (fer_cfg_param_flt_t *)ferCfgParamByType(c, name, FER_CFG_PARAM_FLT);
    if (!v)
        return -1;

    *val = v->val;
    return 0;
}

int ferCfgParamV2(const fer_cfg_t *c, const char *name, fer_vec2_t *val)
{
    fer_cfg_param_v2_t *v;

    v = (fer_cfg_param_v2_t *)ferCfgParamByType(c, name, FER_CFG_PARAM_V2);
    if (!v)
        return -1;

    ferVec2Copy(val, &v->val);
    return 0;
}

int ferCfgParamV3(const fer_cfg_t *c, const char *name, fer_vec3_t *val)
{
    fer_cfg_param_v3_t *v;

    v = (fer_cfg_param_v3_t *)ferCfgParamByType(c, name, FER_CFG_PARAM_V3);
    if (!v)
        return -1;

    ferVec3Copy(val, &v->val);
    return 0;
}



static uint32_t hmapHash(fer_list_t *key, void *data)
{
    fer_cfg_param_t *p;
    p = FER_LIST_ENTRY(key, fer_cfg_param_t, hmap);
    return ferHashDJB2(p->name);
}

static int hmapEq(const fer_list_t *k1, const fer_list_t *k2, void *data)
{
    fer_cfg_param_t *p;
    const char *k;

    k = (const char *)k1;
    p = FER_LIST_ENTRY(k2, fer_cfg_param_t, hmap);
    return strcmp(k, p->name) == 0;
}

static void ferCfgParamDel(fer_cfg_param_t *p)
{
    fer_cfg_param_str_t *str;
    fer_cfg_param_arr_t *arr;
    fer_cfg_param_str_arr_t *str_arr;
    size_t i;

    if ((p->type & (0xff >> 1)) == FER_CFG_PARAM_STR){
        str = (fer_cfg_param_str_t *)p;
        FER_FREE(str->val);
    }else if (p->type == (FER_CFG_PARAM_ARR | FER_CFG_PARAM_STR)){
        str_arr = (fer_cfg_param_str_arr_t *)p;
        for (i = 0; i < str_arr->len; i++){
            FER_FREE(str_arr->val[i]);
        }
        FER_FREE(str_arr->val);
    }else if ((p->type & FER_CFG_PARAM_ARR) == FER_CFG_PARAM_ARR){
        arr = (fer_cfg_param_arr_t *)p;
        FER_FREE(arr->val);
    }

    FER_FREE(p->name);
    FER_FREE(p);
}

static fer_cfg_param_t *ferCfgParamByType(const fer_cfg_t *c, const char *name,
                                          uint8_t type)
{
    fer_list_t *item;
    fer_cfg_param_t *p;

    item = ferHMapGet(c->params, (fer_list_t *)name);
    if (!item)
        return NULL;

    p = FER_LIST_ENTRY(item, fer_cfg_param_t, hmap);
    if (p->type != FER_CFG_PARAM_STR)
        return NULL;

    return p;
}
