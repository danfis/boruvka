#include <fermat/timer.h>

void ferTimerPrintElapsed(const fer_timer_t *t, FILE *out,
                          const char *format, ...)
{
    va_list ap;

    // print elapsed time
    fprintf(out, "[%02ld:%02ld.%03ld]",
            ferTimerElapsedM(t),
            ferTimerElapsedS(t),
            ferTimerElapsedMs(t));

    // print the rest
    va_start(ap, format);
    vfprintf(out, format, ap);
    va_end(ap);
}
