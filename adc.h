/*============================================================
 * File    : adc.h
 * Project : EnviroTime - Digital Clock with Temperature Monitor
 * Author  : Embedded Systems Team
 * Date    : 02/05/2026
 * Brief   : On-chip ADC driver function prototypes for LPC2148.
 *
 * The LPC2148 has a 10-bit ADC (AD0) with 8 channels (AD0.0-AD0.7).
 * This driver uses single software-triggered conversions.
 *
 * LM35 CONNECTION (Reference Config):
 *   LM35 output → P0.28 (AD0.1 = Channel 1)
 *   Use LM35_CHANNEL (CH1) defined in adc_defines.h.
 *
 * INCLUDE THIS FILE in any module that needs ADC readings.
 * Implementation is in adc.c. Constants are in adc_defines.h.
 *============================================================*/

#ifndef ADC_H
#define ADC_H

#include "types.h"

/* InitADC(chNo)
 * Configures PINSEL1 to set the specified channel pin as analog input,
 * powers on the ADC via PCONP, and sets the clock divider in AD0CR.
 * Must be called once per channel before ReadADC().
 * Param: chNo – channel number (CH0 to CH3, defined in adc_defines.h) */
void  InitADC(u32 chNo);

/* ReadADC(chNo, voltage, raw)
 * Performs a single software-triggered ADC conversion on 'chNo'.
 * Waits for the DONE flag, then extracts the 10-bit result.
 * Param: chNo    – channel to convert (must have been initialized)
 * Param: voltage – pointer to store the calculated voltage (0.0–3.3V)
 * Param: raw     – pointer to store the 10-bit raw result (0–1023)   */
void  ReadADC(u32 chNo, f32 *voltage, u32 *raw);

#endif /* ADC_H */
