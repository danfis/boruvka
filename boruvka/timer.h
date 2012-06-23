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
                                    moment when ferTimerStop() was called */
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
_fer_inline void ferTimerStart(bor_timer_t *t);

/**
 * Stops timer and computes elapsed time from last ferTimerStart() call.
 */
_fer_inline void ferTimerStop(bor_timer_t *t);

/**
 * Returns elapsed time.
 */
_fer_inline const struct timespec *ferTimerElapsed(const bor_timer_t *t);


/**
 * Works as usual fprintf(3) but output is printed with prefix elapsed time
 * in format [minutes:seconds.milisec].
 */
void ferTimerPrintElapsed(const bor_timer_t *t, FILE *out,
                          const char *format, ...);
/**
 * Stops timer and prints output same as ferTimerPrintElapsed().
 */
void ferTimerStopAndPrintElapsed(bor_timer_t *t, FILE *out,
                                 const char *format, ...);

/**
 * Equivalent to ferTimerPrintElapsed() expect it is called with va_list
 * instead of variable number of arguments.
 */
_fer_inline void ferTimerPrintElapsed2(const bor_timer_t *t, FILE *out,
                                       const char *format, va_list ap);

/**
 * Returns nanosecond part of elapsed time.
 */
_fer_inline long ferTimerElapsedNs(const bor_timer_t *t);

/**
 * Returns microsecond part of elapsed time.
 */
_fer_inline long ferTimerElapsedUs(const bor_timer_t *t);

/**
 * Returns milisecond part of elapsed time.
 */
_fer_inline long ferTimerElapsedMs(const bor_timer_t *t);

/**
 * Returns second part of elapsed time.
 */
_fer_inline long ferTimerElapsedS(const bor_timer_t *t);

/**
 * Returns minute part of elapsed time.
 */
_fer_inline long ferTimerElapsedM(const bor_timer_t *t);

/**
 * Returns hour part of elapsed time.
 */
_fer_inline long ferTimerElapsedH(const bor_timer_t *t);


/**
 * Returns elapsed time in ns.
 */
_fer_inline unsigned long ferTimerElapsedInNs(const bor_timer_t *t);

/**
 * Returns elapsed time in us.
 */
_fer_inline unsigned long ferTimerElapsedInUs(const bor_timer_t *t);

/**
 * Returns elapsed time in ms.
 */
_fer_inline unsigned long ferTimerElapsedInMs(const bor_timer_t *t);

/**
 * Returns elapsed time in s.
 */
_fer_inline unsigned long ferTimerElapsedInS(const bor_timer_t *t);

/**
 * Returns elapsed time in m.
 */
_fer_inline unsigned long ferTimerElapsedInM(const bor_timer_t *t);

/**
 * Returns elapsed time in h.
 */
_fer_inline unsigned long ferTimerElapsedInH(const bor_timer_t *t);


/**
 * Returns elapsed time in s as float number
 */
_fer_inline bor_real_t ferTimerElapsedInSF(const bor_timer_t *t);

/**** INLINES ****/
_fer_inline void ferTimerStart(bor_timer_t *t)
{
    clock_gettime(CLOCK_MONOTONIC, &t->t_start);
}

_fer_inline void ferTimerStop(bor_timer_t *t)
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

_fer_inline const struct timespec *ferTimerElapsed(const bor_timer_t *t)
{
    return &t->t_elapsed;
}

_fer_inline void ferTimerPrintElapsed2(const bor_timer_t *t, FILE *out,
                                       const char *format, va_list ap)
{
    /* print elapsed time */
    fprintf(out, "[%02ld:%02ld.%03ld]",
            ferTimerElapsedM(t),
            ferTimerElapsedS(t),
            ferTimerElapsedMs(t));

    /* print the rest */
    vfprintf(out, format, ap);
}

_fer_inline long ferTimerElapsedNs(const bor_timer_t *t)
{
    return t->t_elapsed.tv_nsec % 1000L;
}

_fer_inline long ferTimerElapsedUs(const bor_timer_t *t)
{
    return (t->t_elapsed.tv_nsec / 1000L) % 1000L;
}

_fer_inline long ferTimerElapsedMs(const bor_timer_t *t)
{
    return t->t_elapsed.tv_nsec / 1000000L;
}

_fer_inline long ferTimerElapsedS(const bor_timer_t *t)
{
    return t->t_elapsed.tv_sec % 60L;
}

_fer_inline long ferTimerElapsedM(const bor_timer_t *t)
{
    return (t->t_elapsed.tv_sec / 60L) % 60L;
}

_fer_inline long ferTimerElapsedH(const bor_timer_t *t)
{
    return t->t_elapsed.tv_sec / 3600L;
}


_fer_inline unsigned long ferTimerElapsedInNs(const bor_timer_t *t)
{
    return t->t_elapsed.tv_nsec + t->t_elapsed.tv_sec * 1000000000L;
}

_fer_inline unsigned long ferTimerElapsedInUs(const bor_timer_t *t)
{
    return t->t_elapsed.tv_nsec / 1000L + t->t_elapsed.tv_sec * 1000000L;
}

_fer_inline unsigned long ferTimerElapsedInMs(const bor_timer_t *t)
{
    return t->t_elapsed.tv_nsec / 1000000L + t->t_elapsed.tv_sec * 1000L;
}

_fer_inline unsigned long ferTimerElapsedInS(const bor_timer_t *t)
{
    return t->t_elapsed.tv_sec;
}

_fer_inline unsigned long ferTimerElapsedInM(const bor_timer_t *t)
{
    return t->t_elapsed.tv_sec / 60L;
}

_fer_inline unsigned long ferTimerElapsedInH(const bor_timer_t *t)
{
    return t->t_elapsed.tv_sec / 3600L;
}

_fer_inline bor_real_t ferTimerElapsedInSF(const bor_timer_t *t)
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

