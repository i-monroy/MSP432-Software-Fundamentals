/*
 * File: main.c
 * Author: i-monroy
 *
 * Project:
 *     GPIO Button Input
 *
 * Description:
 *     Controls two onboard LEDs using two external push buttons.
 *     Button 1 controls an LED while held, and Button 2 toggles an LED.
 *
 * Hardware:
 *     MSP432P401R LaunchPad
 *
 * LEDs:
 *     P1.0 - Onboard red LED
 *     P2.0 - Red channel of the onboard RGB LED
 *
 * Buttons:
 *     P4.1 - External button configured with an internal pull-up resistor
 *     P4.6 - External button configured with an internal pull-down resistor
 *
 * IDE:
 *     Code Composer Studio (CCS) 12.8.1
 */

#include "msp.h"

/*
 * Short delay used to reduce mechanical button bounce.
 * See README Section 4.7.
 */
#define BUTTON_DEBOUNCE_DELAY_CYCLES (50000U)

int main(void)
{
    /* Stop the watchdog timer to prevent periodic resets. */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    /* Configure the LED and button pins for GPIO operation. See README Section 4.2. */
    P1->SEL0 &= ~BIT0;
    P1->SEL1 &= ~BIT0;

    P2->SEL0 &= ~BIT0;
    P2->SEL1 &= ~BIT0;

    P4->SEL0 &= ~(BIT1 | BIT6);
    P4->SEL1 &= ~(BIT1 | BIT6);

    /* Configure P1.0 and P2.0 as outputs to drive the onboard LEDs. */
    P1->DIR |= BIT0;
    P2->DIR |= BIT0;

    /* Configure P4.1 and P4.6 as button inputs. See README Section 4.3. */
    P4->DIR &= ~(BIT1 | BIT6);

    /* Enable the internal resistor for each button input. See README Section 4.4. */
    P4->REN |= BIT1 | BIT6;

    /* Configure P4.1 as an active-low input using an internal pull-up resistor. */
    P4->OUT |= BIT1;

    /* Configure P4.6 as an active-high input using an internal pull-down resistor. */
    P4->OUT &= ~BIT6;

    /* Start with both onboard LEDs turned off. */
    P1->OUT &= ~BIT0;
    P2->OUT &= ~BIT0;

    while (1)
    {
        /*
         * Continuously match the P1.0 LED state to the pull-up button state.
         * See README Section 4.5.
         */
        if ((P4->IN & BIT1) == 0U)
        {
            P1->OUT |= BIT0;
        }
        else
        {
            P1->OUT &= ~BIT0;
        }

        /*
         * Detect an active-high press from the pull-down button.
         * See README Section 4.6.
         */
        if ((P4->IN & BIT6) != 0U)
        {
            /* Allow the button signal time to settle before confirming the press. */
            __delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);

            if ((P4->IN & BIT6) != 0U)
            {
                /* Toggle the RGB LED's red channel once for each confirmed press. */
                P2->OUT ^= BIT0;

                /* Wait for Button 2 to be released before accepting another press. */
                while ((P4->IN & BIT6) != 0U)
                {
                    /* Intentionally empty. */
                }

                /* Allow the button signal time to settle after release. */
                __delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);
            }
        }
    }
}
