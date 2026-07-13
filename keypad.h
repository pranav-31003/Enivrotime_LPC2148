/*============================================================
 * File         : keypad.h
 * Project      : EnviroTime – Digital Clock + Temperature Monitor
 * Target       : LPC2148 (ARM7TDMI-S)
 * Author       : Embedded Systems Team
 * Date         : 02/05/2026
 * Description  : Function prototypes for the 4x4 matrix keypad
 *                driver. Pin configuration in keypad.c header.
 *
 * HARDWARE PIN CONNECTIONS:
 * ┌──────────────┬──────────────────────────────────┐
 * │  Signal      │  LPC2148 Pin                     │
 * ├──────────────┼──────────────────────────────────┤
 * │  ROW 0       │  P1.20  (OUTPUT – driven LOW)    │
 * │  ROW 1       │  P1.21  (OUTPUT – driven LOW)    │
 * │  ROW 2       │  P1.22  (OUTPUT – driven LOW)    │
 * │  ROW 3       │  P1.23  (OUTPUT – driven LOW)    │
 * │  COL 0       │  P1.16  (INPUT  – pulled HIGH)   │
 * │  COL 1       │  P1.17  (INPUT  – pulled HIGH)   │
 * │  COL 2       │  P1.18  (INPUT  – pulled HIGH)   │
 * │  COL 3       │  P1.19  (INPUT  – pulled HIGH)   │
 * └──────────────┴──────────────────────────────────┘
 *
 * KEYPAD CHARACTER LAYOUT:
 *   [ 1 ] [ 2 ] [ 3 ] [ A ]
 *   [ 4 ] [ 5 ] [ 6 ] [ B ]
 *   [ 7 ] [ 8 ] [ 9 ] [ C ]
 *   [ * ] [ 0 ] [ # ] [ D ]
 *
 * KEY USAGE IN EnviroTime:
 *   '#' = Confirm / Enter value
 *   '*' = Backspace / delete last digit
 *   'D' = Cancel / go back
 *   '1'-'9','0' = Numeric input
 *============================================================*/

#ifndef KEYPAD_H
#define KEYPAD_H

#include "types.h"

/*------------------------------------------------------------
 * InitKPM   – Configure ROW and COL GPIO pins
 * ColScan   – Returns 1 if NO key pressed, 0 if key pressed
 * RowCheck  – Scans rows; returns row number (0-3) of pressed key
 * ColCheck  – Reads columns; returns col number (0-3) of pressed key
 * KeyScan   – Full scan: waits, identifies and returns key char
 * ReadNum2  – Reads multi-digit number; stores value and digit count
 *------------------------------------------------------------*/
void InitKPM(void);
u8   ColScan(void);
u8   RowCheck(void);
u8   ColCheck(void);
u8   KeyScan(void);
void ReadNum2(u32 *numOut, u8 *digitCount);

#endif /* KEYPAD_H */
