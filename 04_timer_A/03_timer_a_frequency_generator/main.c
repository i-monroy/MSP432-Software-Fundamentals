/*
 * File: main.c
 * Author: i-monroy
 *
 * Project:
 *     Timer_A Frequency Generator
 *
 * Description:
 *     Generates an approximately 1 kHz square wave using
 *     the Timer_A0 hardware output on P2.4.
 *
 * Hardware:
 *     MSP432P401R LaunchPad
 *
 * Output:
 *     P2.4 - Timer_A0.1 output
 *
 * IDE:
 *     Code Composer Studio (CCS) 12.8.1
 */

#include "msp.h"

/* Timer_A0 counts for a 1 kHz period and 50% duty cycle. See README Section 4.3. */
#define TIMER_A_PERIOD_COUNTS       (750U)
#define TIMER_A_HALF_PERIOD_COUNTS  (375U)

/* Function Prototypes */
void TimerA_outputInit(void);

/* Main */
int main(void)
{
    /* Stop the watchdog timer to prevent periodic resets. */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    /* Initialize the Timer_A0 hardware output on P2.4. */
    TimerA_outputInit();

    while (1)
    {
        /* Timer_A0 generates the output signal in hardware. See README Section 4.7. */
    }
}

/* Function Definitions */
void TimerA_outputInit(void)
{
    /* Configure P2.4 as the Timer_A0.1 peripheral output. See README Section 4.2. */
    P2->DIR  |= BIT4;
    P2->SEL0 |= BIT4;
    P2->SEL1 &= ~BIT4;

    /* Stop and clear Timer_A0 before configuration. */
    TIMER_A0->CTL = TIMER_A_CTL_CLR;

    /* Set CCR0 for an approximately 1 ms timer period. See README Section 4.4. */
    TIMER_A0->CCR[0] = TIMER_A_PERIOD_COUNTS - 1U;

    /* Set CCR1 halfway through the period for approximately 50% duty cycle. See README Section 4.5. */
    TIMER_A0->CCR[1] = TIMER_A_HALF_PERIOD_COUNTS;

    /* Configure TA0.1 for reset/set hardware output mode. See README Section 4.6. */
    TIMER_A0->CCTL[1] = TIMER_A_CCTLN_OUTMOD_7;

    /* Use SMCLK divided by 4 and start Timer_A0 in up mode. See README Sections 4.3 and 4.4. */
    TIMER_A0->CTL = (TIMER_A_CTL_SSEL__SMCLK |
                     TIMER_A_CTL_ID__4       |
                     TIMER_A_CTL_MC__UP);
}
