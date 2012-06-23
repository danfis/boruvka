/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_TIMER_H__
#define __BOR_TIMER_H__

#include <boruvka/core.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Timer
 * ======
 */
struct _bor_timer_t {
    struct timespec t_start;   /*!< Time when timer was started */
    struct timespec t_elapsed; /*!< Elapsed time from start of timer in
                                    moment when borTimerStop() was called */
};
typedef struct _bor_timer_t bor_timer_t;

/**
 * Functions
 * ----------
 */

/**
 * Starts timer.
 *
 * In fact, this only fills .t_start member of bor_timer_t struct.
 */
_bor_inline void borTimerStart(bor_timer_t *t);

/**
 * Stops timer and computes elapsed time from last borTimerStart() call.
 */
_bor_inline void borTimerStop(bor_timer_t *t);

/**
 * Returns elapsed time.
 */
_bor_inline const struct timespec *borTimerElapsed(const bor_timer_t *t);


/**
 * Works as usual fprintf(3) but output is printed with prefix elapsed time
 * in format [minutes:seconds.milisec].
 */
void borTimerPrintElapsed(const bor_timer_t *t, FILE *out,
                          const char *format, ...);
/**
 * Stops timer and prints output same as borTimerPrintElapsed().
 */
void borTimerStopAndPrintElapsed(bor_timer_t *t, FILE *out,
                                 const char *format, ...);

/**
 * Equivalent to borTimerPrintElapsed() expect it is called with va_list
 * instead of variable number of arguments.
 */
_bor_inline void borTimerPrintElapsed2(const bor_timer_t *t, FILE *out,
                                       const char *format, va_list ap);

/**
 * Returns nanosecond part of elapsed time.
 */
_bor_inline long borTimerElapsedNs(const bor_timer_t *t);

/**
 * Returns microsecond part of elapsed time.
 */
_bor_inline long borTimerElapsedUs(const bor_timer_t *t);

/**
 * Returns milisecond part of elapsed time.
 */
_bor_inline long borTimerElapsedMs(const bor_timer_t *t);

/**
 * Returns second part of elapsed time.
 */
_bor_inline long borTimerElapsedS(const bor_timer_t *t);

/**
 * Returns minute part of elapsed time.
 */
_bor_inline long borTimerElapsedM(const bor_timer_t *t);

/**
 * Returns hour part of elapsed time.
 */
_bor_inline long borTimerElapsedH(const bor_timer_t *t);


/**
 * Returns elapsed time in ns.
 */
_bor_inline unsigned long borTimerElapsedInNs(const bor_timer_t *t);

/**
 * Returns elapsed time in us.
 */
_bor_inline unsigned long borTimerElapsedInUs(const bor_timer_t *t);

/**
 * Returns elapsed time in ms.
 */
_bor_inline unsigned long borTimerElapsedInMs(const bor_timer_t *t);

/**
 * Returns elapsed time in s.
 */
_bor_inline unsigned long borTimerElapsedInS(const bor_timer_t *t);

/**
 * Returns elapsed time in m.
 */
_bor_inline unsigned long borTimerElapsedInM(const bor_timer_t *t);

/**
 * Returns elapsed time in h.
 */
_bor_inline unsigned long borTimerElapsedInH(const bor_timer_t *t);


/**
 * Returns elapsed time in s as float number
 */
_bor_inline bor_real_t borTimerElapsedInSF(const bor_timer_t *t);

/**** INLINES ****/
_bor_inline void borTimerStart(bor_timer_t *t)
{
    clock_gettime(CLOCK_MONOTONIC, &t->t_start);
}

_bor_inline void borTimerStop(bor_timer_t *t)
{
    struct timespec cur;
    clock_gettime(CLOCK_MONOTONIC, &cur);

    /* compute diff */
    if (cur.tv_nsec > t->t_start.tv_nsec){
        t->t_elapsed.tv_nsec = cur.tv_nsec - t->t_start.tv_nsec;
        t->t_elapsed.tv_sec = cur.tv_sec - t->t_start.tv_sec;
    }else{
        t->t_elapsed.tv_nsec = cur.tv_nsec + 1000000000L - t->t_start.tv_nsec;
        t->t_elapsed.tv_sec = cur.tv_sec - 1 - t->t_start.tv_sec;
    }
}

_bor_inline const struct timespec *borTimerElapsed(const bor_timer_t *t)
{
    return &t->t_elapsed;
}

_bor_inline void borTimerPrintElapsed2(const bor_timer_t *t, FILE *out,
                                       const char *format, va_list ap)
{
    /* print elapsed time */
    fprintf(out, "[%02ld:%02ld.%03ld]",
            borTimerElapsedM(t),
            borTimerElapsedS(t),
            borTimerElapsedMs(t));

    /* print the rest */
    vfprintf(out, format, ap);
}

_bor_inline long borTimerElapsedNs(const bor_timer_t *t)
{
    return t->t_elapsed.tv_nsec % 1000L;
}

_bor_inline long borTimerElapsedUs(const bor_timer_t *t)
{
    return (t->t_elapsed.tv_nsec / 1000L) % 1000L;
}

_bor_inline long borTimerElapsedMs(const bor_timer_t *t)
{
    return t->t_elapsed.tv_nsec / 1000000L;
}

_bor_inline long borTimerElapsedS(const bor_timer_t *t)
{
    return t->t_elapsed.tv_sec % 60L;
}

_bor_inline long borTimerElapsedM(const bor_timer_t *t)
{
    return (t->t_elapsed.tv_sec / 60L) % 60L;
}

_bor_inline long borTimerElapsedH(const bor_timer_t *t)
{
    return t->t_elapsed.tv_sec / 3600L;
}


_bor_inline unsigned long borTimerElapsedInNs(const bor_timer_t *t)
{
    return t->t_elapsed.tv_nsec + t->t_elapsed.tv_sec * 1000000000L;
}

_bor_inline unsigned long borTimerElapsedInUs(const bor_timer_t *t)
{
    return t->t_elapsed.tv_nsec / 1000L + t->t_elapsed.tv_sec * 1000000L;
}

_bor_inline unsigned long borTimerElapsedInMs(const bor_timer_t *t)
{
    return t->t_elapsed.tv_nsec / 1000000L + t->t_elapsed.tv_sec * 1000L;
}

_bor_inline unsigned long borTimerElapsedInS(const bor_timer_t *t)
{
    return t->t_elapsed.tv_sec;
}

_bor_inline unsigned long borTimerElapsedInM(const bor_timer_t *t)
{
    return t->t_elapsed.tv_sec / 60L;
}

_bor_inline unsigned long borTimerElapsedInH(const bor_timer_t *t)
{
    return t->t_elapsed.tv_sec / 3600L;
}

_bor_inline bor_real_t borTimerElapsedInSF(const bor_timer_t *t)
{
    bor_real_t time;
    time  = t->t_elapsed.tv_nsec / BOR_REAL(1000000000.);
    time += t->t_elapsed.tv_sec;
    return time;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_TIMER_H__ */

