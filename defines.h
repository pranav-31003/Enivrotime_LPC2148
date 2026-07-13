/*============================================================
 * File         : defines.h
 * Project      : EnviroTime – Digital Clock + Temperature Monitor
 * Target       : LPC2148 (ARM7TDMI-S)
 * Author       : Embedded Systems Team
 * Date         : 02/05/2026
 * Description  : Bit-level and byte-level register manipulation
 *                macros. These avoid magic numbers and raw shifts
 *                when accessing LPC2148 peripheral registers.
 *============================================================*/

#ifndef DEFINES_H
#define DEFINES_H

/*------------------------------------------------------------
 * SETBIT(WORD, BP)
 * Sets bit BP in register WORD to logic 1.
 * Example: SETBIT(IODIR0, 15)  --> makes P0.15 an output pin
 *------------------------------------------------------------*/
#define SETBIT(WORD, BP)          ( (WORD) |=  (1UL << (BP)) )

/*------------------------------------------------------------
 * CLRBIT(WORD, BP)
 * Clears bit BP in register WORD to logic 0.
 * Example: CLRBIT(IODIR0, 15)  --> makes P0.15 an input pin
 *------------------------------------------------------------*/
#define CLRBIT(WORD, BP)          ( (WORD) &= ~(1UL << (BP)) )

/*------------------------------------------------------------
 * CPLBIT(WORD, BP)
 * Toggles bit BP in register WORD (0 becomes 1, 1 becomes 0).
 * Example: CPLBIT(IOPIN0, 15)  --> flips state of P0.15
 *------------------------------------------------------------*/
#define CPLBIT(WORD, BP)          ( (WORD) ^=  (1UL << (BP)) )

/*------------------------------------------------------------
 * READBIT(WORD, BP)
 * Reads the value (0 or 1) of bit BP from register WORD.
 * Example: if (READBIT(IOPIN0, 19) == 0) means P0.19 is LOW
 *------------------------------------------------------------*/
#define READBIT(WORD, BP)         ( ((WORD) >> (BP)) & 1UL )

/*------------------------------------------------------------
 * WRITEBIT(WORD, BP, BIT)
 * Writes a specific bit value (0 or 1) into bit BP of WORD.
 * Example: WRITEBIT(IOPIN0, 15, 1) drives P0.15 HIGH
 *------------------------------------------------------------*/
#define WRITEBIT(WORD, BP, BIT)   ( (WORD) = (((WORD) & ~(1UL << (BP))) \
                                             | ((u32)(BIT) << (BP))) )

/*------------------------------------------------------------
 * WRITENIBBLE(WORD, SBP, NIBBLE)
 * Writes a 4-bit value NIBBLE into WORD starting at bit SBP.
 * Example: WRITENIBBLE(IODIR0, 8, 0xF) sets P0.8-P0.11 output
 *------------------------------------------------------------*/
#define WRITENIBBLE(WORD, SBP, NIBBLE) \
    ( (WORD) = (((WORD) & ~(0x0FUL << (SBP))) | ((u32)(NIBBLE) << (SBP))) )

/*------------------------------------------------------------
 * WRITEBYTE(WORD, SBP, BYTE)
 * Writes an 8-bit value BYTE into WORD starting at bit SBP.
 * Example: WRITEBYTE(IODIR0, 8, 0xFF) sets P0.8-P0.15 output
 *          WRITEBYTE(IOPIN0, 8, 'A')  sends 'A' on P0.8-P0.15
 *------------------------------------------------------------*/
#define WRITEBYTE(WORD, SBP, BYTE) \
    ( (WORD) = (((WORD) & ~(0xFFUL << (SBP))) | ((u32)(BYTE) << (SBP))) )

#endif /* DEFINES_H */
