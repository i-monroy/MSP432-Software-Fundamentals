# Timer_A LED Delay 🐇

This example blinks the onboard red LED using **Timer_A0** without interrupts.

Timer_A0 runs continuously in **up mode**, while the program polls the `CCR0` compare flag to determine when approximately 0.5 seconds have elapsed.

---

## 1. Overview

The MSP432P401R contains Timer_A peripherals that can be used for timing, interrupts, signal generation, capture, and other hardware-timing applications.

This first Timer_A example focuses on the basic timer operation.

The program:

1. Configures the onboard red LED as an output.
2. Configures Timer_A0 to use `SMCLK`.
3. Divides the timer clock to obtain a suitable counting frequency.
4. Places Timer_A0 in up mode.
5. Uses `CCR0` to create an approximately 0.5-second timer period.
6. Polls the `CCR0` compare flag.
7. Toggles the LED whenever the timer period completes.

No Timer_A interrupt is used in this example.

The basic execution flow is:

```text
Timer_A0 counts upward
        ↓
Timer_A0 reaches CCR0
        ↓
CCIFG is set
        ↓
main() detects CCIFG
        ↓
Clear CCIFG
        ↓
Toggle LED
        ↓
Timer_A0 continues
```

---

## 2. Hardware Used

| Item | Description |
| --- | --- |
| Microcontroller Board | MSP432P401R LaunchPad |
| LED | Onboard red LED |
| LED Pin | `P1.0` |
| Timer | Timer_A0 |
| IDE | Code Composer Studio 12.8.1 |

No external components are required.

---

## 3. Pinout / Wiring

The example uses the onboard red LED connected to `P1.0`.

| Component | MSP432P401R Pin | Purpose |
| --- | --- | --- |
| Onboard Red LED | `P1.0` | Timer output indication |

Because the LED is already connected on the LaunchPad, no external wiring is required.

---

## 4. Code Walkthrough

### 4.1 Initialize the Red LED

The onboard red LED is connected to `P1.0`.

The pin is first configured for GPIO operation:

```c
P1->SEL0 &= ~BIT0;
P1->SEL1 &= ~BIT0;
```

`P1.0` is then configured as an output:

```c
P1->DIR |= BIT0;
```

The LED begins turned off:

```c
P1->OUT &= ~BIT0;
```

---

### 4.2 Understanding Timer_A

Timer_A is a hardware timer peripheral available on the MSP432P401R.

Unlike SysTick, which is part of the ARM Cortex-M4 processor core, Timer_A is an MSP432 peripheral with additional timer features.

Timer_A can be used for applications such as:

- Periodic timing
- Timer interrupts
- Output signal generation
- PWM generation
- Input capture
- Frequency and pulse-width measurement

This example uses **Timer_A0** as a basic periodic timer.

Timer_A0 receives a clock signal and increments its counter based on that clock:

```text
Clock Source
     ↓
Clock Dividers
     ↓
Timer_A0 Counter
     ↓
Compare against CCR0
     ↓
CCIFG
```

The timer counter is 16 bits wide, giving a counting range of:

```text
0 through 65,535
```

The clock frequency and timer dividers therefore determine how much real time can be represented by those counts.

---

### 4.3 Stop and Clear Timer_A0

Timer_A0 is stopped and cleared before it is configured:

```c
TIMER_A0->CTL = TIMER_A_CTL_CLR;
```

`CLR` resets the Timer_A counter and divider logic.

This provides a known starting state before the timer configuration is applied.

The timer is not started until its clock source and operating mode are configured later.

---

### 4.4 Timer Clock, Dividers, and Counts

This is one of the most important concepts when configuring a hardware timer.

A timer counts incoming clock ticks.

If the timer clock operates at:

```text
93,750 Hz
```

then Timer_A receives:

```text
93,750 timer ticks every second
```

Therefore:

```text
1.00 second → 93,750 counts
0.50 second → 46,875 counts
0.25 second → 23,437.5 counts
0.10 second →  9,375 counts
```

The basic relationship is:

```text
Timer Counts = Timer Frequency × Desired Time
```

or:

```text
Desired Time = Timer Counts / Timer Frequency
```

#### Timer_A0 Clock Frequency

This example assumes the default approximately 3 MHz `SMCLK`.

Timer_A0 first divides `SMCLK` by 8:

```c
TIMER_A_CTL_ID__8
```

Therefore:

```text
3,000,000 Hz
    ÷ 8
────────────
375,000 Hz
```

If Timer_A0 used only this divider, a 0.5-second period would require:

```text
375,000 × 0.5
      =
187,500 counts
```

However, Timer_A is a 16-bit timer:

```text
Maximum count = 65,535
```

`187,500` does not fit inside the 16-bit timer range.

Timer_A therefore provides an additional expansion divider through the `EX0` register.

This example uses:

```c
TIMER_A0->EX0 = TIMER_A_EX0_IDEX__4;
```

which divides the clock by another factor of 4:

```text
3,000,000
    ÷ 8
    ÷ 4
────────────
93,750 Hz
```

The complete timer clock equation is therefore:

```text
                    SMCLK
Timer Frequency = ─────────────
                  ID × IDEX
```

For this example:

```text
                   3,000,000
Timer Frequency = ────────────
                     8 × 4

                = 93,750 Hz
```

Timer_A0 now increments approximately **93,750 times per second**.

For a 0.5-second period:

```text
93,750 × 0.5
     =
46,875 counts
```

This value fits inside the 16-bit timer range.

#### Changing the Divider Changes the Timer Period

Suppose the expansion divider were changed from 4 to 2:

```text
3,000,000
    ÷ 8
    ÷ 2
────────────
187,500 Hz
```

Timer_A would now increment 187,500 times per second.

Using the same 46,875 timer counts would produce:

```text
46,875 / 187,500
       =
0.25 seconds
```

Therefore:

```text
÷8 and ÷4 → 93,750 Hz  → 46,875 counts ≈ 0.50 s

÷8 and ÷2 → 187,500 Hz → 46,875 counts ≈ 0.25 s
```

The number stored in `CCR0` only represents an amount of time when the timer clock frequency is also known.

#### Divider and Resolution Tradeoff

Increasing the clock divider allows Timer_A to represent longer periods using its 16-bit counter.

However, it also makes each individual timer tick represent more time.

For example:

```text
Timer Frequency     Approximate Time per Tick

375,000 Hz                2.67 µs
187,500 Hz                5.33 µs
 93,750 Hz               10.67 µs
 46,875 Hz               21.33 µs
```

Therefore:

```text
Faster timer clock
       ↓
Better timing resolution
       ↓
Shorter maximum timer period


Slower timer clock
       ↓
Lower timing resolution
       ↓
Longer maximum timer period
```

Selecting timer dividers is therefore a balance between the required timing range and timing resolution.

---

### 4.5 Timer_A Up Mode

Timer_A supports multiple operating modes.

This example uses **up mode**:

```c
TIMER_A_CTL_MC__UP
```

In up mode, Timer_A counts upward from zero toward the value stored in `CCR0`.

Conceptually:

```text
0 → 1 → 2 → 3 → ... → CCR0
↑                         │
└─────────────────────────┘
          repeat
```

After completing the timer period, the timer returns to the beginning of the next period and continues counting.

This makes up mode useful for generating periodic timing events.

The timer is started with:

```c
TIMER_A0->CTL = (TIMER_A_CTL_SSEL__SMCLK |
                 TIMER_A_CTL_ID__8       |
                 TIMER_A_CTL_MC__UP);
```

The three settings mean:

```text
SSEL__SMCLK → Use SMCLK as the Timer_A0 clock source

ID__8       → Divide the input clock by 8

MC__UP      → Operate Timer_A0 in up mode
```

The additional divide-by-4 setting is configured separately through:

```c
TIMER_A0->EX0 = TIMER_A_EX0_IDEX__4;
```

Together, the two dividers reduce the approximately 3 MHz `SMCLK` to approximately 93.75 kHz.

---

### 4.6 Configure CCR0

Capture/Compare Register 0 is configured using:

```c
TIMER_A0->CCR[0] = TIMER_A_HALF_SECOND_COUNTS - 1U;
```

The timer count constant is:

```c
#define TIMER_A_HALF_SECOND_COUNTS (46875U)
```

The timer period contains approximately 46,875 timer ticks.

Because the timer count includes zero, the value written to `CCR0` is:

```text
46,875 - 1
=
46,874
```

A smaller example makes this easier to visualize.

If a period requires 5 timer counts:

```text
0
1
2
3
4
```

there are already five count states.

Therefore:

```text
5 counts → CCR0 = 4
```

The same idea gives:

```text
46,875 counts → CCR0 = 46,874
```

---

### 4.7 CCR0 Compare Flag

Timer_A capture/compare channels contain control registers.

For `CCR0`, the corresponding control register is:

```c
TIMER_A0->CCTL[0]
```

When Timer_A0 reaches the `CCR0` compare event, the `CCIFG` flag is set.

The flag can be checked using:

```c
TIMER_A0->CCTL[0] & TIMER_A_CCTLN_CCIFG
```

Before starting the timer, any existing flag is cleared:

```c
TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
```

This prevents an old pending flag from being interpreted as a new timer event.

The important distinction is:

```text
CCR0  → At what timer value should the event occur?

CCIFG → Has that compare event occurred?
```

---

### 4.8 Poll the CCR0 Compare Flag

This example does not use Timer_A interrupts.

Instead, the main loop continuously checks `CCIFG`:

```c
if ((TIMER_A0->CCTL[0] & TIMER_A_CCTLN_CCIFG) != 0U)
```

Conceptually, the program repeatedly asks:

```text
"Has Timer_A0 reached CCR0?"
```

If the flag is not set, the loop continues polling.

When the flag becomes set:

```c
TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
```

clears the flag.

The LED is then toggled:

```c
P1->OUT ^= BIT0;
```

The complete process is:

```text
Timer_A0 starts counting
        ↓
0 → 1 → 2 → ... → CCR0
        ↓
CCIFG becomes 1
        ↓
main() detects CCIFG
        ↓
Clear CCIFG
        ↓
Toggle P1.0
        ↓
Timer_A0 continues
        ↓
Repeat
```

Because the LED toggles approximately every 0.5 seconds:

```text
OFF ──0.5 s──> ON ──0.5 s──> OFF
```

one complete LED on/off cycle takes approximately 1 second.

---

### 4.9 Timer_A0 vs. SysTick

Timer_A0 and SysTick can both generate periodic timing events, but they are different hardware resources.

| SysTick | Timer_A |
| --- | --- |
| Part of the ARM Cortex-M4 core | MSP432 peripheral |
| 24-bit counter | 16-bit counter |
| Primarily a system time-base timer | General-purpose timing peripheral |
| Uses `LOAD` for its period | Can use `CCR0` to define an up-mode period |
| Uses `COUNTFLAG` when polling | Uses `CCIFG` for a compare event |
| Counts downward | Can operate in multiple counting modes |
| Limited signal-generation features | Supports capture/compare and hardware outputs |

For the polling examples, their basic operation looks similar:

```text
SysTick                         Timer_A0

LOAD                            CCR0
  ↓                               ↓
Timer runs                      Timer runs
  ↓                               ↓
Terminal event                  Compare event
  ↓                               ↓
COUNTFLAG                       CCIFG
  ↓                               ↓
main() polls                    main() polls
  ↓                               ↓
Toggle LED                      Toggle LED
```

The differences become more important when Timer_A capture/compare channels and hardware signal-generation features are used.

---

## 5. Expected Result

After programming the MSP432P401R:

1. The onboard red LED begins turned off.
2. Timer_A0 begins counting using the divided `SMCLK`.
3. Approximately every 0.5 seconds, Timer_A0 reaches its `CCR0` compare event.
4. `CCIFG` becomes set.
5. The main loop detects and clears the flag.
6. The onboard red LED toggles.

The result should appear approximately as:

```text
Time        LED

0.0 s       OFF
0.5 s       ON
1.0 s       OFF
1.5 s       ON
2.0 s       OFF
...
```

The LED therefore completes approximately one full blink cycle every second.

---

## 6. Register Summary

| Register / Setting | Purpose |
| --- | --- |
| `WDT_A->CTL` | Controls the watchdog timer |
| `P1->SEL0`, `P1->SEL1` | Configure P1.0 for GPIO |
| `P1->DIR` | Configures P1.0 as an output |
| `P1->OUT` | Controls the onboard red LED |
| `TIMER_A0->CTL` | Controls Timer_A0 clock source, divider, mode, and clear operation |
| `TIMER_A0->EX0` | Configures the expansion clock divider |
| `TIMER_A0->CCR[0]` | Defines the Timer_A0 up-mode period |
| `TIMER_A0->CCTL[0]` | Controls capture/compare channel 0 and contains `CCIFG` |
| `TIMER_A_CTL_SSEL__SMCLK` | Selects SMCLK as the timer clock source |
| `TIMER_A_CTL_ID__8` | Divides the timer input clock by 8 |
| `TIMER_A_EX0_IDEX__4` | Divides the timer clock by an additional factor of 4 |
| `TIMER_A_CTL_MC__UP` | Places Timer_A0 in up mode |
| `TIMER_A_CCTLN_CCIFG` | Indicates that the CCR0 compare event occurred |

---

## 7. Common Problems

### The LED does not blink

Verify that Timer_A0 is started in up mode:

```c
TIMER_A_CTL_MC__UP
```

Also verify that `SMCLK` is selected:

```c
TIMER_A_CTL_SSEL__SMCLK
```

and that the main loop is checking:

```c
TIMER_A_CCTLN_CCIFG
```

---

### The LED blinks at the wrong speed

Verify both clock dividers:

```c
TIMER_A_CTL_ID__8
```

and:

```c
TIMER_A_EX0_IDEX__4
```

The timer period depends on both the timer clock frequency and the `CCR0` value.

For this example:

```text
3 MHz ÷ 8 ÷ 4
      =
93.75 kHz
```

and approximately:

```text
46,875 counts / 93,750 counts per second
=
0.5 seconds
```

Changing either divider changes the relationship between timer counts and elapsed time.

---

### Why can I not simply use 1,500,000 counts like SysTick?

Timer_A is a 16-bit timer.

Its counter can represent values from:

```text
0 through 65,535
```

A value such as:

```text
1,500,000
```

does not fit.

The timer clock is therefore divided so fewer timer counts are required for the desired period.

---

### Can Timer_A generate a one-second period?

Yes, depending on the selected clock frequency and dividers.

With the configuration in this example:

```text
3 MHz ÷ 8 ÷ 4 = 93,750 Hz
```

one second would require approximately:

```text
93,750 counts
```

which exceeds the 16-bit range.

However, increasing the total clock division can reduce the number of counts required.

Another common approach is to generate a shorter hardware period and count multiple timer events in software.

For example:

```text
0.5-second Timer_A event
          ↓
Software event #1
          ↓
0.5-second Timer_A event
          ↓
Software event #2
          ↓
1 second has elapsed
```

This hardware-timer-plus-software-counter technique can be used to create much longer time intervals.

---

### Why is `CCIE` not enabled?

This example intentionally uses polling.

The program checks:

```c
CCIFG
```

inside the main loop rather than allowing Timer_A0 to interrupt the processor.

The next example enables the Timer_A0 `CCR0` interrupt and handles the timer event inside an interrupt service routine.

---

## 8. Next Example

The next example uses the same Timer_A0 timing concept but replaces polling with an interrupt.

Instead of:

```text
Timer_A0 reaches CCR0
        ↓
CCIFG set
        ↓
main() checks CCIFG
        ↓
Toggle LED
```

the next example will use:

```text
Timer_A0 reaches CCR0
        ↓
Timer_A0 interrupt request
        ↓
Timer_A0 ISR
        ↓
Toggle LED
```

This provides a direct comparison between **polling** and **interrupt-driven Timer_A operation**.