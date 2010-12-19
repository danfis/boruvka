#ifndef __MG_CONFIG_H__
#define __MG_CONFIG_H__

ifdef(`USE_SINGLE', `#define MG_SINGLE')
ifdef(`USE_DOUBLE', `#define MG_DOUBLE')
ifdef(`USE_SSE', `#define MG_SSE')
ifdef(`USE_SSE', `ifdef(`USE_SINGLE', `#define MG_SSE_SINGLE', `#define MG_SSE_DOUBLE')')

#endif /* __MG_CONFIG_H__ */
