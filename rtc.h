/*============================================================
 * File    : rtc.h
 * Project : EnviroTime - Digital Clock with Temperature Monitor
 * Author  : Embedded Systems Team
 * Date    : 02/05/2026
 * Brief   : Internal RTC driver function prototypes for LPC2148
 *============================================================*/

#ifndef RTC_H
#define RTC_H

#include "types.h"

/* Day of Week string array (extern - defined in rtc.c) */
extern const char week[][4];

/*-------------- RTC Initialization -------------------------*/
void RTC_Init(void);

/*-------------- Get RTC Values -----------------------------*/
void GetRTCTimeInfo(s32 *hour, s32 *minute, s32 *second);
void GetRTCDateInfo(s32 *date,  s32 *month,  s32 *year);
void GetRTCDay(s32 *day);

/*-------------- Set RTC Values -----------------------------*/
void SetRTCTimeInfo(u32 hour, u32 minute, u32 second);
void SetRTCDateInfo(u32 date, u32 month,  u32 year);
void SetRTCDay(u32 day);

/*-------------- Display on LCD -----------------------------*/
void DisplayRTCTime(u32 hour,  u32 minute, u32 second);
void DisplayRTCDate(u32 date,  u32 month,  u32 year);
void DisplayRTCDay(u32 dow);

#endif /* RTC_H */
