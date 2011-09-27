#ifndef __FER_CONFIG_H__
#define __FER_CONFIG_H__

ifdef(`USE_SINGLE', `#define FER_SINGLE')
ifdef(`USE_DOUBLE', `#define FER_DOUBLE')
ifdef(`USE_MEMCHECK', `#define FER_MEMCHECK')
ifdef(`USE_SSE', `#define FER_SSE')
ifdef(`USE_SSE', `ifdef(`USE_SINGLE', `#define FER_SSE_SINGLE', `#define FER_SSE_DOUBLE')')
ifdef(`DEBUG', `#define FER_DEBUG')
ifdef(`USE_OPENCL', `#define FER_OPENCL')

#endif /* __FER_CONFIG_H__ */
