/*
 * File: main.c
 * Author: i-monroy
 *
 * Project:
 *     Timer_A LED Delay
 *
 * Description:
 *     Blinks the onboard red LED using Timer_A0
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
 * Number of Timer_A0 counts for approximately 0.5 seconds.
 * See README Section 4.4.
 */
#define TIMER_A_HALF_SECOND_COUNTS    (46875U)

/* Function Prototypes */
void LED_redLEDInit(void);
void TimerA_timerInit(void);

/* Main */
int main(void)
{
    /* Stop the watchdog timer to prevent periodic resets. */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    /* Initialize the onboard red LED and Timer_A0. */
    LED_redLEDInit();
    TimerA_timerInit();

    while (1)
    {
        /*
         * Poll the CCR0 compare flag to determine when Timer_A0
         * reaches the configured count. See README Section 4.8.
         */
        if ((TIMER_A0->CCTL[0] & TIMER_A_CCTLN_CCIFG) != 0U)
        {
            /* Clear the CCR0 compare flag before the next timer period. */
            TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

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

void TimerA_timerInit(void)
{
    /*
     * Stop and clear Timer_A0 before configuration.
     * See README Section 4.3.
     */
    TIMER_A0->CTL = TIMER_A_CTL_CLR;

    /*
     * Divide the Timer_A0 clock by an additional factor of 4.
     * See README Section 4.4.
     */
    TIMER_A0->EX0 = TIMER_A_EX0_IDEX__4;

    /*
     * Set CCR0 for approximately 0.5 seconds.
     * See README Section 4.6.
     */
    TIMER_A0->CCR[0] = TIMER_A_HALF_SECOND_COUNTS - 1U;

    /*
     * Clear any existing CCR0 compare flag before starting the timer.
     * See README Section 4.7.
     */
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

    /*
     * Use SMCLK, divide it by 8, and start Timer_A0 in up mode.
     * See README Sections 4.4 and 4.5.
     */
    TIMER_A0->CTL = (TIMER_A_CTL_SSEL__SMCLK |
                     TIMER_A_CTL_ID__8       |
                     TIMER_A_CTL_MC__UP);
}
