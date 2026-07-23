#ifndef I2C_H
#define I2C_H

/* Sets SDA and SCL to their idle HIGH state. Call once at startup. */
void i2c_init(void);

/* Issues an I2C START condition (SDA falls while SCL is high). */
void i2c_start(void);

/* Issues an I2C STOP condition (SDA rises while SCL is high). */
void i2c_stop(void);

/* Writes one byte to the bus, MSB first.
 * Returns 1 if the slave ACKnowledged, 0 if it NACKed. */
bit i2c_write(unsigned char dat);

/* Reads one byte from the bus, MSB first.
 * `ack` = 1 tells the slave "send more bytes after this one",
 * `ack` = 0 tells the slave "this is the last byte I need." */
unsigned char i2c_read(bit ack);

#endif
