/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#include <string.h>
#include <fermat/opts.h>
#include <fermat/parse.h>
#include <fermat/dbg.h>
#include <fermat/alloc.h>

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
};
typedef struct _fer_opt_t fer_opt_t;

static fer_opt_t **opts = NULL;
static size_t opts_len = 0;


static fer_opt_t *findOpt(char *arg);
static fer_opt_t *findOptLong(char *arg);
static fer_opt_t *findOptShort(char arg);

static void optNoArg(fer_opt_t *opt);
static int optArg(fer_opt_t *opt, const char *arg);

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

    if (opts[i]->type == FER_OPTS_NONE && opts[i]->set)
        *(int *)opts[i]->set = 0;

    return i;
}

void ferOptsClear(void)
{
    size_t i;

    for (i = 0; i < opts_len; i++){
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

static fer_opt_t *findOpt(char *arg)
{
    if (arg[0] == '-'){
        if (arg[1] == '-'){
            return findOptLong(arg + 2);
        }else if (arg[1] != 0x0 && arg[2] == 0x0){
            return findOptShort(arg[1]);
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

static int optArgLong(fer_opt_t *opt, const char *arg)
{
    void (*cb)(const char *, char, long);
    long val;

    if (ferParseLong(arg, strend(arg), &val, NULL) != 0){
        if (opt->long_name){
            fprintf(stderr, "Invalid argument of --%s option.\n", opt->long_name);
        }else{
            fprintf(stderr, "Invalid argument of -%c option.\n", opt->short_name);
        }
        return -1;
    }

    if (opt->set){
        *(long *)opt->set = val;
    }

    if (opt->callback){
        cb = (void (*)(const char *, char, long))opt->callback;
        cb(opt->long_name, opt->short_name, val);
    }

    return 0;
}

static int optArgInt(fer_opt_t *opt, const char *arg)
{
    void (*cb)(const char *, char, int);
    long val;

    if (ferParseLong(arg, strend(arg), &val, NULL) != 0){
        if (opt->long_name){
            fprintf(stderr, "Invalid argument of --%s option.\n", opt->long_name);
        }else{
            fprintf(stderr, "Invalid argument of -%c option.\n", opt->short_name);
        }
        return -1;
    }

    if (opt->set){
        *(int *)opt->set = (int)val;
    }

    if (opt->callback){
        cb = (void (*)(const char *, char, int))opt->callback;
        cb(opt->long_name, opt->short_name, (int)val);
    }

    return 0;
}

static int optArgFlt(fer_opt_t *opt, const char *arg)
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
        case FER_OPTS_FLT:
            return optArgFlt(opt, arg);
        case FER_OPTS_STR:
            return optArgStr(opt, arg);
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
