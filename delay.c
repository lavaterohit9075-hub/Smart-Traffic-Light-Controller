#include "delay.h"

/* =====================================================================
 *  SOFTWARE DELAY ROUTINES
 *  ---------------------------------------------------------------
 *  The 8051 has hardware timers, but for a project like this a
 *  simple "busy-wait" software delay is easier to read and is the
 *  standard approach used in most academic 8051 traffic-light
 *  projects. The trade-off is that the CPU does nothing else while
 *  waiting (which is fine here, since we don't need multitasking).
 *
 *  These loop counts are approximate and calibrated for an
 *  11.0592 MHz crystal (the default clock in most Proteus 8051
 *  templates). If you simulate at a different crystal frequency,
 *  the real-world delay will drift — adjust the inner loop count
 *  until the LED timing looks correct.
 * ===================================================================== */

/* Delay for roughly `us` microseconds. */
void delay_us(unsigned int us)
{
    unsigned char i;

    /* Outer loop: one iteration per microsecond requested */
    while (us--)
    {
        /* Inner loop: burns a few CPU cycles to approximate 1 us.
         * The exact count (2 here) was tuned for 11.0592 MHz --
         * each empty for-loop iteration takes a small, fixed
         * number of machine cycles, so looping twice gives us
         * roughly a 1 us delay at this clock speed. */
        for (i = 0; i < 2; i++);
    }
}

/* Delay for roughly `ms` milliseconds, built on top of delay_us(). */
void delay_ms(unsigned int ms)
{
    unsigned int i;

    /* 1 millisecond = 1000 microseconds, so we just call
     * delay_us(1000) once for every millisecond requested. */
    for (i = 0; i < ms; i++)
    {
        delay_us(1000);
    }
}
