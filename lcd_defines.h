/*============================================================
 * File         : lcd_defines.h
 * Project      : EnviroTime – Digital Clock + Temperature Monitor
 * Target       : LPC2148 (ARM7TDMI-S)
 * Author       : Embedded Systems Team
 * Date         : 02/05/2026
 * Description  : HD44780 LCD command codes and hardware pin
 *                definitions for the 16x2 LCD interface.
 *
 * HARDWARE PIN CONNECTIONS (Reference / Authoritative Config):
 * ┌──────────────┬─────────────────────────────────────────┐
 * │  LCD Signal  │  LPC2148 Connection                     │
 * ├──────────────┼─────────────────────────────────────────┤
 * │  D0 – D7    │  P0.0  – P0.7   (8-bit data bus)        │
 * │  RS          │  P0.8           (Register Select)        │
 * │  RW          │  Tied to GND    (always write-only)      │
 * │  EN          │  P0.9           (Enable strobe)          │
 * │  VSS (Pin1)  │  GND                                    │
 * │  VDD (Pin2)  │  +5V                                    │
 * │  VEE (Pin3)  │  10K pot middle pin (contrast adjust)   │
 * │  A   (Pin15) │  +5V via 100Ω   (LED backlight anode)   │
 * │  K   (Pin16) │  GND            (LED backlight cathode)  │
 * └──────────────┴─────────────────────────────────────────┘
 *
 * IMPORTANT CHANGE FROM PREVIOUS REVISION:
 *   Old config used P0.8-P0.15 for data and P0.16-P0.18 for control.
 *   New (reference) config uses P0.0-P0.7 for data bus and
 *   P0.8/P0.9 for RS/EN. The RW pin is now hardwired to GND,
 *   freeing P0.10, P0.11 for the Edit and Alarm switches.
 *
 * NOTE: P0.0–P0.9 default to GPIO after reset. No PINSEL change needed.
 *============================================================*/

#ifndef LCD_DEFINES_H
#define LCD_DEFINES_H

/*------------------------------------------------------------
 * HD44780 Instruction Set – Standard Command Codes
 * These are sent to the LCD via CmdLCD() with RS = 0
 *------------------------------------------------------------*/
#define CLEAR_LCD           0x01  /* Clear entire display; cursor to home  */
#define RET_CUR_HOME        0x02  /* Move cursor to Line1 Col0             */
#define SHIFT_CUR_RIGHT     0x06  /* Entry mode: auto-increment cursor     */
#define SHIFT_CUR_LEFT      0x07  /* Entry mode: auto-decrement cursor     */
#define DSP_OFF             0x08  /* Turn display OFF (data preserved)     */
#define DSP_ON_CUR_OFF      0x0C  /* Display ON, cursor invisible          */
#define DSP_ON_CUR_ON       0x0E  /* Display ON, cursor visible underline  */
#define DSP_ON_CUR_BLINK    0x0F  /* Display ON, cursor blinking block     */
#define SHIFT_DISP_LEFT     0x10  /* Shift entire display left by 1        */
#define SHIFT_DISP_RIGHT    0x14  /* Shift entire display right by 1       */
#define MODE_8BIT_1LINE     0x30  /* 8-bit data bus, 1-line display        */
#define MODE_8BIT_2LINE     0x38  /* 8-bit data bus, 2-line display 5x7    */
#define MODE_4BIT_1LINE     0x20  /* 4-bit data bus, 1-line display        */
#define MODE_4BIT_2LINE     0x28  /* 4-bit data bus, 2-line display        */
#define GOTO_LINE1_POS0     0x80  /* DDRAM addr: Line 1, Column 0          */
#define GOTO_LINE2_POS0     0xC0  /* DDRAM addr: Line 2, Column 0          */
#define GOTO_CGRAM_START    0x40  /* CGRAM addr: custom character slot 0   */

/*------------------------------------------------------------
 * LCD Hardware Pin Bit Positions on LPC2148 PORT0
 *
 * LCD_DATA = 0  → 8-bit data bus occupies PORT0 bits 0 to 7
 *                 (P0.0=D0, P0.1=D1, ..., P0.7=D7)
 * LCD_RS   = 8  → Register Select pin on P0.8
 *                 0 = send a command, 1 = send a character/data
 * LCD_EN   = 9  → Enable strobe pin on P0.9
 *                 HIGH then LOW pulse latches data into LCD
 *
 * RW pin: Hardwired to GND on PCB. No MCU pin is allocated.
 *         This permanently selects write-only mode which is
 *         all we need (we never read back from the LCD).
 *------------------------------------------------------------*/
#define LCD_DATA   0    /* Data bus start bit: P0.0 – P0.7   */
#define LCD_RS     8    /* Register Select  : P0.8            */
#define LCD_EN     9    /* Enable (strobe)  : P0.9            */

/*------------------------------------------------------------
 * Compatibility Aliases
 * Used throughout rtc.c, menu.c, security.c, main.c
 *------------------------------------------------------------*/
#define LCD_LINE1   GOTO_LINE1_POS0   /* 0x80 – cursor to Line1 Col0 */
#define LCD_LINE2   GOTO_LINE2_POS0   /* 0xC0 – cursor to Line2 Col0 */
#define LCD_CLEAR   CLEAR_LCD         /* 0x01 – clear display         */

#endif /* LCD_DEFINES_H */
