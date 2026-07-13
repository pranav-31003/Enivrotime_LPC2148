/*============================================================
 * File    : buzzer.h
 * Project : EnviroTime - Digital Clock with Temperature Monitor
 * Author  : Embedded Systems Team
 * Date    : 02/05/2026
 * Brief   : Buzzer driver function prototypes for LPC2148.
 *
 * HARDWARE PIN CONNECTION (Reference / Authoritative Config):
 *   Buzzer control output → P0.12  (active HIGH)
 *   P0.12 = HIGH → buzzer ON
 *   P0.12 = LOW  → buzzer OFF
 *
 * TYPICAL DRIVE CIRCUIT:
 *   LPC2148 P0.12 → 1K resistor → NPN transistor (BC547) base
 *   Transistor collector → Buzzer (+) → +5V
 *   Transistor emitter   → GND
 *   Buzzer (-)           → GND
 *   (The transistor provides enough current to drive the buzzer
 *    since LPC2148 GPIO can only source/sink ~4 mA directly.)
 *
 * CHANGE FROM PREVIOUS REVISION:
 *   Old config: Buzzer on P0.15
 *   New config: Buzzer on P0.12 (matches reference pin_config.h)
 *============================================================*/

#ifndef BUZZER_H
#define BUZZER_H

#include "types.h"

/*-------------- Buzzer Pin Definition ---------------------*/
/* BUZZER_PIN = 12 → P0.12 on LPC2148 PORT0              */
#define BUZZER_PIN   12U   /* Buzzer output on P0.12 (active HIGH) */

/*-------------- Function Prototypes -----------------------*/

/* BuzzerInit() – Configure P0.12 as OUTPUT and set it LOW (buzzer OFF).
 * Call once during system initialization.                              */
void BuzzerInit(void);

/* BuzzerOn()  – Drive P0.12 HIGH to turn the buzzer ON.               */
void BuzzerOn(void);

/* BuzzerOff() – Drive P0.12 LOW to turn the buzzer OFF.               */
void BuzzerOff(void);

/* BuzzerBeep(ms) – Turn buzzer ON for 'ms' milliseconds, then OFF.
 * Includes a 100 ms silent pause after the beep.                       */
void BuzzerBeep(u32 ms);

/* BuzzerAlert(count) – Generate 'count' short beeps (200 ms each).
 * Used for password wrong-attempt warnings and lockout alerts.          */
void BuzzerAlert(u8 count);

#endif /* BUZZER_H */
