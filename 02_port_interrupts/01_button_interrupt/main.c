/*
 * File: main.c
 * Author: i-monroy
 *
 * Project:
 *     GPIO Button Interrupt
 *
 * Description:
 *     Toggles the onboard red LED each time the onboard button is pressed.
 *
 * Hardware:
 *     MSP432P401R LaunchPad
 *
 * LED:
 *     P1.0 - Onboard red LED
 *
 * Button:
 *     P1.1 - Onboard push button
 *
 * IDE:
 *     Code Composer Studio (CCS) 12.8.1
 */

#include "msp.h"

/* Short delay used to reduce mechanical button bounce. */
#define BUTTON_DEBOUNCE_DELAY_CYCLES (150000U)

/* Function Prototypes */
void LED_initializeRedLED(void);
void Button_initializeOnBoardButton(void);

/* Main */
int main(void)
{
    /* Stop the watchdog timer to prevent periodic resets. */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    /* Configure the onboard red LED. */
    LED_initializeRedLED();

    /* Configure the onboard button and its Port 1 interrupt. */
    Button_initializeOnBoardButton();

    /*
     * Enable Port 1 interrupts in the Nested Vectored Interrupt Controller.
     * See README Section 4.5.
     */
    NVIC_EnableIRQ(PORT1_IRQn);

    /*
     * Enable interrupts globally so the CPU can respond to interrupt requests.
     * See README Section 4.6.
     */
    __enable_irq();

    while (1)
    {
        /*
         * No polling is required here. Button presses are handled by
         * the Port 1 interrupt service routine. See README Section 4.7.
         */
    }
}

/* Configure P1.0 as an output for the onboard red LED. */
void LED_initializeRedLED(void)
{
    /* Configure P1.0 for GPIO operation. */
    P1->SEL0 &= ~BIT0;
    P1->SEL1 &= ~BIT0;

    /* Configure P1.0 as an output to drive the onboard red LED. */
    P1->DIR |= BIT0;

    /* Start with the red LED turned off. */
    P1->OUT &= ~BIT0;
}

/* Configure P1.1 as an active-low button input with a falling-edge interrupt. */
void Button_initializeOnBoardButton(void)
{
    /* Configure P1.1 for GPIO operation. */
    P1->SEL0 &= ~BIT1;
    P1->SEL1 &= ~BIT1;

    /* Configure P1.1 as an input. */
    P1->DIR &= ~BIT1;

    /* Enable the internal resistor for P1.1. */
    P1->REN |= BIT1;

    /* Configure P1.1 with an internal pull-up resistor. */
    P1->OUT |= BIT1;

    /*
     * Trigger the interrupt when P1.1 transitions from high to low.
     * See README Section 4.2.
     */
    P1->IES |= BIT1;

    /*
     * Clear any pending P1.1 interrupt flag before enabling the interrupt.
     * See README Section 4.3.
     */
    P1->IFG &= ~BIT1;

    /*
     * Enable interrupt generation for P1.1.
     * See README Section 4.4.
     */
    P1->IE |= BIT1;
}

/*
 * Port 1 interrupt service routine.
 * See README Section 4.8.
 */
void PORT1_IRQHandler(void)
{
    /* Check whether P1.1 generated the Port 1 interrupt. */
    if ((P1->IFG & BIT1) != 0U)
    {
        /* Allow the button signal time to settle before handling the press. */
        __delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);

        /* Toggle the onboard red LED once for each button interrupt. */
        P1->OUT ^= BIT0;

        /* Clear the P1.1 interrupt flag before leaving the ISR. */
        P1->IFG &= ~BIT1;
    }
}
