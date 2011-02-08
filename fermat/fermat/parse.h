/***
 * fermat
 * -------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __FER_PARSE_H__
#define __FER_PARSE_H__

#include <fermat/vec2.h>
#include <fermat/vec3.h>
#include <fermat/vec4.h>

/**
 * Parse
 * ======
 */

/**
 * Parses given string that starts at str ands ends at strend (exclusively)
 * into floating point number *val* and if next is non-NULL stores there
 * pointer to string after parsed number.
 * Returns 0 on success, -1 otherwise.
 */
int ferParseReal(const char *str, const char *strend, fer_real_t *val, char **next);

/**
 * Parses given string that starts at str and ends at strend (exclusively)
 * into given vector and if next is not NULL stores there pointer after
 * parsed vector. Vector must formatted as three floating point numbers
 * separated by whitespaces.
 * Returns 0 on success -1 otherwise.
 */
int ferParseVec3(const char *str, const char *strend, fer_vec3_t *v, char **next);

/**
 * Similar function to ferParseVec3() but parses 2D vectors.
 */
int ferParseVec2(const char *str, const char *strend, fer_vec2_t *v, char **next);

/**
 * Similar function to ferParseVec3() but parses 4D vectors.
 */
int ferParseVec4(const char *str, const char *strend, fer_vec4_t *v, char **next);

/**
 * Parses given string that starts at str ands ends at strend (exclusively)
 * into long int number *val* and if next is non-NULL stores there
 * pointer to string after parsed number.
 * Returns 0 on success, -1 otherwise.
 */
int ferParseLong(const char *str, const char *strend, long *val, char **next);

#endif /* __FER_PARSE_H__ */
