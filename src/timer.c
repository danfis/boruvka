#include <fermat/timer.h>

void ferTimerPrintElapsed(const fer_timer_t *t, FILE *out,
                          const char *format, ...)
{
    va_list ap; 

    va_start(ap, format);
    ferTimerPrintElapsed2(t, out, format, ap);
    va_end(ap);
}

void ferTimerStopAndPrintElapsed(fer_timer_t *t, FILE *out,
                                 const char *format, ...)
{
    va_list ap; 

    ferTimerStop(t);
    va_start(ap, format);
    ferTimerPrintElapsed2(t, out, format, ap);
    va_end(ap);
}
