/*============================================================
 * File    : adc_defines.h
 * Project : EnviroTime - Digital Clock with Temperature Monitor
 * Author  : Embedded Systems Team
 * Date    : 02/05/2026
 * Brief   : ADC register bit definitions and channel pin macros
 *           for LPC2148 on-chip 10-bit ADC (AD0).
 *
 * LM35 SENSOR CONNECTION (Reference / Authoritative Config):
 *   LM35 OUT → P0.28 (AD0.1 = ADC Channel 1)
 *   LM35 VCC → +5V
 *   LM35 GND → GND
 *
 * CHANGE FROM PREVIOUS REVISION:
 *   Old config: LM35 on P0.27 (CH0 / AIN0)
 *   New config: LM35 on P0.28 (CH1 / AIN1) per reference pin_config.h
 *
 * LM35 TEMPERATURE FORMULA:
 *   The LM35 outputs 10 mV per degree Celsius.
 *   ADC gives 0–1023 for 0V to 3.3V supply.
 *   Voltage (mV) = (raw * 3300) / 1023
 *   Temperature  = Voltage / 10
 *   Combined:    Temp(°C) = (raw * 330) / 1023
 *
 * ADC CLOCK CALCULATION (for reference):
 *   PCLK = 15 MHz (CCLK/4 = 60MHz/4)
 *   Max ADC clock = 4.5 MHz
 *   CLKDIV = (PCLK / ADC_CLK_MAX) - 1 = (15/4.5) - 1 ≈ 2
 *   Actual ADC clock ≈ 15/3 = 5 MHz (slightly over; use CLKDIV=3
 *   for 3.75 MHz to stay safely below 4.5 MHz maximum)
 *============================================================*/

#ifndef ADC_DEFINES_H
#define ADC_DEFINES_H

/*-------------- Clock Configuration -----------------------*/
#define FOSC_ADC         12000000UL         /* Oscillator: 12 MHz       */
#define CCLK_ADC         (FOSC_ADC * 5UL)  /* CPU clock:  60 MHz       */
#define PCLK_ADC         (CCLK_ADC / 4UL)  /* Peripheral: 15 MHz       */
#define ADC_CLK_MAX      4500000UL          /* Max ADC clock: 4.5 MHz   */
/* CLKDIV = (PCLK / ADC_CLK_MAX) - 1
 * = (15000000 / 4500000) - 1 = 3.33 - 1 ≈ floor = 2
 * We use 3 to ensure ADC clock stays safely under 4.5 MHz:
 * Actual = PCLK / (CLKDIV+1) = 15MHz / 4 = 3.75 MHz ✓          */
#define ADC_CLKDIV       3UL                /* Clock divider value      */

/*-------------- AD0CR Register Bit Positions --------------*/
#define ADC_CLKDIV_BITS  8    /* Clock divider field @ AD0CR bits [15:8]  */
#define ADC_PDN_BIT      21   /* Power-Down control: 1 = ADC powered ON   */
#define ADC_START_BIT    24   /* Start conversion: bits [26:24] = 001     */

/*-------------- AD0GDR Register Bit Positions -------------*/
#define ADC_DATA_BITS    6    /* ADC 10-bit result @ AD0GDR bits [15:6]   */
#define ADC_DONE_BIT     31   /* Conversion complete flag: 1 = done       */

/*-------------- Channel Pin Select Masks ------------------*/
/* These values are applied to PINSEL1 to enable analog input function.
 * PINSEL1 controls pin functions for P0.16 to P0.31.
 * Each channel needs bits [n:n+1] = 01 in PINSEL1.             */
#define AIN0_PINSEL_MASK  (0x03UL << 22)   /* P0.27 → AD0.0 (CH0) */
#define AIN1_PINSEL_MASK  (0x03UL << 24)   /* P0.28 → AD0.1 (CH1) ← LM35 */
#define AIN2_PINSEL_MASK  (0x03UL << 26)   /* P0.29 → AD0.2 (CH2) */
#define AIN3_PINSEL_MASK  (0x03UL << 28)   /* P0.30 → AD0.3 (CH3) */

/*-------------- Channel Number Defines --------------------*/
#define CH0   0U   /* ADC Channel 0 (P0.27 / AD0.0) */
#define CH1   1U   /* ADC Channel 1 (P0.28 / AD0.1) ← LM35 in this project */
#define CH2   2U   /* ADC Channel 2 (P0.29 / AD0.2) */
#define CH3   3U   /* ADC Channel 3 (P0.30 / AD0.3) */

/*-------------- LM35 Active Channel -----------------------*/
/* This define makes it easy to change the LM35 channel in one place */
#define LM35_CHANNEL   CH1   /* LM35 sensor is on ADC Channel 1 (P0.28) */

#endif /* ADC_DEFINES_H */
