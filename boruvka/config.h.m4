#ifndef __BOR_CONFIG_H__
#define __BOR_CONFIG_H__

ifdef(`USE_SINGLE', `#define BOR_SINGLE')
ifdef(`USE_DOUBLE', `#define BOR_DOUBLE')
ifdef(`USE_MEMCHECK', `#define BOR_MEMCHECK')
ifdef(`USE_SSE', `#define BOR_SSE')
ifdef(`USE_SSE', `ifdef(`USE_SINGLE', `#define BOR_SSE_SINGLE', `#define BOR_SSE_DOUBLE')')
ifdef(`DEBUG', `#define BOR_DEBUG')
ifdef(`USE_OPENCL', `#define BOR_OPENCL')
ifdef(`CD_TIME_MEASURE', `#define BOR_CD_TIME_MEASURE')

#endif /* __BOR_CONFIG_H__ */
