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

%{
#include <fermat/cfg.h>
#include <fermat/parse.h>
#include "src/cfg-lexer.h"

%}

%option noyywrap
%option yylineno
%option reentrant
%option nounput
%option noinput
%option extra-type="fer_cfg_lex_t *"

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
TYPE_V2      \:v2
TYPE_V3      \:v3
TYPE_STR_ARR \:s\[[1-9][0-9]*\]
TYPE_FLT_ARR \:f\[[1-9][0-9]*\]
TYPE_V2_ARR  \:v2\[[1-9][0-9]*\]
TYPE_V3_ARR  \:v3\[[1-9][0-9]*\]

ASSIGN {WHITESPACE}*={WHITESPACE}*

VAL_STR '.*'
VAL_FLT {SIGN}?({ZERO}|{NONZERO}{DIGIT}*)({DOT}{DIGIT}*)?({EXP}{SIGN}?{DIGIT}+)?

%s val
%%

<INITIAL,val>{NAME} {
    BEGIN(INITIAL);
    yyextra->name = yytext;
    yyextra->namelen = yyleng;
    yyextra->type = NULL;
    yyextra->lineno = yylineno;
    return T_NAME;
}

<INITIAL>{TYPE_STR} {
    yyextra->type = yytext;
    yyextra->lineno = yylineno;
    return T_TYPE_STR;
}
<INITIAL>{TYPE_FLT} {
    yyextra->type = yytext;
    yyextra->lineno = yylineno;
    return T_TYPE_FLT;
}
<INITIAL>{TYPE_V2} {
    yyextra->type = yytext;
    yyextra->lineno = yylineno;
    return T_TYPE_VV;
}
<INITIAL>{TYPE_V3} {
    yyextra->type = yytext;
    yyextra->lineno = yylineno;
    return T_TYPE_VVV;
}
<INITIAL>{TYPE_STR_ARR} {
    yyextra->type = yytext;
    yyextra->lineno = yylineno;
    return T_TYPE_STR_ARR;
}
<INITIAL>{TYPE_FLT_ARR} {
    yyextra->type = yytext;
    yyextra->lineno = yylineno;
    return T_TYPE_FLT_ARR;
}
<INITIAL>{TYPE_V2_ARR} {
    yyextra->type = yytext;
    yyextra->lineno = yylineno;
    return T_TYPE_VV_ARR;
}
<INITIAL>{TYPE_V3_ARR} {
    yyextra->type = yytext;
    yyextra->lineno = yylineno;
    return T_TYPE_VVV_ARR;
}

<INITIAL>{ASSIGN} {
    BEGIN(val);
    // TODO
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
    if (ferParseReal(yytext, yytext + yyleng, &yyextra->flt, NULL) != 0){
        return T_ERROR;
    }
    return T_FLT;
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
