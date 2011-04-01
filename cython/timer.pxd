from core cimport *

cdef extern from "../fermat/timer.h":
    ctypedef int fer_timer_t

    void ferTimerStart(fer_timer_t *t)
    void ferTimerStop(fer_timer_t *t)
    long ferTimerElapsedNs(fer_timer_t *t)
    long ferTimerElapsedUs(fer_timer_t *t)
    long ferTimerElapsedMs(fer_timer_t *t)
    long ferTimerElapsedS(fer_timer_t *t)
    long ferTimerElapsedM(fer_timer_t *t)
    long ferTimerElapsedH(fer_timer_t *t)
    unsigned long ferTimerElapsedInNs(fer_timer_t *t)
    unsigned long ferTimerElapsedInUs(fer_timer_t *t)
    unsigned long ferTimerElapsedInMs(fer_timer_t *t)
    unsigned long ferTimerElapsedInS(fer_timer_t *t)
    unsigned long ferTimerElapsedInM(fer_timer_t *t)
    unsigned long ferTimerElapsedInH(fer_timer_t *t)
