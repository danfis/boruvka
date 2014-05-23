/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
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

#include <stdarg.h>
#include <boruvka/cfg.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>
#include <boruvka/hfunc.h>
#include <boruvka/vec.h>
#include "src/cfg-lexer.h"

typedef void *yyscan_t;
extern uint16_t yylex(yyscan_t s);
extern int yylex_init_extra(bor_cfg_lex_t *extra, yyscan_t *s);
extern void yyset_in(FILE * in_str, yyscan_t s);
extern int yylex_destroy(yyscan_t s);

struct _bor_cfg_parser_t {
    yyscan_t scanner;
    bor_cfg_lex_t val;
    uint16_t tok;
    char *name;
};
typedef struct _bor_cfg_parser_t bor_cfg_parser_t;


#define BOR_CFG_PARAM \
    char *name; \
    uint8_t type; \
    bor_list_t htable
struct _bor_cfg_param_t {
    BOR_CFG_PARAM;
};
typedef struct _bor_cfg_param_t bor_cfg_param_t;

#define CFG_PARAM(type, ctype) \
    struct _bor_cfg_param_ ## type ## _t { \
        BOR_CFG_PARAM; \
        ctype val; \
    }; \
    typedef struct _bor_cfg_param_ ## type ## _t bor_cfg_param_ ## type ## _t
CFG_PARAM(str, char *);
CFG_PARAM(flt, bor_real_t);
CFG_PARAM(int, int);
CFG_PARAM(v2, bor_vec2_t);
CFG_PARAM(v3, bor_vec3_t);


struct _bor_cfg_param_arr_t {
    BOR_CFG_PARAM;
    void *val;
    size_t len;
};
typedef struct _bor_cfg_param_arr_t bor_cfg_param_arr_t;
#define CFG_PARAM_ARR(type, ctype) \
    struct _bor_cfg_param_ ## type ## _arr_t { \
        BOR_CFG_PARAM; \
        ctype *val; \
        size_t len; \
    }; \
    typedef struct _bor_cfg_param_ ## type ## _arr_t bor_cfg_param_ ## type ## _arr_t
CFG_PARAM_ARR(str, char *);
CFG_PARAM_ARR(flt, bor_real_t);
CFG_PARAM_ARR(int, int);
CFG_PARAM_ARR(v2, bor_vec2_t);
CFG_PARAM_ARR(v3, bor_vec3_t);



static uint64_t htableHash(const bor_list_t *key, void *data);
static int htableEq(const bor_list_t *k1, const bor_list_t *k2, void *data);

static void borCfgParamDel(bor_cfg_param_t *p);
static bor_cfg_param_t *borCfgParam(const bor_cfg_t *c, const char *name);
static bor_cfg_param_t *borCfgParamByType(const bor_cfg_t *c, const char *name,
                                          uint8_t type);
static void borCfgParamInsert(bor_cfg_t *c, bor_cfg_param_t *p);

static void borCfgParse(bor_cfg_t *c, bor_cfg_parser_t *parser);


/*
static void __dumpParams(bor_cfg_t *c)
{
    int i, j;
    bor_list_t *item;
    bor_cfg_param_t *p;

    for (i = 0; i < (int)c->params->size; i++){
        BOR_LIST_FOR_EACH(c->params->table + i, item){
            p = BOR_LIST_ENTRY(item, bor_cfg_param_t, hmap);
            fprintf(stdout, "[%04d]: `%s', %02x:", i, p->name, p->type);

            if (p->type == BOR_CFG_PARAM_STR){
                fprintf(stdout, "s: `%s'", ((bor_cfg_param_str_t *)p)->val);
            }else if (p->type == BOR_CFG_PARAM_FLT){
                fprintf(stdout, "f: %f", (float)((bor_cfg_param_flt_t *)p)->val);
            }else if (p->type == BOR_CFG_PARAM_V2){
                fprintf(stdout, "v2: ");
                borVec2Print(&((bor_cfg_param_v2_t *)p)->val, stdout);
            }else if (p->type == BOR_CFG_PARAM_V3){
                fprintf(stdout, "v3: ");
                borVec3Print(&((bor_cfg_param_v3_t *)p)->val, stdout);
            }else if (p->type == (BOR_CFG_PARAM_STR | BOR_CFG_PARAM_ARR)){
                bor_cfg_param_str_arr_t *ps = (bor_cfg_param_str_arr_t *)p;

                fprintf(stdout, "s[%d]:", ps->len);
                for (j = 0; j < ps->len; j++){
                    fprintf(stdout, " `%s'", ps->val[j]);
                }
            }else if (p->type == (BOR_CFG_PARAM_FLT | BOR_CFG_PARAM_ARR)){
                bor_cfg_param_flt_arr_t *pf = (bor_cfg_param_flt_arr_t *)p;

                fprintf(stdout, "f[%d]:", pf->len);
                for (j = 0; j < pf->len; j++){
                    fprintf(stdout, " %f", (float)pf->val[j]);
                }
            }else if (p->type == (BOR_CFG_PARAM_V2 | BOR_CFG_PARAM_ARR)){
                bor_cfg_param_v2_arr_t *pv2 = (bor_cfg_param_v2_arr_t *)p;

                fprintf(stdout, "v2[%d]:", pv2->len);
                for (j = 0; j < pv2->len; j++){
                    fprintf(stdout, " (");
                    borVec2Print(&pv2->val[j], stdout);
                    fprintf(stdout, ")");
                }
            }else if (p->type == (BOR_CFG_PARAM_V3 | BOR_CFG_PARAM_ARR)){
                bor_cfg_param_v3_arr_t *pv3 = (bor_cfg_param_v3_arr_t *)p;

                fprintf(stdout, "v3[%d]:", pv3->len);
                for (j = 0; j < pv3->len; j++){
                    fprintf(stdout, " (");
                    borVec3Print(&pv3->val[j], stdout);
                    fprintf(stdout, ")");
                }
            }
            fprintf(stdout, "\n");
        }
    }
}
*/

bor_cfg_t *borCfgRead(const char *filename)
{
    bor_cfg_t *c;
    FILE *fin;
    bor_cfg_parser_t parser;

    // open cfg file
    fin = fopen(filename, "r");
    if (!fin){
        fprintf(stderr, "Boruvka :: Cfg :: No such file: `%s'.\n", filename);
        return NULL;
    }

    c = BOR_ALLOC(bor_cfg_t);
    c->params = borHTableNew(htableHash, htableEq, c);

    // init parser
    if (yylex_init_extra(&parser.val, &parser.scanner) != 0){
        fprintf(stderr, "Boruvka :: Cfg :: Can't initialize parser!\n");
        borCfgDel(c);
        return NULL;
    }
    yyset_in(fin, parser.scanner);

    borCfgParse(c, &parser);

    // free parser
    yylex_destroy(parser.scanner);
    fclose(fin);

    /* __dumpParams(c); */

    return c;
}


void borCfgDel(bor_cfg_t *c)
{
    bor_list_t list, *item;
    bor_cfg_param_t *p;

    borListInit(&list);
    borHTableGather(c->params, &list);

    while (!borListEmpty(&list)){
        item = borListNext(&list);
        p = BOR_LIST_ENTRY(item, bor_cfg_param_t, htable);
        borListDel(item);
        borCfgParamDel(p);
    }

    borHTableDel(c->params);
    BOR_FREE(c);
}

int borCfgHaveParam(const bor_cfg_t *c, const char *name)
{
    bor_list_t *item;
    bor_cfg_param_t q;
    q.name = (char *)name;

    item = borHTableFind(c->params, &q.htable);
    return item != NULL;
}

uint8_t borCfgParamType(const bor_cfg_t *c, const char *name)
{
    bor_list_t *item;
    bor_cfg_param_t *p, q;
    q.name = (char *)name;

    item = borHTableFind(c->params, &q.htable);
    if (!item)
        return BOR_CFG_PARAM_NONE;

    p = BOR_LIST_ENTRY(item, bor_cfg_param_t, htable);
    return p->type;
}


int borCfgParamIsArr(const bor_cfg_t *c, const char *name)
{
    return (borCfgParamType(c, name) & BOR_CFG_PARAM_ARR) == BOR_CFG_PARAM_ARR;
}

#define PARAM_IS_FN(Type, TYPE) \
    int borCfgParamIs ## Type(const bor_cfg_t *c, const char *name) \
    { \
        return (borCfgParamType(c, name) & (0xff >> 1)) == BOR_CFG_PARAM_ ## TYPE; \
    }
PARAM_IS_FN(Str, STR)
PARAM_IS_FN(Flt, FLT)
PARAM_IS_FN(Int, INT)
PARAM_IS_FN(V2, V2)
PARAM_IS_FN(V3, V3)


#define PARAM_FN_CP(Type, type, TYPE, ctype, CP) \
    int borCfgParam ## Type(const bor_cfg_t *c, \
                            const char *name, \
                            ctype *val) \
    { \
        bor_cfg_param_ ## type ## _t *v; \
        \
        v = (bor_cfg_param_ ## type ## _t *)borCfgParamByType(c, name, BOR_CFG_PARAM_ ## TYPE); \
        if (!v) \
            return -1; \
        \
        CP; \
        return 0; \
    }

#define PARAM_FN(Type, type, TYPE, ctype) \
    PARAM_FN_CP(Type, type, TYPE, ctype, *val = v->val)

#define PARAM_FN_ARR(Type, type, TYPE, ctype) \
    int borCfgParam ## Type ## Arr(const bor_cfg_t *c, \
                                   const char *name, \
                                   ctype **val, size_t *len) \
    { \
        bor_cfg_param_ ## type ## _arr_t *v; \
        \
        v = (bor_cfg_param_ ## type ## _arr_t *)borCfgParamByType(c, name, \
                                        BOR_CFG_PARAM_ ## TYPE | BOR_CFG_PARAM_ARR); \
        if (!v) \
            return -1; \
        \
        *val = v->val; \
        *len = v->len; \
        return 0; \
    }

PARAM_FN(Str, str, STR, const char *)
PARAM_FN(Flt, flt, FLT, bor_real_t)
PARAM_FN(Int, int, INT, int)
PARAM_FN_CP(V2, v2, V2, bor_vec2_t, borVec2Copy(val, &v->val))
PARAM_FN_CP(V3, v3, V3, bor_vec3_t, borVec3Copy(val, &v->val))

PARAM_FN_ARR(Str, str, STR, char *)
PARAM_FN_ARR(Flt, flt, FLT, const bor_real_t)
PARAM_FN_ARR(Int, int, INT, const int)
PARAM_FN_ARR(V2, v2, V2, const bor_vec2_t)
PARAM_FN_ARR(V3, v3, V3, const bor_vec3_t)



static const char *_borCfgScanNext(const char *format, char *name, char *type);
int borCfgScan(const bor_cfg_t *c, const char *format, ...)
{
    char name[1024];
    char type[5];
    const char *s;
    int store;
    va_list ap;
    bor_real_t *vf;
    int *vi;
    const bor_real_t **vfa;
    const int **via;
    const char **vs;
    char ***vsa;
    bor_vec2_t *v2;
    const bor_vec2_t **v2a;
    bor_vec3_t *v3;
    const bor_vec3_t **v3a;
    bor_cfg_param_t *param;
    size_t len, *vlen;

    va_start(ap, format);

    s = format;
    while ((s = _borCfgScanNext(s, name, type)) != NULL){
        param = borCfgParam(c, name);
        if (!param){
            fprintf(stderr, "Boruvka :: CfgScan :: No such parameter: `%s'.\n", name);
            return -1;
        }

        if (strcmp(type, "f") == 0){
            vf = va_arg(ap, bor_real_t *);
            store = borCfgParamFlt(c, name, vf);
        }else if (strcmp(type, "i") == 0){
            vi = va_arg(ap, int *);
            store = borCfgParamInt(c, name, vi);
        }else if (strcmp(type, "s") == 0){
            vs = va_arg(ap, const char **);
            store = borCfgParamStr(c, name, vs);
        }else if (strcmp(type, "v2") == 0){
            v2 = va_arg(ap, bor_vec2_t *);
            store = borCfgParamV2(c, name, v2);
        }else if (strcmp(type, "v3") == 0){
            v3 = va_arg(ap, bor_vec3_t *);
            store = borCfgParamV3(c, name, v3);

        }else if (strcmp(type, "f[]") == 0){
            vfa = va_arg(ap, const bor_real_t **);
            store = borCfgParamFltArr(c, name, vfa, &len);
        }else if (strcmp(type, "i[]") == 0){
            via = va_arg(ap, const int **);
            store = borCfgParamIntArr(c, name, via, &len);
        }else if (strcmp(type, "s[]") == 0){
            vsa = va_arg(ap, char ***);
            store = borCfgParamStrArr(c, name, vsa, &len);
        }else if (strcmp(type, "v2[]") == 0){
            v2a = va_arg(ap, const bor_vec2_t **);
            store = borCfgParamV2Arr(c, name, v2a, &len);
        }else if (strcmp(type, "v3[]") == 0){
            v3a = va_arg(ap, const bor_vec3_t **);
            store = borCfgParamV3Arr(c, name, v3a, &len);

        }else if (strcmp(type, "f#") == 0){
            vlen = va_arg(ap, size_t *);
            store = borCfgParamFltArr(c, name, (const bor_real_t **)&vf, vlen);
        }else if (strcmp(type, "i#") == 0){
            vlen = va_arg(ap, size_t *);
            store = borCfgParamIntArr(c, name, (const int **)&vf, vlen);
        }else if (strcmp(type, "s#") == 0){
            vlen = va_arg(ap, size_t *);
            store = borCfgParamStrArr(c, name, (char ***)&vs, vlen);
        }else if (strcmp(type, "v2#") == 0){
            vlen = va_arg(ap, size_t *);
            store = borCfgParamV2Arr(c, name, (const bor_vec2_t **)&v2, vlen);
        }else if (strcmp(type, "v3#") == 0){
            vlen = va_arg(ap, size_t *);
            store = borCfgParamV3Arr(c, name, (const bor_vec3_t **)&v3, vlen);
        }

        if (store != 0){
            fprintf(stderr, "Boruvka :: CfgScan :: Invalid type of `%s'.\n", name);
            return -1;
        }
    }

    va_end(ap);
    return 0;
}

#define IS_WS(s) \
    (*s == ' ' || *s == '\t')
#define SKIP_WS(s) \
    while (*s != 0x0 && IS_WS(s)) ++s
static const char *_borCfgScanNext(const char *format, char *name, char *type)
{
    const char *s = format;
    int i;

    SKIP_WS(s);
    for (i = 0; *s != 0x0 && *s != ':'; ++i, ++s){
        name[i] = *s;
    }
    name[i] = 0x0;
    if (i == 0)
        return NULL;
    if (*s == ':')
        ++s;

    for (i = 0; *s != 0x0 && !IS_WS(s); ++i, ++s){
        type[i] = *s;
    }
    type[i] = 0x0;
    if (i == 0)
        return NULL;

    return s;
}





static uint64_t htableHash(const bor_list_t *key, void *data)
{
    bor_cfg_param_t *p;
    p = BOR_LIST_ENTRY(key, bor_cfg_param_t, htable);
    return borHashDJB2(p->name);
}

static int htableEq(const bor_list_t *k1, const bor_list_t *k2, void *data)
{
    bor_cfg_param_t *p1, *p2;

    p1 = BOR_LIST_ENTRY(k1, bor_cfg_param_t, htable);
    p2 = BOR_LIST_ENTRY(k2, bor_cfg_param_t, htable);
    return strcmp(p1->name, p2->name) == 0;
}

static void borCfgParamDel(bor_cfg_param_t *p)
{
    bor_cfg_param_str_t *str;
    bor_cfg_param_arr_t *arr;
    bor_cfg_param_str_arr_t *str_arr;
    size_t i;

    if (p->type == BOR_CFG_PARAM_STR){
        str = (bor_cfg_param_str_t *)p;
        free(str->val);
    }else if (p->type == (BOR_CFG_PARAM_ARR | BOR_CFG_PARAM_STR)){
        str_arr = (bor_cfg_param_str_arr_t *)p;
        for (i = 0; i < str_arr->len; i++){
            free(str_arr->val[i]);
        }
        BOR_FREE(str_arr->val);
    }else if ((p->type & BOR_CFG_PARAM_ARR) == BOR_CFG_PARAM_ARR){
        arr = (bor_cfg_param_arr_t *)p;
        BOR_FREE(arr->val);
    }

    free(p->name);
    BOR_FREE(p);
}

static bor_cfg_param_t *borCfgParam(const bor_cfg_t *c, const char *name)
{
    bor_list_t *item;
    bor_cfg_param_t *p, q;
    q.name = (char *)name;

    item = borHTableFind(c->params, &q.htable);
    if (!item)
        return NULL;

    p = BOR_LIST_ENTRY(item, bor_cfg_param_t, htable);
    return p;
}

static bor_cfg_param_t *borCfgParamByType(const bor_cfg_t *c, const char *name,
                                          uint8_t type)
{
    bor_cfg_param_t *p;

    p = borCfgParam(c, name);
    if (!p || p->type != type)
        return NULL;
    return p;
}

static void borCfgParamInsert(bor_cfg_t *c, bor_cfg_param_t *p)
{
    borHTableInsert(c->params, &p->htable);
}



#define NEXT(parser) (parser)->tok = yylex((parser)->scanner)
#define SET_NAME(param, parser) \
    (param)->name = (parser)->name; \
    (parser)->name = NULL

static void parseName(bor_cfg_t *c, bor_cfg_parser_t *parser);
static void parseStr(bor_cfg_t *c, bor_cfg_parser_t *parser);
static void parseFlt(bor_cfg_t *c, bor_cfg_parser_t *parser);
static void parseInt(bor_cfg_t *c, bor_cfg_parser_t *parser);
static void parseV2(bor_cfg_t *c, bor_cfg_parser_t *parser);
static void parseV3(bor_cfg_t *c, bor_cfg_parser_t *parser);
static void parseStrArr(bor_cfg_t *c, bor_cfg_parser_t *parser);
static void parseFltArr(bor_cfg_t *c, bor_cfg_parser_t *parser);
static void parseIntArr(bor_cfg_t *c, bor_cfg_parser_t *parser);
static void parseV2Arr(bor_cfg_t *c, bor_cfg_parser_t *parser);
static void parseV3Arr(bor_cfg_t *c, bor_cfg_parser_t *parser);

static void borCfgParse(bor_cfg_t *c, bor_cfg_parser_t *parser)
{
    NEXT(parser);
    while (parser->tok != 0){
        if (parser->tok == T_NAME){
            parseName(c, parser);
        }else{
            fprintf(stderr, "Boruvka :: Cfg :: Error on line %u.\n", parser->val.lineno);
            NEXT(parser);
        }
    }
}

static void parseName(bor_cfg_t *c, bor_cfg_parser_t *parser)
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
            case T_TYPE_INT:
                NEXT(parser);
                parseInt(c, parser);
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
            case T_TYPE_INT_ARR:
                NEXT(parser);
                parseIntArr(c, parser);
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
        }else if (parser->tok == T_INT){
            parseInt(c, parser);
        }
    }

    if (parser->name)
        free(parser->name);
}


#define PARSE_FN(Type, ttype, TYPE, CP) \
    static void parse ## Type(bor_cfg_t *c, bor_cfg_parser_t *parser) \
    { \
        bor_cfg_param_ ## ttype ## _t *p; \
        \
        if (parser->tok != T_ ## TYPE){ \
            fprintf(stderr, "Boruvka :: Cfg :: Invalid value on line %d.\n", parser->val.lineno); \
        }else{ \
            p = BOR_ALLOC(bor_cfg_param_ ## ttype ## _t); \
            SET_NAME(p, parser); \
            p->type = BOR_CFG_PARAM_ ## TYPE; \
            CP; \
            borCfgParamInsert(c, (bor_cfg_param_t *)p); \
            NEXT(parser); \
        } \
    }
PARSE_FN(Str, str, STR,
         p->val = strndup(parser->val.str + 1, parser->val.strlen - 2))
PARSE_FN(Flt, flt, FLT, p->val = parser->val.flt)
PARSE_FN(Int, int, INT, p->val = parser->val.integer)


#define PARSE_FN_VEC(SIZE) \
    static void parseV ## SIZE(bor_cfg_t *c, bor_cfg_parser_t *parser) \
    { \
        bor_cfg_param_v ## SIZE ## _t *p; \
        int i; \
        \
        p = BOR_ALLOC(bor_cfg_param_v ## SIZE ## _t); \
        SET_NAME(p, parser); \
        p->type = BOR_CFG_PARAM_V ## SIZE; \
        borCfgParamInsert(c, (bor_cfg_param_t *)p); \
        \
        for (i = 0; i < SIZE; i++){ \
            if (parser->tok != T_FLT){ \
                fprintf(stderr, "Boruvka :: Cfg :: Invalid value on line %d.\n", parser->val.lineno); \
                break; \
            }else{ \
                borVecSet((bor_vec_t *)&p->val, i, parser->val.flt); \
            } \
            NEXT(parser); \
        } \
        \
        for (; i < SIZE; i++){ \
            borVecSet((bor_vec_t *)&p->val, i, BOR_ZERO); \
        } \
    }
PARSE_FN_VEC(2)
PARSE_FN_VEC(3)


#define PARSE_FN_ARR(Type, ttype, TYPE, ctype, CP, DEFAULT) \
    static void parse ## Type ##Arr(bor_cfg_t *c, bor_cfg_parser_t *parser) \
    { \
        bor_cfg_param_ ## ttype ## _arr_t *p; \
        int i, len; \
        \
        p = BOR_ALLOC(bor_cfg_param_ ## ttype ## _arr_t); \
        SET_NAME(p, parser); \
        p->type = BOR_CFG_PARAM_ ## TYPE | BOR_CFG_PARAM_ARR; \
        borCfgParamInsert(c, (bor_cfg_param_t *)p); \
        \
        len = atoi(parser->val.type + 3); \
        p->val = BOR_ALLOC_ARR(ctype, len); \
        p->len = len; \
        \
        for (i = 0; i < len; i++){ \
            if (parser->tok != T_ ## TYPE){ \
                fprintf(stderr, "Boruvka :: Cfg :: Invalid value on line %d.\n", parser->val.lineno); \
                break; \
            }else{ \
                CP; \
            } \
            NEXT(parser); \
        } \
        \
        for (; i < len; i++){ \
            p->val[i] = DEFAULT; \
        } \
    }
PARSE_FN_ARR(Str, str, STR, char *, 
             p->val[i] = strndup(parser->val.str + 1, parser->val.strlen - 2),
             NULL)
PARSE_FN_ARR(Flt, flt, FLT, bor_real_t, p->val[i] = parser->val.flt, BOR_ZERO)
PARSE_FN_ARR(Int, int, INT, int, p->val[i] = parser->val.integer, 0)


#define PARSE_FN_VEC_ARR(SIZE) \
    static void parseV ## SIZE ## Arr(bor_cfg_t *c, bor_cfg_parser_t *parser) \
    { \
        bor_cfg_param_v ## SIZE ## _arr_t *p; \
        int i, j, len, ok; \
        \
        p = BOR_ALLOC(bor_cfg_param_v ## SIZE ## _arr_t); \
        SET_NAME(p, parser); \
        p->type = BOR_CFG_PARAM_V ## SIZE | BOR_CFG_PARAM_ARR; \
        borCfgParamInsert(c, (bor_cfg_param_t *)p); \
        \
        len = atoi(parser->val.type + 4); \
        p->val = BOR_ALLOC_ARR(bor_vec ## SIZE ## _t, len); \
        p->len = len; \
        \
        ok = 1; \
        for (i = 0; i < len && ok; i++){ \
            for (j = 0; j < SIZE; j++){ \
                if (parser->tok != T_FLT){ \
                    fprintf(stderr, "Boruvka :: Cfg :: Invalid value on line %d.\n", parser->val.lineno); \
                    ok = 0; \
                    break; \
                }else{ \
                    borVecSet((bor_vec_t *)&p->val[i], j, parser->val.flt); \
                } \
                NEXT(parser); \
            } \
        } \
    }
PARSE_FN_VEC_ARR(2)
PARSE_FN_VEC_ARR(3)
