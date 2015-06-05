#ifndef __BOR_CONFIG_H__
#define __BOR_CONFIG_H__

#include <boruvka/config_endian.h>

ifdef(`USE_SINGLE', `#define BOR_SINGLE')
ifdef(`USE_DOUBLE', `#define BOR_DOUBLE')
ifdef(`USE_MEMCHECK', `#define BOR_MEMCHECK')
ifdef(`USE_MEMCHECK', `#define BOR_MEMCHECK_REPORT_THRESHOLD' MEMCHECK_REPORT_THRESHOLD)
ifdef(`USE_SSE', `#define BOR_SSE')
ifdef(`USE_SSE', `ifdef(`USE_SINGLE', `#define BOR_SSE_SINGLE', `#define BOR_SSE_DOUBLE')')
ifdef(`DEBUG', `#define BOR_DEBUG')
ifdef(`USE_OPENCL', `#define BOR_OPENCL')
ifdef(`USE_HDF5', `#define BOR_HDF5')
ifdef(`USE_GSL', `#define BOR_GSL')
ifdef(`CD_TIME_MEASURE', `#define BOR_CD_TIME_MEASURE')

#endif /* __BOR_CONFIG_H__ */
