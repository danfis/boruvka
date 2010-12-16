#ifndef __MG_PARSE_H__
#define __MG_PARSE_H__

#include <mg/vec3.h>

/**
 * Parses given string that starts at str ands ends at strend (exclusively)
 * into floating point number *val* and if next is non-NULL stores there
 * pointer to string after parsed number.
 * Returns 0 on success, -1 otherwise.
 */
int mgParseReal(const char *str, const char *strend, mg_real_t *val, char **next);

/**
 * Parses given string that starts at str and ends at strend (exclusively)
 * into given vector and if next is not NULL stores there pointer after
 * parsed vector. Vector must formatted as three floating point numbers
 * separated by whitespaces.
 * Returns 0 on success -1 otherwise.
 */
int mgParseVec3(const char *str, const char *strend, mg_vec3_t *v, char **next);

#endif /* __MG_PARSE_H__ */
