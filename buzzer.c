/*============================================================
 * File    : buzzer.c
 * Project : EnviroTime - Digital Clock with Temperature Monitor
 * Author  : Embedded Systems Team
 * Date    : 02/05/2026
 * Brief   : Buzzer driver for LPC2148.
 *           Controls a buzzer via a transistor on P0.12.
 *
 * HARDWARE CONNECTIONS (Reference / Authoritative Config):
 *   P0.12 → 1K resistor → NPN transistor (e.g. BC547) Base
 *   Transistor Collector → Buzzer (+) terminal → +5V rail
 *   Transistor Emitter   → GND
 *   Buzzer (-) terminal  → GND
 *
 * HOW IT WORKS:
 *   P0.12 HIGH (1) → transistor conducts → current flows → buzzer sounds
 *   P0.12 LOW  (0) → transistor off      → no current    → buzzer silent
 *
 * WHY A TRANSISTOR:
 *   LPC2148 GPIO pins can only source/sink ~4 mA safely.
 *   A typical buzzer needs 20-100 mA. The NPN transistor acts as
 *   a switch that allows the +5V rail to drive the buzzer while
 *   the MCU only supplies a small base current through the 1K resistor.
 *
 * CHANGE FROM PREVIOUS REVISION:
 *   Old config: Buzzer on P0.15
 *   New config: Buzzer on P0.12 (matches reference pin_config.h)
 *============================================================*/

#include <LPC21xx.h>
#include "types.h"
#include "defines.h"
#include "buzzer.h"
#include "delay.h"

/*------------------------------------------------------------
 * BuzzerInit()
 * Brief : Configures P0.12 as a GPIO output and ensures the
 *         buzzer starts in the OFF state (LOW).
 *         Must be called once during system initialization.
 *------------------------------------------------------------*/
void BuzzerInit(void)
{
    /* Set bit 12 of IODIR0 → P0.12 becomes an OUTPUT pin */
    IODIR0 |= (1UL << BUZZER_PIN);

    /* Drive P0.12 LOW → buzzer OFF at startup */
    IOCLR0 = (1UL << BUZZER_PIN);
}

/*------------------------------------------------------------
 * BuzzerOn()
 * Brief : Turns the buzzer ON by driving P0.12 HIGH.
 *         The transistor conducts and buzzer sounds until
 *         BuzzerOff() is called.
 *------------------------------------------------------------*/
void BuzzerOn(void)
{
    /* IOSET0 bit 12 → P0.12 = HIGH → transistor ON → buzzer sounds */
    IOSET0 = (1UL << BUZZER_PIN);
}

/*------------------------------------------------------------
 * BuzzerOff()
 * Brief : Turns the buzzer OFF by driving P0.12 LOW.
 *         The transistor stops conducting and buzzer goes silent.
 *------------------------------------------------------------*/
void BuzzerOff(void)
{
    /* IOCLR0 bit 12 → P0.12 = LOW → transistor OFF → buzzer silent */
    IOCLR0 = (1UL << BUZZER_PIN);
}

/*------------------------------------------------------------
 * BuzzerBeep()
 * Brief : Produces a single beep of a specified duration.
 *         1. Turns buzzer ON
 *         2. Waits 'ms' milliseconds
 *         3. Turns buzzer OFF
 *         4. Waits 100 ms (silent gap between consecutive beeps)
 *
 * Param : ms – beep duration in milliseconds (e.g. 200 = 200 ms)
 *------------------------------------------------------------*/
void BuzzerBeep(u32 ms)
{
    BuzzerOn();           /* Start the beep          */
    delay_ms(ms);         /* Keep buzzer ON for 'ms' */
    BuzzerOff();          /* End the beep            */
    delay_ms(100U);       /* 100 ms silent pause so consecutive beeps
                           * are clearly distinguishable to the user  */
}

/*------------------------------------------------------------
 * BuzzerAlert()
 * Brief : Generates 'count' short beeps (200 ms each) in sequence.
 *         Used for:
 *           - Wrong password attempt  → 2 beeps
 *           - System locked alert     → 5 beeps
 *           - Access denied warning   → 2 beeps
 *
 * Param : count – number of beeps to generate (1 to 10 recommended)
 *------------------------------------------------------------*/
void BuzzerAlert(u8 count)
{
    u8 i;
    for (i = 0U; i < count; i++)
    {
        BuzzerBeep(200U);   /* 200 ms beep + 100 ms gap each iteration */
    }
}
