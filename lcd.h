/*============================================================
 * File         : lcd.h
 * Project      : EnviroTime – Digital Clock + Temperature Monitor
 * Target       : LPC2148 (ARM7TDMI-S)
 * Author       : Embedded Systems Team
 * Date         : 02/05/2026
 * Description  : Function prototypes for the 16x2 HD44780 LCD
 *                driver operating in 8-bit interface mode.
 *
 * CONNECTED HARDWARE (Reference Pin Config):
 *   Data bus D0-D7 : P0.0 – P0.7
 *   Register Select: P0.8  (RS)
 *   Enable strobe  : P0.9  (EN)
 *   Read/Write     : Tied to GND (write-only, no MCU pin needed)
 *
 * INCLUDE THIS FILE in any module that needs to write to the LCD.
 * Implementation is in lcd.c.  Pin definitions are in lcd_defines.h.
 *============================================================*/

#ifndef LCD_H
#define LCD_H

#include "types.h"
#include "lcd_defines.h"

/*------------------------------------------------------------
 * WriteLCD  – Low-level: write one byte to data bus + EN pulse
 *             Called by CmdLCD and CharLCD; rarely called directly.
 *
 * CmdLCD    – Send a command byte to the LCD (RS = 0).
 *             Use for: clear, cursor set, init commands.
 *
 * CharLCD   – Send a character byte to the LCD (RS = 1).
 *             The character appears at the current cursor position.
 *
 * InitLCD   – Full HD44780 power-on initialization sequence.
 *             Must be called ONCE before any other LCD function.
 *
 * StrLCD    – Display a null-terminated ASCII string at cursor.
 *
 * IntLCD    – Display a signed 32-bit integer as decimal digits.
 *             Handles negative numbers with a '-' prefix.
 *
 * Lcd2DigitLCD – Display a value as exactly 2 digits (with leading 0).
 *                Examples: 7 → "07", 23 → "23". Used for HH:MM:SS.
 *
 * ClearLCD  – Clear display and return cursor to home (Line1, Col0).
 *------------------------------------------------------------*/
void WriteLCD(u8 byte);
void CmdLCD(u8 cmd);
void CharLCD(u8 asciiVal);
void InitLCD(void);
void StrLCD(u8 *s);
void IntLCD(s32 num);
void Lcd2DigitLCD(u8 val);
void ClearLCD(void);

#endif /* LCD_H */
