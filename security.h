/*============================================================
 * File    : security.h
 * Project : EnviroTime - Digital Clock with Temperature Monitor
 * Author  : Embedded Systems Team
 * Date    : 02/05/2026
 * Brief   : Password-based security module prototypes
 *           Protects Edit Mode with 4-digit keypad password
 *============================================================*/

#ifndef SECURITY_H
#define SECURITY_H

#include "types.h"

/*-------------- Security Configuration --------------------*/
#define PASSWORD_LEN        4U    /* Number of digits in password     */
#define MAX_WRONG_ATTEMPTS  3U    /* Lock after this many wrong tries  */
#define LOCK_DURATION_MS    30000U /* Lock duration: 30 seconds        */

/*-------------- Return Codes ------------------------------*/
#define AUTH_SUCCESS        1U
#define AUTH_FAIL           0U
#define AUTH_LOCKED         2U

/*-------------- Function Prototypes -----------------------*/
void SecurityInit(void);          /* Initialize default password         */
u8   AuthenticateUser(void);      /* Prompt and verify password (keypad) */
void ChangePassword(void);        /* Allow user to change password       */
void ResetAttemptCounter(void);   /* Reset wrong-attempt counter to 0    */

#endif /* SECURITY_H */
