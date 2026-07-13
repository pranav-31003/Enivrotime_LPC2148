/*============================================================
 * File         : menu.c
 * Project      : EnviroTime – Digital Clock + Temperature Monitor
 * Target       : LPC2148 (ARM7TDMI-S)
 * Author       : Embedded Systems Team
 * Date         : 02/05/2026
 * Description  : Secure edit menu and alarm management module.
 *
 * MENU STRUCTURE (after successful password authentication):
 *   ┌─────────────────────────────────────────────────────┐
 *   │  1 = CLK Setting                                    │
 *   │      └─ 1 = Time  (HH:MM:SS)                       │
 *   │         2 = Date  (DD/MM/YYYY)                      │
 *   │         3 = Day   (0=SUN to 6=SAT)                  │
 *   │  2 = Alarm        (HH:MM:SS + ON/OFF)               │
 *   │  3 = Password     (Change Password)                 │
 *   │  4 = Settings Set (Save and Return)                 │
 *   └─────────────────────────────────────────────────────┘
 *
 * INPUT KEY MAPPING:
 *   '#' = Confirm value entry
 *   '*' = Backspace / delete last digit
 *   'D' = Cancel / go back
 *   '1'-'4' = Menu selection
 *
 * KEYPAD FUNCTIONS USED:
 *   KeyScan()  – waits for and returns one key press character
 *   ColScan()  – returns 0 if a key is pressed, 1 if no key
 *============================================================*/

#include <LPC21xx.h>
#include "types.h"
#include "menu.h"
#include "lcd.h"
#include "keypad.h"
#include "rtc.h"
#include "buzzer.h"
#include "security.h"
#include "delay.h"
#include "defines.h"

/*------------------------------------------------------------
 * Alarm Global Variables
 * Defined here, declared extern in menu.h
 * Accessible from main.c and other modules via menu.h
 *------------------------------------------------------------*/
s32 alarm_hour      = 0;    /* Alarm trigger hour (0-23)          */
s32 alarm_min       = 0;    /* Alarm trigger minute (0-59)        */
s32 alarm_sec       = 0;    /* Alarm trigger second (0-59)        */
u8  alarm_enabled   = 0U;   /* 0 = alarm OFF, 1 = alarm ON        */
u8  alarm_triggered = 0U;   /* 1 = alarm is currently ringing     */

/*------------------------------------------------------------
 * Private Function Prototypes
 * Only accessible within this file
 *------------------------------------------------------------*/
static void EditTime(void);
static void EditDate(void);
static void EditDay(void);
static void ClkSetting(void);   /* Unified CLK Setting sub-menu (Time+Date+Day) */
static void SetAlarmTime(void);
static s32  InputValue(u8 *prompt, s32 minVal, s32 maxVal);

/*============================================================
 * Function : MenuInit
 * Brief    : Initializes alarm state variables to safe defaults.
 *            Call once during system startup in main.c.
 *============================================================*/
void MenuInit(void)
{
    alarm_hour      = 0;
    alarm_min       = 0;
    alarm_sec       = 0;
    alarm_enabled   = 0U;
    alarm_triggered = 0U;
}

/*============================================================
 * Function : InputValue  (PRIVATE)
 * Brief    : Generic numeric input function.
 *            Shows a prompt on Line1, collects digit key presses,
 *            validates the entered value against a min/max range,
 *            and returns the valid value.
 *
 *   Keys:
 *     '0'-'9' → append digit
 *     '*'     → backspace (remove last digit)
 *     '#'     → confirm entry
 *     'D'     → cancel (returns -1)
 *
 * Param    : prompt  – message string shown on LCD Line1
 *            minVal  – minimum acceptable value (inclusive)
 *            maxVal  – maximum acceptable value (inclusive)
 * Return   : Entered value if valid and in range
 *            -1 if user cancelled with 'D'
 *============================================================*/
static s32 InputValue(u8 *prompt, s32 minVal, s32 maxVal)
{
    s32 value    = 0;    /* Accumulated numeric value         */
    u8  key;             /* Key character from KeyScan()      */
    u8  hasInput = 0U;   /* 1 = at least one digit entered    */

    /* Show prompt and entry area on LCD */
    ClearLCD();
    CmdLCD(LCD_LINE1);
    StrLCD(prompt);         /* Show instruction on Line 1         */
    CmdLCD(LCD_LINE2);
    CharLCD('>');           /* Show '>' as entry cursor on Line 2 */

    while (1)
    {
        /* Wait for a key press (KeyScan blocks until key is pressed) */
        key = KeyScan();

        if (key == '#')
        {
            /*--------------------------------------------------
             * '#' = Confirm entry
             *--------------------------------------------------*/
            if (hasInput == 0U)
            {
                /* Nothing entered yet – ignore confirm */
                continue;
            }

            if ((value >= minVal) && (value <= maxVal))
            {
                /* Value is within range – accept and return */
                return value;
            }
            else
            {
                /* Value out of range – show error and reset */
                ClearLCD();
                CmdLCD(LCD_LINE1);
                StrLCD((u8 *)"Out of Range!");
                CmdLCD(LCD_LINE2);
                StrLCD((u8 *)"Min:");
                IntLCD(minVal);
                StrLCD((u8 *)" Max:");
                IntLCD(maxVal);
                delay_ms(1500U);

                /* Reset and show prompt again */
                value    = 0;
                hasInput = 0U;
                ClearLCD();
                CmdLCD(LCD_LINE1);
                StrLCD(prompt);
                CmdLCD(LCD_LINE2);
                CharLCD('>');
            }
        }
        else if (key == '*')
        {
            /*--------------------------------------------------
             * '*' = Backspace – remove last digit
             *--------------------------------------------------*/
            value /= 10;   /* Integer division removes last digit */
            CmdLCD(LCD_LINE2);
            StrLCD((u8 *)"     ");   /* Erase current line 2 content */
            CmdLCD(LCD_LINE2);
            CharLCD('>');
            if (value > 0)
            {
                IntLCD(value);   /* Redisplay remaining digits */
            }
            else
            {
                hasInput = 0U;   /* Back to empty state */
            }
        }
        else if (key == 'D')
        {
            /*--------------------------------------------------
             * 'D' = Cancel – return -1 to signal cancellation
             *--------------------------------------------------*/
            return -1;
        }
        else if ((key >= '0') && (key <= '9'))
        {
            /*--------------------------------------------------
             * Digit key – append digit to current value
             *--------------------------------------------------*/
            if (value < 9999)
            {
                value    = (value * 10) + (key - '0');
                hasInput = 1U;

                /* Show updated value on Line 2 */
                CmdLCD(LCD_LINE2);
                CharLCD('>');
                IntLCD(value);
            }
            /* Silently ignore digits that would exceed 9999 */
        }
        /* All other keys (A, B, C) are silently ignored */
    }
}

/*============================================================
 * Function : EditTime  (PRIVATE)
 * Brief    : Sub-menu to edit RTC Hour, Minute, and Second.
 *            LCD shows: "1H 2M 3S" / "D=Back"
 *            Press 1 to edit Hour, 2 for Minute, 3 for Second.
 *            Press D to return to main edit menu.
 *============================================================*/
static void EditTime(void)
{
    s32 val;   /* Value returned from InputValue() */
    u8  key;   /* Menu selection key from KeyScan() */

    while (1)
    {
        /* Show time edit sub-menu */
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"1H 2M 3S");
        CmdLCD(LCD_LINE2);
        StrLCD((u8 *)"D=Back");

        /* Wait for user menu selection */
        key = KeyScan();

        switch (key)
        {
            case '1':
                /* Edit Hour: valid range 0 to 23 */
                val = InputValue((u8 *)"Set Hour(0-23)", 0, 23);
                if (val >= 0) { HOUR = (u32)val; }   /* Write to RTC */
                break;

            case '2':
                /* Edit Minute: valid range 0 to 59 */
                val = InputValue((u8 *)"Set Min(0-59)", 0, 59);
                if (val >= 0) { MIN = (u32)val; }    /* Write to RTC */
                break;

            case '3':
                /* Edit Second: valid range 0 to 59 */
                val = InputValue((u8 *)"Set Sec(0-59)", 0, 59);
                if (val >= 0) { SEC = (u32)val; }    /* Write to RTC */
                break;

            case 'D':
                return;   /* Back to main edit menu */

            default:
                break;    /* Any other key – redisplay sub-menu */
        }
    }
}

/*============================================================
 * Function : EditDate  (PRIVATE)
 * Brief    : Sub-menu to edit RTC Date, Month, and Year.
 *            LCD shows: "1D 2M 3Y" / "D=Back"
 *            Press 1 for Date, 2 for Month, 3 for Year.
 *============================================================*/
static void EditDate(void)
{
    s32 val;
    u8  key;

    while (1)
    {
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"1D 2M 3Y");
        CmdLCD(LCD_LINE2);
        StrLCD((u8 *)"D=Back");

        key = KeyScan();

        switch (key)
        {
            case '1':
                /* Edit Date (day of month): valid range 1 to 31 */
                val = InputValue((u8 *)"Set Date(1-31)", 1, 31);
                if (val >= 0) { DOM = (u32)val; }
                break;

            case '2':
                /* Edit Month: valid range 1 to 12 */
                val = InputValue((u8 *)"Set Month(1-12)", 1, 12);
                if (val >= 0) { MONTH = (u32)val; }
                break;

            case '3':
                /* Edit Year: valid range 2000 to 4095 */
                val = InputValue((u8 *)"Set Year(2000-)", 2000, 4095);
                if (val >= 0) { YEAR = (u32)val; }
                break;

            case 'D':
                return;

            default:
                break;
        }
    }
}

/*============================================================
 * Function : EditDay  (PRIVATE)
 * Brief    : Sub-menu to edit the Day of Week in RTC.
 *            LPC2148 DOW register: 0=SUN, 1=MON … 6=SAT
 *            Brief hint shown first, then InputValue collects 0-6.
 *============================================================*/
static void EditDay(void)
{
    s32 val;

    /* Show brief hint before InputValue clears screen */
    ClearLCD();
    CmdLCD(LCD_LINE1);
    StrLCD((u8 *)"0=SUN 6=SAT");
    delay_ms(1000U);

    val = InputValue((u8 *)"Set Day(0-6)", 0, 6);

    if (val >= 0)
    {
        DOW = (u32)val;   /* Write day of week to RTC */
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"Day Updated!");
        delay_ms(1000U);
    }
}

/*============================================================
 * Function : ClkSetting  (PRIVATE)
 * Brief    : Unified "CLK Setting" sub-menu that consolidates
 *            Time, Date, and Day into a single context-sensitive
 *            group, accessible as option 1 from the main menu.
 *
 *   LCD Line1: "1=Time 2=Date"
 *   LCD Line2: "3=Day  D=Back"
 *
 *   Press 1 → EditTime()  (set HH:MM:SS)
 *   Press 2 → EditDate()  (set DD/MM/YYYY)
 *   Press 3 → EditDay()   (set day 0=SUN to 6=SAT)
 *   Press D → Return to main edit menu
 *============================================================*/
static void ClkSetting(void)
{
    u8 key;   /* Menu selection key from KeyScan() */

    while (1)
    {
        /* Display CLK Setting sub-menu on LCD */
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"1=Time 2=Date");
        CmdLCD(LCD_LINE2);
        StrLCD((u8 *)"3=Day  D=Back");

        /* Wait for user selection */
        key = KeyScan();

        switch (key)
        {
            case '1':
                /* Navigate into Time edit sub-menu */
                EditTime();
                break;

            case '2':
                /* Navigate into Date edit sub-menu */
                EditDate();
                break;

            case '3':
                /* Navigate into Day-of-week edit */
                EditDay();
                break;

            case 'D':
                /* 'D' = Back to main edit menu */
                return;

            default:
                break;   /* Any other key – redisplay sub-menu */
        }
    }
}

/*============================================================
 * Brief    : Sub-menu to set alarm hour and minute, and
 *            to toggle alarm ON or OFF.
 *
 *   LCD Line1: "1=Set 2=On/Off"
 *   LCD Line2: Current alarm time and state e.g. "Alm:07:30 ON"
 *
 *   Press 1 → Enter alarm HH and MM
 *   Press 2 → Toggle alarm ON ↔ OFF
 *============================================================*/
static void SetAlarmTime(void)
{
    s32 val;
    u8  key;

    while (1)
    {
        /* Show current alarm status */
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"1=Set 2=On/Off");
        CmdLCD(LCD_LINE2);
        StrLCD((u8 *)"Alm:");
        Lcd2DigitLCD((u8)alarm_hour);
        CharLCD(':');
        Lcd2DigitLCD((u8)alarm_min);
        CharLCD(':');
        Lcd2DigitLCD((u8)alarm_sec);
        CharLCD(' ');
        StrLCD((u8 *)(alarm_enabled ? "ON" : "OFF"));

        key = KeyScan();

        switch (key)
        {
            case '1':
                /*--------------------------------------------------
                 * Set alarm time: get hour, minute, and second
                 *--------------------------------------------------*/
                val = InputValue((u8 *)"Alarm Hr(0-23)", 0, 23);
                if (val >= 0)
                {
                    alarm_hour = val;

                    val = InputValue((u8 *)"Alarm Min(0-59)", 0, 59);
                    if (val >= 0)
                    {
                        alarm_min = val;

                        /* Bug Fix 1: Collect seconds so they are not
                         * silently forced to zero when not entered  */
                        val = InputValue((u8 *)"Alarm Sec(0-59)", 0, 59);
                        if (val >= 0)
                        {
                            alarm_sec       = val;
                            alarm_enabled   = 1U;   /* Auto-enable alarm when set */
                            alarm_triggered = 0U;   /* Reset trigger flag         */

                            /* Confirm alarm time on LCD */
                            ClearLCD();
                            CmdLCD(LCD_LINE1);
                            StrLCD((u8 *)"Alarm Set:");
                            CmdLCD(LCD_LINE2);
                            Lcd2DigitLCD((u8)alarm_hour);
                            CharLCD(':');
                            Lcd2DigitLCD((u8)alarm_min);
                            CharLCD(':');
                            Lcd2DigitLCD((u8)alarm_sec);
                            delay_ms(1500U);
                        }
                    }
                }
                break;

            case '2':
                /*--------------------------------------------------
                 * Toggle alarm ON/OFF
                 *--------------------------------------------------*/
                alarm_enabled   = (alarm_enabled == 0U) ? 1U : 0U;
                alarm_triggered = 0U;   /* Reset trigger state on toggle */
                BuzzerOff();            /* Stop any active buzzer         */

                ClearLCD();
                CmdLCD(LCD_LINE1);
                StrLCD((u8 *)"Alarm:");
                StrLCD((u8 *)(alarm_enabled ? "ENABLED" : "DISABLED"));
                delay_ms(1000U);
                break;

            case '3':
                /*--------------------------------------------------
                 * Bug Fix 2: SW2 (Reset) inside Set Alarm menu
                 * Pressing '3' (mapped to Reset) clears all alarm
                 * settings and disables the alarm completely.
                 * In your keypad layout map the Reset function to
                 * key '3' inside this sub-menu, or press 'B'.
                 *--------------------------------------------------*/
                /* Fall-through: also handle 'B' as Reset */
            case 'B':
                alarm_hour      = 0;
                alarm_min       = 0;
                alarm_sec       = 0;
                alarm_enabled   = 0U;
                alarm_triggered = 0U;
                BuzzerOff();

                ClearLCD();
                CmdLCD(LCD_LINE1);
                StrLCD((u8 *)"Alarm Reset!");
                CmdLCD(LCD_LINE2);
                StrLCD((u8 *)"Alarm OFF");
                delay_ms(1200U);
                break;

            case 'D':
                /* 'D' = Back to main edit menu */
                return;

            default:
                break;
        }
    }
}

/*============================================================
 * Function : EnterEditMenu
 * Brief    : Main edit menu displayed after successful password
 *            authentication. Presents a consolidated 4-option
 *            menu on the LCD.
 *
 * NEW MENU STRUCTURE:
 *   '1' → CLK Setting  (sub-menu: 1=Time, 2=Date, 3=Day)
 *   '2' → Alarm        (set alarm HH:MM:SS / ON/OFF)
 *   '3' → Password     (change password)
 *   '4' → Settings Set (save and return to monitoring mode)
 *   'D' → Exit immediately without confirmation
 *
 * LCD display cycles between two label rows:
 *   Line1: "1=CLK  2=Alarm"
 *   Line2: "3=Pwd  4=Set"
 *
 * ATM-style auto-timeout (Bug Fix 3):
 *   If no key is pressed for MENU_TIMEOUT_MS milliseconds,
 *   the menu exits automatically and returns to monitoring.
 *============================================================*/
void EnterEditMenu(void)
{
    u8  key;
    u8  stay        = 1U;   /* Loop control: 1 = stay in menu, 0 = exit */
    u32 idleCount   = 0U;   /* Counts idle poll cycles with no key press */

    /*----------------------------------------------------------
     * Bug Fix 3: ATM-style auto-timeout
     * MENU_TIMEOUT_MS = total ms before auto-exit (e.g. 15000 = 15s)
     * MENU_POLL_MS    = poll interval per cycle (e.g. 100 ms)
     * MENU_IDLE_LIMIT = MENU_TIMEOUT_MS / MENU_POLL_MS cycles
     *----------------------------------------------------------*/
#define MENU_TIMEOUT_MS  15000U   /* 15 seconds idle → auto-exit  */
#define MENU_POLL_MS     100U     /* Check for key every 100 ms   */
#define MENU_IDLE_LIMIT  (MENU_TIMEOUT_MS / MENU_POLL_MS)

    while (stay == 1U)
    {
        /* Display consolidated main edit menu on LCD:
         *   Line1: "1=CLK  2=Alarm"
         *   Line2: "3=Pwd  4=Set"                         */
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"1=CLK  2=Alarm");
        CmdLCD(LCD_LINE2);
        StrLCD((u8 *)"3=Pwd  4=Set");

        /*------------------------------------------------------
         * ATM-style idle detection:
         * Poll ColScan() every MENU_POLL_MS ms.
         * ColScan() returns 0 when ANY key is pressed, 1 if none.
         * If no key within MENU_IDLE_LIMIT cycles → auto-exit.
         *------------------------------------------------------*/
        idleCount = 0U;
        while (ColScan() != 0U)          /* Wait for any key press  */
        {
            delay_ms(MENU_POLL_MS);
            idleCount++;
            if (idleCount >= MENU_IDLE_LIMIT)
            {
                /* Timeout reached – show message and exit menu */
                ClearLCD();
                CmdLCD(LCD_LINE1);
                StrLCD((u8 *)"Menu Timeout");
                CmdLCD(LCD_LINE2);
                StrLCD((u8 *)"Returning...");
                delay_ms(1200U);
                return;   /* Auto-exit to main monitoring loop */
            }
        }

        /* A key is now pressed – read it via KeyScan */
        key = KeyScan();

        switch (key)
        {
            case '1':
                /*----------------------------------------------
                 * Option 1: CLK Setting
                 * Unified sub-menu containing Time, Date, Day.
                 * LCD shows: "1=Time 2=Date" / "3=Day  D=Back"
                 *----------------------------------------------*/
                ClkSetting();
                break;

            case '2':
                /*----------------------------------------------
                 * Option 2: Alarm
                 * Formerly option 4 in the previous structure.
                 * Set alarm HH:MM:SS and toggle ON/OFF.
                 *----------------------------------------------*/
                SetAlarmTime();
                break;

            case '3':
                /*----------------------------------------------
                 * Option 3: Password
                 * Formerly option 5 in the previous structure.
                 * Verify old password then set new password.
                 *----------------------------------------------*/
                ChangePassword();
                break;

            case '4':
                /*----------------------------------------------
                 * Option 4: Settings Set
                 * Formerly option 6 (Exit/Save) in the previous
                 * structure. Saves all settings and returns to
                 * the main monitoring loop.
                 *----------------------------------------------*/
                ClearLCD();
                CmdLCD(LCD_LINE1);
                StrLCD((u8 *)"Settings Saved");
                CmdLCD(LCD_LINE2);
                StrLCD((u8 *)"Returning...");
                BuzzerBeep(150U);    /* Short confirmation beep */
                delay_ms(1200U);
                stay = 0U;           /* Exit the menu loop      */
                break;

            case 'D':
                /* 'D' exits without explicit confirmation */
                stay = 0U;
                break;

            default:
                break;
        }
    }
}

/*============================================================
 * Function : CheckAlarm
 * Brief    : Called continuously in the main loop.
 *            Compares current RTC time with the stored alarm time.
 *            When Hour and Minute match, activates the buzzer
 *            and shows an alarm message on the LCD.
 *            The alarm is only triggered once (alarm_triggered flag
 *            prevents repeated activation in the same minute).
 *
 * Note     : Seconds are not compared – the match window is
 *            the entire minute (60 seconds), so the alarm
 *            cannot be missed even with brief LCD delays.
 *============================================================*/
void CheckAlarm(void)
{
    s32 curHour, curMin, curSec;   /* Current RTC values */

    /* Only check if alarm is enabled and not already ringing */
    if ((alarm_enabled == 0U) || (alarm_triggered == 1U))
    {
        return;   /* Nothing to do */
    }

    /* Read current time from RTC registers */
    GetRTCTimeInfo(&curHour, &curMin, &curSec);

    /* Match hour, minute, and second with alarm settings */
    if ((curHour == alarm_hour) && (curMin == alarm_min) && (curSec == alarm_sec))
    {
        alarm_triggered = 1U;   /* Set flag so alarm fires only once */

        /* Show alarm notification on LCD */
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"** ALARM!! **");
        CmdLCD(LCD_LINE2);
        StrLCD((u8 *)"Press SW2 Stop");

        /* Activate buzzer (stays ON until user presses alarm switch) */
        BuzzerOn();
    }
}
