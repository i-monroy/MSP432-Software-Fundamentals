# SysTick Interrupt LED Delay 🦬

This example blinks the onboard red LED using periodic SysTick interrupts.

Unlike the previous SysTick example, the main loop does not continuously check `COUNTFLAG`. Instead, SysTick generates an exception when the timer reaches zero, causing the processor to automatically execute `SysTick_Handler()`.

---

## 1. Overview

The SysTick timer is a 24-bit down-counter built into the ARM Cortex-M4 processor.

The previous example demonstrated SysTick using polling. The main loop continuously checked `COUNTFLAG` to determine whether the timer had reached zero.

This example uses the same SysTick timer differently.

SysTick is configured to generate an interrupt approximately every 0.5 seconds using the default 3 MHz processor clock. When the counter reaches zero, the processor automatically executes:

```c
void SysTick_Handler(void)
```

The interrupt service routine toggles the onboard red LED.

This example demonstrates:

- Periodic SysTick interrupts
- The SysTick `TICKINT` control bit
- The `SysTick_Handler()` interrupt service routine
- Global interrupt enable
- Interrupt-driven operation without polling
- Automatic SysTick reload
- How SysTick exceptions differ from peripheral interrupts
- Polling versus interrupt-driven SysTick operation

---

## 2. Hardware Used

| Item | Description |
| --- | --- |
| Microcontroller Board | MSP432P401R LaunchPad |
| LED | Onboard red LED |
| LED Pin | `P1.0` |
| IDE | Code Composer Studio 12.8.1 |

No external components are required for this example.

---

## 3. Pinout / Wiring

The example uses the onboard red LED connected to `P1.0`.

| Component | MSP432P401R Pin | Configuration |
| --- | --- | --- |
| Onboard Red LED | `P1.0` | GPIO output |

Because the LED is built into the MSP432P401R LaunchPad, no external wiring is required.

---

## 4. Code Walkthrough

### 4.1 Initialize the Red LED

The onboard red LED is initialized using:

```c
LED_redLEDInit();
```

`P1.0` is configured for GPIO operation:

```c
P1->SEL0 &= ~BIT0;
P1->SEL1 &= ~BIT0;
```

The pin is then configured as an output:

```c
P1->DIR |= BIT0;
```

The LED begins turned off:

```c
P1->OUT &= ~BIT0;
```

The GPIO configuration is the same as the previous examples. The main difference in this project is how SysTick notifies the processor that the timer has expired.

---

### 4.2 Configure the SysTick Interval

This example uses approximately 0.5 seconds as the SysTick interval.

The program defines:

```c
#define SYSTICK_HALF_SECOND_COUNTS (1500000U)
```

Using the default processor clock of approximately 3 MHz:

```text
3 MHz = 3,000,000 clock cycles per second
```

Therefore:

```text
3,000,000 cycles/second × 0.5 seconds
= 1,500,000 cycles
```

The general relationship is:

```text
Timer Counts = Clock Frequency × Desired Time
```

For this example:

```text
Timer Counts = 3,000,000 Hz × 0.5 s

Timer Counts = 1,500,000
```

The LED therefore changes state approximately every 0.5 seconds.

---

### 4.3 Configure LOAD and VAL

SysTick is first disabled while the timer is configured:

```c
SysTick->CTRL = 0U;
```

The reload value is then configured:

```c
SysTick->LOAD = SYSTICK_HALF_SECOND_COUNTS - 1U;
```

For this example:

```text
LOAD = 1,500,000 - 1

LOAD = 1,499,999
```

The subtraction is necessary because the countdown includes zero:

```text
1,499,999
1,499,998
    ...
      2
      1
      0
```

This produces 1,500,000 timer counts.

The current counter is then cleared:

```c
SysTick->VAL = 0U;
```

Writing to `VAL` clears the current SysTick counter value and provides a clean starting state.

When SysTick begins operating, the configured `LOAD` value is used and the timer counts downward.

The basic relationship remains:

```text
LOAD → Where should the countdown reload from?

VAL  → Where is the countdown currently?

CTRL → How should SysTick operate?
```

---

### 4.4 Enable the SysTick Interrupt

After `LOAD` and `VAL` are configured, SysTick is enabled using:

```c
SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk |
                 SysTick_CTRL_TICKINT_Msk   |
                 SysTick_CTRL_ENABLE_Msk);
```

Three control bits are enabled.

#### CLKSOURCE

```c
SysTick_CTRL_CLKSOURCE_Msk
```

This selects the processor clock as the SysTick clock source.

#### TICKINT

```c
SysTick_CTRL_TICKINT_Msk
```

This enables the SysTick exception.

When SysTick reaches zero while `TICKINT` is enabled, the processor can execute `SysTick_Handler()`.

This is the primary difference from the previous polling example.

The polling example used:

```c
SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk |
                 SysTick_CTRL_ENABLE_Msk);
```

The interrupt example adds:

```c
SysTick_CTRL_TICKINT_Msk
```

The difference can be summarized as:

```text
Polling Example

CLKSOURCE = 1
TICKINT   = 0
ENABLE    = 1

SysTick reaches zero
        ↓
COUNTFLAG
        ↓
main() detects event
```

Compared with:

```text
Interrupt Example

CLKSOURCE = 1
TICKINT   = 1
ENABLE    = 1

SysTick reaches zero
        ↓
SysTick exception
        ↓
SysTick_Handler()
```

#### ENABLE

```c
SysTick_CTRL_ENABLE_Msk
```

This starts the SysTick counter.

The complete configuration can therefore be remembered as:

```text
CLKSOURCE → Use the processor clock

TICKINT   → Generate a SysTick exception when
            the timer reaches zero

ENABLE    → Start the SysTick counter
```

---

### 4.5 Enable Interrupts Globally

After SysTick is configured, interrupts are enabled globally:

```c
__enable_irq();
```

This allows the processor to respond to enabled interrupts and exceptions.

Unlike the previous polling example, this program depends on interrupt-driven execution. The SysTick exception must therefore be allowed to interrupt normal program execution.

The interrupt path can be viewed as:

```text
SysTick enabled
      ↓
Counter reaches zero
      ↓
TICKINT enabled
      ↓
SysTick exception becomes pending
      ↓
Global interrupt state allows execution
      ↓
SysTick_Handler()
```

---

### 4.6 Main Loop Without Polling

The main loop contains no SysTick polling:

```c
while (1)
{
    /*
     * No delay or LED polling is required here.
     * The LED is controlled by the SysTick interrupt.
     */
}
```

The previous SysTick example required:

```c
if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U)
```

inside the main loop.

That is no longer necessary.

Instead, the processor can continue executing the main program while SysTick operates independently.

When the timer reaches zero, the processor temporarily leaves its normal execution and runs `SysTick_Handler()`.

In this simple example, the main loop has no other work to perform. In a larger embedded application, the main loop could perform other tasks while SysTick provides periodic timing in the background.

---

### 4.7 SysTick_Handler()

The SysTick interrupt service routine is:

```c
void SysTick_Handler(void)
{
    P1->OUT ^= BIT0;
}
```

`SysTick_Handler()` is not called manually by the program.

When SysTick reaches zero and its interrupt is enabled, the processor automatically executes this handler.

The statement:

```c
P1->OUT ^= BIT0;
```

toggles the onboard red LED.

Therefore, every SysTick expiration changes the LED state:

```text
Timer expires → LED OFF → ON

Timer expires → LED ON  → OFF

Timer expires → LED OFF → ON

...
```

After `SysTick_Handler()` finishes, the processor returns to the code that was executing before the exception occurred.

---

### 4.8 SysTick Interrupt Execution Flow

The complete interrupt-driven process can be visualized as:

```text
          main()
            │
            │
            │        SysTick counts down
            │                ↓
            │             VAL = 0
            │                ↓
            │        SysTick exception
            │                ↓
            ├──────► SysTick_Handler()
            │                ↓
            │          Toggle P1.0
            │                ↓
            │        Return from handler
            ◄────────────────┘
            │
            │
            │        SysTick automatically
            │        begins another period
            │
            ▼
         Continue
```

SysTick automatically reloads its configured value and continues counting as long as the timer remains enabled.

The application does not need to manually restart the timer after each interrupt.

---

### 4.9 Polling vs. Interrupt-Driven SysTick

The previous SysTick example and this example use the same hardware timer and approximately the same timing interval.

The main difference is how the processor responds when SysTick reaches zero.

#### Polling

```text
SysTick reaches zero
        ↓
COUNTFLAG set
        ↓
main() reads CTRL
        ↓
COUNTFLAG detected
        ↓
Toggle LED
```

The processor must repeatedly check whether the timer has expired.

#### Interrupt

```text
SysTick reaches zero
        ↓
SysTick exception
        ↓
SysTick_Handler()
        ↓
Toggle LED
```

The processor does not need to continuously check the timer.

This makes interrupt-driven timing useful when the processor needs to perform other work between periodic timer events.

A simple way to remember the difference is:

```text
Polling:

"Did the timer finish?"
"Did the timer finish?"
"Did the timer finish?"
"Did the timer finish?"
        ↓
       Yes


Interrupt:

"Tell me when the timer finishes."
        ↓
Continue other work
        ↓
SysTick_Handler()
```

---

### 4.10 Why NVIC_EnableIRQ() Is Not Required

The previous Port Interrupt examples used functions such as:

```c
NVIC_EnableIRQ(PORT4_IRQn);
```

This was required because Port 4 is a microcontroller peripheral interrupt connected to the Cortex-M interrupt system as an external IRQ.

SysTick is different.

SysTick is part of the ARM Cortex-M4 processor core and generates a core exception rather than a normal external peripheral IRQ.

Therefore, this example does not require:

```c
NVIC_EnableIRQ(...);
```

The SysTick exception is enabled through the `TICKINT` bit:

```c
SysTick_CTRL_TICKINT_Msk
```

The difference can be visualized as:

```text
Port Interrupt

GPIO pin
   ↓
Port peripheral
   ↓
Interrupt request
   ↓
NVIC external IRQ
   ↓
PORT4_IRQHandler()
```

Compared with:

```text
SysTick

Cortex-M SysTick
      ↓
Counter reaches zero
      ↓
SysTick exception
      ↓
SysTick_Handler()
```

This is why no `SysTick_IRQn` needs to be enabled using `NVIC_EnableIRQ()` in this example.

---

### 4.11 Why No Interrupt Flag Is Manually Cleared

The Port Interrupt examples also required the program to clear GPIO interrupt flags manually:

```c
P4->IFG &= ~BIT1;
```

This was necessary because the GPIO peripheral maintains interrupt flags in its `IFG` register.

SysTick does not use a GPIO-style `IFG` register.

When the SysTick exception is accepted and `SysTick_Handler()` executes, the exception is handled through the Cortex-M exception mechanism. There is no peripheral interrupt flag that must be manually cleared inside the handler.

Therefore, the SysTick ISR can remain:

```c
void SysTick_Handler(void)
{
    P1->OUT ^= BIT0;
}
```

There is no additional statement such as:

```text
Clear SysTick IFG
```

because no such peripheral `IFG` operation is required.

This is an important difference between the two interrupt sources:

```text
Port Interrupt                     SysTick

External peripheral event          Core timer event
        ↓                                  ↓
Port IFG set                       SysTick exception
        ↓                                  ↓
PORTx_IRQHandler()                 SysTick_Handler()
        ↓                                  ↓
Clear IFG manually                 No peripheral IFG to clear
```

SysTick does have `COUNTFLAG` in its `CTRL` register, but `COUNTFLAG` is a status indication and is not a GPIO-style interrupt flag that must be cleared by `SysTick_Handler()`.

---

## 5. Expected Result

After programming the MSP432P401R LaunchPad:

1. The onboard red LED begins turned off.
2. SysTick begins counting downward.
3. After approximately 0.5 seconds, SysTick reaches zero.
4. A SysTick exception is generated.
5. The processor automatically executes `SysTick_Handler()`.
6. The onboard red LED toggles on.
7. SysTick begins another timing period.
8. After approximately another 0.5 seconds, the LED toggles off.
9. The process repeats continuously.

The approximate LED behavior is:

```text
0.0 s    LED OFF
0.5 s    LED ON
1.0 s    LED OFF
1.5 s    LED ON
2.0 s    LED OFF
...
```

The LED changes state approximately every 0.5 seconds and completes one full ON/OFF cycle approximately every second.

---

## 6. Register Summary

| Register / Function | Purpose |
| --- | --- |
| `WDT_A->CTL` | Controls the watchdog timer |
| `P1->SEL0`, `P1->SEL1` | Select GPIO operation for P1.0 |
| `P1->DIR` | Configures P1.0 as an output |
| `P1->OUT` | Controls the onboard red LED |
| `SysTick->LOAD` | Stores the SysTick reload value |
| `SysTick->VAL` | Contains the current SysTick counter value |
| `SysTick->CTRL` | Controls and reports SysTick operation |
| `SysTick_CTRL_CLKSOURCE_Msk` | Selects the processor clock |
| `SysTick_CTRL_TICKINT_Msk` | Enables the SysTick exception |
| `SysTick_CTRL_ENABLE_Msk` | Enables the SysTick counter |
| `__enable_irq()` | Enables interrupts globally |
| `SysTick_Handler()` | Handles SysTick exceptions |

---

## 7. Common Problems

### The LED does not blink

Verify that all three required SysTick control bits are enabled:

```c
SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk |
                 SysTick_CTRL_TICKINT_Msk   |
                 SysTick_CTRL_ENABLE_Msk);
```

Also verify that global interrupts are enabled:

```c
__enable_irq();
```

and that `P1.0` is configured as a GPIO output.

### The LED blinks at the wrong speed

The timer interval depends on the clock frequency driving SysTick.

This example assumes the default processor clock of approximately 3 MHz and uses:

```c
#define SYSTICK_HALF_SECOND_COUNTS (1500000U)
```

If the processor clock is changed, the SysTick reload calculation must also be changed.

The general relationship is:

```text
Timer Counts = Clock Frequency × Desired Time
```

### SysTick_Handler() never executes

Verify that:

```c
SysTick_CTRL_TICKINT_Msk
```

is included in the SysTick `CTRL` configuration.

Also verify that global interrupts are enabled using:

```c
__enable_irq();
```

and that the handler is named exactly:

```c
void SysTick_Handler(void)
```

### Where is NVIC_EnableIRQ()?

SysTick is a Cortex-M core exception rather than a normal external peripheral IRQ.

It therefore does not require:

```c
NVIC_EnableIRQ(...);
```

See Section 4.10 for more information.

### Where is the interrupt flag cleared?

SysTick does not require a GPIO-style interrupt flag to be manually cleared inside `SysTick_Handler()`.

See Section 4.11 for more information.

---

## 8. Next Example

The next example applies SysTick timing to a larger application.

Instead of controlling only one blinking LED, SysTick will be used to control the timing of a binary counter.

This demonstrates how the same timer concepts can be reused as part of a more complete embedded application.