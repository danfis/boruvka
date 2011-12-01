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

#include <stdarg.h>
#include <fermat/cfg.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>
#include <fermat/hfunc.h>
#include <fermat/vec.h>
#include "src/cfg-lexer.h"

typedef void *yyscan_t;
extern uint16_t yylex(yyscan_t s);
extern int yylex_init_extra(fer_cfg_lex_t *extra, yyscan_t *s);
extern void yyset_in(FILE * in_str, yyscan_t s);
extern int yylex_destroy(yyscan_t s);

struct _fer_cfg_parser_t {
    yyscan_t scanner;
    fer_cfg_lex_t val;
    uint16_t tok;
    char *name;
};
typedef struct _fer_cfg_parser_t fer_cfg_parser_t;


#define FER_CFG_PARAM \
    char *name; \
    uint8_t type; \
    fer_list_t hmap
struct _fer_cfg_param_t {
    FER_CFG_PARAM;
};
typedef struct _fer_cfg_param_t fer_cfg_param_t;

#define CFG_PARAM(type, ctype) \
    struct _fer_cfg_param_ ## type ## _t { \
        FER_CFG_PARAM; \
        ctype val; \
    }; \
    typedef struct _fer_cfg_param_ ## type ## _t fer_cfg_param_ ## type ## _t
CFG_PARAM(str, char *);
CFG_PARAM(flt, fer_real_t);
CFG_PARAM(int, int);
CFG_PARAM(v2, fer_vec2_t);
CFG_PARAM(v3, fer_vec3_t);


struct _fer_cfg_param_arr_t {
    FER_CFG_PARAM;
    void *val;
    size_t len;
};
typedef struct _fer_cfg_param_arr_t fer_cfg_param_arr_t;
#define CFG_PARAM_ARR(type, ctype) \
    struct _fer_cfg_param_ ## type ## _arr_t { \
        FER_CFG_PARAM; \
        ctype *val; \
        size_t len; \
    }; \
    typedef struct _fer_cfg_param_ ## type ## _arr_t fer_cfg_param_ ## type ## _arr_t
CFG_PARAM_ARR(str, char *);
CFG_PARAM_ARR(flt, fer_real_t);
CFG_PARAM_ARR(int, int);
CFG_PARAM_ARR(v2, fer_vec2_t);
CFG_PARAM_ARR(v3, fer_vec3_t);



static uint32_t hmapHash(fer_list_t *key, void *data);
static int hmapEq(const fer_list_t *k1, const fer_list_t *k2, void *data);

static void ferCfgParamDel(fer_cfg_param_t *p);
static fer_cfg_param_t *ferCfgParam(const fer_cfg_t *c, const char *name);
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

#define PARAM_IS_FN(Type, TYPE) \
    int ferCfgParamIs ## Type(const fer_cfg_t *c, const char *name) \
    { \
        return (ferCfgParamType(c, name) & (0xff >> 1)) == FER_CFG_PARAM_ ## TYPE; \
    }
PARAM_IS_FN(Str, STR)
PARAM_IS_FN(Flt, FLT)
PARAM_IS_FN(Int, INT)
PARAM_IS_FN(V2, V2)
PARAM_IS_FN(V3, V3)


#define PARAM_FN_CP(Type, type, TYPE, ctype, CP) \
    int ferCfgParam ## Type(const fer_cfg_t *c, \
                            const char *name, \
                            ctype *val) \
    { \
        fer_cfg_param_ ## type ## _t *v; \
        \
        v = (fer_cfg_param_ ## type ## _t *)ferCfgParamByType(c, name, FER_CFG_PARAM_ ## TYPE); \
        if (!v) \
            return -1; \
        \
        CP; \
        return 0; \
    }

#define PARAM_FN(Type, type, TYPE, ctype) \
    PARAM_FN_CP(Type, type, TYPE, ctype, *val = v->val)

#define PARAM_FN_ARR(Type, type, TYPE, ctype) \
    int ferCfgParam ## Type ## Arr(const fer_cfg_t *c, \
                                   const char *name, \
                                   ctype **val, size_t *len) \
    { \
        fer_cfg_param_ ## type ## _arr_t *v; \
        \
        v = (fer_cfg_param_ ## type ## _arr_t *)ferCfgParamByType(c, name, \
                                        FER_CFG_PARAM_ ## TYPE | FER_CFG_PARAM_ARR); \
        if (!v) \
            return -1; \
        \
        *val = v->val; \
        *len = v->len; \
        return 0; \
    }

PARAM_FN(Str, str, STR, const char *)
PARAM_FN(Flt, flt, FLT, fer_real_t)
PARAM_FN(Int, int, INT, int)
PARAM_FN_CP(V2, v2, V2, fer_vec2_t, ferVec2Copy(val, &v->val))
PARAM_FN_CP(V3, v3, V3, fer_vec3_t, ferVec3Copy(val, &v->val))

PARAM_FN_ARR(Str, str, STR, char *)
PARAM_FN_ARR(Flt, flt, FLT, const fer_real_t)
PARAM_FN_ARR(Int, int, INT, const int)
PARAM_FN_ARR(V2, v2, V2, const fer_vec2_t)
PARAM_FN_ARR(V3, v3, V3, const fer_vec3_t)



static const char *_ferCfgScanNext(const char *format, char *name, char *type);
int ferCfgScan(const fer_cfg_t *c, const char *format, ...)
{
    char name[1024];
    char type[5];
    const char *s;
    int store;
    va_list ap;
    fer_real_t *vf;
    int *vi;
    const fer_real_t **vfa;
    const int **via;
    const char **vs;
    char ***vsa;
    fer_vec2_t *v2;
    const fer_vec2_t **v2a;
    fer_vec3_t *v3;
    const fer_vec3_t **v3a;
    fer_cfg_param_t *param;
    size_t len, *vlen;

    va_start(ap, format);

    s = format;
    while ((s = _ferCfgScanNext(s, name, type)) != NULL){
        param = ferCfgParam(c, name);
        if (!param){
            fprintf(stderr, "Fermat :: CfgScan :: No such parameter: `%s'.\n", name);
            return -1;
        }

        if (strcmp(type, "f") == 0){
            vf = va_arg(ap, fer_real_t *);
            store = ferCfgParamFlt(c, name, vf);
        }else if (strcmp(type, "i") == 0){
            vi = va_arg(ap, int *);
            store = ferCfgParamInt(c, name, vi);
        }else if (strcmp(type, "s") == 0){
            vs = va_arg(ap, const char **);
            store = ferCfgParamStr(c, name, vs);
        }else if (strcmp(type, "v2") == 0){
            v2 = va_arg(ap, fer_vec2_t *);
            store = ferCfgParamV2(c, name, v2);
        }else if (strcmp(type, "v3") == 0){
            v3 = va_arg(ap, fer_vec3_t *);
            store = ferCfgParamV3(c, name, v3);

        }else if (strcmp(type, "f[]") == 0){
            vfa = va_arg(ap, const fer_real_t **);
            store = ferCfgParamFltArr(c, name, vfa, &len);
        }else if (strcmp(type, "i[]") == 0){
            via = va_arg(ap, const int **);
            store = ferCfgParamIntArr(c, name, via, &len);
        }else if (strcmp(type, "s[]") == 0){
            vsa = va_arg(ap, char ***);
            store = ferCfgParamStrArr(c, name, vsa, &len);
        }else if (strcmp(type, "v2[]") == 0){
            v2a = va_arg(ap, const fer_vec2_t **);
            store = ferCfgParamV2Arr(c, name, v2a, &len);
        }else if (strcmp(type, "v3[]") == 0){
            v3a = va_arg(ap, const fer_vec3_t **);
            store = ferCfgParamV3Arr(c, name, v3a, &len);

        }else if (strcmp(type, "f#") == 0){
            vlen = va_arg(ap, size_t *);
            store = ferCfgParamFltArr(c, name, (const fer_real_t **)&vf, vlen);
        }else if (strcmp(type, "i#") == 0){
            vlen = va_arg(ap, size_t *);
            store = ferCfgParamIntArr(c, name, (const int **)&vf, vlen);
        }else if (strcmp(type, "s#") == 0){
            vlen = va_arg(ap, size_t *);
            store = ferCfgParamStrArr(c, name, (char ***)&vs, vlen);
        }else if (strcmp(type, "v2#") == 0){
            vlen = va_arg(ap, size_t *);
            store = ferCfgParamV2Arr(c, name, (const fer_vec2_t **)&v2, vlen);
        }else if (strcmp(type, "v3#") == 0){
            vlen = va_arg(ap, size_t *);
            store = ferCfgParamV3Arr(c, name, (const fer_vec3_t **)&v3, vlen);
        }

        if (store != 0){
            fprintf(stderr, "Fermat :: CfgScan :: Invalid type of `%s'.\n", name);
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
static const char *_ferCfgScanNext(const char *format, char *name, char *type)
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

static fer_cfg_param_t *ferCfgParam(const fer_cfg_t *c, const char *name)
{
    fer_list_t *item;
    fer_cfg_param_t *p, q;
    q.name = (char *)name;

    item = ferHMapGet(c->params, &q.hmap);
    if (!item)
        return NULL;

    p = FER_LIST_ENTRY(item, fer_cfg_param_t, hmap);
    return p;
}

static fer_cfg_param_t *ferCfgParamByType(const fer_cfg_t *c, const char *name,
                                          uint8_t type)
{
    fer_cfg_param_t *p;

    p = ferCfgParam(c, name);
    if (!p || p->type != type)
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
static void parseInt(fer_cfg_t *c, fer_cfg_parser_t *parser);
static void parseV2(fer_cfg_t *c, fer_cfg_parser_t *parser);
static void parseV3(fer_cfg_t *c, fer_cfg_parser_t *parser);
static void parseStrArr(fer_cfg_t *c, fer_cfg_parser_t *parser);
static void parseFltArr(fer_cfg_t *c, fer_cfg_parser_t *parser);
static void parseIntArr(fer_cfg_t *c, fer_cfg_parser_t *parser);
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
    static void parse ## Type(fer_cfg_t *c, fer_cfg_parser_t *parser) \
    { \
        fer_cfg_param_ ## ttype ## _t *p; \
        \
        if (parser->tok != T_ ## TYPE){ \
            fprintf(stderr, "Fermat :: Cfg :: Invalid value on line %d.\n", parser->val.lineno); \
        }else{ \
            p = FER_ALLOC(fer_cfg_param_ ## ttype ## _t); \
            SET_NAME(p, parser); \
            p->type = FER_CFG_PARAM_ ## TYPE; \
            CP; \
            ferCfgParamInsert(c, (fer_cfg_param_t *)p); \
            NEXT(parser); \
        } \
    }
PARSE_FN(Str, str, STR,
         p->val = strndup(parser->val.str + 1, parser->val.strlen - 2))
PARSE_FN(Flt, flt, FLT, p->val = parser->val.flt)
PARSE_FN(Int, int, INT, p->val = parser->val.integer)


#define PARSE_FN_VEC(SIZE) \
    static void parseV ## SIZE(fer_cfg_t *c, fer_cfg_parser_t *parser) \
    { \
        fer_cfg_param_v ## SIZE ## _t *p; \
        int i; \
        \
        p = FER_ALLOC(fer_cfg_param_v ## SIZE ## _t); \
        SET_NAME(p, parser); \
        p->type = FER_CFG_PARAM_V ## SIZE; \
        ferCfgParamInsert(c, (fer_cfg_param_t *)p); \
        \
        for (i = 0; i < SIZE; i++){ \
            if (parser->tok != T_FLT){ \
                fprintf(stderr, "Fermat :: Cfg :: Invalid value on line %d.\n", parser->val.lineno); \
                break; \
            }else{ \
                ferVecSet((fer_vec_t *)&p->val, i, parser->val.flt); \
            } \
            NEXT(parser); \
        } \
        \
        for (; i < SIZE; i++){ \
            ferVecSet((fer_vec_t *)&p->val, i, FER_ZERO); \
        } \
    }
PARSE_FN_VEC(2)
PARSE_FN_VEC(3)


#define PARSE_FN_ARR(Type, ttype, TYPE, ctype, CP, DEFAULT) \
    static void parse ## Type ##Arr(fer_cfg_t *c, fer_cfg_parser_t *parser) \
    { \
        fer_cfg_param_ ## ttype ## _arr_t *p; \
        int i, len; \
        \
        p = FER_ALLOC(fer_cfg_param_ ## ttype ## _arr_t); \
        SET_NAME(p, parser); \
        p->type = FER_CFG_PARAM_ ## TYPE | FER_CFG_PARAM_ARR; \
        ferCfgParamInsert(c, (fer_cfg_param_t *)p); \
        \
        len = atoi(parser->val.type + 3); \
        p->val = FER_ALLOC_ARR(ctype, len); \
        p->len = len; \
        \
        for (i = 0; i < len; i++){ \
            if (parser->tok != T_ ## TYPE){ \
                fprintf(stderr, "Fermat :: Cfg :: Invalid value on line %d.\n", parser->val.lineno); \
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
PARSE_FN_ARR(Flt, flt, FLT, fer_real_t, p->val[i] = parser->val.flt, FER_ZERO)
PARSE_FN_ARR(Int, int, INT, int, p->val[i] = parser->val.integer, 0)


#define PARSE_FN_VEC_ARR(SIZE) \
    static void parseV ## SIZE ## Arr(fer_cfg_t *c, fer_cfg_parser_t *parser) \
    { \
        fer_cfg_param_v ## SIZE ## _arr_t *p; \
        int i, j, len, ok; \
        \
        p = FER_ALLOC(fer_cfg_param_v ## SIZE ## _arr_t); \
        SET_NAME(p, parser); \
        p->type = FER_CFG_PARAM_V ## SIZE | FER_CFG_PARAM_ARR; \
        ferCfgParamInsert(c, (fer_cfg_param_t *)p); \
        \
        len = atoi(parser->val.type + 4); \
        p->val = FER_ALLOC_ARR(fer_vec ## SIZE ## _t, len); \
        p->len = len; \
        \
        ok = 1; \
        for (i = 0; i < len && ok; i++){ \
            for (j = 0; j < SIZE; j++){ \
                if (parser->tok != T_FLT){ \
                    fprintf(stderr, "Fermat :: Cfg :: Invalid value on line %d.\n", parser->val.lineno); \
                    ok = 0; \
                    break; \
                }else{ \
                    ferVecSet((fer_vec_t *)&p->val[i], j, parser->val.flt); \
                } \
                NEXT(parser); \
            } \
        } \
    }
PARSE_FN_VEC_ARR(2)
PARSE_FN_VEC_ARR(3)
