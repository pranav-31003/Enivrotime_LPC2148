/*============================================================
 * File    : switch.c
 * Project : EnviroTime - Digital Clock with Temperature Monitor
 * Author  : Embedded Systems Team
 * Date    : 02/05/2026
 * Brief   : Switch polling driver for LPC2148.
 *           Reads two active-LOW push-button switches with
 *           software debouncing.
 *
 * HARDWARE CONNECTIONS (Reference / Authoritative Config):
 *   EDIT  switch → P0.10 (active LOW, external 10K pull-up to 3.3V)
 *   ALARM switch → P0.11 (active LOW, external 10K pull-up to 3.3V)
 *
 * ACTIVE LOW OPERATION:
 *   When switch is NOT pressed: pin = HIGH (1) — pull-up holds it high
 *   When switch IS pressed:     pin = LOW  (0) — switch pulls to GND
 *   So we check: if (pin == 0) → switch is pressed
 *
 * DEBOUNCING:
 *   Mechanical switches bounce (rapidly toggle) for ~5-20 ms when pressed.
 *   Without debounce, one press can look like many presses.
 *   Fix: detect LOW → wait 20 ms → confirm still LOW → accept as real press.
 *
 * CHANGE FROM PREVIOUS REVISION:
 *   Old config: EDIT=P0.16, ALARM=P0.17 (conflicted with LCD RS/RW lines)
 *   New config: EDIT=P0.10, ALARM=P0.11 (dedicated GPIO, no conflict)
 *============================================================*/

#include <LPC21xx.h>
#include "types.h"
#include "defines.h"
#include "switch.h"
#include "delay.h"

/*------------------------------------------------------------
 * SwitchInit()
 * Brief : Sets EDIT (P0.10) and ALARM (P0.11) switch pins as
 *         GPIO inputs by clearing their IODIR0 bits.
 *         LPC2148 GPIO defaults to input, but this call makes
 *         the intent explicit and safe across all compiler settings.
 *------------------------------------------------------------*/
void SwitchInit(void)
{
    /* Clear direction bit → pin becomes INPUT
     * EDIT_SWITCH_PIN  = 10 → clears bit 10 of IODIR0 → P0.10 = INPUT  */
    IODIR0 &= ~(1UL << EDIT_SWITCH_PIN);

    /* ALARM_SWITCH_PIN = 11 → clears bit 11 of IODIR0 → P0.11 = INPUT  */
    IODIR0 &= ~(1UL << ALARM_SWITCH_PIN);
}

/*------------------------------------------------------------
 * IsEditSwitchPressed()
 * Brief : Checks whether the EDIT switch (P0.10) is pressed.
 *         Uses 20 ms debounce: first detects LOW, waits, confirms LOW.
 *
 * Return: 1 (u8) = switch IS pressed
 *         0 (u8) = switch is NOT pressed
 *
 * Note  : Does NOT wait for release. Caller must handle that
 *         if needed (e.g., wait for next loop iteration).
 *------------------------------------------------------------*/
u8 IsEditSwitchPressed(void)
{
    /* READBIT(IOPIN0, 10) reads bit 10 of IOPIN0 = current state of P0.10
     * Value 0 means the pin is LOW = switch IS pressed (active LOW)       */
    if (READBIT(IOPIN0, EDIT_SWITCH_PIN) == 0U)
    {
        delay_ms(20U);   /* Debounce: wait 20 ms before re-checking */

        /* Re-read: if still LOW, this is a genuine key press, not bounce */
        if (READBIT(IOPIN0, EDIT_SWITCH_PIN) == 0U)
        {
            return 1U;   /* Confirmed: EDIT switch is pressed */
        }
    }
    return 0U;   /* Switch is not pressed (or was just noise) */
}

/*------------------------------------------------------------
 * IsAlarmSwitchPressed()
 * Brief : Checks whether the ALARM STOP switch (P0.11) is pressed.
 *         Uses 20 ms debounce identical to IsEditSwitchPressed().
 *
 * Return: 1 (u8) = switch IS pressed
 *         0 (u8) = switch is NOT pressed
 *
 * Usage : Called inside the alarm ringing loop in main.c.
 *         When user presses this switch, the buzzer is stopped.
 *------------------------------------------------------------*/
u8 IsAlarmSwitchPressed(void)
{
    /* READBIT(IOPIN0, 11) reads bit 11 = state of P0.11
     * Value 0 means the pin is LOW = switch IS pressed (active LOW)       */
    if (READBIT(IOPIN0, ALARM_SWITCH_PIN) == 0U)
    {
        delay_ms(20U);   /* Debounce: wait 20 ms */

        /* Confirm still pressed */
        if (READBIT(IOPIN0, ALARM_SWITCH_PIN) == 0U)
        {
            return 1U;   /* Confirmed: ALARM switch is pressed */
        }
    }
    return 0U;   /* Switch is not pressed */
}
