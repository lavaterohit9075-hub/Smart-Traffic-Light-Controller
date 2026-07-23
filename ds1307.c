#include <reg51.h>
#include "ds1307.h"
#include "i2c.h"

/* =====================================================================
 *  DS1307 RTC (Real-Time Clock) DRIVER
 *  ---------------------------------------------------------------
 *  The DS1307 is an I2C real-time-clock chip. It has an internal
 *  set of registers (0x00 - 0x07) that hold the current seconds,
 *  minutes, hours, day, date, month, and year -- all stored in
 *  BCD (Binary-Coded Decimal) format, NOT plain binary. That's why
 *  every value read from/written to the chip must be converted
 *  with bcd_to_dec() / dec_to_bcd() below.
 *
 *  Fixed 7-bit I2C address of the DS1307 is 0x68. When you shift
 *  that left by 1 and add the read/write bit, you get:
 *    Write address = 0xD0  (0x68 << 1 | 0)
 *    Read  address = 0xD1  (0x68 << 1 | 1)
 * ===================================================================== */

#define DS1307_ADDR_WRITE   0xD0
#define DS1307_ADDR_READ    0xD1

/* Convert a BCD byte (e.g. 0x25) into normal decimal (25). */
static unsigned char bcd_to_dec(unsigned char bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

/* Convert a normal decimal value (e.g. 25) into BCD (0x25).
 * Only used when we WRITE a starting time into the RTC. */
static unsigned char dec_to_bcd(unsigned char dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}

/*
 * ---------------------------------------------------------------
 * ONE-TIME RTC TIME-SET ROUTINE
 * ---------------------------------------------------------------
 * The DS1307 keeps running on its own coin-cell battery, so once
 * you've told it the correct time, it will keep counting forever
 * (even when the 8051 is powered off) until the battery dies.
 *
 * That means you only need to WRITE a starting time into it ONCE.
 * The block below is commented out by default so the clock is NOT
 * reset every time the program runs. To set the time:
 *   1. Uncomment the block below.
 *   2. Change the hour/minute/second values to the time you want.
 *   3. Build and run the simulation ONE time.
 *   4. Comment the block back out and rebuild, so future runs
 *      just READ the time instead of overwriting it again.
 * ---------------------------------------------------------------
 */
void ds1307_init(void)
{
    /* ---- Example: set time to 09:30:00 (24-hour format) ----
    i2c_start();
    i2c_write(DS1307_ADDR_WRITE);     // select DS1307 in write mode
    i2c_write(0x00);                  // point to the "seconds" register
    i2c_write(dec_to_bcd(0));         // write seconds  (register 0x00)
    i2c_write(dec_to_bcd(30));        // write minutes  (register 0x01)
    i2c_write(dec_to_bcd(9));         // write hours    (register 0x02),
                                       // bit 6 = 0 selects 24-hour mode
    i2c_stop();
    --------------------------------------------------------- */
}

/* Read just the SECONDS register (0x00) from the RTC. */
unsigned char ds1307_get_second(void)
{
    unsigned char sec;

    /* Step 1: tell the RTC which register we want to read from. */
    i2c_start();
    i2c_write(DS1307_ADDR_WRITE);
    i2c_write(0x00);                  /* seconds register address */

    /* Step 2: restart the bus in READ mode and pull the byte back. */
    i2c_start();
    i2c_write(DS1307_ADDR_READ);
    sec = i2c_read(0);                /* 0 = NACK, since this is the only/last byte we want */
    i2c_stop();

    /* Bit 7 of the seconds register is a "clock halt" flag, not
     * part of the value, so we mask it off before converting. */
    return bcd_to_dec(sec & 0x7F);
}

/* Read just the MINUTES register (0x01) from the RTC. */
unsigned char ds1307_get_minute(void)
{
    unsigned char min;

    i2c_start();
    i2c_write(DS1307_ADDR_WRITE);
    i2c_write(0x01);                  /* minutes register address */

    i2c_start();
    i2c_write(DS1307_ADDR_READ);
    min = i2c_read(0);
    i2c_stop();

    return bcd_to_dec(min);
}

/* Read just the HOURS register (0x02) from the RTC.
 * This is the value main.c actually uses to decide night mode. */
unsigned char ds1307_get_hour(void)
{
    unsigned char hr;

    i2c_start();
    i2c_write(DS1307_ADDR_WRITE);
    i2c_write(0x02);                  /* hours register address */

    i2c_start();
    i2c_write(DS1307_ADDR_READ);
    hr = i2c_read(0);
    i2c_stop();

    /* Bits 6-7 of the hours register control 12/24-hour mode and
     * AM/PM, not the hour value itself, so mask them off. We
     * assume the chip is running in 24-hour mode. */
    return bcd_to_dec(hr & 0x3F);
}
