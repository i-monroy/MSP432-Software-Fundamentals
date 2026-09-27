# Timer_A LED Delay Interrupt 🦉

This example blinks the onboard red LED using periodic **Timer_A0 CCR0 interrupts**.

Timer_A0 operates in up mode and generates a compare interrupt approximately every 0.5 seconds. Instead of continuously polling the timer flag inside `main()`, the processor executes `TA0_0_IRQHandler()` whenever the Timer_A0 CCR0 interrupt occurs.

---

## 1. Overview

The previous Timer_A example used polling to determine when Timer_A0 reached its `CCR0` value.

This example performs the same basic timing operation using an interrupt.

The program:

1. Configures the onboard red LED as an output.
2. Configures Timer_A0 to use `SMCLK`.
3. Divides the timer clock to obtain a suitable counting frequency.
4. Places Timer_A0 in up mode.
5. Uses `CCR0` to create an approximately 0.5-second timer period.
6. Enables the Timer_A0 CCR0 interrupt.
7. Enables the Timer_A0 CCR0 interrupt in the NVIC.
8. Enables interrupts globally.
9. Toggles the LED inside `TA0_0_IRQHandler()`.

The basic execution flow is:

```text
Timer_A0 counts upward
        ↓
Timer_A0 reaches CCR0
        ↓
CCR0 interrupt request
        ↓
NVIC
        ↓
TA0_0_IRQHandler()
        ↓
Clear CCIFG
        ↓
Toggle LED
```

Unlike the polling example, `main()` does not continuously check the Timer_A0 compare flag.

---

## 2. Hardware Used

| Item | Description |
| --- | --- |
| Microcontroller Board | MSP432P401R LaunchPad |
| LED | Onboard red LED |
| LED Pin | `P1.0` |
| Timer | Timer_A0 |
| Interrupt | Timer_A0 CCR0 |
| IDE | Code Composer Studio 12.8.1 |

No external components are required.

---

## 3. Pinout / Wiring

The example uses the onboard red LED connected to `P1.0`.

| Component | MSP432P401R Pin | Purpose |
| --- | --- | --- |
| Onboard Red LED | `P1.0` | Timer interrupt indication |

Because the LED is already connected on the LaunchPad, no external wiring is required.

---

## 4. Code Walkthrough

### 4.1 Initialize the Red LED

The onboard red LED is connected to `P1.0`.

The pin is configured for GPIO operation:

```c
P1->SEL0 &= ~BIT0;
P1->SEL1 &= ~BIT0;
```

It is then configured as an output:

```c
P1->DIR |= BIT0;
```

The LED begins turned off:

```c
P1->OUT &= ~BIT0;
```

---

### 4.2 Timer_A Interrupt Operation

Timer_A0 is a hardware timer peripheral that can generate an interrupt when a configured timer event occurs.

This example uses Capture/Compare Register 0:

```text
CCR0
```

Timer_A0 counts upward until the `CCR0` compare event occurs.

With the CCR0 interrupt enabled, the event causes an interrupt request instead of requiring the program to continuously poll `CCIFG`.

```text
Timer_A0
   ↓
Counts upward
   ↓
Reaches CCR0
   ↓
CCR0 compare event
   ↓
Interrupt request
   ↓
TA0_0_IRQHandler()
```

This allows the main program to perform other work while the hardware timer operates independently.

---

### 4.3 Stop and Clear Timer_A0

Timer_A0 is stopped and cleared before configuration:

```c
TIMER_A0->CTL = TIMER_A_CTL_CLR;
```

`CLR` clears the Timer_A counter and divider logic.

This gives the timer a known starting state before its clock source, dividers, compare value, and operating mode are configured.

---

### 4.4 Timer Clock and Divider Configuration

This example assumes the default approximately 3 MHz `SMCLK`.

Timer_A0 uses two clock dividers.

The first divider is configured through the Timer_A control register:

```c
TIMER_A_CTL_ID__8
```

This divides `SMCLK` by 8.

The expansion divider is configured using:

```c
TIMER_A0->EX0 = TIMER_A_EX0_IDEX__4;
```

This divides the clock by an additional factor of 4.

The resulting Timer_A0 clock is approximately:

```text
3,000,000 Hz
    ÷ 8
    ÷ 4
────────────
93,750 Hz
```

Therefore, Timer_A0 increments approximately:

```text
93,750 times per second
```

The general relationship is:

```text
                    Source Clock
Timer Frequency = ──────────────────
                  ID Divider × IDEX
```

For this example:

```text
                   3,000,000
Timer Frequency = ───────────
                      8 × 4

                = 93,750 Hz
```

A 0.5-second period therefore requires approximately:

```text
93,750 × 0.5
     =
46,875 timer counts
```

Because Timer_A is a 16-bit timer, its counter is limited to values between:

```text
0 and 65,535
```

The clock dividers allow the desired 0.5-second period to fit within this range.

#### Changing the Timer Period

The timer period depends on both the timer frequency and the number of counts.

The relationship is:

```text
Timer Counts = Timer Frequency × Desired Time
```

and:

```text
Desired Time = Timer Counts / Timer Frequency
```

For example, if the expansion divider were changed to:

```c
TIMER_A_EX0_IDEX__8
```

the Timer_A0 clock would become:

```text
3,000,000 ÷ 8 ÷ 8
=
46,875 Hz
```

Using 46,875 timer counts would then produce approximately:

```text
46,875 counts
─────────────
46,875 Hz

= 1 second
```

Increasing the divider slows the timer clock, allowing longer periods to fit inside the 16-bit counter at the cost of lower timing resolution.

---

### 4.5 Configure CCR0 and Up Mode

The timer count constant is:

```c
#define TIMER_A_HALF_SECOND_COUNTS (46875U)
```

`CCR0` is configured using:

```c
TIMER_A0->CCR[0] = TIMER_A_HALF_SECOND_COUNTS - 1U;
```

Because the timer includes zero as one of its count states, approximately 46,875 timer ticks are represented by:

```text
CCR0 = 46,874
```

Timer_A0 is then started in up mode:

```c
TIMER_A0->CTL = (TIMER_A_CTL_SSEL__SMCLK |
                 TIMER_A_CTL_ID__8       |
                 TIMER_A_CTL_MC__UP);
```

The configuration selects:

```text
SSEL__SMCLK → Use SMCLK as the Timer_A0 clock source

ID__8       → Divide SMCLK by 8

MC__UP      → Operate Timer_A0 in up mode
```

In up mode, the timer repeatedly counts toward the `CCR0` value:

```text
0 → 1 → 2 → 3 → ... → CCR0
↑                         │
└─────────────────────────┘
          repeat
```

Each completed timer period produces a CCR0 compare event.

---

### 4.6 Enable the CCR0 Interrupt

The CCR0 interrupt is enabled using the `CCIE` bit:

```c
TIMER_A0->CCTL[0] |= TIMER_A_CCTLN_CCIE;
```

`CCIE` stands for **Capture/Compare Interrupt Enable**.

This allows the CCR0 compare event to generate an interrupt request.

Any existing compare flag is also cleared before the timer begins:

```c
TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
```

This prevents a previously pending flag from being treated as a new timer event.

The distinction between the two bits is:

```text
CCIE  → Should this compare event generate an interrupt?

CCIFG → Has the compare event occurred?
```

---

### 4.7 Enable the Interrupt in the NVIC

Enabling `CCIE` allows Timer_A0 to generate the interrupt request, but the corresponding interrupt must also be enabled in the ARM Nested Vectored Interrupt Controller (NVIC).

This is done using:

```c
NVIC_EnableIRQ(TA0_0_IRQn);
```

`TA0_0_IRQn` identifies the Timer_A0 CCR0 interrupt to the NVIC.

Global interrupts are then enabled:

```c
__enable_irq();
```

The interrupt path can therefore be visualized as:

```text
Timer_A0 reaches CCR0
        ↓
CCIE enabled?
        ↓
Timer_A0 requests interrupt
        ↓
TA0_0_IRQn enabled in NVIC?
        ↓
Global interrupts enabled?
        ↓
TA0_0_IRQHandler()
```

All of these conditions must allow the interrupt before the ISR can execute.

#### `TA0_0_IRQn` vs. `TA0_0_IRQHandler()`

These names serve different purposes.

```text
TA0_0_IRQn
    ↓
Interrupt number used to configure the NVIC


TA0_0_IRQHandler()
    ↓
Interrupt service routine executed by the processor
```

Therefore:

```c
NVIC_EnableIRQ(TA0_0_IRQn);
```

is correct.

The ISR itself is:

```c
void TA0_0_IRQHandler(void)
```

This is similar to GPIO port interrupts:

```text
PORT1_IRQn          → NVIC interrupt identifier
PORT1_IRQHandler()  → Interrupt service routine

TA0_0_IRQn          → NVIC interrupt identifier
TA0_0_IRQHandler()  → Interrupt service routine
```

---

### 4.8 Main Loop

After Timer_A0 and the interrupt system are configured, the main loop does not need to poll the timer:

```c
while (1)
{
    /* Timer_A0 controls the LED through interrupts. */
}
```

Timer_A0 continues operating independently in hardware.

Approximately every 0.5 seconds, the CCR0 compare event generates an interrupt and temporarily redirects program execution to:

```c
TA0_0_IRQHandler()
```

After the ISR finishes, execution returns to the code that was previously running.

In this example, that is simply the empty main loop.

---

### 4.9 Clear CCIFG Early in the ISR

The Timer_A0 CCR0 interrupt service routine is:

```c
void TA0_0_IRQHandler(void)
{
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

    P1->OUT ^= BIT0;
}
```

The order of these two operations is important on the MSP432.

The `CCIFG` flag is intentionally cleared **immediately after entering the ISR**:

```c
TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
```

The LED is toggled afterward:

```c
P1->OUT ^= BIT0;
```

#### Why Clear the Flag First?

Peripheral register writes are not necessarily observed everywhere in the processor immediately.

If the interrupt flag is cleared as the final instruction before leaving the ISR:

```c
void TA0_0_IRQHandler(void)
{
    P1->OUT ^= BIT0;

    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
}
```

the processor can return from the ISR before the cleared interrupt state has fully propagated.

The interrupt may then still appear pending long enough for the processor to enter the ISR again.

Conceptually:

```text
CCR0 interrupt
      ↓
Enter ISR
      ↓
Toggle LED ON
      ↓
Clear CCIFG
      ↓
Immediately exit ISR
      ↓
Interrupt still appears pending
      ↓
Enter ISR again
      ↓
Toggle LED OFF
```

The two LED toggles can occur extremely close together.

Instead of visibly remaining on for the expected timer period, the LED may appear very dim or behave unexpectedly.

Clearing `CCIFG` at the beginning of the ISR gives the peripheral interrupt-clear operation time to propagate while the remaining ISR instructions execute:

```text
CCR0 interrupt
      ↓
Enter ISR
      ↓
Clear CCIFG
      ↓
Toggle LED
      ↓
Exit ISR
      ↓
Interrupt clear has propagated
      ↓
Wait for next CCR0 event
```

For this reason, this example clears the interrupt flag immediately:

```c
void TA0_0_IRQHandler(void)
{
    /* Clear the CCR0 compare interrupt flag immediately. */
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

    /* Toggle the onboard red LED. */
    P1->OUT ^= BIT0;
}
```

This is an important reminder that register operations which appear nearly identical in C can have different effects because the microcontroller hardware continues operating independently of the processor.

---

### 4.10 Complete Program Execution Flow

After initialization, the program operates as follows:

```text
Start
  ↓
Stop watchdog timer
  ↓
Initialize P1.0
  ↓
Configure Timer_A0
  ↓
Configure CCR0
  ↓
Enable CCR0 interrupt
  ↓
Start Timer_A0
  ↓
Enable TA0_0_IRQn
  ↓
Enable global interrupts
  ↓
Enter main loop
  │
  │
  │       Timer_A0 counts independently
  │                 ↓
  │          Timer reaches CCR0
  │                 ↓
  │         Interrupt requested
  │                 ↓
  └──────→ TA0_0_IRQHandler()
                    ↓
              Clear CCIFG
                    ↓
                Toggle LED
                    ↓
               Return to main
```

The CPU no longer needs to repeatedly check whether the timer period has completed.

---

## 5. Expected Result

After programming the MSP432P401R:

1. The onboard red LED begins turned off.
2. Timer_A0 begins counting.
3. Approximately every 0.5 seconds, the CCR0 compare event occurs.
4. Timer_A0 generates an interrupt request.
5. `TA0_0_IRQHandler()` executes.
6. `CCIFG` is cleared.
7. The onboard red LED toggles.

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

The LED completes approximately one full on/off cycle every second.

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
| `TIMER_A0->CCTL[0]` | Controls capture/compare channel 0 |
| `TIMER_A_CTL_SSEL__SMCLK` | Selects SMCLK as the timer clock source |
| `TIMER_A_CTL_ID__8` | Divides the timer input clock by 8 |
| `TIMER_A_EX0_IDEX__4` | Divides the timer clock by an additional factor of 4 |
| `TIMER_A_CTL_MC__UP` | Places Timer_A0 in up mode |
| `TIMER_A_CCTLN_CCIE` | Enables the CCR0 compare interrupt |
| `TIMER_A_CCTLN_CCIFG` | Indicates a CCR0 compare event |
| `TA0_0_IRQn` | Timer_A0 CCR0 interrupt identifier used by the NVIC |

---

## 7. Common Problems

### The LED does not blink

Verify that the CCR0 interrupt is enabled:

```c
TIMER_A0->CCTL[0] |= TIMER_A_CCTLN_CCIE;
```

Verify that the interrupt is enabled in the NVIC:

```c
NVIC_EnableIRQ(TA0_0_IRQn);
```

and that global interrupts are enabled:

```c
__enable_irq();
```

Also verify that Timer_A0 is running in up mode:

```c
TIMER_A_CTL_MC__UP
```

---

### The LED is extremely dim or behaves unexpectedly

Check where `CCIFG` is cleared inside `TA0_0_IRQHandler()`.

Clear the flag immediately after entering the ISR:

```c
void TA0_0_IRQHandler(void)
{
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

    P1->OUT ^= BIT0;
}
```

Avoid placing the explicit flag clear as the final operation before returning from the ISR.

Clearing the flag early gives the interrupt-clear operation time to propagate before the processor exits the handler.

See Section 4.9 for a detailed explanation.

---

### The compiler says `TA0_0_IRQHandler` is undefined

Make sure the NVIC is enabled using the IRQ identifier:

```c
NVIC_EnableIRQ(TA0_0_IRQn);
```

not the ISR function name:

```c
NVIC_EnableIRQ(TA0_0_IRQHandler);
```

Remember:

```text
TA0_0_IRQn          → NVIC interrupt identifier

TA0_0_IRQHandler()  → Interrupt service routine
```

---

### The LED blinks at the wrong speed

Verify both Timer_A clock dividers:

```c
TIMER_A_CTL_ID__8
```

and:

```c
TIMER_A_EX0_IDEX__4
```

For the configuration used in this example:

```text
3 MHz ÷ 8 ÷ 4
=
93.75 kHz
```

Approximately 46,875 timer counts therefore represent 0.5 seconds.

Changing the clock dividers changes the amount of time represented by the same `CCR0` value.

---

### Why is the main loop empty?

The timer no longer needs to be polled by the CPU.

Timer_A0 operates independently in hardware and generates an interrupt whenever the CCR0 event occurs.

The empty loop makes this behavior easy to see:

```c
while (1)
{
    /* Timer_A0 controls the LED through interrupts. */
}
```

A larger application could perform other work inside `main()` while Timer_A0 continues generating periodic interrupts.

---

## 8. Next Example

The next Timer_A example moves beyond using the timer only as a software time base.

Timer_A can use its capture/compare hardware to interact directly with output signals.

The next example will demonstrate how Timer_A can generate a periodic hardware signal that can be observed and measured externally.

This introduces an important difference between simply executing code periodically and allowing the timer peripheral itself to control signal timing.