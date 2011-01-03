#ifndef __FER_PARSE_H__
#define __FER_PARSE_H__

#include <fermat/vec3.h>

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

#endif /* __FER_PARSE_H__ */
