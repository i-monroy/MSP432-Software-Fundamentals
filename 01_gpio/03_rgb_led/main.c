/*
 * File: main.c
 * Author: i-monroy
 *
 * Project:
 *     GPIO RGB LED Colors
 *
 * Description:
 *     Cycles through the available colors of the onboard RGB LED
 *     using a fixed delay between each color.
 *
 * Hardware:
 *     MSP432P401R LaunchPad
 *
 * LEDs:
 *     P2.0 - Red channel of the onboard RGB LED
 *     P2.1 - Green channel of the onboard RGB LED
 *     P2.2 - Blue channel of the onboard RGB LED
 *
 * IDE:
 *     Code Composer Studio (CCS) 12.8.1
 */

#include "msp.h"
#include <stdint.h>

/* Delay between RGB LED colors. */
#define RGB_LED_DELAY_CYCLES (2000000U)

int main(void)
{
    /* Stop the watchdog timer to prevent periodic resets. */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    /* Configure P2.0, P2.1, and P2.2 for GPIO operation. */
    P2->SEL0 &= ~(BIT0 | BIT1 | BIT2);
    P2->SEL1 &= ~(BIT0 | BIT1 | BIT2);

    /* Configure all three RGB LED channels as outputs. */
    P2->DIR |= BIT0 | BIT1 | BIT2;

    /* Start with all RGB LED channels turned off. */
    P2->OUT &= ~(BIT0 | BIT1 | BIT2);

    /* Start with binary 001 to display the first RGB color. */
    uint8_t rgb_led_counter = 1U;

    while (1)
    {
        /* Clear the previous RGB LED color. */
        P2->OUT &= ~(BIT0 | BIT1 | BIT2);

        /*
         * Map the three counter bits to the RGB LED outputs.
         * See README Section 4.5.
         */
        P2->OUT |= rgb_led_counter;

        /* Keep the current color visible before changing to the next one. */
        __delay_cycles(RGB_LED_DELAY_CYCLES);

        /* Advance to the next RGB LED color combination. */
        rgb_led_counter++;

        /* Restart after displaying all seven RGB color combinations. */
        if (rgb_led_counter >= 8)
        {
            rgb_led_counter = 1;
        }
    }
}
