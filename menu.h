/*============================================================
 * File    : menu.h
 * Project : EnviroTime - Digital Clock with Temperature Monitor
 * Author  : Embedded Systems Team
 * Date    : 02/05/2026
 * Brief   : Secure edit menu module prototypes
 *           Provides menus for editing RTC, alarm, and password
 *============================================================*/

#ifndef MENU_H
#define MENU_H

#include "types.h"

/*-------------- Alarm Time Global Variables ---------------*/
/* Defined in menu.c, accessible wherever menu.h is included */
extern s32 alarm_hour;
extern s32 alarm_min;
extern s32 alarm_sec;
extern u8  alarm_enabled;
extern u8  alarm_triggered;

/*-------------- Function Prototypes -----------------------*/
void MenuInit(void);          /* Initialize alarm and menu state     */
void EnterEditMenu(void);     /* Main edit menu (after auth success) */
void CheckAlarm(void);        /* Check RTC time vs alarm; ring buzzer */

#endif /* MENU_H */
