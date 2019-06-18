/***
 * boruvka
 * --------
 * Copyright (c)2018 Daniel Fiser <danfis@danfis.cz>,
 * Faculty of Electrical Engineering, Czech Technical University in Prague.
 * All rights reserved.
 *
 * This file is part of boruvka.
 *
 * Distributed under the OSI-approved BSD License (the "License");
 * see accompanying file BDS-LICENSE for details or see
 * <http://www.opensource.org/licenses/bsd-license.php>.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

#ifndef __BOR_ERR_H__
#define __BOR_ERR_H__

#include <boruvka/timer.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Maximal length of an error message */
#define BOR_ERR_MSG_MAXLEN 256
/** Maximal length of an error prefix */
#define BOR_ERR_MSG_PREFIX_MAXLEN 32
/** Maximal depth of a trace */
#define BOR_ERR_TRACE_DEPTH 32

struct bor_err_trace {
    const char *filename;
    int line;
    const char *func;
};
typedef struct bor_err_trace bor_err_trace_t;

struct bor_err {
    bor_err_trace_t trace[BOR_ERR_TRACE_DEPTH];
    int trace_depth;
    int trace_more;
    char msg_prefix[BOR_ERR_MSG_PREFIX_MAXLEN];
    char msg[BOR_ERR_MSG_MAXLEN];
    int err;

    FILE *warn_out;
    FILE *info_out;
    int info_print_resources_disabled;
    bor_timer_t info_timer;
    int info_timer_init;
};
typedef struct bor_err bor_err_t;

#define BOR_ERR_INIT { 0 }

/**
 * Initialize error structure.
 */
void borErrInit(bor_err_t *err);

/**
 * Set error prefix that is printed on the error line.
 */
void borErrSetPrefix(bor_err_t *err, const char *prefix);

/**
 * Returns true if an error message is set.
 */
int borErrIsSet(const bor_err_t *err);

/**
 * Print the stored error message.
 */
void borErrPrint(const bor_err_t *err, int with_traceback, FILE *fout);

/**
 * Enable/disable warnings.
 * Sets the output stream, if fout is NULL the warnings are disabled.
 */
void borErrWarnEnable(bor_err_t *err, FILE *fout);

/**
 * Enable/disable info messages.
 */
void borErrInfoEnable(bor_err_t *err, FILE *fout);

/**
 * Disable printing resources with BOR_INFO
 */
void borErrInfoDisablePrintResources(bor_err_t *err, int disable);



/**
 * Sets error message and starts tracing the calls.
 */
#define BOR_ERR(E, format, ...) \
    _borErr((E), __FILE__, __LINE__, __func__, format, __VA_ARGS__)
#define BOR_ERR2(E, msg) \
    _borErr((E), __FILE__, __LINE__, __func__, msg)

/**
 * Same as BOR_ERR() but also returns the value V immediatelly.
 */
#define BOR_ERR_RET(E, V, format, ...) do { \
        BOR_ERR((E), format, __VA_ARGS__); \
        return (V); \
    } while (0)
#define BOR_ERR_RET2(E, V, msg) do { \
        BOR_ERR2((E), msg); \
        return (V); \
    } while (0)


/**
 * Fatal error that causes exit.
 */
#define BOR_FATAL(format, ...) do { \
        fprintf(stderr, "FATAL ERROR: %s:%d [%s]: " format, \
                __FILE__, __LINE__, __func__, __VA_ARGS__); \
        exit(-1); \
    } while (0)
#define BOR_FATAL2(msg) do { \
        fprintf(stderr, "FATAL ERROR: %s:%d [%s]: " msg, \
                __FILE__, __LINE__, __func__); \
        exit(-1); \
    } while (0)

/**
 * Prints warning.
 */
#define BOR_WARN(E, format, ...) \
    _borWarn((E), __FILE__, __LINE__, __func__, format, __VA_ARGS__)
#define BOR_WARN2(E, msg) \
    _borWarn((E), __FILE__, __LINE__, __func__, msg)

/**
 * Prints info line with timestamp.
 */
#define BOR_INFO(E, format, ...) \
    _borInfo((E), __FILE__, __LINE__, __func__, format, __VA_ARGS__)
#define BOR_INFO2(E, msg) \
    _borInfo((E), __FILE__, __LINE__, __func__, msg)

/**
 * Trace the error -- record the current file, line and function.
 */
#define BOR_TRACE(E) \
   _borTrace((E), __FILE__, __LINE__, __func__)

/**
 * Same as BOR_TRACE() but also returns the value V.
 */
#define BOR_TRACE_RET(E, V) do { \
        BOR_TRACE(E); \
        return (V); \
    } while (0)

/**
 * Prepends the message before the current error message and trace the
 * call.
 */
#define BOR_TRACE_PREPEND(E, format, ...) do { \
        _borErrPrepend((E), format, __VA_ARGS__); \
        BOR_TRACE(E); \
    } while (0)
#define BOR_TRACE_PREPEND_RET(E, V, format, ...) do { \
        _borErrPrepend((E), format, __VA_ARGS__); \
        BOR_TRACE_RET((E), V); \
    } while (0)


void _borErr(bor_err_t *err, const char *filename, int line, const char *func,
             const char *format, ...);
void _borErrPrepend(bor_err_t *err, const char *format, ...);
void _borTrace(bor_err_t *err, const char *fn, int line, const char *func);
void _borWarn(bor_err_t *err, const char *filename, int line, const char *func,
              const char *format, ...);
void _borInfo(bor_err_t *err, const char *filename, int line, const char *func,
              const char *format, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_ERR_H__ */
