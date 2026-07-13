/*============================================================
 * File    : adc.c
 * Project : EnviroTime - Digital Clock with Temperature Monitor
 * Author  : Embedded Systems Team
 * Date    : 02/05/2026
 * Brief   : On-chip 10-bit ADC driver for LPC2148.
 *           Supports single-channel software-triggered conversions.
 *
 * LM35 SENSOR CONNECTION (Reference / Authoritative Config):
 *   LM35 OUT → P0.28  (AD0.1 = ADC Channel 1)
 *   LM35 VCC → +5V
 *   LM35 GND → GND
 *
 * CHANGE FROM PREVIOUS REVISION:
 *   Old config: LM35 on P0.27 (CH0 / AIN0)
 *   New config: LM35 on P0.28 (CH1 / AIN1) per reference pin_config.h
 *
 * HOW ADC WORKS ON LPC2148:
 *   1. Configure PINSEL1 to set the pin as analog input (not GPIO)
 *   2. Power on ADC via PCONP register (bit 12)
 *   3. Set up AD0CR: select channel, set clock divider, power on ADC
 *   4. Start conversion by writing START bits in AD0CR
 *   5. Poll the DONE bit (bit 31) in AD0GDR until conversion completes
 *   6. Read 10-bit result from AD0GDR bits [15:6]
 *   7. Convert raw count to voltage: V = raw * (3.3 / 1023)
 *
 * LM35 TEMPERATURE FORMULA:
 *   LM35 outputs 10 mV per °C → 250 mV at 25°C
 *   ADC result: raw = V * 1023 / 3.3
 *   Temperature = Voltage * 100 = (raw * 3.3 / 1023) * 100
 *   Simplified: Temp(°C) = (raw * 330) / 1023
 *============================================================*/

#include <LPC21xx.h>
#include "types.h"
#include "adc_defines.h"
#include "adc.h"
#include "delay.h"

#ifndef AD0GDR
#define AD0GDR  (*((volatile unsigned long *) 0xE0034004))
#endif

/*------------------------------------------------------------
 * InitADC()
 * Brief : Initializes the ADC for the specified channel.
 *
 *   Step 1: Configure PINSEL1 to connect the pin to the ADC
 *           function (rather than leaving it as GPIO).
 *   Step 2: Enable ADC peripheral power in PCONP (bit 12).
 *   Step 3: Configure AD0CR with clock divider and power-on.
 *
 * Param : chNo – ADC channel number to initialize:
 *           CH0 = P0.27 (AIN0)
 *           CH1 = P0.28 (AIN1) ← LM35 in this project
 *           CH2 = P0.29 (AIN2)
 *           CH3 = P0.30 (AIN3)
 *------------------------------------------------------------*/
void InitADC(u32 chNo)
{
    /*----------------------------------------------------------
     * Step 1: Configure the analog pin via PINSEL1
     * PINSEL1 controls pin functions for P0.16 to P0.31.
     * Setting the two bits for each pin to '01' selects the
     * ADC analog input function for that pin.
     *----------------------------------------------------------*/
    switch (chNo)
    {
        case 0U:
            /* P0.27 → AD0.0 (AIN0): PINSEL1 bits [23:22] = 01 */
            PINSEL1 &= ~AIN0_PINSEL_MASK;   /* Clear bits 23:22 */
            PINSEL1 |=  (0x01UL << 22);      /* Set to 01 = AIN0 */
            break;

        case 1U:
            /* P0.28 → AD0.1 (AIN1): PINSEL1 bits [25:24] = 01
             * THIS IS THE LM35 CHANNEL in the reference design  */
            PINSEL1 &= ~AIN1_PINSEL_MASK;   /* Clear bits 25:24 */
            PINSEL1 |=  (0x01UL << 24);      /* Set to 01 = AIN1 */
            break;

        case 2U:
            /* P0.29 → AD0.2 (AIN2): PINSEL1 bits [27:26] = 01 */
            PINSEL1 &= ~AIN2_PINSEL_MASK;   /* Clear bits 27:26 */
            PINSEL1 |=  (0x01UL << 26);      /* Set to 01 = AIN2 */
            break;

        case 3U:
            /* P0.30 → AD0.3 (AIN3): PINSEL1 bits [29:28] = 01 */
            PINSEL1 &= ~AIN3_PINSEL_MASK;   /* Clear bits 29:28 */
            PINSEL1 |=  (0x01UL << 28);      /* Set to 01 = AIN3 */
            break;

        default:
            return;   /* Invalid channel: do nothing */
    }

    /*----------------------------------------------------------
     * Step 2: Enable ADC power via PCONP register
     * PCONP bit 12 = 1 turns on power to the ADC peripheral.
     * Without this, AD0CR writes have no effect.
     *----------------------------------------------------------*/
    PCONP |= (1UL << 12);   /* Bit 12 of PCONP = ADC power enable */

    /*----------------------------------------------------------
     * Step 3: Configure AD0CR (ADC Control Register)
     *
     * Bits [7:0]  = channel select mask (1 << chNo selects channel)
     * Bits [15:8] = CLKDIV: ADC clock = PCLK / (CLKDIV + 1)
     *              ADC_CLKDIV = 3 → ADC clock = 15MHz/4 = 3.75MHz ✓
     * Bit 21      = PDN: 1 = ADC is powered and operational
     *
     * START bits [26:24] remain 0 here → no immediate conversion.
     * Conversion is started in ReadADC() when needed.
     *----------------------------------------------------------*/
    ADCR = (1UL << chNo)                          /* Select channel        */
         | (ADC_CLKDIV << ADC_CLKDIV_BITS)        /* Set clock divider     */
         | (1UL << ADC_PDN_BIT);                  /* Power on ADC          */
}

/*------------------------------------------------------------
 * ReadADC()
 * Brief : Performs a single software-triggered ADC conversion
 *         on the specified channel and returns both the raw
 *         10-bit count and the equivalent voltage.
 *
 * Param : chNo    – channel number to read (must be initialized)
 * Param : voltage – pointer to store the result in Volts (0.0-3.3V)
 * Param : raw     – pointer to store the 10-bit raw result (0-1023)
 *
 * For the LM35 on CH1 (P0.28):
 *   Temperature(°C) = (*voltage) * 100.0f
 *   Or using raw:   = (*raw * 330) / 1023
 *------------------------------------------------------------*/
void ReadADC(u32 chNo, f32 *voltage, u32 *raw)
{
    /*----------------------------------------------------------
     * Step 1: Clear the current channel selection in AD0CR
     * Bits [7:0] of AD0CR select which channel to convert.
     * We must clear them before selecting the new channel.
     *----------------------------------------------------------*/
    ADCR &= 0xFFFFFF00UL;   /* Clear channel select bits [7:0] */

    /*----------------------------------------------------------
     * Step 2: Select the channel and start conversion
     * Setting bits [26:24] = 001 in AD0CR means "start now".
     * (1UL << ADC_START_BIT) = (1 << 24) = software start.
     *----------------------------------------------------------*/
    ADCR |= (1UL << ADC_START_BIT) | (1UL << chNo);

    /*----------------------------------------------------------
     * Step 3: Short settling delay for the ADC input multiplexer
     * The mux needs a few microseconds to fully switch channels.
     *----------------------------------------------------------*/
    delay_us(5);

    /*----------------------------------------------------------
     * Step 4: Wait for conversion to complete
     * Bit 31 (ADC_DONE_BIT) of AD0GDR becomes 1 when done.
     * A 10-bit conversion takes about 11 ADC clock cycles.
     * At 3.75 MHz: ~2.9 µs per conversion.
     *----------------------------------------------------------*/
    while (((AD0GDR >> ADC_DONE_BIT) & 1UL) == 0UL)
    {
        /* Busy-wait: poll until DONE flag goes HIGH */
    }

    /*----------------------------------------------------------
     * Step 5: Stop conversion trigger
     * Clear the START bits [26:24] to prevent repeated starts.
     *----------------------------------------------------------*/
    ADCR &= ~(1UL << ADC_START_BIT);

    /*----------------------------------------------------------
     * Step 6: Extract 10-bit result from AD0GDR
     * The result sits in bits [15:6] of AD0GDR.
     * Shift right by ADC_DATA_BITS (6) then mask with 0x3FF (1023).
     *----------------------------------------------------------*/
    *raw = (AD0GDR >> ADC_DATA_BITS) & 0x3FFUL;

    /*----------------------------------------------------------
     * Step 7: Convert raw count to voltage
     * ADC is 10-bit (0-1023) for 0V to 3.3V reference.
     * Voltage = raw * (3.3 / 1023)
     *----------------------------------------------------------*/
    *voltage = (f32)(*raw) * (3.3f / 1023.0f);
}
