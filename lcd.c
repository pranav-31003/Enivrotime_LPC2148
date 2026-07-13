/*============================================================
 * File         : lcd.c
 * Project      : EnviroTime – Digital Clock + Temperature Monitor
 * Target       : LPC2148 (ARM7TDMI-S)
 * Author       : Embedded Systems Team
 * Date         : 02/05/2026
 * Description  : 16x2 LCD driver for LPC2148 in 8-bit mode.
 *                Uses WRITEBYTE macro to place data on P0.0-P0.7.
 *                RS and EN control lines on P0.8 and P0.9.
 *                RW line is permanently tied to GND on PCB.
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
 * HOW LCD 8-BIT MODE WORKS:
 *   1. Set RS = 0 to select command register (for commands)
 *      OR RS = 1 to select data register (for characters)
 *   2. Place the full 8-bit byte on data pins D0-D7 (P0.0-P0.7)
 *   3. Pulse EN HIGH then LOW – the LCD latches data on falling edge
 *   4. Wait for LCD to finish processing (busy delay)
 *   RW is always LOW (tied to GND) so we are always writing.
 *
 * NOTE: P0.0–P0.9 are GPIO by default after reset. No PINSEL change.
 *============================================================*/

#include <LPC21xx.h>
#include "types.h"
#include "defines.h"
#include "lcd_defines.h"
#include "lcd.h"
#include "delay.h"

/*------------------------------------------------------------
 * Function : WriteLCD
 * Brief    : Low-level byte write to LCD data bus.
 *            Places the byte on P0.0-P0.7 using WRITEBYTE macro,
 *            then generates a HIGH-to-LOW pulse on EN (P0.9)
 *            to latch the data into the LCD.
 *            RW is already LOW (GND), so no RW manipulation needed.
 * Param    : byte – 8-bit value to send (command or character data)
 *------------------------------------------------------------*/
void WriteLCD(u8 byte)
{
    /* Place the byte on data pins P0.0 to P0.7
     * WRITEBYTE(register, start_bit, value) clears then sets
     * the 8-bit field starting at LCD_DATA (bit 0) in IOPIN0.    */
    WRITEBYTE(IOPIN0, LCD_DATA, byte);

    /* Generate EN HIGH → LOW pulse: LCD latches data on falling edge.
     * EN is on P0.9 (LCD_EN = 9).                                  */
    IOSET0 = (1UL << LCD_EN);    /* EN = 1 – data is valid and stable */
    delay_us(1);                  /* Hold time: must be > 230 ns       */
    IOCLR0 = (1UL << LCD_EN);    /* EN = 0 – falling edge latches data */
    delay_ms(2);                  /* Wait for LCD to finish processing  */
}

/*------------------------------------------------------------
 * Function : CmdLCD
 * Brief    : Sends a command byte to the LCD instruction register.
 *            RS = 0 selects the instruction/command register.
 *            Examples: clear display, set cursor position, etc.
 * Param    : cmd – HD44780 command byte (see lcd_defines.h)
 *------------------------------------------------------------*/
void CmdLCD(u8 cmd)
{
    /* RS = 0 : select instruction/command register
     * RS is on P0.8 (LCD_RS = 8)                                   */
    IOCLR0 = (1UL << LCD_RS);

    /* Send the command byte via the 8-bit data bus */
    WriteLCD(cmd);
}

/*------------------------------------------------------------
 * Function : CharLCD
 * Brief    : Sends an ASCII character to the LCD data register.
 *            RS = 1 selects the DDRAM data register.
 *            The character appears at the current cursor position.
 * Param    : asciiVal – ASCII character code to display
 *------------------------------------------------------------*/
void CharLCD(u8 asciiVal)
{
    /* RS = 1 : select data register (DDRAM)
     * RS is on P0.8 (LCD_RS = 8)                                   */
    IOSET0 = (1UL << LCD_RS);

    /* Send the character byte via the 8-bit data bus */
    WriteLCD(asciiVal);
}

/*------------------------------------------------------------
 * Function : InitLCD
 * Brief    : Performs full HD44780 power-on initialization.
 *            Must be called ONCE before any other LCD function.
 *
 *   Step 1 – Configure GPIO direction for all LCD pins:
 *            P0.0-P0.7 (data bus) and P0.8 (RS), P0.9 (EN) as OUTPUT
 *   Step 2 – Power-on stabilization delay (> 15 ms)
 *   Step 3 – Software reset sequence (3x 0x30) per HD44780 datasheet
 *   Step 4 – Function Set: 8-bit mode, 2 lines, 5x7 font
 *   Step 5 – Display ON, cursor OFF
 *   Step 6 – Clear display
 *   Step 7 – Entry mode: cursor auto-increments (moves right)
 *------------------------------------------------------------*/
void InitLCD(void)
{
    /*----------------------------------------------------------
     * Step 1: Configure GPIO pin directions
     * All LCD data and control pins are on PORT0 and must be outputs.
     *
     * Data bus: P0.0 to P0.7 → set 8 bits starting at bit LCD_DATA (0)
     * RS:       P0.8         → set individually (LCD_RS = 8)
     * EN:       P0.9         → set individually (LCD_EN = 9)
     *
     * RW: tied to GND on PCB, so no IODIR bit is set for it.
     *----------------------------------------------------------*/
    WRITEBYTE(IODIR0, LCD_DATA, 0xFF);   /* P0.0-P0.7 = OUTPUT (data bus) */
    SETBIT(IODIR0, LCD_RS);              /* P0.8 RS   = OUTPUT             */
    SETBIT(IODIR0, LCD_EN);              /* P0.9 EN   = OUTPUT             */

    /*----------------------------------------------------------
     * Step 2: Power-on delay
     * HD44780 requires > 15 ms after VDD reaches 4.5 V before
     * receiving the first command. We wait 15 ms to be safe.
     *----------------------------------------------------------*/
    delay_ms(15);

    /* Ensure RS is 0 (command mode) before reset sequence */
    IOCLR0 = (1UL << LCD_RS);

    /*----------------------------------------------------------
     * Step 3: Software reset sequence (HD44780 datasheet flow)
     * Send 0x30 three times with specific delays to reliably
     * place the LCD into a known state regardless of its
     * initial power condition.
     *----------------------------------------------------------*/
    CmdLCD(0x30);    /* Reset attempt 1 – enter 8-bit mode            */
    delay_ms(4);     /* Wait > 4.1 ms (datasheet requirement)         */
    delay_us(100);
    CmdLCD(0x30);    /* Reset attempt 2                               */
    delay_us(100);   /* Wait > 100 µs                                 */
    CmdLCD(0x30);    /* Reset attempt 3 – LCD is now reliably reset   */

    /*----------------------------------------------------------
     * Step 4: Function Set – 8-bit bus, 2-line display, 5x7 font
     * MODE_8BIT_2LINE = 0x38
     *----------------------------------------------------------*/
    CmdLCD(MODE_8BIT_2LINE);

    /*----------------------------------------------------------
     * Step 5: Display Control – display ON, cursor OFF, blink OFF
     * DSP_ON_CUR_OFF = 0x0C
     *----------------------------------------------------------*/
    CmdLCD(DSP_ON_CUR_OFF);

    /*----------------------------------------------------------
     * Step 6: Clear display – all positions show spaces,
     *         cursor returns to Line 1, Column 0
     * CLEAR_LCD = 0x01; this command takes > 1.52 ms
     *----------------------------------------------------------*/
    CmdLCD(CLEAR_LCD);
    delay_ms(2);     /* Extra wait after clear command */

    /*----------------------------------------------------------
     * Step 7: Entry Mode – cursor auto-moves right after each char,
     *         display does not shift. SHIFT_CUR_RIGHT = 0x06
     *----------------------------------------------------------*/
    CmdLCD(SHIFT_CUR_RIGHT);
}

/*------------------------------------------------------------
 * Function : StrLCD
 * Brief    : Sends each character of a null-terminated C string
 *            to the LCD using CharLCD(). Stops at '\0'.
 * Param    : s – pointer to the ASCII string to display
 *------------------------------------------------------------*/
void StrLCD(u8 *s)
{
    while (*s)
    {
        CharLCD(*s++);   /* Send current character, then advance pointer */
    }
}

/*------------------------------------------------------------
 * Function : IntLCD
 * Brief    : Converts a signed 32-bit integer to ASCII digits
 *            and displays them at the current LCD cursor position.
 *            Handles negative numbers with a leading '-' sign.
 *            Handles zero as a special case.
 * Param    : num – signed 32-bit integer to display
 *------------------------------------------------------------*/
void IntLCD(s32 num)
{
    s32 i = 0;
    u8  digitBuf[10];   /* Buffer to hold extracted digits (reversed order) */

    /* Special case: display '0' directly and return */
    if (num == 0)
    {
        CharLCD('0');
        return;
    }

    /* Handle negative numbers: print '-' then work with absolute value */
    if (num < 0)
    {
        CharLCD('-');   /* Print minus sign */
        num = -num;     /* Convert to positive for digit extraction */
    }

    /* Extract digits in reverse order (least significant first)
     * Example: 123 → digitBuf = ['3','2','1'], i = 3          */
    while (num > 0)
    {
        digitBuf[i++] = (u8)((num % 10) + '0');   /* ASCII: '0'=48 */
        num /= 10;
    }

    /* Print digits in correct order (most significant first) */
    for (--i; i >= 0; i--)
    {
        CharLCD(digitBuf[i]);
    }
}

/*------------------------------------------------------------
 * Function : Lcd2DigitLCD
 * Brief    : Displays a value as exactly 2 digits with leading
 *            zero padding.
 *            Examples: 7 → "07",  23 → "23",  0 → "00"
 *            Used to display hours, minutes, seconds, day, month.
 * Param    : val – value to display (valid range: 0 to 99)
 *------------------------------------------------------------*/
void Lcd2DigitLCD(u8 val)
{
    CharLCD((u8)((val / 10) + '0'));   /* Tens  digit (e.g. '0' for 7) */
    CharLCD((u8)((val % 10) + '0'));   /* Units digit (e.g. '7' for 7) */
}

/*------------------------------------------------------------
 * Function : ClearLCD
 * Brief    : Clears the entire LCD display and returns the cursor
 *            to Line 1, Column 0 (home position).
 *            Uses CLEAR_LCD (0x01) command which takes > 1.52 ms.
 *------------------------------------------------------------*/
void ClearLCD(void)
{
    CmdLCD(CLEAR_LCD);   /* Send clear command to LCD */
    delay_ms(2);          /* Wait for clear to complete (> 1.52 ms) */
}
