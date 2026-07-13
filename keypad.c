/*============================================================
 * File         : keypad.c
 * Project      : EnviroTime – Digital Clock + Temperature Monitor
 * Target       : LPC2148 (ARM7TDMI-S)
 * Author       : Embedded Systems Team
 * Date         : 02/05/2026
 * Description  : 4x4 Matrix Keypad driver using row-scan method.
 *
 * HOW A MATRIX KEYPAD WORKS:
 *   - ROW pins are OUTPUTS. We drive them LOW one at a time.
 *   - COL pins are INPUTS pulled HIGH externally (or by LPC2148).
 *   - When a key is pressed it connects one ROW to one COL wire.
 *   - If ROW is driven LOW and key pressed → that COL reads LOW.
 *   - By checking which COL went LOW we find the column.
 *   - The [row][col] position maps to a character via a LUT.
 *
 * SCANNING METHOD:
 *   Step 1 – Drive ALL rows LOW. Read columns. If any COL = LOW
 *             then some key is pressed (ColScan detects this).
 *   Step 2 – Drive ROW0 LOW, others HIGH. Check COLs.
 *             If a COL is LOW → key is in ROW0.
 *   Step 3 – Repeat for ROW1, ROW2, ROW3 (RowCheck does this).
 *   Step 4 – Once row found, read columns to find column number
 *             (ColCheck does this).
 *   Step 5 – Use keypadMap[row][col] to get the key character.
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
 * KEYPAD LAYOUT (matches keypadMap table below):
 *   [ 1 ] [ 2 ] [ 3 ] [ A ]   ← ROW 0
 *   [ 4 ] [ 5 ] [ 6 ] [ B ]   ← ROW 1
 *   [ 7 ] [ 8 ] [ 9 ] [ C ]   ← ROW 2
 *   [ * ] [ 0 ] [ # ] [ D ]   ← ROW 3
 *    COL0  COL1  COL2  COL3
 *============================================================*/

#include <LPC21xx.h>
#include "types.h"
#include "defines.h"
#include "keypad.h"
#include "lcd.h"
#include "delay.h"

/*------------------------------------------------------------
 * Row Pin Bit Positions on PORT1 (all OUTPUTS)
 *------------------------------------------------------------*/
#define KEYPAD_ROW0   20   /* P1.20 – Row 0 (top row)    */
#define KEYPAD_ROW1   21   /* P1.21 – Row 1              */
#define KEYPAD_ROW2   22   /* P1.22 – Row 2              */
#define KEYPAD_ROW3   23   /* P1.23 – Row 3 (bottom row) */

/*------------------------------------------------------------
 * Column Pin Bit Positions on PORT1 (all INPUTS)
 *------------------------------------------------------------*/
#define KEYPAD_COL0   16   /* P1.16 – Column 0 (left)    */
#define KEYPAD_COL1   17   /* P1.17 – Column 1           */
#define KEYPAD_COL2   18   /* P1.18 – Column 2           */
#define KEYPAD_COL3   19   /* P1.19 – Column 3 (right)   */

/*------------------------------------------------------------
 * Row and Column masks for setting/clearing multiple pins
 *------------------------------------------------------------*/
#define ROW_MASK  ( (1UL<<KEYPAD_ROW0) | (1UL<<KEYPAD_ROW1) | \
                    (1UL<<KEYPAD_ROW2) | (1UL<<KEYPAD_ROW3) )

#define COL_MASK  ( (1UL<<KEYPAD_COL0) | (1UL<<KEYPAD_COL1) | \
                    (1UL<<KEYPAD_COL2) | (1UL<<KEYPAD_COL3) )

/*------------------------------------------------------------
 * Key Character Lookup Table
 * keypadMap[row][col] gives the character for that key.
 * Example: keypadMap[0][2] = '3' (Row0, Col2)
 *          keypadMap[3][3] = 'D' (Row3, Col3)
 *------------------------------------------------------------*/
static const u8 keypadMap[4][4] =
{
    /*  COL0   COL1   COL2   COL3  */
    {   '1',   '2',   '3',   'A'  },   /* ROW 0 – top row    */
    {   '4',   '5',   '6',   'B'  },   /* ROW 1              */
    {   '7',   '8',   '9',   'C'  },   /* ROW 2              */
    {   '*',   '0',   '#',   'D'  }    /* ROW 3 – bottom row */
};

/*============================================================
 * Function : InitKPM
 * Brief    : Initializes GPIO pins for the 4x4 matrix keypad.
 *
 *   ROW pins (P1.20-P1.23) → set as OUTPUT, driven LOW
 *                             (grounding rows for scanning)
 *   COL pins (P1.16-P1.19) → set as INPUT
 *                             (pulled HIGH externally, reads LOW
 *                              when a key connects it to a row)
 *============================================================*/
void InitKPM(void)
{
    /* --- Configure ROW pins as OUTPUT --- */
    IODIR1 |= (1UL << KEYPAD_ROW0);   /* P1.20 = OUTPUT */
    IODIR1 |= (1UL << KEYPAD_ROW1);   /* P1.21 = OUTPUT */
    IODIR1 |= (1UL << KEYPAD_ROW2);   /* P1.22 = OUTPUT */
    IODIR1 |= (1UL << KEYPAD_ROW3);   /* P1.23 = OUTPUT */

    /* --- Configure COL pins as INPUT --- */
    /* Clearing IODIR bits makes them inputs (LPC2148 default) */
    IODIR1 &= ~(1UL << KEYPAD_COL0);  /* P1.16 = INPUT  */
    IODIR1 &= ~(1UL << KEYPAD_COL1);  /* P1.17 = INPUT  */
    IODIR1 &= ~(1UL << KEYPAD_COL2);  /* P1.18 = INPUT  */
    IODIR1 &= ~(1UL << KEYPAD_COL3);  /* P1.19 = INPUT  */

    /* --- Drive all ROW pins LOW --- */
    /* With all rows LOW, any pressed key will pull its COL LOW */
    IOCLR1 = (1UL << KEYPAD_ROW0);
    IOCLR1 = (1UL << KEYPAD_ROW1);
    IOCLR1 = (1UL << KEYPAD_ROW2);
    IOCLR1 = (1UL << KEYPAD_ROW3);
}

/*============================================================
 * Function : ColScan
 * Brief    : Checks if ANY key is currently pressed by reading
 *            all 4 column pins while all rows are LOW.
 *            If any COL reads LOW → that column's key is pressed.
 *
 * Return   : 1 = NO key pressed (all cols HIGH)
 *            0 = Some key IS pressed (at least one col LOW)
 *============================================================*/
u8 ColScan(void)
{
    u8 col0, col1, col2, col3;   /* Individual column pin states */

    /* Read each column pin (1 = HIGH = not pressed, 0 = LOW = pressed) */
    col0 = (u8)((IOPIN1 >> KEYPAD_COL0) & 1UL);   /* Read P1.16 */
    col1 = (u8)((IOPIN1 >> KEYPAD_COL1) & 1UL);   /* Read P1.17 */
    col2 = (u8)((IOPIN1 >> KEYPAD_COL2) & 1UL);   /* Read P1.18 */
    col3 = (u8)((IOPIN1 >> KEYPAD_COL3) & 1UL);   /* Read P1.19 */

    /* If ANY column is LOW (0) then a key is being pressed */
    if ((col0 == 0U) || (col1 == 0U) || (col2 == 0U) || (col3 == 0U))
    {
        return 0U;   /* Key IS pressed */
    }

    return 1U;   /* No key pressed */
}

/*============================================================
 * Function : RowCheck
 * Brief    : Identifies WHICH ROW contains the pressed key.
 *            Activates one row at a time (drives it LOW, others
 *            HIGH) and checks if any column reads LOW.
 *            The row whose activation causes a LOW on any COL
 *            is the row that contains the pressed key.
 *
 * Return   : Row number 0, 1, 2, or 3
 *            Returns 0 by default if detection fails (safety).
 *============================================================*/
u8 RowCheck(void)
{
    u8 rowIndex;   /* Row being tested in each iteration */

    for (rowIndex = 0U; rowIndex < 4U; rowIndex++)
    {
        /* Step A: Drive ALL rows HIGH (deactivate all rows) */
        IOSET1 = (1UL << KEYPAD_ROW0);
        IOSET1 = (1UL << KEYPAD_ROW1);
        IOSET1 = (1UL << KEYPAD_ROW2);
        IOSET1 = (1UL << KEYPAD_ROW3);

        /* Step B: Drive ONLY the current row LOW (activate it) */
        IOCLR1 = (1UL << (KEYPAD_ROW0 + rowIndex));

        /* Step C: Small settling delay for signals to stabilize */
        delay_us(5);

        /* Step D: Check if any column went LOW (key in this row?) */
        if (ColScan() == 0U)   /* 0 means a key IS pressed */
        {
            break;   /* This is the row with the pressed key */
        }
    }

    /* Restore all rows to LOW for normal scanning operation */
    IOCLR1 = (1UL << KEYPAD_ROW0);
    IOCLR1 = (1UL << KEYPAD_ROW1);
    IOCLR1 = (1UL << KEYPAD_ROW2);
    IOCLR1 = (1UL << KEYPAD_ROW3);

    return rowIndex;   /* Return the row number found (0-3) */
}

/*============================================================
 * Function : ColCheck
 * Brief    : Identifies WHICH COLUMN contains the pressed key.
 *            Reads each column pin. The pin that reads LOW (0)
 *            is the column connected to the pressed key.
 *
 * Return   : Column number 0, 1, 2, or 3
 *            Returns 3 by default if not found (rightmost col).
 *============================================================*/
u8 ColCheck(void)
{
    u8 colIndex;   /* Column being checked in each iteration */

    for (colIndex = 0U; colIndex < 4U; colIndex++)
    {
        /* If this column pin is LOW (0) → key is in this column */
        if (((IOPIN1 >> (KEYPAD_COL0 + colIndex)) & 1UL) == 0UL)
        {
            break;   /* Found the column */
        }
    }

    return colIndex;   /* Return the column number found (0-3) */
}

/*============================================================
 * Function : KeyScan
 * Brief    : Full keypad scan — waits for a key press, identifies
 *            the key using RowCheck + ColCheck, then waits for
 *            key release before returning the character.
 *            This function BLOCKS until user presses a key.
 *
 *   Flow:
 *   1. Wait until ColScan() detects any key (polling loop)
 *   2. Debounce: wait 20 ms and confirm key is still pressed
 *   3. Find row using RowCheck()
 *   4. Find column using ColCheck()
 *   5. Validate row/col (safety: ignore phantom detections)
 *   6. Look up character in keypadMap[row][col]
 *   7. Wait for key release + 10ms post-release debounce
 *   8. Return the character
 *
 * Return   : ASCII character of the pressed key
 *            e.g. '0'-'9', '*', '#', 'A'-'D'
 *============================================================*/
u8 KeyScan(void)
{
    u8 rowFound;     /* Row number of the pressed key    */
    u8 colFound;     /* Column number of the pressed key */
    u8 keyChar;      /* ASCII character to return        */

    while (1)
    {
        /*------------------------------------------------------
         * Step 1: Wait until some key is pressed
         * ColScan() returns 0 when a key is pressed
         *------------------------------------------------------*/
        while (ColScan() == 1U)
        {
            /* Busy-wait: no key is pressed yet, keep polling */
        }

        /*------------------------------------------------------
         * Step 2: Debounce delay
         * Mechanical contacts bounce for a few milliseconds.
         * Wait 20 ms then re-check to confirm it's a real press.
         *------------------------------------------------------*/
        delay_ms(20);

        if (ColScan() == 1U)
        {
            /* Signal disappeared → was just electrical noise.
               Restart the polling loop. */
            continue;
        }

        /*------------------------------------------------------
         * Step 3: Find which row has the pressed key
         *------------------------------------------------------*/
        rowFound = RowCheck();

        /*------------------------------------------------------
         * Step 4: Find which column has the pressed key
         *------------------------------------------------------*/
        colFound = ColCheck();

        /*------------------------------------------------------
         * Step 5: Safety check
         * If key was released exactly during scanning,
         * rowFound or colFound will be out-of-bounds (= 4).
         * Ignore this ghost press and restart.
         *------------------------------------------------------*/
        if ((rowFound >= 4U) || (colFound >= 4U))
        {
            continue;   /* Phantom press – restart polling */
        }

        /*------------------------------------------------------
         * Step 6: Lookup the character from the keypad map
         *------------------------------------------------------*/
        keyChar = keypadMap[rowFound][colFound];

        /*------------------------------------------------------
         * Step 7: Wait for key to be fully released
         * ColScan() returns 1 when no key is pressed
         *------------------------------------------------------*/
        while (ColScan() == 0U)
        {
            /* Wait until user lifts their finger off the key */
        }
        delay_ms(10);   /* Post-release debounce */

        /*------------------------------------------------------
         * Step 8: Return the valid key character
         *------------------------------------------------------*/
        return keyChar;
    }
}

/*============================================================
 * Function : ReadNum2
 * Brief    : Reads a multi-digit number from the keypad.
 *            Each digit key ('0'-'9') appends to the number.
 *            Shows each digit on the LCD as it is typed.
 *
 *   Key Mapping During Input:
 *     '0'-'9' → append digit to current number
 *     '*'     → backspace (remove last digit)
 *     '#'     → confirm / finish input (like ENTER key)
 *     'D'     → cancel input (returns 0 with digitCount = 0)
 *     'A','B','C' → ignored
 *
 * Param    : numOut     – pointer to store the final number
 *            digitCount – pointer to store how many digits entered
 *                         (useful to detect empty / cancelled input)
 *
 * Example  : User presses 2, 5, '#'
 *            → *numOut = 25, *digitCount = 2
 *============================================================*/
void ReadNum2(u32 *numOut, u8 *digitCount)
{
    u8  key;                  /* Current key pressed by user        */
    u8  digits[10];           /* Buffer storing each digit (0-9)    */
    u8  count  = 0U;          /* How many digits entered so far      */
    u32 value  = 0UL;         /* Accumulated numeric value           */
    u8  i;                    /* Loop counter for final calculation  */

    /* Initialize output parameters to safe defaults */
    *numOut     = 0UL;
    *digitCount = 0U;

    /* Keep collecting keys until '#' (confirm) or 'D' (cancel) */
    while (1)
    {
        key = KeyScan();   /* Wait for and get one key press */

        if (key == '#')
        {
            /*--------------------------------------------------
             * '#' = Confirm/Enter
             * Calculate final value from digit buffer and return
             *--------------------------------------------------*/
            value = 0UL;
            for (i = 0U; i < count; i++)
            {
                /* Build number: e.g. digits[2,5] → 2*10 + 5 = 25 */
                value = (value * 10UL) + (u32)digits[i];
            }
            *numOut     = value;
            *digitCount = count;
            return;
        }
        else if (key == '*')
        {
            /*--------------------------------------------------
             * '*' = Backspace
             * Remove the last entered digit if any exist
             *--------------------------------------------------*/
            if (count > 0U)
            {
                count--;   /* Reduce digit count by one */

                /* Erase the last character on LCD Line 2 */
                /* Move cursor back one position and overwrite with space */
                CmdLCD(LCD_LINE2);
                /* Reprint everything except the deleted digit */
                value = 0UL;
                for (i = 0U; i < count; i++)
                {
                    value = (value * 10UL) + (u32)digits[i];
                }
                /* Clear line and redisplay remaining digits */
                StrLCD((u8 *)"          ");   /* Erase line 2       */
                CmdLCD(LCD_LINE2);
                if (count == 0U)
                {
                    CharLCD('>');   /* Show empty prompt when nothing left */
                }
                else
                {
                    CharLCD('>');
                    /* Reprint each stored digit */
                    for (i = 0U; i < count; i++)
                    {
                        CharLCD((u8)(digits[i] + '0'));
                    }
                }
            }
        }
        else if (key == 'D')
        {
            /*--------------------------------------------------
             * 'D' = Cancel
             * Return 0 with digitCount = 0 to signal cancellation
             *--------------------------------------------------*/
            *numOut     = 0UL;
            *digitCount = 0U;
            return;
        }
        else if ((key >= '0') && (key <= '9'))
        {
            /*--------------------------------------------------
             * Digit Key '0' to '9'
             * Accept up to 4 digits maximum (covers 0–9999)
             *--------------------------------------------------*/
            if (count < 4U)
            {
                digits[count] = (u8)(key - '0');   /* Store digit value (0-9) */
                count++;

                /* Show the digit on LCD immediately */
                CharLCD(key);
            }
            /* If count >= 4, silently ignore additional digits */
        }
        /* All other keys (A, B, C) are silently ignored */
    }
}
