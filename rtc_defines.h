/*============================================================
 * File    : rtc_defines.h
 * Project : EnviroTime - Digital Clock with Temperature Monitor
 * Author  : Embedded Systems Team
 * Date    : 02/05/2026
 * Brief   : RTC register macros and clock prescaler values
 *           for LPC2148 running at 60 MHz
 *============================================================*/

#ifndef RTC_DEFINES_H
#define RTC_DEFINES_H

/*-------------- System Clock Defines ----------------------*/
#define FOSC            12000000UL        /* External oscillator: 12 MHz  */
#define CCLK            (5UL * FOSC)      /* CPU clock: 60 MHz (PLL x5)   */
#define PCLK            (CCLK / 4UL)      /* Peripheral clock: 15 MHz     */

/*-------------- RTC Prescaler Values ----------------------*/
/* LPC2148 RTC uses internal 32.768 kHz oscillator via PCLK */
/* Uncomment _LPC2148 to use internal RTC clock directly     */
#define _LPC2148

#define PREINT_VAL      ((PCLK / 32768UL) - 1UL)
#define PREFRAC_VAL     (PCLK - (PREINT_VAL + 1UL) * 32768UL)

/*-------------- CCR Register Bit Defines ------------------*/
#define RTC_ENABLE      (1UL << 0)    /* Bit 0: Enable RTC clock   */
#define RTC_RESET       (1UL << 1)    /* Bit 1: Reset RTC          */
#define RTC_TEST        (1UL << 2)    /* Bit 2: RTC test mode      */
#define RTC_CLKSRC      (1UL << 4)    /* Bit 4: Use internal osc   */

#endif /* RTC_DEFINES_H */
