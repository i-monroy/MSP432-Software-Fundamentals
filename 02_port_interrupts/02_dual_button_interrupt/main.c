/*
 * File: main.c
 * Author: i-monroy
 *
 * Project:
 *     Dual Button Interrupt (Pull-Up and Pull-Down)
 *
 * Description:
 *     Controls two onboard LEDs using two external GPIO interrupts.
 *     Button 1 controls an LED while held, and Button 2 toggles an LED.
 *
 * Hardware:
 *     MSP432P401R LaunchPad
 *
 * LEDs:
 *     P1.0 - Onboard red LED
 *     P2.0 - Red channel of the onboard RGB LED
 *
 * Button Interrupts:
 *     P4.1 - External button configured with an internal pull-up resistor
 *     P4.6 - External button configured with an internal pull-down resistor
 *
 * IDE:
 *     Code Composer Studio (CCS) 12.8.1
 */

#include "msp.h"

/* Short delay used to reduce mechanical button bounce. */
#define BUTTON_DEBOUNCE_DELAY_CYCLES (150000U)

/* Function Prototypes */
void LED_redLEDsInit(void);
void Button_pullUpButtonInit(void);
void Button_pullDownButtonInit(void);

/* Main */
int main(void)
{
    /* Stop the watchdog timer to prevent periodic resets. */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    /* Configure both onboard LEDs. */
    LED_redLEDsInit();

    /* Configure both external button interrupts. */
    Button_pullUpButtonInit();
    Button_pullDownButtonInit();

    /* Enable Port 4 interrupts in the NVIC. */
    NVIC_EnableIRQ(PORT4_IRQn);

    /* Enable interrupts globally. */
    __enable_irq();

    while (1)
    {
        /* No button polling is required in the main loop. */
    }
}

/* Configure P1.0 and P2.0 as outputs for the onboard LEDs. */
void LED_redLEDsInit(void)
{
    /* Configure both LED pins for GPIO operation. */
    P1->SEL0 &= ~BIT0;
    P1->SEL1 &= ~BIT0;

    P2->SEL0 &= ~BIT0;
    P2->SEL1 &= ~BIT0;

    /* Configure both LED pins as outputs. */
    P1->DIR |= BIT0;
    P2->DIR |= BIT0;

    /* Start with both LEDs turned off. */
    P1->OUT &= ~BIT0;
    P2->OUT &= ~BIT0;
}

/* Configure P4.1 as an active-low input using an internal pull-up resistor. */
void Button_pullUpButtonInit(void)
{
    /* Configure P4.1 for GPIO operation. */
    P4->SEL0 &= ~BIT1;
    P4->SEL1 &= ~BIT1;

    /* Configure P4.1 as an input. */
    P4->DIR &= ~BIT1;

    /* Enable the internal resistor and select pull-up operation. */
    P4->REN |= BIT1;
    P4->OUT |= BIT1;

    /* Initially trigger when the button is pressed: high-to-low. */
    P4->IES |= BIT1;

    /* Clear any pending interrupt flag before enabling the interrupt. */
    P4->IFG &= ~BIT1;

    /* Enable interrupts for P4.1. */
    P4->IE |= BIT1;
}

/* Configure P4.6 as an active-high input using an internal pull-down resistor. */
void Button_pullDownButtonInit(void)
{
    /* Configure P4.6 for GPIO operation. */
    P4->SEL0 &= ~BIT6;
    P4->SEL1 &= ~BIT6;

    /* Configure P4.6 as an input. */
    P4->DIR &= ~BIT6;

    /* Enable the internal resistor and select pull-down operation. */
    P4->REN |= BIT6;
    P4->OUT &= ~BIT6;

    /* Trigger when the button is pressed: low-to-high. */
    P4->IES &= ~BIT6;

    /* Clear any pending interrupt flag before enabling the interrupt. */
    P4->IFG &= ~BIT6;

    /* Enable interrupts for P4.6. */
    P4->IE |= BIT6;
}

/* Port 4 interrupt service routine. */
void PORT4_IRQHandler(void)
{
    /*
     * P4.1 pull-up button:
     * Press turns P1.0 on and release turns P1.0 off.
     */
    if ((P4->IFG & BIT1) != 0U)
    {
        __delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);

        if ((P4->IN & BIT1) == 0U)
        {
            /* Button is pressed: turn the LED on. */
            P1->OUT |= BIT0;

            /* Next interrupt should occur when the button is released. */
            P4->IES &= ~BIT1;
        }
        else
        {
            /* Button is released: turn the LED off. */
            P1->OUT &= ~BIT0;

            /* Next interrupt should occur when the button is pressed. */
            P4->IES |= BIT1;
        }

        /* Clear the P4.1 interrupt flag. */
        P4->IFG &= ~BIT1;
    }

    /*
     * P4.6 pull-down button:
     * Each confirmed press toggles P2.0.
     */
    if ((P4->IFG & BIT6) != 0U)
    {
        __delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);

        if ((P4->IN & BIT6) != 0U)
        {
            /* Toggle the RGB LED's red channel. */
            P2->OUT ^= BIT0;
        }

        /* Clear the P4.6 interrupt flag. */
        P4->IFG &= ~BIT6;
    }
}
