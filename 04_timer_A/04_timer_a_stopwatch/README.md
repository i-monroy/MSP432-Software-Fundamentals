# Timer_A Stopwatch 🦩

## 1. Overview

This example uses **Timer_A0 interrupts** to implement a simple digital stopwatch on the MSP432P401R.

Timer_A0 generates an interrupt approximately once every second. While the stopwatch is running, each Timer_A0 interrupt increments the elapsed time and updates the LCD display.

The two onboard push buttons control the stopwatch:

- **P1.1** starts and stops the stopwatch.
- **P1.4** resets the stopwatch while it is stopped.

The onboard LEDs indicate the current stopwatch state:

- **Red LED (P1.0)** - Stopwatch stopped
- **Green LED (P2.1)** - Stopwatch running

The elapsed time is displayed in:

```text
HH:MM:SS
```

format.

This example combines concepts from the previous Timer_A and Port Interrupt examples to create a complete timer-based application.

---

## 2. Hardware Used

- MSP432P401R LaunchPad
- 16x2 LCD
- Two onboard push buttons
- Two onboard LEDs
- Jumper wires for the LCD connection

---

## 3. Pinout / Controls

### Onboard Controls

| Component | MSP432 Pin | Function |
|---|---|---|
| S1 Button | P1.1 | Start / stop stopwatch |
| S2 Button | P1.4 | Reset stopwatch |
| Red LED | P1.0 | Stopwatch stopped |
| Green RGB LED | P2.1 | Stopwatch running |

Both onboard buttons are active-low and use the MSP432's internal pull-up resistors.

### Stopwatch Controls

| Button | Stopwatch State | Action |
|---|---|---|
| P1.1 | Stopped | Start |
| P1.1 | Running | Stop |
| P1.4 | Stopped | Reset to `00:00:00` |
| P1.4 | Running | No reset |

The reset button is intentionally ignored while the stopwatch is running.

---

## 4. Code Walkthrough

### 4.1 Program Operation

The stopwatch is primarily controlled through two interrupt sources:

```text
              Timer_A0
                 |
                 | ~1 second
                 v
        TA0_0_IRQHandler()
                 |
                 +---- Update elapsed time
                 |
                 +---- Update LCD


         P1.1 / P1.4 Buttons
                 |
                 v
         PORT1_IRQHandler()
                 |
                 +---- Start / stop
                 |
                 +---- Reset
```

After initialization, the main loop does not need to continuously poll the timer or buttons:

```c
while (1)
{
    /* Stopwatch operation is handled through interrupts. */
}
```

Timer_A provides the stopwatch time base, while Port 1 interrupts handle user input.

---

### 4.2 LED State Indicators

Two onboard LEDs indicate whether the stopwatch is running or stopped.

```c
P1->OUT |= BIT0;
P2->OUT &= ~BIT1;
```

The stopwatch initially starts in the stopped state, so:

```text
Red LED   = ON
Green LED = OFF
```

When P1.1 starts or stops the stopwatch, both LEDs are toggled:

```c
P1->OUT ^= BIT0;
P2->OUT ^= BIT1;
```

This creates the following behavior:

| Stopwatch State | Red LED | Green LED |
|---|---|---|
| Stopped | ON | OFF |
| Running | OFF | ON |

---

### 4.3 LCD Initialization

The LCD is initialized and cleared when the program starts:

```c
lcdInit();
lcdClear();
```

The first row displays:

```text
TimerA Stopwatch
```

The second row displays the elapsed time:

```text
    HH:MM:SS
```

The LCD is updated using:

```c
LCD_updateStopWatchLCD();
```

The display is refreshed from the Timer_A0 interrupt so that the displayed time follows the stopwatch count.

---

### 4.4 Timer_A0 One-Second Time Base

The stopwatch requires an event approximately once every second.

This example assumes the default SMCLK is approximately:

```text
SMCLK ≈ 3 MHz
```

Timer_A0 uses two clock dividers.

The primary divider is configured for divide-by-8:

```c
TIMER_A_CTL_ID__8
```

The expansion divider is also configured for divide-by-8:

```c
TIMER_A0->EX0 = TIMER_A_EX0_IDEX__8;
```

The total clock division is therefore:

```text
8 x 8 = 64
```

The resulting Timer_A0 clock is approximately:

```text
3,000,000 Hz / 64
= 46,875 Hz
```

Timer_A0 therefore receives approximately:

```text
46,875 timer ticks per second
```

The program defines:

```c
#define TIMER_A_ONE_SECOND_COUNTS (46875U)
```

CCR0 is then configured as:

```c
TIMER_A0->CCR[0] = TIMER_A_ONE_SECOND_COUNTS - 1U;
```

which produces:

```text
CCR0 = 46,875 - 1
     = 46,874
```

Timer_A0 operates in up mode, so the timer repeatedly counts through approximately 46,875 timer states.

At approximately 46,875 timer ticks per second:

```text
46,875 counts
---------------- = approximately 1 second
46,875 counts/s
```

This gives the stopwatch its approximately one-second time base.

---

### 4.5 Timer_A0 Interrupt

The CCR0 interrupt is enabled using:

```c
TIMER_A0->CCTL[0] |= TIMER_A_CCTLN_CCIE;
```

The Timer_A0 CCR0 interrupt is then enabled in the NVIC:

```c
NVIC_EnableIRQ(TA0_0_IRQn);
```

Global interrupts are enabled using:

```c
__enable_irq();
```

Timer_A0 calls:

```c
TA0_0_IRQHandler()
```

approximately once every second.

At the beginning of the ISR, the CCR0 interrupt flag is cleared:

```c
TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
```

The flag is cleared immediately so that the peripheral has sufficient time to process the clear before the ISR exits.

This follows the same early-flag-clear approach demonstrated in the previous Timer_A interrupt example.

If the stopwatch is running, the ISR then increments the elapsed time.

---

### 4.6 Stopwatch Timekeeping and LCD Formatting

The stopwatch stores its elapsed time in a structure:

```c
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
```

The primary time values are:

```text
seconds
minutes
hours
```

When the stopwatch is enabled, every Timer_A0 interrupt increments the seconds:

```c
Time_StopWatchTime.seconds++;
```

After 60 seconds:

```c
if (Time_StopWatchTime.seconds >= 60U)
{
    Time_StopWatchTime.minutes++;
    Time_StopWatchTime.seconds = 0U;
}
```

After 60 minutes:

```c
if (Time_StopWatchTime.minutes >= 60U)
{
    Time_StopWatchTime.hours++;
    Time_StopWatchTime.minutes = 0U;
}
```

After 24 hours:

```c
if (Time_StopWatchTime.hours >= 24U)
{
    Time_StopWatchTime.hours = 0U;
}
```

The stopwatch therefore follows:

```text
00:00:00
   |
   v
00:00:01
   |
   v
   ...
   |
   v
00:00:59
   |
   v
00:01:00
```

### Separating the Display Digits

Each time value is separated into individual decimal digits before being displayed.

For example:

```c
Time_StopWatchTime.first_digit_seconds =
    Time_StopWatchTime.seconds % 10U;

Time_StopWatchTime.second_digit_seconds =
    Time_StopWatchTime.seconds / 10U;
```

For a value such as:

```text
seconds = 47
```

the operations produce:

```text
47 % 10 = 7
47 / 10 = 4
```

Therefore:

```text
Tens digit = 4
Ones digit = 7
```

The same operation is performed for seconds, minutes, and hours.

The digits are then formatted into the LCD buffer:

```c
sprintf(buffer, "    %d%d:%d%d:%d%d",
        Time_StopWatchTime.second_digit_hours,
        Time_StopWatchTime.first_digit_hours,
        Time_StopWatchTime.second_digit_minutes,
        Time_StopWatchTime.first_digit_minutes,
        Time_StopWatchTime.second_digit_seconds,
        Time_StopWatchTime.first_digit_seconds);
```

This produces the familiar:

```text
HH:MM:SS
```

display.

---

### 4.7 Stopwatch State and Button Lockout

Software flags are stored in:

```c
volatile uint8_t timer_a_flags;
```

Two bit masks are used:

```c
#define START_STOP_FLAG  (0x01U)
#define ENABLE_STOPWATCH (0x02U)
```

Each bit represents a different software condition.

#### `ENABLE_STOPWATCH`

This flag represents whether the stopwatch is running.

When the flag is set:

```c
timer_a_flags & ENABLE_STOPWATCH
```

the Timer_A0 ISR increments the elapsed time.

P1.1 toggles the flag:

```c
timer_a_flags ^= ENABLE_STOPWATCH;
```

This switches between:

```text
STOPPED <----> RUNNING
```

#### `START_STOP_FLAG`

This flag provides a simple button input lockout.

Timer_A0 sets the flag once per timer period:

```c
timer_a_flags |= START_STOP_FLAG;
```

When P1.1 successfully performs a start/stop action, the Port 1 ISR clears it:

```c
timer_a_flags &= ~START_STOP_FLAG;
```

Another start/stop action cannot occur until Timer_A0 sets the flag again.

This prevents repeated start/stop actions from occurring rapidly from a single button interaction.

> **Note:** This is a simple timer-based input lockout for this example. It should not be considered a precise or general-purpose switch debounce implementation.

---

### 4.8 Port 1 Button Interrupts

Both onboard buttons are configured as active-low inputs using internal pull-up resistors:

```c
P1->DIR &= ~(BIT1 | BIT4);
P1->REN |=  (BIT1 | BIT4);
P1->OUT |=  (BIT1 | BIT4);
```

Because the buttons are active-low:

```text
Released = HIGH
Pressed  = LOW
```

Both buttons therefore use a falling-edge interrupt:

```c
P1->IES |= (BIT1 | BIT4);
```

The Port 1 interrupt is enabled in the NVIC:

```c
NVIC_EnableIRQ(PORT1_IRQn);
```

Both buttons share:

```c
PORT1_IRQHandler()
```

The ISR checks each interrupt flag independently.

#### P1.1 - Start / Stop

```c
if ((P1->IFG & BIT1) != 0U)
```

If the start/stop input is currently allowed, pressing P1.1:

1. Clears the start/stop input flag.
2. Toggles the red and green LEDs.
3. Toggles the stopwatch running state.

The hardware interrupt flag is cleared before leaving the ISR:

```c
P1->IFG &= ~BIT1;
```

#### P1.4 - Reset

```c
if ((P1->IFG & BIT4) != 0U)
```

The stopwatch can only be reset when it is stopped:

```c
if ((timer_a_flags & ENABLE_STOPWATCH) == 0U)
```

The complete stopwatch structure is reset using:

```c
Time_StopWatchTime = (struct Time_StopWatchTime_s){0};
```

This sets all members of the structure back to zero.

The LCD is then refreshed to display:

```text
00:00:00
```

The hardware interrupt flag is cleared before leaving the ISR:

```c
P1->IFG &= ~BIT4;
```

---

### 4.9 Complete Program Flow

After startup:

```text
Program Start
     |
     v
Initialize LCD
     |
     v
Initialize LEDs
     |
     v
Initialize Timer_A0
     |
     v
Initialize Buttons
     |
     v
Enable Interrupts
     |
     v
+-----------------------+
|       Main Loop       |
|                       |
|   No polling needed   |
+-----------------------+
       ^           ^
       |           |
       |           |
 Timer_A0 ISR   Port 1 ISR
       |           |
       |           +---- Start / Stop
       |           |
       |           +---- Reset
       |
       +---- One-second time base
       |
       +---- Increment stopwatch
       |
       +---- Update LCD
```

The result combines Timer_A interrupts and GPIO interrupts into a single application.

---

## 5. Expected Result

When the MSP432P401R starts:

```text
TimerA Stopwatch
    00:00:00
```

should appear on the LCD.

The red onboard LED should be ON, indicating that the stopwatch is stopped.

### Press P1.1

The stopwatch begins counting:

```text
TimerA Stopwatch
    00:00:01
```

then:

```text
TimerA Stopwatch
    00:00:02
```

and continues approximately once per second.

The LEDs change to:

```text
Red LED   = OFF
Green LED = ON
```

### Press P1.1 Again

The stopwatch stops at its current value.

The LEDs return to:

```text
Red LED   = ON
Green LED = OFF
```

### Press P1.4 While Stopped

The stopwatch resets to:

```text
TimerA Stopwatch
    00:00:00
```

### Press P1.4 While Running

The reset request is ignored and the stopwatch continues counting.

---

## 6. Register Summary

| Register | Purpose |
|---|---|
| `P1->SEL0 / SEL1` | Configures P1.0, P1.1, and P1.4 for GPIO operation |
| `P1->DIR` | Configures the red LED as an output and buttons as inputs |
| `P1->REN` | Enables the internal button resistors |
| `P1->OUT` | Selects pull-up resistors and controls the red LED |
| `P1->IES` | Selects falling-edge button interrupts |
| `P1->IE` | Enables P1.1 and P1.4 interrupt generation |
| `P1->IFG` | Indicates pending Port 1 interrupt events |
| `P2->SEL0 / SEL1` | Configures P2.1 for GPIO operation |
| `P2->DIR` | Configures the green LED as an output |
| `P2->OUT` | Controls the green LED |
| `TIMER_A0->CTL` | Selects the Timer_A0 clock, divider, and operating mode |
| `TIMER_A0->EX0` | Applies the Timer_A0 expansion divider |
| `TIMER_A0->CCR[0]` | Defines the approximately one-second period |
| `TIMER_A0->CCTL[0]` | Controls the CCR0 interrupt and interrupt flag |

---

## 7. Common Problems

### Stopwatch Does Not Count

Verify that:

- Timer_A0 is configured for SMCLK.
- Both divide-by-8 clock dividers are configured.
- The CCR0 interrupt is enabled.
- `TA0_0_IRQn` is enabled in the NVIC.
- Global interrupts are enabled.

Also verify that the stopwatch has been started using P1.1.

### Stopwatch Counts at the Wrong Speed

This example assumes:

```text
SMCLK ≈ 3 MHz
```

with an overall Timer_A0 divider of:

```text
8 x 8 = 64
```

If SMCLK is configured differently, the stopwatch timing will also change.

This example is intended to demonstrate Timer_A operation and does not configure a precision timekeeping clock.

### Buttons Do Not Respond

Verify that P1.1 and P1.4 are configured as:

- GPIO inputs
- Internal pull-up resistors enabled
- Falling-edge interrupts
- Interrupt generation enabled

Also verify that:

```c
NVIC_EnableIRQ(PORT1_IRQn);
```

and:

```c
__enable_irq();
```

have been called.

### Start / Stop Button Does Not Respond Immediately

The start/stop input uses a simple Timer_A-based lockout.

After an accepted start/stop action, another action is not accepted until Timer_A0 re-enables the input flag on a later timer event.

This behavior is intentional for this example.

### Reset Button Does Not Work While Counting

This is intentional.

P1.4 only resets the stopwatch when:

```text
Stopwatch = STOPPED
```

The reset request is ignored while the stopwatch is running.

### LCD Does Not Display the Time

Verify that:

- The LCD is wired correctly.
- The LCD library is included in the project.
- `lcdInit()` executes successfully.
- The LCD is receiving the required power and control signals.

---

## 8. Timer_A Section Complete

This stopwatch completes the Timer_A examples in this repository:

```text
03_Timer_A/
├── 01_timer_a_delay/
├── 02_timer_a_interrupt/
├── 03_timer_a_frequency_generator/
└── 04_timer_a_stopwatch/
```

Together, these examples demonstrate three different ways Timer_A can interact with software and hardware:

```text
Polling
   |
   v
CPU checks Timer_A status

Interrupts
   |
   v
Timer_A notifies the CPU

Hardware Output
   |
   v
Timer_A directly controls an output

Application
   |
   v
Timer_A provides a time base for a stopwatch
```

The next section of the repository continues with additional MSP432 peripherals and applications.