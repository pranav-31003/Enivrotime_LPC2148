 /*============================================================
 * File    : rtc.c
 * Project : EnviroTime - Digital Clock with Temperature Monitor
 * Author  : Embedded Systems Team
 * Date    : 02/05/2026
 * Brief   : Internal RTC driver for LPC2148
 *           Uses LPC2148 on-chip RTC with 32.768 kHz clock
 *============================================================*/

#include <LPC21xx.H>
#include "types.h"
#include "rtc_defines.h"
#include "rtc.h"
#include "lcd.h"

/*-------------- Day of Week Strings -----------------------*/
/* Index: 0=SUN, 1=MON, 2=TUE, 3=WED, 4=THU, 5=FRI, 6=SAT */
const char week[][4] = { "SUN","MON","TUE","WED","THU","FRI","SAT" };

/*------------------------------------------------------------
 * RTC_Init()
 * Brief : Initializes the LPC2148 on-chip RTC
 *         Resets RTC, selects clock source, enables counting
 *------------------------------------------------------------*/
void RTC_Init(void)
{
    /* Step 1: Reset the RTC to clear all counters */
    CCR = RTC_RESET;

#ifdef _LPC2148
    /* Step 2: Use internal 32.768 kHz oscillator (LPC2148 feature) */
    CCR = RTC_ENABLE | RTC_CLKSRC;
#else
    /* Step 2: Use PCLK-derived clock via prescaler registers */
    PREINT = PREINT_VAL;
    PREFRAC = PREFRAC_VAL;
    CCR = RTC_ENABLE;
#endif
}

/*------------------------------------------------------------
 * GetRTCTimeInfo()
 * Brief : Reads current Hour, Minute, Second from RTC
 * Param : hour   - pointer to store hour   (0-23)
 *         minute - pointer to store minute (0-59)
 *         second - pointer to store second (0-59)
 *------------------------------------------------------------*/
void GetRTCTimeInfo(s32 *hour, s32 *minute, s32 *second)
{
    *hour   = (s32)HOUR;
    *minute = (s32)MIN;
    *second = (s32)SEC;
}

/*------------------------------------------------------------
 * GetRTCDateInfo()
 * Brief : Reads current Date, Month, Year from RTC
 * Param : date  - pointer to store day of month (1-31)
 *         month - pointer to store month         (1-12)
 *         year  - pointer to store year          (0-4095)
 *------------------------------------------------------------*/
void GetRTCDateInfo(s32 *date, s32 *month, s32 *year)
{
    *date  = (s32)DOM;
    *month = (s32)MONTH;
    *year  = (s32)YEAR;
}

/*------------------------------------------------------------
 * GetRTCDay()
 * Brief : Reads current Day of Week from RTC
 * Param : day - pointer to store DOW (0=SUN ... 6=SAT)
 *------------------------------------------------------------*/
void GetRTCDay(s32 *day)
{
    *day = (s32)DOW;
}

/*------------------------------------------------------------
 * SetRTCTimeInfo()
 * Brief : Sets Hour, Minute, Second in RTC registers
 * Param : hour   - hour to set   (0-23)
 *         minute - minute to set (0-59)
 *         second - second to set (0-59)
 *------------------------------------------------------------*/
void SetRTCTimeInfo(u32 hour, u32 minute, u32 second)
{
    HOUR = hour;
    MIN  = minute;
    SEC  = second;
}

/*------------------------------------------------------------
 * SetRTCDateInfo()
 * Brief : Sets Date, Month, Year in RTC registers
 * Param : date  - day of month to set (1-31)
 *         month - month to set         (1-12)
 *         year  - year to set          (0-4095)
 *------------------------------------------------------------*/
void SetRTCDateInfo(u32 date, u32 month, u32 year)
{
    DOM   = date;
    MONTH = month;
    YEAR  = year;
}

/*------------------------------------------------------------
 * SetRTCDay()
 * Brief : Sets Day of Week in RTC register
 * Param : day - day value (0=SUN ... 6=SAT)
 *------------------------------------------------------------*/
void SetRTCDay(u32 day)
{
    DOW = day;
}

/*------------------------------------------------------------
 * DisplayRTCTime()
 * Brief : Formats and displays HH:MM:SS on LCD Line 1
 * Param : hour, minute, second - current time values
 *------------------------------------------------------------*/
void DisplayRTCTime(u32 hour, u32 minute, u32 second)
{
    CmdLCD(LCD_LINE1);

    /* Display HH:MM:SS */
    CharLCD((u8)((hour   / 10) + '0'));
    CharLCD((u8)((hour   % 10) + '0'));
    CharLCD(':');
    CharLCD((u8)((minute / 10) + '0'));
    CharLCD((u8)((minute % 10) + '0'));
    CharLCD(':');
    CharLCD((u8)((second / 10) + '0'));
    CharLCD((u8)((second % 10) + '0'));
}

/*------------------------------------------------------------
 * DisplayRTCDate()
 * Brief : Formats and displays DD/MM/YYYY on LCD Line 2
 * Param : date, month, year - current date values
 *------------------------------------------------------------*/
void DisplayRTCDate(u32 date, u32 month, u32 year)
{
    CmdLCD(LCD_LINE2);

    /* Display DD/MM/YYYY */
    CharLCD((u8)((date  / 10) + '0'));
    CharLCD((u8)((date  % 10) + '0'));
    CharLCD('/');
    CharLCD((u8)((month / 10) + '0'));
    CharLCD((u8)((month % 10) + '0'));
    CharLCD('/');

    /* Display full 4-digit year */
    CharLCD((u8)(((year / 1000) % 10) + '0'));
    CharLCD((u8)(((year / 100)  % 10) + '0'));
    CharLCD((u8)(((year / 10)   % 10) + '0'));
    CharLCD((u8)( (year         % 10) + '0'));
}

/*------------------------------------------------------------
 * DisplayRTCDay()
 * Brief : Displays 3-letter day name at LCD Line 2 Col 12
 * Param : dow - Day of Week (0=SUN ... 6=SAT)
 *------------------------------------------------------------*/
void DisplayRTCDay(u32 dow)
{
    /* Position cursor at Line 2, Column 12 (after DD/MM/YYYY + space) */
    CmdLCD(0xCC);
    StrLCD((u8 *)week[dow]);
}
