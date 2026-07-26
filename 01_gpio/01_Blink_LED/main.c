/*
 * File: main.c
 * Author: i-monroy
 *
 * Project:
 *     GPIO Blink LED
 *
 * Description:
 *     Blinks the onboard red LED connected to P1.0.
 *
 * Hardware:
 *     MSP432P401R LaunchPad
 *
 * LED:
 *     P1.0 - Red LED
 *
 * IDE:
 *     Code Composer Studio (CCS) 12.8.1
 */

#include "msp.h"

/* Delay between LED state changes. See README Section 4.5. */
#define LED_BLINK_DELAY_CYCLES (1000000U)

int main(void)
{
    /* Stop the watchdog timer to prevent periodic resets. See README Section 4.1. */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    /* Configure P1.0 for GPIO mode. See README Section 4.2. */
    P1->SEL0 &= ~BIT0;
    P1->SEL1 &= ~BIT0;

    /* Configure P1.0 as an output to drive the onboard red LED. See README Section 4.3. */
    P1->DIR |= BIT0;

    /* Start with the red LED turned off. See README Section 4.4. */
    P1->OUT &= ~BIT0;

    while (1)
    {
        /* Turn on the onboard red LED. */
        P1->OUT |= BIT0;
        __delay_cycles(LED_BLINK_DELAY_CYCLES);

        /* Turn off the onboard red LED. */
        P1->OUT &= ~BIT0;
        __delay_cycles(LED_BLINK_DELAY_CYCLES);
    }
}