/*============================================================
 * File         : delay.h
 * Project      : EnviroTime – Digital Clock + Temperature Monitor
 * Target       : LPC2148 (ARM7TDMI-S) @ 60 MHz
 * Author       : Embedded Systems Team
 * Date         : 02/05/2026
 * Description  : Function prototypes for software busy-wait
 *                delay routines calibrated for 60 MHz CCLK.
 *============================================================*/

#ifndef DELAY_H
#define DELAY_H

#include "types.h"

void delay_us(u32 us);   /* Microsecond busy-wait delay */
void delay_ms(u32 ms);   /* Millisecond busy-wait delay */
void delay_s (u32 s);    /* Second      busy-wait delay */

#endif /* DELAY_H */
