/* =====================================================================
 *  SMART TRAFFIC LIGHT CONTROLLER - 8051 (Keil C)
 *  ---------------------------------------------------------------
 *  Simulates a 4-way road junction (North / South / East / West).
 *
 *  FEATURES:
 *    1. Normal 2-phase traffic cycle:
 *         North+South green together <-> East+West green together,
 *         with a yellow "caution" gap in between each switch.
 *    2. Emergency vehicle override:
 *         An interrupt (INT0, pin P3.2) simulates an ambulance
 *         sensor. When triggered, the junction immediately clears
 *         a path, regardless of what phase it was in.
 *    3. RTC-based night mode:
 *         A DS1307 real-time clock is read every cycle. During
 *         late-night hours, the junction stops the normal cycle
 *         and just blinks amber on all sides (like real-world
 *         intersections do overnight).
 *
 *  MCU      : AT89C51 / AT89S52 (8051 family)
 *  Compiler : Keil uVision (C51)
 *  Simulate : Proteus 8 Professional
 * ===================================================================== */

#include <reg51.h>
#include "traffic.h"   /* pin mapping + timing constants           */
#include "delay.h"      /* delay_ms() / delay_us()                  */
#include "ds1307.h"     /* RTC driver, used for night-mode decision  */

/* ---------------------------------------------------------------
 * `emergency_flag` is a single BIT variable (8051 supports true
 * bit-addressable variables) shared between the ISR and main().
 * The ISR ONLY sets this flag -- it does NOT do the actual signal
 * switching itself, because interrupt service routines should be
 * kept as short as possible. The real work happens in the main
 * loop, which checks this flag on every pass.
 * --------------------------------------------------------------- */
bit emergency_flag = 0;

/* ---- Function prototypes (implemented further down this file) ---- */
void all_red(void);
void ns_green_ew_red(void);
void ns_yellow(void);
void ew_green_ns_red(void);
void ew_yellow(void);
void night_blink_mode(void);
void handle_emergency(void);

void main(void)
{
    unsigned char current_hour;   /* holds the RTC hour, re-read every loop */

    /* -----------------------------------------------------------
     * Configure external interrupt 0 (INT0), wired to P3.2, which
     * is where the emergency-vehicle sensor / push-button connects.
     * ----------------------------------------------------------- */
    IT0 = 1;      /* 1 = INT0 triggers on a FALLING EDGE (button press pulls it low) */
    EX0 = 1;      /* enable the INT0 interrupt source specifically                   */
    EA  = 1;      /* master interrupt enable switch -- must be 1 for ANY interrupt   */

    i2c_init();       /* set SDA/SCL lines to their idle HIGH state */
    ds1307_init();    /* no-op unless you've uncommented the "set time" block inside it */

    /* Start the junction in a safe all-red state before entering the main loop. */
    all_red();
    delay_ms(500);

    /* -----------------------------------------------------------
     * MAIN CONTROL LOOP
     * Runs forever. On every pass it checks, in priority order:
     *   1. Is there an emergency vehicle waiting?      (highest priority)
     *   2. Is it currently "night time" per the RTC?
     *   3. Otherwise, run the normal 4-phase cycle.
     * ----------------------------------------------------------- */
    while (1)
    {
        /* ---------- PRIORITY 1: Emergency vehicle ---------- */
        if (emergency_flag)
        {
            handle_emergency();   /* clear the junction for the vehicle   */
            emergency_flag = 0;   /* reset the flag now that we've handled it */
            continue;              /* skip straight back to the top of the loop */
        }

        /* ---------- PRIORITY 2: Night mode check ---------- */
        current_hour = ds1307_get_hour();   /* ask the RTC what hour it is right now */

        /* Night window can "wrap around" midnight (e.g. 22 -> 6),
         * so we check with OR: either "at or after start hour"
         * OR "before end hour" puts us in the night window. */
        if (current_hour >= NIGHT_START_HOUR || current_hour < NIGHT_END_HOUR)
        {
            night_blink_mode();   /* flash amber instead of the normal cycle */
            continue;
        }

        /* ---------- PRIORITY 3: Normal daytime traffic cycle ---------- */
        ns_green_ew_red();        /* North+South GO, East+West STOP */
        delay_ms(GREEN_TIME);

        ns_yellow();               /* North+South CAUTION, East+West still STOP */
        delay_ms(YELLOW_TIME);

        ew_green_ns_red();        /* East+West GO, North+South STOP */
        delay_ms(GREEN_TIME);

        ew_yellow();               /* East+West CAUTION, North+South still STOP */
        delay_ms(YELLOW_TIME);

        /* Loop repeats from the top -- back to ns_green_ew_red() */
    }
}

/* =====================================================================
 *  SIGNAL COMBINATION FUNCTIONS
 *  Each function below sets ALL 12 LED pins in one shot, so that at
 *  any given moment the junction is always in one clearly-defined,
 *  safe combination (we never leave pins in a half-changed state).
 * ===================================================================== */

/* Every direction shows RED. Used as a safe "reset" state, e.g.
 * right before/after handling an emergency vehicle. */
void all_red(void)
{
    N_RED = 1; N_YEL = 0; N_GRN = 0;
    S_RED = 1; S_YEL = 0; S_GRN = 0;
    E_RED = 1; E_YEL = 0; E_GRN = 0;
    W_RED = 1; W_YEL = 0; W_GRN = 0;
}

/* North & South GREEN (traffic flows), East & West RED (traffic stops). */
void ns_green_ew_red(void)
{
    N_RED = 0; N_YEL = 0; N_GRN = 1;
    S_RED = 0; S_YEL = 0; S_GRN = 1;
    E_RED = 1; E_YEL = 0; E_GRN = 0;
    W_RED = 1; W_YEL = 0; W_GRN = 0;
}

/* North & South YELLOW (warn that green is about to end),
 * East & West stay RED throughout this transition. */
void ns_yellow(void)
{
    N_RED = 0; N_YEL = 1; N_GRN = 0;
    S_RED = 0; S_YEL = 1; S_GRN = 0;
    E_RED = 1; E_YEL = 0; E_GRN = 0;
    W_RED = 1; W_YEL = 0; W_GRN = 0;
}

/* East & West GREEN (traffic flows), North & South RED (traffic stops).
 * Mirror image of ns_green_ew_red(). */
void ew_green_ns_red(void)
{
    N_RED = 1; N_YEL = 0; N_GRN = 0;
    S_RED = 1; S_YEL = 0; S_GRN = 0;
    E_RED = 0; E_YEL = 0; E_GRN = 1;
    W_RED = 0; W_YEL = 0; W_GRN = 1;
}

/* East & West YELLOW, North & South stay RED throughout. */
void ew_yellow(void)
{
    N_RED = 1; N_YEL = 0; N_GRN = 0;
    S_RED = 1; S_YEL = 0; S_GRN = 0;
    E_RED = 0; E_YEL = 1; E_GRN = 0;
    W_RED = 0; W_YEL = 1; W_GRN = 0;
}

/* =====================================================================
 *  EMERGENCY VEHICLE HANDLING
 *  ---------------------------------------------------------------
 *  Simplified for a single sensor placed on the North approach
 *  (wired to P3.2 / INT0). Sequence when triggered:
 *
 *    1. Force ALL directions to red and hold briefly -- this is a
 *       safety gap so nobody is already mid-junction with a green
 *       light when we suddenly change things.
 *    2. Give North an immediate GREEN so the emergency vehicle can
 *       pass through unobstructed.
 *    3. Switch North to YELLOW to warn it's ending.
 *    4. Return to all-red briefly before handing control back to
 *       the normal cycle (or another emergency, or night mode).
 *
 *  NOTE: To support all 4 approaches having their own sensor, you
 *  would replace this single hardcoded "North" sequence with a
 *  parameter (e.g. handle_emergency(direction)) and 4 separate
 *  interrupt inputs (or a polled sensor array) instead of one
 *  shared INT0 line.
 * ===================================================================== */
void handle_emergency(void)
{
    all_red();
    delay_ms(300);            /* safety gap so no one is caught mid-junction */

    N_RED = 0; N_GRN = 1;     /* clear the path for the emergency vehicle */
    delay_ms(EMERGENCY_GREEN_TIME);

    N_GRN = 0; N_YEL = 1;     /* warn that the emergency green is ending */
    delay_ms(YELLOW_TIME);

    all_red();
    delay_ms(300);            /* settle back to a safe state before resuming */
}

/* =====================================================================
 *  NIGHT / BLINKING MODE
 *  ---------------------------------------------------------------
 *  Between NIGHT_START_HOUR and NIGHT_END_HOUR (see traffic.h),
 *  traffic is usually light, so instead of making vehicles wait
 *  through a full red phase, every direction simply flashes amber
 *  as a "proceed with caution" signal -- exactly how many real
 *  intersections behave late at night.
 * ===================================================================== */
void night_blink_mode(void)
{
    /* Amber ON for all 4 directions */
    N_RED = 0; N_GRN = 0; N_YEL = 1;
    S_RED = 0; S_GRN = 0; S_YEL = 1;
    E_RED = 0; E_GRN = 0; E_YEL = 1;
    W_RED = 0; W_GRN = 0; W_YEL = 1;
    delay_ms(500);   /* amber stays lit for half a second */

    /* Amber OFF for all 4 directions -> creates the "blinking" effect */
    N_YEL = 0; S_YEL = 0; E_YEL = 0; W_YEL = 0;
    delay_ms(500);   /* stays off for half a second, then the loop repeats */
}

/* =====================================================================
 *  INTERRUPT SERVICE ROUTINE - Emergency vehicle sensor (INT0 / P3.2)
 *  ---------------------------------------------------------------
 *  `interrupt 0` is Keil C51 syntax that tells the compiler this
 *  function is the handler for interrupt vector 0, which is INT0
 *  on the 8051. It fires automatically in hardware whenever P3.2
 *  sees a falling edge (button pressed / sensor triggered), since
 *  we set IT0 = 1 in main().
 *
 *  Kept deliberately tiny: just raise the flag and return. All the
 *  real work happens back in the main loop, which is safer and
 *  easier to debug than doing everything inside an ISR.
 * ===================================================================== */
void emergency_ISR(void) interrupt 0
{
    emergency_flag = 1;
}
