/*============================================================
 * File         : types.h
 * Project      : EnviroTime – Digital Clock + Temperature Monitor
 * Target       : LPC2148 (ARM7TDMI-S) @ 60 MHz
 * Author       : Embedded Systems Team
 * Date         : 02/05/2026
 * Description  : Platform-independent fixed-width data type
 *                definitions. Using short aliases (u8, s32 etc.)
 *                keeps register-level code clean and portable.
 *============================================================*/

#ifndef TYPES_H
#define TYPES_H

typedef unsigned char       u8;   /* Unsigned  8-bit  :  0 to 255           */
typedef signed   char       s8;   /* Signed    8-bit  : -128 to 127         */
typedef unsigned short int  u16;  /* Unsigned 16-bit  :  0 to 65535         */
typedef signed   short int  s16;  /* Signed   16-bit  : -32768 to 32767     */
typedef unsigned long  int  u32;  /* Unsigned 32-bit  :  0 to 4294967295    */
typedef signed   long  int  s32;  /* Signed   32-bit  : -2147483648 to ...  */
typedef float               f32;  /* 32-bit single-precision floating point  */
typedef double              f64;  /* 64-bit double-precision floating point  */

#endif /* TYPES_H */
