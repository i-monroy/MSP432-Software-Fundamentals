/*
 * File: main.c
 * Author: i-monroy
 *
 * Project:
 *     Timer_A Stopwatch
 *
 * Description:
 *     Implements a start, stop, and reset stopwatch using Timer_A0
 *     interrupts and displays the elapsed time on an LCD.
 *
 * Hardware:
 *     MSP432P401R LaunchPad
 *
 * LEDs:
 *     P1.0 - Onboard red LED (stopwatch stopped)
 *     P2.1 - Green channel of onboard RGB LED (stopwatch running)
 *
 * Buttons:
 *     P1.1 - Start/stop stopwatch
 *     P1.4 - Reset stopwatch while stopped
 *
 * IDE:
 *     Code Composer Studio (CCS) 12.8.1
 */

#include "msp.h"
#include <lcdLib.h>
#include <stdio.h>

/* Timer_A0 counts for approximately one second. See README Section 4.4. */
#define TIMER_A_ONE_SECOND_COUNTS    (46875U)

/* Software flags used for button lockout and stopwatch state. See README Section 4.7. */
#define START_STOP_FLAG              (0x01U)
#define ENABLE_STOPWATCH             (0x02U)

/* Stores the stopwatch time and individual display digits. See README Section 4.6. */
struct Time_StopWatchTime_s
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;

    uint8_t first_digit_seconds;
    uint8_t second_digit_seconds;
    uint8_t first_digit_minutes;
    uint8_t second_digit_minutes;
    uint8_t first_digit_hours;
    uint8_t second_digit_hours;
};

struct Time_StopWatchTime_s Time_StopWatchTime = {0};

/* Shared between the Timer_A0 and Port 1 interrupt handlers. */
volatile uint8_t timer_a_flags = 0U;

char buffer[16];

/* Function Prototypes */
void LED_redAndGreenLEDInit(void);
void TimerA_timerInit(void);
void Button_initializeOnBoardButton(void);
void LCD_updateStopWatchLCD(void);

/* Main */
int main(void)
{
    /* Stop the watchdog timer to prevent periodic resets. */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    /* Initialize and clear the LCD. See README Section 4.3. */
    lcdInit();
    lcdClear();

    /* Initialize the LEDs, Timer_A0, and onboard buttons. */
    LED_redAndGreenLEDInit();
    TimerA_timerInit();
    Button_initializeOnBoardButton();

    /* Enable the Timer_A0 CCR0 and Port 1 interrupts in the NVIC. */
    NVIC_EnableIRQ(TA0_0_IRQn);
    NVIC_EnableIRQ(PORT1_IRQn);

    /* Enable interrupts globally. See README Section 4.5. */
    __enable_irq();

    while (1)
    {
        /* Stopwatch operation is handled through interrupts. */
    }
}

/* Function Definitions */
void LED_redAndGreenLEDInit(void)
{
    /* Configure P1.0 and P2.1 for GPIO operation. */
    P1->SEL0 &= ~BIT0;
    P1->SEL1 &= ~BIT0;
    P2->SEL0 &= ~BIT1;
    P2->SEL1 &= ~BIT1;

    /* Configure P1.0 and P2.1 as outputs. */
    P1->DIR |= BIT0;
    P2->DIR |= BIT1;

    /* Indicate that the stopwatch initially starts in the stopped state. */
    P1->OUT |= BIT0;
    P2->OUT &= ~BIT1;
}

void TimerA_timerInit(void)
{
    /* Stop and clear Timer_A0 before configuration. */
    TIMER_A0->CTL = TIMER_A_CTL_CLR;

    /* Apply an additional divide-by-8 expansion divider. See README Section 4.4. */
    TIMER_A0->EX0 = TIMER_A_EX0_IDEX__8;

    /* Set CCR0 for an approximately one-second timer period. See README Section 4.4. */
    TIMER_A0->CCR[0] = TIMER_A_ONE_SECOND_COUNTS - 1U;

    /* Enable the Timer_A0 CCR0 compare interrupt. See README Section 4.5. */
    TIMER_A0->CCTL[0] |= TIMER_A_CCTLN_CCIE;

    /* Clear any pending CCR0 compare flag before starting Timer_A0. */
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

    /* Use SMCLK divided by 64 overall and start Timer_A0 in up mode. */
    TIMER_A0->CTL = (TIMER_A_CTL_SSEL__SMCLK |
                     TIMER_A_CTL_ID__8       |
                     TIMER_A_CTL_MC__UP);
}

/* Configure P1.1 and P1.4 as active-low button interrupt inputs. */
void Button_initializeOnBoardButton(void)
{
    /* Configure P1.1 and P1.4 for GPIO operation. */
    P1->SEL0 &= ~(BIT1 | BIT4);
    P1->SEL1 &= ~(BIT1 | BIT4);

    /* Configure both buttons as inputs with internal pull-up resistors. */
    P1->DIR &= ~(BIT1 | BIT4);
    P1->REN |=  (BIT1 | BIT4);
    P1->OUT |=  (BIT1 | BIT4);

    /* Trigger both button interrupts on a high-to-low transition. */
    P1->IES |= (BIT1 | BIT4);

    /* Clear pending flags before enabling the button interrupts. */
    P1->IFG &= ~(BIT1 | BIT4);

    /* Enable interrupt generation for P1.1 and P1.4. See README Section 4.8. */
    P1->IE |= (BIT1 | BIT4);
}

void LCD_updateStopWatchLCD(void)
{
    /* Display the stopwatch title on the first LCD row. */
    lcdSetText("TimerA Stopwatch", 0, 0);

    /* Format the elapsed time as HH:MM:SS. See README Section 4.6. */
    sprintf(buffer, "    %d%d:%d%d:%d%d",
            Time_StopWatchTime.second_digit_hours,
            Time_StopWatchTime.first_digit_hours,
            Time_StopWatchTime.second_digit_minutes,
            Time_StopWatchTime.first_digit_minutes,
            Time_StopWatchTime.second_digit_seconds,
            Time_StopWatchTime.first_digit_seconds);

    lcdSetText(buffer, 0, 1);
}

/* Timer_A0 CCR0 interrupt service routine. */
void TA0_0_IRQHandler(void)
{
    /* Clear CCIFG immediately to prevent unintended ISR re-entry. See README Section 4.5. */
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

    /* Re-enable start/stop button input once per Timer_A0 period. */
    timer_a_flags |= START_STOP_FLAG;

    /* Update the elapsed time only while the stopwatch is running. */
    if ((timer_a_flags & ENABLE_STOPWATCH) != 0U)
    {
        Time_StopWatchTime.seconds++;

        /* Advance minutes after 60 seconds. */
        if (Time_StopWatchTime.seconds >= 60U)
        {
            Time_StopWatchTime.minutes++;
            Time_StopWatchTime.seconds = 0U;
        }

        /* Advance hours after 60 minutes. */
        if (Time_StopWatchTime.minutes >= 60U)
        {
            Time_StopWatchTime.hours++;
            Time_StopWatchTime.minutes = 0U;
        }

        /* Restart the stopwatch after 24 hours. */
        if (Time_StopWatchTime.hours >= 24U)
        {
            Time_StopWatchTime.hours = 0U;
        }

        /* Separate each time value into decimal digits for the LCD. */
        Time_StopWatchTime.first_digit_seconds  = Time_StopWatchTime.seconds % 10U;
        Time_StopWatchTime.second_digit_seconds = Time_StopWatchTime.seconds / 10U;
        Time_StopWatchTime.first_digit_minutes  = Time_StopWatchTime.minutes % 10U;
        Time_StopWatchTime.second_digit_minutes = Time_StopWatchTime.minutes / 10U;
        Time_StopWatchTime.first_digit_hours    = Time_StopWatchTime.hours % 10U;
        Time_StopWatchTime.second_digit_hours   = Time_StopWatchTime.hours / 10U;
    }

    /* Refresh the LCD once per Timer_A0 period. */
    LCD_updateStopWatchLCD();
}

/* Port 1 interrupt service routine. See README Section 4.8. */
void PORT1_IRQHandler(void)
{
    /* P1.1 starts or stops the stopwatch when button input is enabled. */
    if ((P1->IFG & BIT1) != 0U)
    {
        if ((timer_a_flags & START_STOP_FLAG) != 0U)
        {
            /* Lock out another start/stop action until the next Timer_A0 period. */
            timer_a_flags &= ~START_STOP_FLAG;

            /* Update the LEDs to indicate the new stopwatch state. */
            P1->OUT ^= BIT0;
            P2->OUT ^= BIT1;

            /* Toggle between the running and stopped states. */
            timer_a_flags ^= ENABLE_STOPWATCH;
        }

        /* Clear the P1.1 interrupt flag before leaving the ISR. */
        P1->IFG &= ~BIT1;
    }

    /* P1.4 resets the stopwatch only while it is stopped. */
    if ((P1->IFG & BIT4) != 0U)
    {
        if ((timer_a_flags & ENABLE_STOPWATCH) == 0U)
        {
            /* Reset all stopwatch time and display values to zero. */
            Time_StopWatchTime = (struct Time_StopWatchTime_s){0};

            lcdClear();
            LCD_updateStopWatchLCD();
        }

        /* Clear the P1.4 interrupt flag before leaving the ISR. */
        P1->IFG &= ~BIT4;
    }
}
