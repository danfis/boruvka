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

#ifndef __BOR_OPTS_H__
#define __BOR_OPTS_H__

#include <stdio.h>
#include <boruvka/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Parsing Command Line Options
 * =============================
 */


/**
 * Example
 * --------
 * ~~~~~
 * #include <stdio.h>
 * #include <boruvka/opts.h>
 *
 * static void helpcb(const char *l, char s)
 * {
 *     fprintf(stderr, "HelpCB: %s %c\n", l, s);
 *     fprintf(stderr, "\n");
 * }
 *
 * int main(int argc, char *argv[])
 * {
 *     bor_real_t opt1;
 *     int help;
 *     int i;
 *
 *     // define options
 *     borOptsAdd("opt1", 'o', BOR_OPTS_REAL, (void *)&opt1, NULL);
 *     borOptsAdd("help", 'h', BOR_OPTS_NONE, (void *)&help, BOR_OPTS_CB(helpcb));
 *
 *     // parse options
 *     borOpts(&argc, argv);
 *
 *     // print some info
 *     fprintf(stdout, "help: %d\n", help);
 *     fprintf(stdout, "opt1: %f\n", (float)opt1);
 *
 *     // print the rest of options
 *     for (i = 0; i < argc; i++){
 *         fprintf(stdout, "[%02d]: `%s'\n", argv[i]);
 *     }
 *
 *     return 0;
 * }
 *
 * // If compiled to program called test:
 * // $ ./test
 * // > help: 0
 * // > opt1: 0.000000
 * // > [00]: `./test'
 * //
 * // $ ./test --opt1 1.1 --opt
 * // > help: 0
 * // > opt1: 1.100000
 * // > [00]: `./test'
 * // > [01]: `--opt'
 * //
 * // $ ./test -o 2.2 -h
 * // > HelpCB: help h
 * // > 
 * // > help: 1
 * // > opt1: 2.200000
 * // > [00]: `./test'
 * ~~~~~
 *
 *
 * Types
 * ------
 */

/** vvvv */
/**
 * No argument.
 *     1. .set must be [int *] and is set to 1 if option was found and to 0
 *        if it wasn't.
 *     2. .callback must have type void (*)(const char *long_name, char short_name)
 */
#define BOR_OPTS_NONE 0x00

/**
 * Long type.
 *     1. .set must have type [long *]
 *     2. .callback must have type void (*)(const char *long_name, char short_name, long val)
 */
#define BOR_OPTS_LONG 0x01

/**
 * Int.
 *     1. .set - [int *]
 *     2. .callback - void (*)(const char *long_name, char short_name, int val)
 */
#define BOR_OPTS_INT 0x02

/**
 * Real.
 *     1. .set - [bor_real_t *]
 *     2. .callback - void (*)(const char *long_name, char short_name, bor_real_t val)
 */
#define BOR_OPTS_REAL 0x03

/**
 * String.
 *     1. .set - [const char **]
 *     2. .callback - void (*)(const char *long_name, char short_name, const char *)
 */
#define BOR_OPTS_STR 0x04

/**
 * size_t.
 *     1. .set - [size_t *]
 *     2. .callback - void (*)(const char *long_name, char short_name, size_t)
 */
#define BOR_OPTS_SIZE_T 0x05

/**
 * 2-D vector.
 *     1. .set - [bor_vec2_t *]
 *     2. .callback - void (*)(const char *long_name, char short_name, const bor_vec2_t *)
 */
#define BOR_OPTS_V2 0x06

/** ^^^^ */

/**
 * Functions
 * ----------
 */

/**
 * Use this macro for passing callback to borOptsAdd().
 */
#define BOR_OPTS_CB(func) (void (*)(void))(func)
 
/**
 * Adds description of an option:
 *     1. {long_name}: Long name of option. NULL means no long name.
 *     2. {short_name}: Short, one letter long, name of option. 0x0 means no short name.
 *     3. {type}: Type of the option's value. See {Types} section.
 *     4. {set}: If set non-NULL, the value of the option will be assigned
 *        to it. The type of pointer must correspond to the {type}.
 *     5. {callback}: Callback called (if non-NULL) when option detected.
 *        The type of the callback depends on the {type}.
 *
 * Returns ID of the added option.
 */
int borOptsAdd(const char *long_name, char short_name,
                uint32_t type, void *set, void (*callback)(void));

/**
 * Same as {borOptsAdd()} but has additional parameter {desc} where can be
 * passed string description of the option
 */
int borOptsAddDesc(const char *long_name, char short_name,
                   uint32_t type, void *set, void (*callback)(void),
                   const char *desc);

/**
 * Clears all options previously added
 */
void borOptsClear(void);

/**
 * Parses command line options.
 * The first item of {argv} array is skipped.
 * The arguments {argc} and {argv} are modified to contain only the rest of
 * the options that weren't parsed.
 * Returns 0 if all options were successfully parsed.
 */
int borOpts(int *argc, char **argv);


/**
 * Print list of all options
 */
void borOptsPrint(FILE *out, const char *lineprefix);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_OPTS_H__ */
