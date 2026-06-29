/*
 * File: main.c
 * Author:
 * Project:
 *
 * Description:
 *
 * Hardware:
 *     MSP432P401R LaunchPad
 */

#include "msp.h"

/* Macros */


/* Function Prototypes */


/* Main */
int main(void)
{
    /* Stop the watchdog timer to prevent periodic resets. */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    while (1)
    {
        /* Main program loop */
    }
}

/* Function Definitions */