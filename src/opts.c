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

#include <string.h>
#include <boruvka/opts.h>
#include <boruvka/parse.h>
#include <boruvka/dbg.h>
#include <boruvka/alloc.h>

struct _fer_opt_t {
    const char *long_name; /*!< Long name of option. NULL means no long
                                name. */
    char short_name;       /*!< Short, one letter long, name of option. 0x0
                                means no short name */
    uint32_t type;         /*!< Type of the option's value. See {Types} section. */
    void *set;             /*!< If set non-NULL, the value of the option
                                will be assigned to it. The type of pointer
                                must correspond to the {.type} */
    void (*callback)(void);/*!< Callback called (if non-NULL) when option
                                detected. The type of the callback depends
                                on the {.type} */
    char *desc;            /*!< Description of the option */
};
typedef struct _fer_opt_t fer_opt_t;

static fer_opt_t **opts = NULL;
static size_t opts_len = 0;


static fer_opt_t *findOpt(char *arg);
static fer_opt_t *findOptLong(char *arg);
static fer_opt_t *findOptShort(char arg);

static void optNoArg(fer_opt_t *opt);
static int optArg(fer_opt_t *opt, const char *arg);
static void invalidOptErr(const fer_opt_t *opt);

static const char *strend(const char *str);

int ferOptsAdd(const char *long_name, char short_name,
               uint32_t type, void *set, void (*callback)(void))
{
    size_t i;

    if (!long_name && !short_name)
        return -1;

    opts_len += 1;
    opts = FER_REALLOC_ARR(opts, fer_opt_t *, opts_len);


    i = opts_len - 1;
    opts[i] = FER_ALLOC(fer_opt_t);
    opts[i]->long_name  = long_name;
    opts[i]->short_name = short_name;
    opts[i]->type       = type;
    opts[i]->set        = set;
    opts[i]->callback   = callback;
    opts[i]->desc       = NULL;

    if (opts[i]->type == FER_OPTS_NONE && opts[i]->set)
        *(int *)opts[i]->set = 0;

    return i;
}

int ferOptsAddDesc(const char *long_name, char short_name,
                   uint32_t type, void *set, void (*callback)(void),
                   const char *desc)
{
    size_t id, desclen;
    fer_opt_t *opt;
   
    id = ferOptsAdd(long_name, short_name, type, set, callback);
    if (id < 0)
        return -1;

    opt = opts[id];
    if (desc){
        desclen = strlen(desc);
        opt->desc = FER_ALLOC_ARR(char, desclen + 1);
        strcpy(opt->desc, desc);
    }

    return id;
}

void ferOptsClear(void)
{
    size_t i;

    for (i = 0; i < opts_len; i++){
        if (opts[i]->desc)
            FER_FREE(opts[i]->desc);
        FER_FREE(opts[i]);
    }
    FER_FREE(opts);
    opts = NULL;
    opts_len = 0;
}

int ferOpts(int *argc, char **argv)
{
    fer_opt_t *opt;
    int args_remaining;
    int i, ok = 0;
   
    if (*argc <= 1)
        return 0;

    args_remaining = 1;

    for (i = 1; i < *argc; i++){
        opt = findOpt(argv[i]);

        if (opt){
            // found corresponding option
            if (opt->type == FER_OPTS_NONE){
                // option has no argument
                optNoArg(opt);
            }else{
                // option has an argument
                if (i + 1 < *argc){
                    ++i;
                    ok = optArg(opt, argv[i]);
                }else{
                    invalidOptErr(opt);
                    ok = -1;
                }
            }
        }else{
            // no corresponding option
            argv[args_remaining] = argv[i];
            args_remaining += 1;
        }
    }

    *argc = args_remaining;

    return ok;
}

static fer_opt_t *findOpt(char *_arg)
{
    char *arg = _arg;
    fer_opt_t *opt;

    if (arg[0] == '-'){
        if (arg[1] == '-'){
            return findOptLong(arg + 2);
        }else{
            if (arg[1] == 0x0)
                return NULL;

            for (++arg; *arg != 0x0; ++arg){
                opt = findOptShort(*arg);
                if (arg[1] == 0x0){
                    return opt;
                }else if (opt->type == FER_OPTS_NONE){
                    optNoArg(opt);
                }else{
                    fprintf(stderr, "Invalid option %s.\n", _arg);
                    return NULL;
                }
            }
            
        }
    }

    return NULL;
}

static fer_opt_t *findOptLong(char *arg)
{
    size_t i;

    for (i = 0; i < opts_len; i++){
        if (opts[i]->long_name && strcmp(opts[i]->long_name, arg) == 0){
            return opts[i];
        }
    }

    return NULL;
}

static fer_opt_t *findOptShort(char arg)
{
    size_t i;

    for (i = 0; i < opts_len; i++){
        if (arg == opts[i]->short_name){
            return opts[i];
        }
    }

    return NULL;
}

static void optNoArg(fer_opt_t *opt)
{
    void (*cb)(const char *, char);

    if (opt->set){
        *(int *)opt->set = 1;
    }

    if (opt->callback){
        cb = (void (*)(const char *, char))opt->callback;
        cb(opt->long_name, opt->short_name);
    }
}

static void invalidOptErr(const fer_opt_t *opt)
{
    if (opt->long_name){
        fprintf(stderr, "Invalid argument of --%s option.\n", (opt)->long_name);
    }else{ \
        fprintf(stderr, "Invalid argument of -%c option.\n", (opt)->short_name);
    }
}

#define _optArgLong(opt, arg, type) \
    void (*cb)(const char *, char, type); \
    long val; \
    \
    if (ferParseLong((arg), strend(arg), &val, NULL) != 0){ \
        invalidOptErr(opt); \
        return -1; \
    } \
    \
    if ((opt)->set){ \
        *(type *)(opt)->set = (type)val; \
    } \
    \
    if ((opt)->callback){ \
        cb = (void (*)(const char *, char, type))(opt)->callback; \
        cb((opt)->long_name, (opt)->short_name, (type)val); \
    } \
    \
    return 0

static int optArgLong(fer_opt_t *opt, const char *arg)
{
    _optArgLong(opt, arg, long);
}

static int optArgInt(fer_opt_t *opt, const char *arg)
{
    _optArgLong(opt, arg, int);
}

static int optArgSizeT(fer_opt_t *opt, const char *arg)
{
    _optArgLong(opt, arg, size_t);
}

static int optArgReal(fer_opt_t *opt, const char *arg)
{
    void (*cb)(const char *, char, fer_real_t);
    fer_real_t val;

    if (ferParseReal(arg, strend(arg), &val, NULL) != 0){
        if (opt->long_name){
            fprintf(stderr, "Invalid argument of --%s option.\n", opt->long_name);
        }else{
            fprintf(stderr, "Invalid argument of -%c option.\n", opt->short_name);
        }
        return -1;
    }

    if (opt->set){
        *(fer_real_t *)opt->set = val;
    }

    if (opt->callback){
        cb = (void (*)(const char *, char, fer_real_t))opt->callback;
        cb(opt->long_name, opt->short_name, val);
    }

    return 0;
}

static int optArgV2(fer_opt_t *opt, const char *arg)
{
    void (*cb)(const char *, char, const fer_vec2_t *);
    fer_real_t val;
    fer_vec2_t v2;
    const char *arg2;

    arg2 = arg;
    while (*arg2 != 0x0 && *arg2 != ',')
        ++arg2;

    if (*arg2 != ','){
        goto optArgV2_err;
    }

    if (ferParseReal(arg, arg2, &val, NULL) != 0){
        goto optArgV2_err;
    }
    ferVec2SetX(&v2, val);

    ++arg2;
    if (ferParseReal(arg2, strend(arg2), &val, NULL) != 0){
        goto optArgV2_err;
    }
    ferVec2SetY(&v2, val);

    if (opt->set){
        ferVec2Copy((fer_vec2_t *)opt->set, &v2);
    }

    if (opt->callback){
        cb = (void (*)(const char *, char, const fer_vec2_t *))opt->callback;
        cb(opt->long_name, opt->short_name, &v2);
    }

    return 0;

optArgV2_err:
    if (opt->long_name){
        fprintf(stderr, "Invalid argument of --%s option.\n", opt->long_name);
    }else{
        fprintf(stderr, "Invalid argument of -%c option.\n", opt->short_name);
    }
    return -1;
}

static int optArgStr(fer_opt_t *opt, const char *arg)
{
    void (*cb)(const char *, char, const char *);

    if (opt->set){
        *(const char **)opt->set = arg;
    }

    if (opt->callback){
        cb = (void (*)(const char *, char, const char *))opt->callback;
        cb(opt->long_name, opt->short_name, arg);
    }

    return 0;
}

static int optArg(fer_opt_t *opt, const char *arg)
{
    switch(opt->type){
        case FER_OPTS_LONG:
            return optArgLong(opt, arg);
        case FER_OPTS_INT:
            return optArgInt(opt, arg);
        case FER_OPTS_REAL:
            return optArgReal(opt, arg);
        case FER_OPTS_STR:
            return optArgStr(opt, arg);
        case FER_OPTS_SIZE_T:
            return optArgSizeT(opt, arg);
        case FER_OPTS_V2:
            return optArgV2(opt, arg);
        default:
            return -1;
    }

    return 0;
}

static const char *strend(const char *str)
{
    const char *s = str;
    while (*s != 0x0)
        ++s;
    return s;
}


/** Returns maximal length of name parts */
static size_t optsNameLen(void)
{
    size_t i, len, name_len;

    name_len = 0;
    for (i = 0; i < opts_len; i++){
        len = 0;
        if (opts[i]->long_name && opts[i]->short_name){
            // e.g., "-h / --help"
            len = 7; // "-h / --"
            len += strlen(opts[i]->long_name);
        }else if (opts[i]->long_name){
            // e.g., --help
            len = 2 + strlen(opts[i]->long_name);
        }else if (opts[i]->short_name){
            // e.g., -h
            len = 2;
        }

        if (len > name_len)
            name_len = len;
    }

    return name_len;
}

static void printName(fer_opt_t *opt, size_t len, FILE *out)
{
    size_t l = 0;

    if (opt->short_name){
        fprintf(out, "-%c / ", opt->short_name);
        l = 5;
    }

    if (opt->long_name){
        fprintf(out, "--%s", opt->long_name);
        l += 2 + strlen(opt->long_name);
    }

    for (; l < len; l++){
        fprintf(out, " ");
    }
}

static void printType(fer_opt_t *opt, FILE *out)
{
    switch(opt->type){
        case FER_OPTS_NONE:
            fprintf(out, "    ");
            break;
        case FER_OPTS_LONG:
            fprintf(out, "long");
            break;
        case FER_OPTS_INT:
            fprintf(out, "int ");
            break;
        case FER_OPTS_REAL:
            fprintf(out, "flt ");
            break;
        case FER_OPTS_STR:
            fprintf(out, "str ");
            break;
        case FER_OPTS_SIZE_T:
            fprintf(out, "uint");
            break;
        case FER_OPTS_V2:
            fprintf(out, "vec2");
            break;
    }
}

static void printDesc(fer_opt_t *opt, FILE *out)
{
    if (!opt->desc)
        return;

    fprintf(out, opt->desc);
}

void ferOptsPrint(FILE *out, const char *lineprefix)
{
    size_t i;
    size_t name_len;

    name_len = optsNameLen();

    // print option descriptions
    for (i = 0; i < opts_len; i++){
        // print line prefix first
        fprintf(out, "%s", lineprefix);

        // then print name
        printName(opts[i], name_len, out);
        fprintf(out, "  ");

        // print type
        printType(opts[i], out);
        fprintf(out, "  ");

        // print description
        printDesc(opts[i], out);

        fprintf(out, "\n");
    }
}
