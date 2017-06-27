ifdef(`PRELUDE', `PRELUDE')
ifdef(`ARR_NAME',, `define(`ARR_NAME', `v')')dnl
`#include '"HEADER_FILE"

ifdef(`LT', `#define __LT(x, y) '(LT), `#define __LT(x, y) ((x) < (y))')
ifdef(`EQ', `#define __EQ(x, y) '(EQ), `#define __EQ(x, y) ((x) == (y))')

int FUNC_PREFIX`HasId'(const STRUCT_NAME`_t' *s, TYPE v)
{
    // TODO: binary search
    for (int i = 0; i < s->size; ++i){
        if (s->ARR_NAME[i] == v)
            return 1;
    }
    return 0;
}

void FUNC_PREFIX`Add'(STRUCT_NAME`_t' *s, TYPE v)
{
    if (s->size >= s->alloc){
        if (s->alloc == 0)
            s->alloc = 1;
        s->alloc *= 2;
        s->ARR_NAME = BOR_REALLOC_ARR(s->ARR_NAME, TYPE, s->alloc);
    }
    s->ARR_NAME[s->size++] = v;

    if (s->size > 1 && v < s->ARR_NAME[s->size - 2]){
        TYPE *f = s->ARR_NAME + s->size - 1;
        for (; f > s->ARR_NAME && f[0] < f[-1]; --f){
            TYPE tmp = f[0];
            f[0] = f[-1];
            f[-1] = tmp;
        }
        if (f > s->ARR_NAME && f[0] == f[-1]){
            for (--s->size; f != s->ARR_NAME + s->size; ++f)
                *f = f[1];
        }
    }
}

void FUNC_PREFIX`Union'(STRUCT_NAME`_t' *dst, const STRUCT_NAME`_t' *src)
{
    for (int i = 0; i < src->size; ++i)
        FUNC_PREFIX`Add'(dst, src->ARR_NAME[i]);
}

void FUNC_PREFIX`Minus'(STRUCT_NAME`_t' *s1, const STRUCT_NAME`_t' *s2)
{
    int w, i, j;

    for (w = i = j = 0; i < s1->size && j < s2->size;){
        if (s1->ARR_NAME[i] == s2->ARR_NAME[j]){
            ++i;
            ++j;
        }else if (s1->ARR_NAME[i] < s2->ARR_NAME[j]){
            s1->ARR_NAME[w++] = s1->ARR_NAME[i++];
        }else{
            ++j;
        }
    }
    for (; i < s1->size; ++i, ++w)
        s1->ARR_NAME[w] = s1->ARR_NAME[i];
    s1->size = w + s1->size - i;
}

int FUNC_PREFIX`Rm'(STRUCT_NAME`_t' *s, TYPE v)
{
    int i;

    for (i = 0; i < s->size && __LT(s->ARR_NAME[i], v); ++i);
    if (i < s->size && __EQ(s->ARR_NAME[i], v)){
        for (++i; i < s->size; ++i)
            s->ARR_NAME[i - 1] = s->ARR_NAME[i];
        --s->size;
        return 1;
    }
    return 0;
}

