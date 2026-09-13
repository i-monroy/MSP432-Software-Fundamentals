/*
 * File: main.c
 * Author: i-monroy
 *
 * Project:
 *     SysTick LED Delay
 *
 * Description:
 *     Blinks the onboard red LED using the SysTick timer
 *     without interrupts.
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
 * using the default 3 MHz processor clock. See README Section 4.3.
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

    LED_redLEDInit();
    SysTick_sysTickTimerInit();

    while (1)
    {
        /*
         * Check whether SysTick has counted down to zero.
         * See README Section 4.7.
         */
        if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U)
        {
            /* Toggle the onboard red LED. */
            P1->OUT ^= BIT0;
        }
    }
}

/* Function Definitions */
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

void SysTick_sysTickTimerInit(void)
{
    /* Disable SysTick before configuration. See README Section 4.6. */
    SysTick->CTRL = 0U;

    /*
     * Set the reload value for approximately 0.5 seconds.
     * See README Section 4.4.
     */
    SysTick->LOAD = SYSTICK_HALF_SECOND_COUNTS - 1U;

    /*
     * Clear the current SysTick counter value.
     * See README Section 4.5.
     */
    SysTick->VAL = 0U;

    /*
     * Use the processor clock and start SysTick without interrupts.
     * See README Section 4.6.
     */
    SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_ENABLE_Msk);
}
