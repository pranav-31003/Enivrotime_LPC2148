/*============================================================
 * File         : delay.c
 * Project      : EnviroTime – Digital Clock + Temperature Monitor
 * Target       : LPC2148 (ARM7TDMI-S) @ CCLK = 60 MHz
 * Author       : Embedded Systems Team
 * Date         : 02/05/2026
 * Description  : Software busy-wait delay implementation.
 *                Loop count calibrated for 60 MHz CPU clock
 *                (FOSC = 12 MHz, PLL multiplier M = 5).
 *                The 'volatile' keyword prevents the compiler
 *                optimizer from eliminating the empty loop body.
 *============================================================*/

#include "types.h"
#include "delay.h"

/*------------------------------------------------------------
 * Function : delay_us
 * Brief    : Generates an approximate microsecond delay.
 *            At 60 MHz CCLK, approx 15 loop iterations = 1 us.
 * Param    : us – number of microseconds to wait
 *------------------------------------------------------------*/
void delay_us(u32 us)
{
    volatile u32 count = us * 15UL;   /* Scale: 15 iterations per us   */
    while (count--)
    {
        /* Intentional empty busy-wait – compiler must not remove this */
    }
}

/*------------------------------------------------------------
 * Function : delay_ms
 * Brief    : Generates an approximate millisecond delay by
 *            calling delay_us() 1000 times per millisecond.
 * Param    : ms – number of milliseconds to wait
 *------------------------------------------------------------*/
void delay_ms(u32 ms)
{
    while (ms--)
    {
        delay_us(1000UL);   /* 1000 microseconds = 1 millisecond */
    }
}

/*------------------------------------------------------------
 * Function : delay_s
 * Brief    : Generates an approximate second delay by
 *            calling delay_ms() 1000 times per second.
 * Param    : s – number of seconds to wait
 *------------------------------------------------------------*/
void delay_s(u32 s)
{
    while (s--)
    {
        delay_ms(1000UL);   /* 1000 milliseconds = 1 second */
    }
}
