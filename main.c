/*============================================================
 * File         : main.c
 * Project      : EnviroTime – Digital Clock + Temperature Monitor
 * Target       : LPC2148 (ARM7TDMI-S) @ 60 MHz
 * Author       : Embedded Systems Team
 * Date         : 02/05/2026
 *
 * DESCRIPTION:
 *   EnviroTime continuously displays on a 16x2 LCD:
 *     Line 1 → HH:MM:SS  T:XX°C   (current time + temperature)
 *     Line 2 → DD/MM/YYYY  DAY     (current date + day of week)
 *   It also provides:
 *     - Alarm: buzzer sounds when RTC matches alarm time
 *     - Edit menu: password-protected RTC/alarm/password settings
 *
 * LCD DISPLAY FORMAT:
 *   ┌────────────────┐
 *   │12:45:22 T:28C  │  ← Line 1: HH:MM:SS  T:XX°C
 *   │02/05/2026 FRI  │  ← Line 2: DD/MM/YYYY  DAY
 *   └────────────────┘
 *
 * COMPLETE PIN CONNECTIONS (Reference / Authoritative Config):
 * ┌───────────────────┬─────────────────────────────────────────┐
 * │  Peripheral       │  LPC2148 Pins                           │
 * ├───────────────────┼─────────────────────────────────────────┤
 * │  LCD Data D0-D7   │  P0.0  – P0.7   (8-bit data bus)       │
 * │  LCD RS           │  P0.8           (Register Select)       │
 * │  LCD RW           │  Tied to GND    (write-only; no MCU pin)│
 * │  LCD EN           │  P0.9           (Enable strobe)         │
 * │  Edit Switch SW1  │  P0.10          (INPUT, active LOW)     │
 * │  Alarm Switch SW2 │  P0.11          (INPUT, active LOW)     │
 * │  Buzzer output    │  P0.12          (OUTPUT, active HIGH)   │
 * │  LM35 sensor      │  P0.28          (AIN1 – ADC CH1)        │
 * │  Keypad ROW0-3    │  P1.20 – P1.23  (OUTPUT)               │
 * │  Keypad COL0-3    │  P1.16 – P1.19  (INPUT)                │
 * └───────────────────┴─────────────────────────────────────────┘
 *
 * PROGRAM FLOW (main loop):
 *   1. SystemInit_EnviroTime() – init all hardware, show splash
 *   2. Infinite loop:
 *      a. DisplayMonitorScreen() – read RTC + temp, update LCD
 *      b. CheckAlarm()           – ring buzzer if alarm time reached
 *      c. IsAlarmSwitchPressed() – stop buzzer if SW2 pressed
 *      d. IsEditSwitchPressed()  – enter password-protected edit menu
 *      e. delay_ms(200)          – throttle refresh rate
 *
 * MODULE DEPENDENCIES:
 *   main.c → lcd.h, rtc.h, adc.h, keypad.h, buzzer.h, switch.h,
 *             security.h, menu.h, delay.h, types.h, defines.h
 *============================================================*/

#include <LPC21xx.h>
#include "types.h"
#include "defines.h"
#include "delay.h"
#include "lcd.h"
#include "rtc.h"
#include "adc.h"
#include "adc_defines.h"
#include "keypad.h"
#include "buzzer.h"
#include "switch.h"
#include "security.h"
#include "menu.h"

/*------------------------------------------------------------
 * Private Function Prototypes
 * These functions are only used within main.c (not exported)
 *------------------------------------------------------------*/
static void SystemInit_EnviroTime(void);   /* Hardware init + splash     */
static void DisplayMonitorScreen(void);    /* Read and display RTC+temp  */
static s32  ReadTemperature(void);         /* Read LM35 via ADC CH1      */

/*============================================================
 * Function : SystemInit_EnviroTime  (PRIVATE)
 * Brief    : Initializes ALL hardware peripherals in the correct
 *            order before the main monitoring loop begins.
 *            Also sets an initial RTC time/date for demonstration
 *            and shows a 2-second splash screen on the LCD.
 *
 * Initialization order is important:
 *   InitLCD first (so all other modules can display status if needed)
 *   RTC, ADC, Keypad, Buzzer, Switches, Security, Menu in any order after
 *============================================================*/
static void SystemInit_EnviroTime(void)
{
    /* 1. Initialize 16x2 LCD in 8-bit mode
     *    Data bus: P0.0-P0.7 | RS: P0.8 | EN: P0.9
     *    RW is hardwired to GND on PCB (no MCU pin needed)           */
    InitLCD();

    /* 2. Initialize on-chip RTC peripheral
     *    Uses external 32.768 kHz crystal on RTCX1/RTCX2 pins
     *    for accurate 1-second time increments                        */
    RTC_Init();

    /* 3. Initialize ADC for LM35 temperature sensor
     *    LM35 is connected to P0.28 = AD0.1 = Channel 1 (CH1)
     *    PINSEL1[25:24] = 01 selects analog function for P0.28        */
    InitADC(LM35_CHANNEL);   /* LM35_CHANNEL = CH1 = 1, defined in adc_defines.h */

    /* 4. Initialize 4x4 matrix keypad GPIO pins
     *    ROW outputs: P1.20(R0), P1.21(R1), P1.22(R2), P1.23(R3)
     *    COL inputs:  P1.16(C0), P1.17(C1), P1.18(C2), P1.19(C3)    */
    InitKPM();

    /* 5. Initialize buzzer transistor output
     *    Buzzer control: P0.12 (active HIGH, drives NPN transistor)   */
    BuzzerInit();

    /* 6. Initialize EDIT and ALARM push-button switches as GPIO inputs
     *    EDIT  switch (SW1): P0.10 — active LOW, external pull-up
     *    ALARM switch (SW2): P0.11 — active LOW, external pull-up     */
    SwitchInit();

    /* 7. Initialize password security module
     *    Sets default password to "1234", resets wrong-attempt counter */
    SecurityInit();

    /* 8. Initialize alarm and menu state variables to safe defaults
     *    alarm_hour=0, alarm_min=0, alarm_enabled=0, alarm_triggered=0 */
    MenuInit();

    /*----------------------------------------------------------
     * Set Initial RTC Time and Date
     * These values are written to the RTC registers at startup.
     * Modify them to match the actual current time before flashing.
     *
     * SetRTCTimeInfo(HH, MM, SS)     → hours, minutes, seconds
     * SetRTCDateInfo(DD, MM, YYYY)   → day, month, year
     * SetRTCDay(n) → 0=SUN,1=MON,2=TUE,3=WED,4=THU,5=FRI,6=SAT
     *----------------------------------------------------------*/
    SetRTCTimeInfo(12U, 0U, 0U);    /* Start at 12:00:00           */
    SetRTCDateInfo(2U, 5U, 2026U);  /* Date: 02 May 2026           */
    SetRTCDay(5U);                  /* Friday = day index 5        */

    /*----------------------------------------------------------
     * Startup Splash Screen
     * Displayed for 2 seconds on the LCD so the user knows
     * the system has booted and initialized successfully.
     *----------------------------------------------------------*/
    ClearLCD();
    CmdLCD(LCD_LINE1);
    StrLCD((u8 *)"  EnviroTime  ");    /* Project name on Line 1 */
    CmdLCD(LCD_LINE2);
    StrLCD((u8 *)"Digital Clock+T"); /* Description on Line 2  */
    delay_ms(2000U);                  /* Show splash for 2 seconds */
    ClearLCD();
}

/*============================================================
 * Function : ReadTemperature  (PRIVATE)
 * Brief    : Reads the LM35 temperature sensor via ADC Channel 1
 *            (P0.28 = AD0.1) and returns the temperature in °C.
 *
 * LM35 characteristics:
 *   - Outputs exactly 10 mV per degree Celsius
 *   - At 25°C the output voltage is 250 mV (0.250 V)
 *   - At 30°C the output voltage is 300 mV (0.300 V)
 *
 * Conversion:
 *   ReadADC gives voltage in Volts (e.g. 0.28V)
 *   Temperature = voltage × 100  (since 1°C = 10mV = 0.01V)
 *   So: 0.28V × 100 = 28°C
 *
 * Return   : Temperature as a signed integer in degrees Celsius
 *============================================================*/
static s32 ReadTemperature(void)
{
    u32 rawADC;      /* 10-bit ADC raw count (0 = 0V, 1023 = 3.3V) */
    f32 voltage;     /* Calculated voltage in Volts (0.0 to 3.3)   */
    f32 tempC;       /* Temperature in degrees Celsius              */

    /* Perform ADC conversion on Channel 1 (P0.28 = LM35 output)     */
    ReadADC(LM35_CHANNEL, &voltage, &rawADC);

    /* LM35: Temperature(°C) = Voltage(V) × 100
     * because the sensor outputs 10 mV per °C = 0.01 V per °C       */
    tempC = voltage * 100.0f;

    return (s32)tempC;   /* Return as integer (decimal part dropped) */
}

/*============================================================
 * Function : DisplayMonitorScreen  (PRIVATE)
 * Brief    : Reads all RTC registers and temperature, then formats
 *            and writes the information to the 16x2 LCD.
 *
 * LCD output format:
 *   Line 1: HH:MM:SS T:XX°C    (time and temperature)
 *   Line 2: DD/MM/YYYY DAY      (date and 3-letter day name)
 *
 * Example display:
 *   ┌────────────────┐
 *   │12:45:22 T:28C  │
 *   │02/05/2026 FRI  │
 *   └────────────────┘
 *
 * The degree symbol (°) is sent as 0xDF which is the HD44780
 * built-in character for the degree sign.
 *============================================================*/
static void DisplayMonitorScreen(void)
{
    s32 hour, min, sec;      /* RTC time fields (0-23, 0-59, 0-59)  */
    s32 date, month, year;   /* RTC date fields (1-31, 1-12, 0-4095)*/
    s32 dow;                  /* Day of week (0=SUN ... 6=SAT)       */
    s32 temp;                 /* Temperature in °C from LM35         */

    /* Read current time, date, and day of week from RTC registers */
    GetRTCTimeInfo(&hour, &min, &sec);
    GetRTCDateInfo(&date, &month, &year);
    GetRTCDay(&dow);

    /* Read temperature from LM35 sensor via ADC Channel 1 (P0.28) */
    temp = ReadTemperature();

    /*----------------------------------------------------------
     * Build and display Line 1: HH:MM:SS T:XX°C
     * Lcd2DigitLCD() always prints exactly 2 digits (with leading 0)
     *----------------------------------------------------------*/
    CmdLCD(LCD_LINE1);                 /* Position cursor at Line 1 Col 0 */

    Lcd2DigitLCD((u8)hour);  CharLCD(':');  /* HH: */
    Lcd2DigitLCD((u8)min);   CharLCD(':');  /* MM: */
    Lcd2DigitLCD((u8)sec);                  /* SS  */

    CharLCD(' ');   /* Space separating time from temperature */

    /* Temperature display: T:XX°C */
    CharLCD('T');
    CharLCD(':');
    IntLCD(temp);          /* Display temperature value (e.g. "28")   */
    CharLCD(0xDFU);        /* 0xDF = degree symbol (°) on HD44780     */
    CharLCD('C');          /* Celsius unit                            */

    /*----------------------------------------------------------
     * Build and display Line 2: DD/MM/YYYY DAY
     *----------------------------------------------------------*/
    CmdLCD(LCD_LINE2);                 /* Position cursor at Line 2 Col 0 */

    /* Date: DD/MM/YYYY */
    Lcd2DigitLCD((u8)date);   CharLCD('/');  /* DD/ */
    Lcd2DigitLCD((u8)month);  CharLCD('/');  /* MM/ */

    /* Print full 4-digit year (e.g. 2026 → '2','0','2','6') */
    CharLCD((u8)(((year / 1000) % 10) + '0'));   /* Thousands digit */
    CharLCD((u8)(((year /  100) % 10) + '0'));   /* Hundreds digit  */
    CharLCD((u8)(((year /   10) % 10) + '0'));   /* Tens digit      */
    CharLCD((u8)( (year         % 10) + '0'));   /* Units digit     */

    CharLCD(' ');                        /* Space before day name      */
    StrLCD((u8 *)week[dow]);             /* 3-letter day: SUN/MON/...  */
                                         /* week[] array is in rtc.c   */
}

/*============================================================
 * Function : main
 * Brief    : Application entry point.
 *            Initializes all hardware then runs the infinite
 *            monitoring and control loop.
 *
 * MAIN LOOP STEPS:
 *   1. Display current RTC time, date, and LM35 temperature
 *   2. Check if alarm time matches RTC → sound buzzer if yes
 *   3. If alarm is ringing, check if alarm switch (P0.11) pressed
 *      → stop buzzer when user presses it
 *   4. Check if EDIT switch (P0.10) is pressed
 *      → authenticate with password → enter edit menu
 *   5. 200 ms delay to prevent LCD from flickering
 *   6. Repeat forever
 *============================================================*/
int main(void)
{
    u8 authResult;   /* Holds result from AuthenticateUser():
                      * AUTH_SUCCESS=1, AUTH_FAIL=0, AUTH_LOCKED=2 */

    /* Initialize all peripherals and show the 2-second splash screen */
    SystemInit_EnviroTime();

    /*==========================================================
     * INFINITE MAIN LOOP
     * An embedded system never exits — it loops forever.
     *==========================================================*/
    while (1)
    {
        /*------------------------------------------------------
         * STEP 1: Read and display time, date, temperature
         * Refreshes the LCD with the latest RTC values and
         * the latest LM35 reading from ADC Channel 1 (P0.28)
         *------------------------------------------------------*/
        DisplayMonitorScreen();

        /*------------------------------------------------------
         * STEP 2: Check if alarm time has been reached
         * CheckAlarm() reads current RTC hour+minute and compares
         * with alarm_hour/alarm_min (set in the edit menu).
         * If they match and alarm is enabled: BuzzerOn() is called
         * and alarm_triggered is set to 1.
         *------------------------------------------------------*/
        CheckAlarm();

        /*------------------------------------------------------
         * STEP 3: Handle alarm stop button
         * If the alarm is currently ringing (alarm_triggered == 1),
         * continuously check if the user presses the ALARM switch
         * on P0.11 (SW2). When pressed: silence the buzzer and
         * clear the alarm_triggered flag.
         *------------------------------------------------------*/
        if (alarm_triggered == 1U)
        {
            if (IsAlarmSwitchPressed() == 1U)  /* P0.11 pressed? */
            {
                BuzzerOff();            /* Drive P0.12 LOW → buzzer silent  */
                alarm_triggered = 0U;  /* Clear flag: alarm no longer active */

                /* Confirm alarm stop on LCD briefly */
                ClearLCD();
                CmdLCD(LCD_LINE1);
                StrLCD((u8 *)"Alarm Stopped.");
                delay_ms(1000U);   /* Show message for 1 second */
            }
        }

        /*------------------------------------------------------
         * STEP 4: Handle edit mode entry
         * If the user presses the EDIT switch on P0.10 (SW1),
         * prompt for password authentication. If password is
         * correct (AUTH_SUCCESS), open the edit menu where the
         * user can change RTC time/date, alarm settings, or
         * the password itself.
         *------------------------------------------------------*/
        if (IsEditSwitchPressed() == 1U)   /* P0.10 pressed? */
        {
            /* Password-protect the edit menu
             * AuthenticateUser() shows "Enter Password:" on LCD,
             * reads 4 keypad digits, and returns AUTH_SUCCESS if correct. */
            authResult = AuthenticateUser();

            if (authResult == AUTH_SUCCESS)
            {
                /* Correct password → enter the edit menu.
                 * EnterEditMenu() loops until user presses '6' or 'D'. */
                EnterEditMenu();

                /* Clear LCD when returning to normal monitoring mode */
                ClearLCD();
            }
            /* AUTH_FAIL or AUTH_LOCKED: ignore and continue main loop */
        }

        /*------------------------------------------------------
         * STEP 5: Refresh rate delay
         * 200 ms pause prevents constant LCD writes which would
         * cause visible flickering. Gives a smooth ~5 fps update.
         *------------------------------------------------------*/
        delay_ms(200U);

    }  /* End of while(1) – never exits */

    /* This line is unreachable in an embedded system */
    return 0;
}
