/*============================================================
 * File         : security.c
 * Project      : EnviroTime – Digital Clock + Temperature Monitor
 * Target       : LPC2148 (ARM7TDMI-S)
 * Author       : Embedded Systems Team
 * Date         : 02/05/2026
 * Description  : Password-based security module.
 *                Default password is "1234" (four keypad digits).
 *                Locks the system after 3 consecutive wrong attempts
 *                for 30 seconds and sounds buzzer alert.
 *
 * KEYPAD FUNCTION USED:
 *   KeyScan()  – waits for and returns one key press character
 *   ColScan()  – returns 0 if any key is pressed, 1 if none
 *============================================================*/

#include <LPC21xx.h>
#include "types.h"
#include "security.h"
#include "lcd.h"
#include "keypad.h"
#include "buzzer.h"
#include "delay.h"

/*------------------------------------------------------------
 * Module-Private (Static) Variables
 * These are only visible inside security.c
 *------------------------------------------------------------*/
static u8 stored_password[PASSWORD_LEN] = { '1', '2', '3', '4' };
                                       /* Default password: 1234    */
static u8 wrong_attempts = 0U;         /* Count of wrong tries      */
static u8 system_locked  = 0U;         /* 1 = system is locked now  */

/*------------------------------------------------------------
 * Private Function Prototypes
 * Only used inside this file
 *------------------------------------------------------------*/
static void DisplayPasswordPrompt(void);
static u8   GetPasswordInput(u8 *entered);

/*============================================================
 * Function : SecurityInit
 * Brief    : Initializes the security module with the default
 *            password "1234" and resets all lock state.
 *            Call this once during system startup.
 *============================================================*/
void SecurityInit(void)
{
    /* Set default password digits */
    stored_password[0] = '1';
    stored_password[1] = '2';
    stored_password[2] = '3';
    stored_password[3] = '4';

    /* Clear lock state */
    wrong_attempts = 0U;
    system_locked  = 0U;
}

/*============================================================
 * Function : ResetAttemptCounter
 * Brief    : Resets the wrong-attempt counter and unlocks
 *            the system. Called after lock timeout expires.
 *============================================================*/
void ResetAttemptCounter(void)
{
    wrong_attempts = 0U;
    system_locked  = 0U;
}

/*============================================================
 * Function : DisplayPasswordPrompt  (PRIVATE)
 * Brief    : Shows the password entry prompt on the LCD.
 *            Line 1: "Enter Password:"
 *            Line 2: "____" then cursor moves to first position
 *============================================================*/
static void DisplayPasswordPrompt(void)
{
    ClearLCD();
    CmdLCD(LCD_LINE1);
    StrLCD((u8 *)"Enter Password:");
    CmdLCD(LCD_LINE2);
    StrLCD((u8 *)"____");      /* Show 4 underscores as placeholders */
    CmdLCD(LCD_LINE2);         /* Move cursor back to start of Line2 */
}

/*============================================================
 * Function : GetPasswordInput  (PRIVATE)
 * Brief    : Reads exactly PASSWORD_LEN key presses from the
 *            keypad and stores them in the 'entered' buffer.
 *            Shows '*' instead of the actual digit for privacy.
 *            Only digit keys '0'-'9' are accepted.
 *            Pressing 'D' cancels the entry.
 *
 * Param    : entered – buffer to store the pressed key chars
 * Return   : 1 = input complete (got all 4 digits)
 *            0 = cancelled by 'D' key
 *============================================================*/
static u8 GetPasswordInput(u8 *entered)
{
    u8 idx = 0U;   /* Index into entered[] buffer       */
    u8 key;				 /* Character returned by KeyScan()   */
		u8 col;        /* LCD column position for cursor repositioning */

    /* Show the prompt on LCD before collecting input */
    DisplayPasswordPrompt();

    /* Collect exactly PASSWORD_LEN digit keys */
    while (idx < PASSWORD_LEN)
    {
        /* Wait for a key press using KeyScan (blocking) */
        key = KeyScan();

        if (key == 'D')
        {
            /* 'D' = Cancel – user wants to exit without entering */
            return 0U;
        }
        else if ((key >= '0') && (key <= '9'))
        {
            /* Valid digit key: store it and show '*' on LCD */
            entered[idx] = key;
            CharLCD('*');       /* Display '*' to mask the digit   */
            idx++;              /* Move to next digit position      */
        }
        /* All other keys (A, B, C, *, #) are silently ignored */
    }

    return 1U;   /* All PASSWORD_LEN digits collected successfully */
}

/*============================================================
 * Function : AuthenticateUser
 * Brief    : Full authentication flow.
 *            1. Check if system is already locked → show lock msg
 *            2. Prompt user to enter password via keypad
 *            3. Compare entered password vs stored password
 *            4. If correct  → show "Access Granted", return SUCCESS
 *            5. If wrong    → increment counter, show attempts left
 *            6. After 3 wrong attempts → lock for 30 seconds
 *
 * Return   : AUTH_SUCCESS (1) – correct password entered
 *            AUTH_FAIL    (0) – wrong password entered
 *            AUTH_LOCKED  (2) – system was or became locked
 *============================================================*/
u8 AuthenticateUser(void)
{
    u8 entered[PASSWORD_LEN];   /* Buffer for user's typed password  */
    u8 i;                        /* Loop counter for comparison       */
    u8 match  = 1U;              /* 1 = passwords match so far        */
    u8 result;                   /* Return value from GetPasswordInput */

    /*----------------------------------------------------------
     * Check if system is already locked from previous attempts
     *----------------------------------------------------------*/
    if (system_locked == 1U)
    {
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"System Locked!");
        CmdLCD(LCD_LINE2);
        StrLCD((u8 *)"Wait 30 secs");
        BuzzerAlert(3U);                  /* 3 short buzzer beeps   */
        delay_ms(LOCK_DURATION_MS);       /* Wait 30 seconds        */
        ResetAttemptCounter();            /* Unlock after timeout   */
        return AUTH_LOCKED;
    }

    /*----------------------------------------------------------
     * Get password input from user via keypad
     *----------------------------------------------------------*/
    result = GetPasswordInput(entered);

    if (result == 0U)
    {
        /* User cancelled by pressing 'D' */
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"Cancelled.");
        delay_ms(1000U);
        return AUTH_FAIL;
    }

    /*----------------------------------------------------------
     * Compare entered password with stored password digit by digit
     *----------------------------------------------------------*/
    for (i = 0U; i < PASSWORD_LEN; i++)
    {
        if (entered[i] != stored_password[i])
        {
            match = 0U;   /* Mismatch found */
            break;
        }
    }

    if (match == 1U)
    {
        /*------------------------------------------------------
         * Correct password
         *------------------------------------------------------*/
        wrong_attempts = 0U;         /* Reset wrong attempt counter  */
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"Access Granted!");
        BuzzerBeep(100U);            /* Short confirmation beep      */
        delay_ms(1000U);
        return AUTH_SUCCESS;
    }
    else
    {
        /*------------------------------------------------------
         * Wrong password
         *------------------------------------------------------*/
        wrong_attempts++;            /* Increment wrong attempt count */

        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"Access Denied!");
        CmdLCD(LCD_LINE2);

        /* Show remaining attempts before lockout */
        CharLCD((u8)((MAX_WRONG_ATTEMPTS - wrong_attempts) + '0'));
        StrLCD((u8 *)" tries left");

        BuzzerAlert(2U);             /* 2 buzzer beeps for wrong pwd  */
        delay_ms(1500U);

        /* Check if maximum wrong attempts exceeded */
        if (wrong_attempts >= MAX_WRONG_ATTEMPTS)
        {
            /* Lock the system */
            system_locked = 1U;
            ClearLCD();
            CmdLCD(LCD_LINE1);
            StrLCD((u8 *)"System Locked!");
            CmdLCD(LCD_LINE2);
            StrLCD((u8 *)"Wait 30 secs");
            BuzzerAlert(5U);              /* 5 beeps for lockout alert  */
            delay_ms(LOCK_DURATION_MS);   /* Wait 30 seconds            */
            ResetAttemptCounter();
            return AUTH_LOCKED;
        }

        return AUTH_FAIL;
    }
}

/*============================================================
 * Function : ChangePassword
 * Brief    : Allows authenticated user to set a new 4-digit
 *            password. New password must be confirmed by
 *            entering it twice. If both entries match, the
 *            new password replaces the stored one.
 *============================================================*/
void ChangePassword(void)
{
    u8 old_pwd[PASSWORD_LEN];       /* Entry of current/old password  */
    u8 new_pwd[PASSWORD_LEN];       /* First entry of new password    */
    u8 confirm_pwd[PASSWORD_LEN];   /* Second entry for confirmation  */
    u8 i;                            /* Loop counter                   */
    u8 match  = 1U;                  /* 1 = both entries match         */
    u8 result;                       /* Return from GetPasswordInput   */

    /*----------------------------------------------------------
     * Step 1: Verify the OLD (current) password
     *----------------------------------------------------------*/
    ClearLCD();
    CmdLCD(LCD_LINE1);
    StrLCD((u8 *)"Old Password:");
    CmdLCD(LCD_LINE2);
    result = GetPasswordInput(old_pwd);

    if (result == 0U)
    {
        /* User cancelled */
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"Cancelled.");
        delay_ms(1000U);
        return;
    }

    /* Compare old_pwd with stored_password */
    match = 1U;
    for (i = 0U; i < PASSWORD_LEN; i++)
    {
        if (old_pwd[i] != stored_password[i])
        {
            match = 0U;
            break;
        }
    }

    if (match == 0U)
    {
        /* Wrong old password – deny change */
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"Wrong Password!");
        CmdLCD(LCD_LINE2);
        StrLCD((u8 *)"Change Denied.");
        BuzzerAlert(2U);
        delay_ms(1500U);
        return;
    }

    /*----------------------------------------------------------
     * Step 2: Get the new password (first entry)
     *----------------------------------------------------------*/
    ClearLCD();
    CmdLCD(LCD_LINE1);
    StrLCD((u8 *)"New Password:");
    CmdLCD(LCD_LINE2);
    result = GetPasswordInput(new_pwd);

    if (result == 0U)
    {
        /* User cancelled */
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"Cancelled.");
        delay_ms(1000U);
        return;
    }

    /*----------------------------------------------------------
     * Step 3: Get the confirmation password (second entry)
     *----------------------------------------------------------*/
    ClearLCD();
    CmdLCD(LCD_LINE1);
    StrLCD((u8 *)"Confirm Pwd:");
    CmdLCD(LCD_LINE2);
    result = GetPasswordInput(confirm_pwd);

    if (result == 0U)
    {
        /* User cancelled during confirmation */
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"Cancelled.");
        delay_ms(1000U);
        return;
    }

    /*----------------------------------------------------------
     * Step 4: Compare new_pwd and confirm_pwd
     *----------------------------------------------------------*/
    match = 1U;
    for (i = 0U; i < PASSWORD_LEN; i++)
    {
        if (new_pwd[i] != confirm_pwd[i])
        {
            match = 0U;
            break;
        }
    }

    if (match == 1U)
    {
        /* Both entries match – save the new password */
        for (i = 0U; i < PASSWORD_LEN; i++)
        {
            stored_password[i] = new_pwd[i];
        }
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"Password Set!");
        BuzzerBeep(200U);    /* Confirmation beep */
    }
    else
    {
        /* Entries did not match */
        ClearLCD();
        CmdLCD(LCD_LINE1);
        StrLCD((u8 *)"Pwd Mismatch!");
        BuzzerAlert(2U);     /* Error beeps */
    }

    delay_ms(1500U);
}
