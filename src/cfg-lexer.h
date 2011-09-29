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

#ifndef __FER_CFG_LEXER_H__
#define __FER_CFG_LEXER_H__

struct _fer_cfg_lex_t {
    const char *str;
    size_t strlen;
    const char *name;
    size_t namelen;
    const char *type;
    fer_real_t flt;
    unsigned int lineno;
};
typedef struct _fer_cfg_lex_t fer_cfg_lex_t;

#define T_NAME          1
#define T_ERROR         2
#define T_TYPE_STR     (0x80 | 3)
#define T_TYPE_FLT     (0x80 | 4)
#define T_TYPE_VV      (0x80 | 5)
#define T_TYPE_VVV     (0x80 | 6)
#define T_TYPE_STR_ARR (0x80 | 7)
#define T_TYPE_FLT_ARR (0x80 | 8)
#define T_TYPE_VV_ARR  (0x80 | 9)
#define T_TYPE_VVV_ARR (0x80 | 10)
#define T_STR          (0x40 | 11)
#define T_FLT          (0x40 | 12)

#define T_IS_TYPE(tok) \
    ((tok & 0x80) == 0x80)
#define T_IS_VAL(tok) \
    ((tok & 0x40) == 0x40)

#endif /* __FER_CFG_LEXER_H__ */

