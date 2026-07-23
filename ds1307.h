#ifndef DS1307_H
#define DS1307_H

/* One-time helper to write a starting time into the RTC.
 * Does nothing by default -- see the comment block inside
 * ds1307.c for how to use it. */
void ds1307_init(void);

/* Each of these reads one register from the DS1307 over I2C
 * and returns it already converted from BCD to normal decimal. */
unsigned char ds1307_get_hour(void);
unsigned char ds1307_get_minute(void);
unsigned char ds1307_get_second(void);

#endif
