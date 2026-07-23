#ifndef DELAY_H
#define DELAY_H

/* Blocks execution for approximately `us` microseconds. */
void delay_us(unsigned int us);

/* Blocks execution for approximately `ms` milliseconds.
 * Internally just calls delay_us(1000) `ms` times. */
void delay_ms(unsigned int ms);

#endif
