/*
 * File: main.c
 * Author: i-monroy
 *
 * Project:
 *     SysTick 4-Bit Binary Counter
 *
 * Description:
 *     Displays a 4-bit binary counter using four external LEDs.
 *     The counter can operate automatically using SysTick or
 *     manually using onboard push buttons.
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

/* Approximately 0.5 seconds using the default 3 MHz processor clock. */
#define SYSTICK_HALF_SECOND_COUNTS       (1500000U)

/* Timer flags. */
#define UPDATE_LEDS                      (0x01U)
#define MANUAL_COUNTER_BUTTONS_ENABLE    (0x02U)
#define CHANGE_STATE_BUTTON_ENABLE       (0x04U)

/* Counting modes. */
typedef enum
{
    AUTOMATIC,
    MANUAL
} counting_state_t;

/* Function Prototypes */
void LED_onBoardLEDsInit(void);
void LED_binaryLEDsInit(void);
void Button_buttonsInit(void);
void SysTick_sysTickTimerInit(void);

/* Begin in automatic mode. */
volatile counting_state_t counting_state = AUTOMATIC;

/* Start the binary counter at 0. */
volatile int8_t binary_counter = 0;

/* Shared timer/event flags. */
volatile uint8_t timer_flags =
        (UPDATE_LEDS |
         MANUAL_COUNTER_BUTTONS_ENABLE |
         CHANGE_STATE_BUTTON_ENABLE);

/* Main */
int main(void)
{
    /* Stop the watchdog timer to prevent periodic resets. */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    LED_onBoardLEDsInit();
    LED_binaryLEDsInit();
    Button_buttonsInit();
    SysTick_sysTickTimerInit();

    /* Automatic mode indicator starts ON. */
    P2->OUT |= BIT1;

    /* Enable Port 1 and Port 4 interrupts in the NVIC. */
    NVIC_EnableIRQ(PORT1_IRQn);
    NVIC_EnableIRQ(PORT4_IRQn);

    /* Enable interrupts globally. */
    __enable_irq();

    while (1)
    {
        /*
         * Keep the counter within the 4-bit range of 0 through 15.
         */
        if (binary_counter < 0)
        {
            binary_counter = 15;
        }
        else if (binary_counter >= 16)
        {
            binary_counter = 0;
        }

        /*
         * Update the four binary LEDs when requested.
         */
        if ((timer_flags & UPDATE_LEDS) != 0U)
        {
            timer_flags &= ~UPDATE_LEDS;

            /* Clear the previous 4-bit value. */
            P4->OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);

            /* Display the current counter value on P4.0 through P4.3. */
            P4->OUT |= ((uint8_t)binary_counter & 0x0FU);
        }
    }
}

/* Function Definitions */
void LED_onBoardLEDsInit(void)
{
    /* Configure onboard red LED P1.0 and green LED P2.1 as GPIO. */
    P1->SEL0 &= ~BIT0;
    P1->SEL1 &= ~BIT0;
    P2->SEL0 &= ~BIT1;
    P2->SEL1 &= ~BIT1;

    /* Set onboard LED pins as outputs. */
    P1->DIR |= BIT0;
    P2->DIR |= BIT1;

    /* Start both onboard LEDs off. */
    P1->OUT &= ~BIT0;
    P2->OUT &= ~BIT1;
}

void LED_binaryLEDsInit(void)
{
    /* Configure external LEDs P4.0 through P4.3 as GPIO. */
    P4->SEL0 &= ~(BIT0 | BIT1 | BIT2 | BIT3);
    P4->SEL1 &= ~(BIT0 | BIT1 | BIT2 | BIT3);

    /* Set external LEDs as outputs. */
    P4->DIR |= (BIT0 | BIT1 | BIT2 | BIT3);

    /* Start all external LEDs low. */
    P4->OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);
}

void Button_buttonsInit(void)
{
    /* Configure onboard buttons P1.1/P1.4 and external button P4.4 as GPIO. */
    P1->SEL0 &= ~(BIT1 | BIT4);
    P1->SEL1 &= ~(BIT1 | BIT4);
    P4->SEL0 &= ~BIT4;
    P4->SEL1 &= ~BIT4;

    /* Configure buttons as inputs. */
    P1->DIR &= ~(BIT1 | BIT4);
    P4->DIR &= ~BIT4;

    /* Enable internal resistors. */
    P1->REN |= (BIT1 | BIT4);
    P4->REN |= BIT4;

    /* Configure all buttons with pull-up resistors. */
    P1->OUT |= (BIT1 | BIT4);
    P4->OUT |= BIT4;

    /*
     * Active-low pull-up buttons generate a falling edge when pressed.
     */
    P1->IES |= (BIT1 | BIT4);
    P4->IES |= BIT4;

    /* Clear stale interrupt flags before enabling interrupts. */
    P1->IFG &= ~(BIT1 | BIT4);
    P4->IFG &= ~BIT4;

    /* Enable button interrupts. */
    P1->IE |= (BIT1 | BIT4);
    P4->IE |= BIT4;
}

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

void PORT1_IRQHandler(void)
{
    /* Increment button P1.1. */
    if ((P1->IFG & BIT1) != 0U)
    {
        if ((counting_state == MANUAL) &&
            ((timer_flags & MANUAL_COUNTER_BUTTONS_ENABLE) != 0U))
        {
            timer_flags &= ~MANUAL_COUNTER_BUTTONS_ENABLE;
            binary_counter++;
            timer_flags |= UPDATE_LEDS;
        }

        P1->IFG &= ~BIT1;
    }

    /* Decrement button P1.4. */
    if ((P1->IFG & BIT4) != 0U)
    {
        if ((counting_state == MANUAL) &&
            ((timer_flags & MANUAL_COUNTER_BUTTONS_ENABLE) != 0U))
        {
            timer_flags &= ~MANUAL_COUNTER_BUTTONS_ENABLE;
            binary_counter--;
            timer_flags |= UPDATE_LEDS;
        }

        P1->IFG &= ~BIT4;
    }
}

void PORT4_IRQHandler(void)
{
    if ((P4->IFG & BIT4) != 0U)
    {
        if ((timer_flags & CHANGE_STATE_BUTTON_ENABLE) != 0U)
        {
            timer_flags &= ~CHANGE_STATE_BUTTON_ENABLE;

            if (counting_state == AUTOMATIC)
            {
                counting_state = MANUAL;

                /* Red LED indicates manual mode. */
                P1->OUT |= BIT0;
                P2->OUT &= ~BIT1;
            }
            else
            {
                counting_state = AUTOMATIC;

                /* Green LED indicates automatic mode. */
                P1->OUT &= ~BIT0;
                P2->OUT |= BIT1;
            }
        }

        /* Clear the P4.4 interrupt flag. */
        P4->IFG &= ~BIT4;
    }
}

void SysTick_Handler(void)
{
    if (counting_state == AUTOMATIC)
    {
        binary_counter++;
        timer_flags |= UPDATE_LEDS;
    }
    else
    {
        /* Allow another manual counter button press. */
        timer_flags |= MANUAL_COUNTER_BUTTONS_ENABLE;
    }

    /* Allow the mode button to be pressed again. */
    timer_flags |= CHANGE_STATE_BUTTON_ENABLE;
}
