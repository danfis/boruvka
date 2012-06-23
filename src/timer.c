/***
 * Boruvka
 * --------
 * Copyright (c)2010-2012 Daniel Fiser <danfis@danfis.cz>
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

#include <boruvka/timer.h>

void borTimerPrintElapsed(const bor_timer_t *t, FILE *out,
                          const char *format, ...)
{
    va_list ap; 

    va_start(ap, format);
    borTimerPrintElapsed2(t, out, format, ap);
    va_end(ap);
}

void borTimerStopAndPrintElapsed(bor_timer_t *t, FILE *out,
                                 const char *format, ...)
{
    va_list ap; 

    borTimerStop(t);
    va_start(ap, format);
    borTimerPrintElapsed2(t, out, format, ap);
    va_end(ap);
}
