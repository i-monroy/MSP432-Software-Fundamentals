/*
 * File: main.c
 * Author: i-monroy
 *
 * Project:
 *     GPIO Blink LED
 *
 * Description:
 *     Toggles the onboard red LED connected to P1.0.
 *
 * Hardware:
 *     MSP432P401R LaunchPad
 *
 * LED:
 *     P1.0 - Red LED
 */

#include "msp.h"

/* Approximate 1 second delay at 1 MHz MCLK. */
#define LED_BLINK_DELAY_CYCLES (1000000U)

static void LED_Init(void);

int main(void)
{
    /* Stop the watchdog timer to prevent periodic resets. */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    LED_Init();

    while (1)
    {
        /* Turn red LED on. (set P1.0) */
        P1->OUT |= BIT0;
        __delay_cycles(LED_BLINK_DELAY_CYCLES);

        /* Turn red LED off. (clear P1.0) */
        P1->OUT &= ~BIT0;
        __delay_cycles(LED_BLINK_DELAY_CYCLES);
    }
}

static void LED_Init(void)
{
    /* Configure P1.0 for GPIO mode. */
    P1->SEL0 &= ~BIT0;
    P1->SEL1 &= ~BIT0;

    /* Configure P1.0 as a digital output. */
    P1->DIR |= BIT0;

    /* Turn off the red LED before entering the main loop. */
    P1->OUT &= ~BIT0;
}