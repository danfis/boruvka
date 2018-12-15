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

#include <sys/resource.h>
#include <strings.h>
#include <string.h>
#include "boruvka/err.h"

static void borErrPrintMsg(const bor_err_t *err, FILE *fout)
{
    if (!err->err)
        return;

    if (err->msg[0] == 0x0){
        fprintf(fout, "Error: ");
    }else{
        fprintf(fout, "%s", err->msg_prefix);
    }
    fprintf(fout, "%s\n", err->msg);
    fflush(fout);
}

static void borErrPrintTraceback(const bor_err_t *err, FILE *fout)
{
    if (!err->err)
        return;

    for (int i = 0; i < err->trace_depth; ++i){
        for (int j = 0; j < i; ++j)
            fprintf(fout, "  ");
        fprintf(fout, "  ");
        fprintf(fout, "%s:%d (%s)\n",
                err->trace[i].filename,
                err->trace[i].line,
                err->trace[i].func);
    }
    fflush(fout);
}

void borErrInit(bor_err_t *err)
{
    bzero(err, sizeof(*err));
}

void borErrSetPrefix(bor_err_t *err, const char *prefix)
{
    strncpy(err->msg_prefix, prefix, BOR_ERR_MSG_PREFIX_MAXLEN - 1);
    // err->msg_prefix[BOR_ERR_MSG_PREFIX_MAXLEN - 1] is always set to 0
    // from the initialization
}

int borErrIsSet(const bor_err_t *err)
{
    return err->err;
}

void borErrPrint(const bor_err_t *err, int with_traceback, FILE *fout)
{
    borErrPrintMsg(err, fout);
    if (with_traceback)
        borErrPrintTraceback(err, fout);
}

void borErrWarnEnable(bor_err_t *err, FILE *fout)
{
    err->warn_out = fout;
}

void borErrInfoEnable(bor_err_t *err, FILE *fout)
{
    err->info_out = fout;
}

void borErrInfoDisablePrintResources(bor_err_t *err, int disable)
{
    err->info_print_resources_disabled = disable;
}


void _borErr(bor_err_t *err, const char *filename, int line, const char *func,
             const char *format, ...)
{
    if (err == NULL)
        return;

    va_list ap;

    err->trace[0].filename = filename;
    err->trace[0].line = line;
    err->trace[0].func = func;
    err->trace_depth = 1;
    err->trace_more = 0;

    va_start(ap, format);
    vsnprintf(err->msg, BOR_ERR_MSG_MAXLEN, format, ap);
    va_end(ap);
    err->err = 1;
}

void _borErrPrepend(bor_err_t *err, const char *format, ...)
{
    if (err == NULL)
        return;

    va_list ap;
    char msg[BOR_ERR_MSG_MAXLEN];
    int size;

    strcpy(msg, err->msg);
    va_start(ap, format);
    size = vsnprintf(err->msg, BOR_ERR_MSG_MAXLEN, format, ap);
    snprintf(err->msg + size, BOR_ERR_MSG_MAXLEN - size, "%s", msg);
    va_end(ap);

}

void _borTrace(bor_err_t *err, const char *filename, int line, const char *func)
{
    if (err == NULL)
        return;

    if (err->trace_depth == BOR_ERR_TRACE_DEPTH){
        err->trace_more = 1;
    }else{
        err->trace[err->trace_depth].filename = filename;
        err->trace[err->trace_depth].line = line;
        err->trace[err->trace_depth].func = func;
        ++err->trace_depth;
    }
}

void _borWarn(bor_err_t *err, const char *filename, int line, const char *func,
              const char *format, ...)
{
    if (err == NULL)
        return;

    va_list ap;

    if (err->warn_out == NULL)
        return;

    va_start(ap, format);
    fprintf(err->warn_out, "Warning: %s:%d [%s]: ", filename, line, func);
    vfprintf(err->warn_out, format, ap);
    va_end(ap);
    fprintf(err->warn_out, "\n");
    fflush(err->warn_out);
}

void _borInfo(bor_err_t *err, const char *filename, int line, const char *func,
              const char *format, ...)
{
    if (err == NULL)
        return;

    struct rusage usg;
    long peak_mem = 0L;
    va_list ap;

    if (err->info_out == NULL)
        return;
    if (!err->info_timer_init){
        borTimerStart(&err->info_timer);
        err->info_timer_init = 1;
    }

    if (getrusage(RUSAGE_SELF, &usg) == 0)
        peak_mem = usg.ru_maxrss / 1024L;
    va_start(ap, format);
    borTimerStop(&err->info_timer);
    if (!err->info_print_resources_disabled)
        fprintf(err->info_out, "[%.3fs %ldMB] ",
                borTimerElapsedInSF(&err->info_timer), peak_mem);
    vfprintf(err->info_out, format, ap);
    va_end(ap);
    fprintf(err->info_out, "\n");
    fflush(err->info_out);
}
