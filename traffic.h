#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <reg51.h>   /* Special Function Register (SFR) definitions for 8051 */

/* =====================================================================
 *  PIN MAPPING
 *  ---------------------------------------------------------------
 *  We need 4 directions x 3 LEDs (Red/Yellow/Green) = 12 output
 *  lines in total, plus 1 input line for the emergency sensor and
 *  2 lines for the I2C bus to the RTC. Port 1 and Port 2 give us
 *  8 pins each, which is more than enough for 6 signals per port.
 *
 *  P1 -> North & South signals   (these two always move together)
 *  P2 -> East  & West  signals   (these two always move together)
 *  P3.2 (INT0) -> Emergency vehicle sensor / push button (active low)
 *  P0.0 / P0.1 -> Software I2C bus (SDA / SCL) to the DS1307 RTC
 * ===================================================================== */

/* ---- North direction: 3 LEDs on P1.0 - P1.2 ---- */
sbit N_RED = P1^0;   /* North Red LED    */
sbit N_YEL = P1^1;   /* North Yellow LED */
sbit N_GRN = P1^2;   /* North Green LED  */

/* ---- South direction: 3 LEDs on P1.3 - P1.5 ----
 * South is wired on the SAME port as North because, in this
 * simplified 2-phase model, North and South always show the
 * same colour at the same time (they don't cross each other). */
sbit S_RED = P1^3;
sbit S_YEL = P1^4;
sbit S_GRN = P1^5;

/* ---- East direction: 3 LEDs on P2.0 - P2.2 ---- */
sbit E_RED = P2^0;
sbit E_YEL = P2^1;
sbit E_GRN = P2^2;

/* ---- West direction: 3 LEDs on P2.3 - P2.5 ----
 * Same idea as North/South: East and West are paired together. */
sbit W_RED = P2^3;
sbit W_YEL = P2^4;
sbit W_GRN = P2^5;

/* =====================================================================
 *  TIMING CONSTANTS (all values are in MILLISECONDS)
 *  These feed straight into delay_ms() calls in main.c, so you can
 *  change the "feel" of the junction just by editing the numbers
 *  here without touching the logic anywhere else.
 * ===================================================================== */
#define GREEN_TIME            8000   /* how long each direction stays green (8 s)      */
#define YELLOW_TIME           2000   /* how long the yellow/caution phase lasts (2 s)  */
#define EMERGENCY_GREEN_TIME  6000   /* green duration given to an emergency vehicle   */

/* =====================================================================
 *  NIGHT MODE WINDOW (24-HOUR FORMAT, compared against the RTC hour)
 *  If the RTC hour read from the DS1307 falls in this window, the
 *  controller stops the normal cycle and blinks amber instead
 *  (see night_blink_mode() in main.c).
 * ===================================================================== */
#define NIGHT_START_HOUR      22   /* night mode begins at 10 PM (22:00) */
#define NIGHT_END_HOUR         6   /* night mode ends   at  6 AM (06:00) */

#endif
