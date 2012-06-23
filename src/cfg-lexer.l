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

%{
#include <boruvka/cfg.h>
#include <boruvka/parse.h>
#include "src/cfg-lexer.h"

#define YY_DECL uint16_t yylex(yyscan_t yyscanner)

#define F_TYPE(TYPE) \
    yyextra->type = yytext; \
    yyextra->lineno = yylineno; \
    return T_TYPE_ ## TYPE

%}

%option noyywrap
%option yylineno
%option reentrant
%option nounput
%option noinput
%option extra-type="bor_cfg_lex_t *"

DIGIT [0-9]
NONZERO [1-9]
ZERO "0"
SIGN [-+]
DOT "."
WHITESPACE [ \t\n]
NWHITESPACE [^ \t\n]
COMMENT "#"
EXP [eE]
EOL ([ \t]*\n[ \t]*)+

NAME [a-zA-Z_][a-zA-Z0-9_-]*
TYPE_STR     \:s
TYPE_FLT     \:f
TYPE_INT     \:i
TYPE_V2      \:v2
TYPE_V3      \:v3
TYPE_STR_ARR \:s\[[1-9][0-9]*\]
TYPE_FLT_ARR \:f\[[1-9][0-9]*\]
TYPE_INT_ARR \:i\[[1-9][0-9]*\]
TYPE_V2_ARR  \:v2\[[1-9][0-9]*\]
TYPE_V3_ARR  \:v3\[[1-9][0-9]*\]

ASSIGN {WHITESPACE}*={WHITESPACE}*

VAL_STR '.*'
VAL_FLT {SIGN}?({ZERO}|{NONZERO}{DIGIT}*)({DOT}{DIGIT}*)?({EXP}{SIGN}?{DIGIT}+)?
VAL_INT {SIGN}?({ZERO}|{NONZERO}{DIGIT}*)

%s val val_int
%%

<INITIAL,val,val_int>{NAME} {
    BEGIN(INITIAL);
    yyextra->name = yytext;
    yyextra->namelen = yyleng;
    yyextra->type = NULL;
    yyextra->lineno = yylineno;
    return T_NAME;
}

<INITIAL>{TYPE_STR} {
    F_TYPE(STR);
}
<INITIAL>{TYPE_FLT} {
    F_TYPE(FLT);
}
<INITIAL>{TYPE_INT} {
    BEGIN(val_int);
    F_TYPE(INT);
}
<INITIAL>{TYPE_V2} {
    F_TYPE(VV);
}
<INITIAL>{TYPE_V3} {
    F_TYPE(VVV);
}
<INITIAL>{TYPE_STR_ARR} {
    F_TYPE(STR_ARR);
}
<INITIAL>{TYPE_FLT_ARR} {
    F_TYPE(FLT_ARR);
}
<INITIAL>{TYPE_INT_ARR} {
    BEGIN(val_int);
    F_TYPE(INT_ARR);
}
<INITIAL>{TYPE_V2_ARR} {
    F_TYPE(VV_ARR);
}
<INITIAL>{TYPE_V3_ARR} {
    F_TYPE(VVV_ARR);
}

<INITIAL>{ASSIGN} {
    BEGIN(val);
    // TODO
}

<val_int>{ASSIGN} {
}

<val>{VAL_STR} {
    yyextra->str = yytext;
    yyextra->strlen = yyleng;
    yyextra->lineno = yylineno;
    return T_STR;
}
<val>{VAL_FLT} {
    yyextra->lineno = yylineno;
    yyextra->str = yytext;
    yyextra->strlen = yyleng;
    if (borParseReal(yytext, yytext + yyleng, &yyextra->flt, NULL) != 0){
        return T_ERROR;
    }
    return T_FLT;
}
<val_int>{VAL_INT} {
    yyextra->lineno = yylineno;
    yyextra->str = yytext;
    yyextra->strlen = yyleng;
    if (borParseLong(yytext, yytext + yyleng, &yyextra->integer, NULL) != 0){
        return T_ERROR;
    }
    return T_INT;
}
<val>\n{WHITESPACE}*\n {
    BEGIN(INITIAL);
}

{WHITESPACE}+ {}

{COMMENT}[^\n]*\n {}

. {
    yyextra->lineno = yylineno;
    yyextra->str = yytext;
    yyextra->strlen = yyleng;
    return T_ERROR;
}
%%
