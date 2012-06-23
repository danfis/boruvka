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

#ifndef __BOR_CFG_LEXER_H__
#define __BOR_CFG_LEXER_H__

struct _bor_cfg_lex_t {
    const char *str;
    size_t strlen;
    const char *name;
    size_t namelen;
    const char *type;
    bor_real_t flt;
    long integer;
    unsigned int lineno;
};
typedef struct _bor_cfg_lex_t bor_cfg_lex_t;

#define T_NAME          1
#define T_ERROR         2
#define T_TYPE_STR     (0x8000 | 0x010)
#define T_TYPE_FLT     (0x8000 | 0x020)
#define T_TYPE_INT     (0x8000 | 0x030)
#define T_TYPE_VV      (0x8000 | 0x040)
#define T_TYPE_VVV     (0x8000 | 0x050)
#define T_TYPE_STR_ARR (0x8000 | 0x011)
#define T_TYPE_FLT_ARR (0x8000 | 0x021)
#define T_TYPE_INT_ARR (0x8000 | 0x031)
#define T_TYPE_VV_ARR  (0x8000 | 0x041)
#define T_TYPE_VVV_ARR (0x8000 | 0x051)
#define T_STR          (0x4000 | 0x100)
#define T_FLT          (0x4000 | 0x200)
#define T_INT          (0x4000 | 0x300)

#define T_IS_TYPE(tok) \
    ((tok & 0x8000) == 0x8000)
#define T_IS_VAL(tok) \
    ((tok & 0x4000) == 0x4000)

#endif /* __BOR_CFG_LEXER_H__ */

