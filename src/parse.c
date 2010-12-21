#include "mg/parse.h"
#include "mg/dbg.h"

#define NOT_WS(c) \
    ( c != ' ' && c != '\t' && c != '\n')

/* Implementation taken from SVT project. */
int mgParseReal(const char *str, const char *strend, mg_real_t *val, char **next)
{
    char c;
    mg_real_t fract;
    mg_real_t mult;
    mg_real_t num;
    int negative = 0;
    int has_e;

    if (str >= strend)
        return -1;

    // skip initial whitespace
    while (!NOT_WS(*str))
        ++str;

    c = *str;
    *val = MG_ZERO;

    /* process sign */
    if (c == '-'){
        negative = 1;
        c = *++str;
    }else if (c == '+')
        c = *++str;

    /* process initial digits */
    while (c != 0 && NOT_WS(c) && str < strend){
        /* skip to next part */
        if (c == '.' || c == 'e' || c == 'E')
            break;

        /* not number -> invalid string */
        if (c < 48 || c > 57)
            return -1;

        *val = *val * MG_REAL(10.) + (c - 48);
        c = *++str;
    }

    if (!NOT_WS(c) || str >= strend){
        if (next)
            *next = (char *)str;
        return 0;
    }

    /* process decimal part */
    if (c == '.'){
        c = *++str;
        mult = MG_REAL(0.1);
        fract = MG_ZERO;
        while (c != 0 && NOT_WS(c) && str < strend){
            /* skip to next part */
            if (c == 'e' || c == 'E')
                break;

            /* no digit -> invalid string */
            if (c < 48 || c > 57)
                return -1;

            fract = fract + mult * (c - 48);
            mult *= MG_REAL(0.1);
            c = *++str;
        }
        *val += fract;
    }

    /* apply negative flag */
    if (negative)
        *val = *val * MG_REAL(-1.);

    if (!NOT_WS(c) || str >= strend){
        if (next)
            *next = (char *)str;
        return 0;
    }

    /* process exponent part */
    has_e = 0;
    if (c == 'e' || c == 'E'){
        c = *++str;
        negative = 0;
        num = MG_ZERO;

        if (c == '-'){
            negative = 1;
            c = *++str;
        }else if (c == '+')
            c = *++str;

        while (c != 0 && NOT_WS(c) && str < strend){
            if (c < 48 || c > 57)
                return -1;

            num = num * MG_REAL(10.) + (c - 48);
            c = *++str;
            has_e = 1;
        }

        if (negative)
            num *= MG_REAL(-1.);

        if (has_e){
            mult = MG_POW(MG_REAL(10.), num);
            *val *= mult;
        }
    }

    if (next)
        *next = (char *)str;

    return 0;
}


int mgParseVec3(const char *_str, const char *strend, mg_vec3_t *vec, char **n)
{
    mg_real_t v[3];
    size_t i;
    char *str, *next;

    str = (char *)_str;
    for (i = 0; i < 3 && str < strend; i++){
        if (mgParseReal(str, strend, &v[i], &next) != 0)
            break;
        str = next;
    }

    if (i != 3)
        return -1;

    mgVec3Set(vec, v[0], v[1], v[2]);
    if (n)
        *n = str;

    return 0;
}
