#include <reg51.h>
#include "i2c.h"
#include "delay.h"

/* =====================================================================
 *  SOFTWARE (BIT-BANGED) I2C MASTER DRIVER
 *  ---------------------------------------------------------------
 *  The standard 8051 (AT89C51/AT89S52) has NO built-in I2C hardware,
 *  so we "fake" the I2C protocol by toggling two ordinary GPIO pins
 *  in software, following the I2C electrical rules exactly:
 *
 *    - SDA (data line)  -> P0.0
 *    - SCL (clock line) -> P0.1
 *
 *  Both lines must have external pull-up resistors (~4.7k to VCC)
 *  in the circuit/Proteus schematic, because I2C devices only ever
 *  pull the lines LOW themselves; the pull-ups bring them back HIGH.
 *
 *  This driver implements the 4 basic I2C operations needed by the
 *  DS1307 RTC: START, STOP, WRITE one byte, READ one byte.
 * ===================================================================== */

sbit SDA = P0^0;   /* I2C data line  */
sbit SCL = P0^1;   /* I2C clock line */

/* Set both lines to their idle (HIGH) state before any communication. */
void i2c_init(void)
{
    SDA = 1;
    SCL = 1;
}

/* I2C START condition: SDA goes LOW while SCL is still HIGH.
 * This unique signal tells every device on the bus "a new
 * transaction is beginning." */
void i2c_start(void)
{
    SDA = 1;
    SCL = 1;
    delay_us(5);   /* small settle time before changing SDA   */
    SDA = 0;       /* SDA falls while SCL is high -> START     */
    delay_us(5);
    SCL = 0;       /* pull clock low so we can safely change SDA next */
}

/* I2C STOP condition: SDA goes HIGH while SCL is HIGH.
 * This tells every device "the transaction has ended." */
void i2c_stop(void)
{
    SDA = 0;
    SCL = 1;
    delay_us(5);
    SDA = 1;       /* SDA rises while SCL is high -> STOP */
    delay_us(5);
}

/* Sends one byte (dat) out on the bus, MSB first, and returns
 * whether the slave device acknowledged it (1 = ACK, 0 = NACK). */
bit i2c_write(unsigned char dat)
{
    unsigned char i;
    bit ack;

    /* Shift out all 8 bits, most-significant bit first.
     * Data must only change while SCL is LOW, and is "read" by
     * the slave while SCL is HIGH -- that's why we set SDA first,
     * then pulse SCL high-then-low for each bit. */
    for (i = 0; i < 8; i++)
    {
        SDA = (bit)(dat & 0x80);   /* put the top bit of dat onto SDA */
        dat <<= 1;                 /* shift so the next bit is on top */
        SCL = 1;                   /* clock high -> slave reads the bit */
        delay_us(2);
        SCL = 0;                   /* clock low -> ready for next bit   */
    }

    /* --- 9th clock pulse: read the ACK/NACK bit from the slave --- */
    SDA = 1;        /* release SDA so the slave can drive it      */
    SCL = 1;
    delay_us(2);
    ack = !SDA;      /* slave pulls SDA low for ACK, so !SDA = 1 means ACK */
    SCL = 0;

    return ack;
}

/* Reads one byte from the bus, MSB first. The caller decides
 * whether to ACK (ack = 1, "please send more bytes") or
 * NACK (ack = 0, "that's the last byte I need"). */
unsigned char i2c_read(bit ack)
{
    unsigned char i, dat = 0;
    SDA = 1;   /* release the data line so the slave can drive it */

    for (i = 0; i < 8; i++)
    {
        SCL = 1;             /* clock high -> slave puts its next bit on SDA */
        delay_us(2);
        dat <<= 1;           /* make room for the incoming bit           */
        if (SDA) dat |= 0x01; /* read the bit and OR it into the result   */
        SCL = 0;              /* clock low -> slave prepares its next bit */
    }

    /* --- Master now sends ACK (0) or NACK (1) back to the slave --- */
    SDA = !ack;
    SCL = 1;
    delay_us(2);
    SCL = 0;
    SDA = 1;   /* release the line again */

    return dat;
}
