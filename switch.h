/*============================================================
 * File    : switch.h
 * Project : EnviroTime - Digital Clock with Temperature Monitor
 * Author  : Embedded Systems Team
 * Date    : 02/05/2026
 * Brief   : Switch driver function prototypes for LPC2148.
 *
 * HARDWARE PIN CONNECTIONS (Reference / Authoritative Config):
 * ┌──────────────────┬────────────────────────────────────────┐
 * │  Switch          │  LPC2148 Pin                           │
 * ├──────────────────┼────────────────────────────────────────┤
 * │  EDIT  Switch    │  P0.10  (active LOW, external pull-up) │
 * │  ALARM Switch    │  P0.11  (active LOW, external pull-up) │
 * └──────────────────┴────────────────────────────────────────┘
 *
 * WIRING:
 *   One side of each switch → LPC2148 pin (P0.10 or P0.11)
 *   Other side of each switch → GND
 *   External 10K pull-up resistor from each pin to +3.3V
 *
 * ACTIVE LOW LOGIC:
 *   Pin reads HIGH (1) when switch is NOT pressed (pull-up holds it high)
 *   Pin reads LOW  (0) when switch IS pressed (connects to GND)
 *
 * CHANGE FROM PREVIOUS REVISION:
 *   Old config: EDIT=P0.16, ALARM=P0.17 (conflicted with LCD RS/RW)
 *   New config: EDIT=P0.10, ALARM=P0.11 (dedicated GPIO, no conflicts)
 *============================================================*/

#ifndef SWITCH_H
#define SWITCH_H

#include "types.h"

/*-------------- Switch Pin Definitions --------------------*/
/* These bit positions are used with IOPIN0, IODIR0 registers */
#define EDIT_SWITCH_PIN    10U   /* Edit mode switch on P0.10  (active LOW) */
#define ALARM_SWITCH_PIN   11U   /* Alarm stop switch on P0.11 (active LOW) */

/*-------------- Function Prototypes -----------------------*/

/* SwitchInit() – Configure switch GPIO pins as inputs.
 * Call once during system initialization.                   */
void SwitchInit(void);

/* IsEditSwitchPressed() – Poll the EDIT switch with 20ms debounce.
 * Returns 1 if the switch on P0.10 is currently pressed (LOW).
 * Returns 0 if the switch is not pressed (HIGH).            */
u8   IsEditSwitchPressed(void);

/* IsAlarmSwitchPressed() – Poll the ALARM STOP switch with debounce.
 * Returns 1 if the switch on P0.11 is currently pressed (LOW).
 * Returns 0 if the switch is not pressed (HIGH).            */
u8   IsAlarmSwitchPressed(void);

#endif /* SWITCH_H */
