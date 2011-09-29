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
#include "src/cfg-lexer.h"
#include "src/cfg-lexer-gen.h"

struct _fer_cfg_parser_t {
    yyscan_t scanner;
    fer_cfg_lex_t val;
    int tok;
    char *name;
};
typedef struct _fer_cfg_parser_t fer_cfg_parser_t;


static uint32_t hmapHash(fer_list_t *key, void *data);
static int hmapEq(const fer_list_t *k1, const fer_list_t *k2, void *data);

static void ferCfgParamDel(fer_cfg_param_t *p);
static fer_cfg_param_t *ferCfgParamByType(const fer_cfg_t *c, const char *name,
                                          uint8_t type);
static void ferCfgParamInsert(fer_cfg_t *c, fer_cfg_param_t *p);

static void ferCfgParse(fer_cfg_t *c, fer_cfg_parser_t *parser);


/*
static void __dumpParams(fer_cfg_t *c)
{
    int i, j;
    fer_list_t *item;
    fer_cfg_param_t *p;

    for (i = 0; i < (int)c->params->size; i++){
        FER_LIST_FOR_EACH(c->params->table + i, item){
            p = FER_LIST_ENTRY(item, fer_cfg_param_t, hmap);
            fprintf(stdout, "[%04d]: `%s', %02x:", i, p->name, p->type);

            if (p->type == FER_CFG_PARAM_STR){
                fprintf(stdout, "s: `%s'", ((fer_cfg_param_str_t *)p)->val);
            }else if (p->type == FER_CFG_PARAM_FLT){
                fprintf(stdout, "f: %f", (float)((fer_cfg_param_flt_t *)p)->val);
            }else if (p->type == FER_CFG_PARAM_V2){
                fprintf(stdout, "v2: ");
                ferVec2Print(&((fer_cfg_param_v2_t *)p)->val, stdout);
            }else if (p->type == FER_CFG_PARAM_V3){
                fprintf(stdout, "v3: ");
                ferVec3Print(&((fer_cfg_param_v3_t *)p)->val, stdout);
            }else if (p->type == (FER_CFG_PARAM_STR | FER_CFG_PARAM_ARR)){
                fer_cfg_param_str_arr_t *ps = (fer_cfg_param_str_arr_t *)p;

                fprintf(stdout, "s[%d]:", ps->len);
                for (j = 0; j < ps->len; j++){
                    fprintf(stdout, " `%s'", ps->val[j]);
                }
            }else if (p->type == (FER_CFG_PARAM_FLT | FER_CFG_PARAM_ARR)){
                fer_cfg_param_flt_arr_t *pf = (fer_cfg_param_flt_arr_t *)p;

                fprintf(stdout, "f[%d]:", pf->len);
                for (j = 0; j < pf->len; j++){
                    fprintf(stdout, " %f", (float)pf->val[j]);
                }
            }else if (p->type == (FER_CFG_PARAM_V2 | FER_CFG_PARAM_ARR)){
                fer_cfg_param_v2_arr_t *pv2 = (fer_cfg_param_v2_arr_t *)p;

                fprintf(stdout, "v2[%d]:", pv2->len);
                for (j = 0; j < pv2->len; j++){
                    fprintf(stdout, " (");
                    ferVec2Print(&pv2->val[j], stdout);
                    fprintf(stdout, ")");
                }
            }else if (p->type == (FER_CFG_PARAM_V3 | FER_CFG_PARAM_ARR)){
                fer_cfg_param_v3_arr_t *pv3 = (fer_cfg_param_v3_arr_t *)p;

                fprintf(stdout, "v3[%d]:", pv3->len);
                for (j = 0; j < pv3->len; j++){
                    fprintf(stdout, " (");
                    ferVec3Print(&pv3->val[j], stdout);
                    fprintf(stdout, ")");
                }
            }
            fprintf(stdout, "\n");
        }
    }
}
*/

fer_cfg_t *ferCfgRead(const char *filename)
{
    fer_cfg_t *c;
    FILE *fin;
    fer_cfg_parser_t parser;

    // open cfg file
    fin = fopen(filename, "r");
    if (!fin){
        fprintf(stderr, "Fermat :: Cfg :: No such file: `%s'.\n", filename);
        return NULL;
    }

    c = FER_ALLOC(fer_cfg_t);
    c->params = ferHMapNew(1023, hmapHash, hmapEq, c);

    // init parser
    if (yylex_init_extra(&parser.val, &parser.scanner) != 0){
        fprintf(stderr, "Fermat :: Cfg :: Can't initialize parser!\n");
        ferCfgDel(c);
        return NULL;
    }
    yyset_in(fin, parser.scanner);

    ferCfgParse(c, &parser);

    // free parser
    yylex_destroy(parser.scanner);
    fclose(fin);

    /* __dumpParams(c); */

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
    fer_cfg_param_t q;
    q.name = (char *)name;

    item = ferHMapGet(c->params, &q.hmap);
    return item != NULL;
}

uint8_t ferCfgParamType(const fer_cfg_t *c, const char *name)
{
    fer_list_t *item;
    fer_cfg_param_t *p, q;
    q.name = (char *)name;

    item = ferHMapGet(c->params, &q.hmap);
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

int ferCfgParamStrArr(const fer_cfg_t *c, const char *name,
                      char ***val, size_t *len)
{
    fer_cfg_param_str_arr_t *v;

    v = (fer_cfg_param_str_arr_t *)ferCfgParamByType(c, name,
                                    FER_CFG_PARAM_STR | FER_CFG_PARAM_ARR);
    if (!v)
        return -1;

    *val = v->val;
    *len = v->len;
    return 0;
}

int ferCfgParamFltArr(const fer_cfg_t *c, const char *name,
                      const fer_real_t **val, size_t *len)
{
    fer_cfg_param_flt_arr_t *v;

    v = (fer_cfg_param_flt_arr_t *)ferCfgParamByType(c, name,
                                    FER_CFG_PARAM_FLT | FER_CFG_PARAM_ARR);
    if (!v)
        return -1;

    *val = v->val;
    *len = v->len;
    return 0;
}

int ferCfgParamV2Arr(const fer_cfg_t *c, const char *name,
                     const fer_vec2_t **val, size_t *len)
{
    fer_cfg_param_v2_arr_t *v;

    v = (fer_cfg_param_v2_arr_t *)ferCfgParamByType(c, name,
                                    FER_CFG_PARAM_V2 | FER_CFG_PARAM_ARR);
    if (!v)
        return -1;

    *val = v->val;
    *len = v->len;
    return 0;
}

int ferCfgParamV3Arr(const fer_cfg_t *c, const char *name,
                     const fer_vec3_t **val, size_t *len)
{
    fer_cfg_param_v3_arr_t *v;

    v = (fer_cfg_param_v3_arr_t *)ferCfgParamByType(c, name,
                                    FER_CFG_PARAM_V3 | FER_CFG_PARAM_ARR);
    if (!v)
        return -1;

    *val = v->val;
    *len = v->len;
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
    fer_cfg_param_t *p1, *p2;

    p1 = FER_LIST_ENTRY(k1, fer_cfg_param_t, hmap);
    p2 = FER_LIST_ENTRY(k2, fer_cfg_param_t, hmap);
    return strcmp(p1->name, p2->name) == 0;
}

static void ferCfgParamDel(fer_cfg_param_t *p)
{
    fer_cfg_param_str_t *str;
    fer_cfg_param_arr_t *arr;
    fer_cfg_param_str_arr_t *str_arr;
    size_t i;

    if (p->type == FER_CFG_PARAM_STR){
        str = (fer_cfg_param_str_t *)p;
        free(str->val);
    }else if (p->type == (FER_CFG_PARAM_ARR | FER_CFG_PARAM_STR)){
        str_arr = (fer_cfg_param_str_arr_t *)p;
        for (i = 0; i < str_arr->len; i++){
            free(str_arr->val[i]);
        }
        FER_FREE(str_arr->val);
    }else if ((p->type & FER_CFG_PARAM_ARR) == FER_CFG_PARAM_ARR){
        arr = (fer_cfg_param_arr_t *)p;
        FER_FREE(arr->val);
    }

    free(p->name);
    FER_FREE(p);
}

static fer_cfg_param_t *ferCfgParamByType(const fer_cfg_t *c, const char *name,
                                          uint8_t type)
{
    fer_list_t *item;
    fer_cfg_param_t *p, q;
    q.name = (char *)name;

    item = ferHMapGet(c->params, &q.hmap);
    if (!item)
        return NULL;

    p = FER_LIST_ENTRY(item, fer_cfg_param_t, hmap);
    if (p->type != type)
        return NULL;

    return p;
}

static void ferCfgParamInsert(fer_cfg_t *c, fer_cfg_param_t *p)
{
    ferHMapPut(c->params, &p->hmap);
}



#define NEXT(parser) (parser)->tok = yylex((parser)->scanner)
#define SET_NAME(param, parser) \
    (param)->name = (parser)->name; \
    (parser)->name = NULL

static void parseName(fer_cfg_t *c, fer_cfg_parser_t *parser);
static void parseStr(fer_cfg_t *c, fer_cfg_parser_t *parser);
static void parseFlt(fer_cfg_t *c, fer_cfg_parser_t *parser);
static void parseV2(fer_cfg_t *c, fer_cfg_parser_t *parser);
static void parseV3(fer_cfg_t *c, fer_cfg_parser_t *parser);
static void parseStrArr(fer_cfg_t *c, fer_cfg_parser_t *parser);
static void parseFltArr(fer_cfg_t *c, fer_cfg_parser_t *parser);
static void parseV2Arr(fer_cfg_t *c, fer_cfg_parser_t *parser);
static void parseV3Arr(fer_cfg_t *c, fer_cfg_parser_t *parser);

static void ferCfgParse(fer_cfg_t *c, fer_cfg_parser_t *parser)
{
    NEXT(parser);
    while (parser->tok != 0){
        if (parser->tok == T_NAME){
            parseName(c, parser);
        }else{
            fprintf(stderr, "Fermat :: Cfg :: Error on line %u.\n", parser->val.lineno);
            NEXT(parser);
        }
    }
}

static void parseName(fer_cfg_t *c, fer_cfg_parser_t *parser)
{
    parser->name = strdup(parser->val.name);
    NEXT(parser);

    if (T_IS_TYPE(parser->tok)){
        switch(parser->tok){
            case T_TYPE_STR:
                NEXT(parser);
                parseStr(c, parser);
                break;
            case T_TYPE_FLT:
                NEXT(parser);
                parseFlt(c, parser);
                break;
            case T_TYPE_VV:
                NEXT(parser);
                parseV2(c, parser);
                break;
            case T_TYPE_VVV:
                NEXT(parser);
                parseV3(c, parser);
                break;
            case T_TYPE_STR_ARR:
                NEXT(parser);
                parseStrArr(c, parser);
                break;
            case T_TYPE_FLT_ARR:
                NEXT(parser);
                parseFltArr(c, parser);
                break;
            case T_TYPE_VV_ARR:
                NEXT(parser);
                parseV2Arr(c, parser);
                break;
            case T_TYPE_VVV_ARR:
                NEXT(parser);
                parseV3Arr(c, parser);
                break;
        }

    }else if (T_IS_VAL(parser->tok)){
        if (parser->tok == T_STR){
            parseStr(c, parser);
        }else if (parser->tok == T_FLT){
            parseFlt(c, parser);
        }
    }

    if (parser->name)
        free(parser->name);
}

static void parseStr(fer_cfg_t *c, fer_cfg_parser_t *parser)
{
    fer_cfg_param_str_t *p;

    if (parser->tok != T_STR){
        fprintf(stderr, "Fermat :: Cfg :: Invalid value on line %d.\n", parser->val.lineno);
    }else{
        p = FER_ALLOC(fer_cfg_param_str_t);
        SET_NAME(p, parser);
        p->type = FER_CFG_PARAM_STR;
        p->val  = strndup(parser->val.str + 1, parser->val.strlen - 2);
        ferCfgParamInsert(c, (fer_cfg_param_t *)p);
        NEXT(parser);
    }
}

static void parseFlt(fer_cfg_t *c, fer_cfg_parser_t *parser)
{
    fer_cfg_param_flt_t *p;

    if (parser->tok != T_FLT){
        fprintf(stderr, "Fermat :: Cfg :: Invalid value on line %d.\n", parser->val.lineno);
    }else{
        p = FER_ALLOC(fer_cfg_param_flt_t);
        SET_NAME(p, parser);
        p->type = FER_CFG_PARAM_FLT;
        p->val  = parser->val.flt;
        ferCfgParamInsert(c, (fer_cfg_param_t *)p);
        NEXT(parser);
    }
}

static void parseV2(fer_cfg_t *c, fer_cfg_parser_t *parser)
{
    fer_cfg_param_v2_t *p;
    int i;

    p = FER_ALLOC(fer_cfg_param_v2_t);
    SET_NAME(p, parser);
    p->type = FER_CFG_PARAM_V2;
    ferCfgParamInsert(c, (fer_cfg_param_t *)p);

    for (i = 0; i < 2; i++){
        if (parser->tok != T_FLT){
            fprintf(stderr, "Fermat :: Cfg :: Invalid value on line %d.\n", parser->val.lineno);
            break;
        }else{
            ferVec2SetCoord(&p->val, i, parser->val.flt);
        }
        NEXT(parser);
    }

    for (; i < 2; i++){
        ferVec2SetCoord(&p->val, i, FER_ZERO);
    }
}

static void parseV3(fer_cfg_t *c, fer_cfg_parser_t *parser)
{
    fer_cfg_param_v3_t *p;
    int i;

    p = FER_ALLOC(fer_cfg_param_v3_t);
    SET_NAME(p, parser);
    p->type = FER_CFG_PARAM_V3;
    ferCfgParamInsert(c, (fer_cfg_param_t *)p);

    for (i = 0; i < 3; i++){
        if (parser->tok != T_FLT){
            fprintf(stderr, "Fermat :: Cfg :: Invalid value on line %d.\n", parser->val.lineno);
            break;
        }else{
            ferVec3SetCoord(&p->val, i, parser->val.flt);
        }
        NEXT(parser);
    }

    for (; i < 3; i++){
        ferVec3SetCoord(&p->val, i, FER_ZERO);
    }
}

static void parseStrArr(fer_cfg_t *c, fer_cfg_parser_t *parser)
{
    fer_cfg_param_str_arr_t *p;
    int i, len;

    p = FER_ALLOC(fer_cfg_param_str_arr_t);
    SET_NAME(p, parser);
    p->type = FER_CFG_PARAM_STR | FER_CFG_PARAM_ARR;
    ferCfgParamInsert(c, (fer_cfg_param_t *)p);

    len = atoi(parser->val.type + 3);
    p->val = FER_ALLOC_ARR(char *, len);
    p->len = len;

    for (i = 0; i < len; i++){
        if (parser->tok != T_STR){
            fprintf(stderr, "Fermat :: Cfg :: Invalid value on line %d.\n", parser->val.lineno);
            break;
        }else{
            p->val[i] = strndup(parser->val.str + 1, parser->val.strlen - 2);
        }
        NEXT(parser);
    }

    for (; i < len; i++){
        p->val[i] = NULL;
    }
}

static void parseFltArr(fer_cfg_t *c, fer_cfg_parser_t *parser)
{
    fer_cfg_param_flt_arr_t *p;
    int i, len;

    p = FER_ALLOC(fer_cfg_param_flt_arr_t);
    SET_NAME(p, parser);
    p->type = FER_CFG_PARAM_FLT | FER_CFG_PARAM_ARR;
    ferCfgParamInsert(c, (fer_cfg_param_t *)p);

    len = atoi(parser->val.type + 3);
    p->val = FER_ALLOC_ARR(fer_real_t, len);
    p->len = len;

    for (i = 0; i < len; i++){
        if (parser->tok != T_FLT){
            fprintf(stderr, "Fermat :: Cfg :: Invalid value on line %d.\n", parser->val.lineno);
            break;
        }else{
            p->val[i] = parser->val.flt;
        }
        NEXT(parser);
    }

    for (; i < len; i++){
        p->val[i] = FER_ZERO;
    }
}

static void parseV2Arr(fer_cfg_t *c, fer_cfg_parser_t *parser)
{
    fer_cfg_param_v2_arr_t *p;
    int i, j, len, ok;

    p = FER_ALLOC(fer_cfg_param_v2_arr_t);
    SET_NAME(p, parser);
    p->type = FER_CFG_PARAM_V2 | FER_CFG_PARAM_ARR;
    ferCfgParamInsert(c, (fer_cfg_param_t *)p);

    len = atoi(parser->val.type + 4);
    p->val = FER_ALLOC_ARR(fer_vec2_t, len);
    p->len = len;

    ok = 1;
    for (i = 0; i < len && ok; i++){
        for (j = 0; j < 2; j++){
            if (parser->tok != T_FLT){
                fprintf(stderr, "Fermat :: Cfg :: Invalid value on line %d.\n", parser->val.lineno);
                ok = 0;
                break;
            }else{
                ferVec2SetCoord(&p->val[i], j, parser->val.flt);
            }
            NEXT(parser);
        }
    }
}

static void parseV3Arr(fer_cfg_t *c, fer_cfg_parser_t *parser)
{
    fer_cfg_param_v3_arr_t *p;
    int i, j, len, ok;

    p = FER_ALLOC(fer_cfg_param_v3_arr_t);
    SET_NAME(p, parser);
    p->type = FER_CFG_PARAM_V3 | FER_CFG_PARAM_ARR;
    ferCfgParamInsert(c, (fer_cfg_param_t *)p);

    len = atoi(parser->val.type + 4);
    p->val = FER_ALLOC_ARR(fer_vec3_t, len);
    p->len = len;

    ok = 1;
    for (i = 0; i < len && ok; i++){
        for (j = 0; j < 3; j++){
            if (parser->tok != T_FLT){
                fprintf(stderr, "Fermat :: Cfg :: Invalid value on line %d.\n", parser->val.lineno);
                ok = 0;
                break;
            }else{
                ferVec3SetCoord(&p->val[i], j, parser->val.flt);
            }
            NEXT(parser);
        }
    }
}

