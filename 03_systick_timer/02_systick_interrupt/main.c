/*
 * File: main.c
 * Author: i-monroy
 *
 * Project:
 *     SysTick Interrupt LED Delay
 *
 * Description:
 *     Toggles the onboard red LED using periodic SysTick interrupts.
 *
 * Hardware:
 *     MSP432P401R LaunchPad
 *
 * LED:
 *     P1.0 - Onboard red LED
 *
 * IDE:
 *     Code Composer Studio (CCS) 12.8.1
 */

#include "msp.h"

/*
 * Number of processor clock cycles for approximately 0.5 seconds
 * when using the default 3 MHz processor clock.
 */
#define SYSTICK_HALF_SECOND_COUNTS (1500000U)

/* Function Prototypes */
void LED_redLEDInit(void);
void SysTick_sysTickTimerInit(void);

/* Main */
int main(void)
{
    /* Stop the watchdog timer to prevent periodic resets. */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    /* Configure the onboard red LED. */
    LED_redLEDInit();

    /* Configure SysTick to generate a periodic interrupt. */
    SysTick_sysTickTimerInit();

    /* Enable interrupts globally. */
    __enable_irq();

    while (1)
    {
        /*
         * No delay or LED polling is required here.
         * The LED is controlled by the SysTick interrupt.
         */
    }
}

/* Configure P1.0 as an output for the onboard red LED. */
void LED_redLEDInit(void)
{
    /* Configure P1.0 for GPIO operation. */
    P1->SEL0 &= ~BIT0;
    P1->SEL1 &= ~BIT0;

    /* Configure P1.0 as an output. */
    P1->DIR |= BIT0;

    /* Start with the red LED turned off. */
    P1->OUT &= ~BIT0;
}

/* Configure SysTick for an approximately 0.5-second periodic interrupt. */
void SysTick_sysTickTimerInit(void)
{
    /* Disable SysTick while configuring the timer. */
    SysTick->CTRL = 0U;

    /*
     * Set the reload value for approximately 0.5 seconds
     * using the default 3 MHz processor clock.
     */
    SysTick->LOAD = SYSTICK_HALF_SECOND_COUNTS - 1U;

    /* Clear the current SysTick counter value. */
    SysTick->VAL = 0U;

    /*
     * Use the processor clock, enable the SysTick interrupt,
     * and start the timer.
     */
    SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_TICKINT_Msk   |
                     SysTick_CTRL_ENABLE_Msk);
}

/* SysTick interrupt service routine. */
void SysTick_Handler(void)
{
    /* Toggle the onboard red LED each time SysTick expires. */
    P1->OUT ^= BIT0;
}
