/*
 * File: main.c
 * Author: i-monroy
 *
 * Project:
 *     GPIO 4-Bit Binary Counter
 *
 * Description:
 *     Displays a 4-bit binary counter using four external LEDs.
 *     Supports automatic counting and manual increment/decrement modes.
 *
 * Hardware:
 *     MSP432P401R LaunchPad
 *
 * LEDs:
 *     P1.0 - Onboard red LED (manual mode)
 *     P2.1 - Green channel of onboard RGB LED (automatic mode)
 *     P4.0 - External LED, bit 0 (LSB)
 *     P4.1 - External LED, bit 1
 *     P4.2 - External LED, bit 2
 *     P4.3 - External LED, bit 3 (MSB)
 *
 * Buttons:
 *     P1.1 - Onboard button, increment counter
 *     P1.4 - Onboard button, decrement counter
 *     P4.4 - External button, switch counting mode
 *
 * IDE:
 *     Code Composer Studio (CCS) 12.8.1
 */

#include "msp.h"
#include <stdint.h>

/* Delay between values while operating in automatic mode. */
#define BIT_LED_DELAY_CYCLES             (2000000U)

/* Short delay used to reduce mechanical button bounce. */
#define BUTTON_DEBOUNCE_DELAY_CYCLES     (50000U)

/* Counting modes. See README Section 4.4. */
typedef enum
{
    AUTOMATIC,
    MANUAL
} counting_state_t;

int main(void)
{
    /* Stop the watchdog timer to prevent periodic resets. */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    /* Configure the onboard LED and button pins for GPIO operation. */
    P1->SEL0 &= ~(BIT0 | BIT1 | BIT4);
    P1->SEL1 &= ~(BIT0 | BIT1 | BIT4);

    P2->SEL0 &= ~BIT1;
    P2->SEL1 &= ~BIT1;

    /* Configure the external LED and mode-button pins for GPIO operation. */
    P4->SEL0 &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4);
    P4->SEL1 &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4);

    /* Configure the onboard and external LED pins as outputs. */
    P1->DIR |= BIT0;
    P2->DIR |= BIT1;
    P4->DIR |= BIT0 | BIT1 | BIT2 | BIT3;

    /* Configure all three push buttons as inputs. */
    P1->DIR &= ~(BIT1 | BIT4);
    P4->DIR &= ~BIT4;

    /* Enable the internal resistors for all button inputs. */
    P1->REN |= BIT1 | BIT4;
    P4->REN |= BIT4;

    /* Configure all three buttons as active-low inputs using pull-up resistors. */
    P1->OUT |= BIT1 | BIT4;
    P4->OUT |= BIT4;

    /* Start in automatic mode: green LED on and red LED off. */
    P1->OUT &= ~BIT0;
    P2->OUT |= BIT1;

    /* Start the 4-bit binary display at 0000. */
    P4->OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);

    /* Begin the first state on automatic */
    counting_state_t counting_state = AUTOMATIC;

    /* Start the binary counter at 0 */
    uint8_t binary_counter = 0U;

    while (1)
    {
        /*
        * Check the external button for a requested mode change.
        * See README Section 4.5.
        */
        if ((P4->IN & BIT4) == 0U)
        {
            __delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);

            if ((P4->IN & BIT4) == 0U)
            {
                if (counting_state == AUTOMATIC)
                {
                    /* Enter manual mode and update the status LEDs. */
                    counting_state = MANUAL;
                    P1->OUT |= BIT0;
                    P2->OUT &= ~BIT1;
                }
                else
                {
                    /* Return to automatic mode and update the status LEDs. */
                    counting_state = AUTOMATIC;
                    P1->OUT &= ~BIT0;
                    P2->OUT |= BIT1;
                }

                /* Wait for the mode button to be released. */
                while ((P4->IN & BIT4) == 0U)
                {
                    /* Intentionally empty. */
                }

                __delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);
            }
        }

        /* Clear the previous 4-bit LED value. */
        P4->OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);

        /*
         * Map the lower four counter bits directly to P4.3:P4.0.
         * See README Section 4.6.
         */
        P4->OUT |= binary_counter;

        if (counting_state == AUTOMATIC)
        {
            /* Keep the current binary value visible before advancing. */
            __delay_cycles(BIT_LED_DELAY_CYCLES);

            /* Advance automatically and wrap from 15 back to 0. */
            binary_counter++;

            if (binary_counter >= 16U)
            {
                binary_counter = 0U;
            }
        }
        else
        {
            /* Increment the counter when the P1.1 button is pressed. */
            if ((P1->IN & BIT1) == 0U)
            {
                __delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);

                if ((P1->IN & BIT1) == 0U)
                {
                    if (binary_counter >= 15U)
                    {
                        binary_counter = 0U;
                    }
                    else
                    {
                        binary_counter++;
                    }

                    /* Wait for the increment button to be released. */
                    while ((P1->IN & BIT1) == 0U)
                    {
                        /* Intentionally empty. */
                    }

                    __delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);
                }
            }

            /* Decrement the counter when the P1.4 button is pressed. */
            else if ((P1->IN & BIT4) == 0U)
            {
                __delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);

                if ((P1->IN & BIT4) == 0U)
                {
                    if (binary_counter == 0U)
                    {
                        binary_counter = 15U;
                    }
                    else
                    {
                        binary_counter--;
                    }

                    /* Wait for the decrement button to be released. */
                    while ((P1->IN & BIT4) == 0U)
                    {
                        /* Intentionally empty. */
                    }

                    __delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);
                }
            }
        }
    }
}
