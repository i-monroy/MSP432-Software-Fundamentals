# SysTick LED Delay 🐿️

This example blinks the onboard red LED using the ARM Cortex-M4 SysTick timer without interrupts.

Instead of using `__delay_cycles()` to create a software delay, the program configures the SysTick hardware timer and continuously checks its `COUNTFLAG` to determine when the timer has expired.

---

## 1. Overview

The SysTick timer is a 24-bit down-counter built into the ARM Cortex-M4 processor.

Unlike the previous GPIO examples that used `__delay_cycles()` for timing, this example uses a hardware timer to generate a periodic event.

SysTick is configured to count approximately 0.5 seconds using the default 3 MHz processor clock. When the timer reaches zero, the `COUNTFLAG` bit is set and the timer automatically reloads its configured value.

The main loop continuously checks `COUNTFLAG`. Each time the timer expires, the onboard red LED is toggled.

This example demonstrates:

- The ARM Cortex-M4 SysTick timer
- Hardware timer-based delays
- The 24-bit SysTick down-counter
- Calculating a timer interval
- The SysTick `LOAD`, `VAL`, and `CTRL` registers
- Polling the SysTick `COUNTFLAG`
- Automatic timer reload
- SysTick operation without interrupts

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

The GPIO configuration is the same as the previous GPIO examples. The main difference in this project is how the timing between LED state changes is generated.

---

### 4.2 Understanding the SysTick Timer

SysTick is a 24-bit timer built into the ARM Cortex-M4 processor.

Unlike a timer that counts upward, SysTick operates as a down-counter.

Its basic operation can be visualized as:

```text
Reload Value
     ↓
1,499,999
1,499,998
1,499,997
    ...
      2
      1
      0
      ↓
Timer expires
      ↓
Reload Value
     ↓
1,499,999
    ...
```

Three registers are especially important when configuring SysTick:

| Register | Purpose |
| --- | --- |
| `LOAD` | Stores the value that SysTick reloads when beginning another countdown |
| `VAL` | Contains the current counter value |
| `CTRL` | Controls and reports the operating state of SysTick |

The relationship can be remembered as:

```text
LOAD → Where should the countdown reload from?

VAL  → Where is the countdown currently?

CTRL → How should SysTick operate?
```

This example configures SysTick to run continuously without generating interrupts.

---

### 4.3 Calculate the SysTick Interval

The duration of a SysTick countdown depends on two things:

1. The clock frequency driving SysTick.
2. The number of timer counts.

This example uses the processor clock as the SysTick clock source.

Using the default processor clock of approximately 3 MHz:

```text
3 MHz = 3,000,000 clock cycles per second
```

Therefore, approximately half a second requires:

```text
3,000,000 cycles/second × 0.5 seconds
= 1,500,000 cycles
```

The program defines this value using:

```c
#define SYSTICK_HALF_SECOND_COUNTS (1500000U)
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

Because the processor clock is not explicitly reconfigured in this example, the timing is described as approximately 0.5 seconds.

---

### 4.4 Configure the Reload Value (LOAD)

The SysTick reload value is configured using:

```c
SysTick->LOAD = SYSTICK_HALF_SECOND_COUNTS - 1U;
```

For this example:

```text
SYSTICK_HALF_SECOND_COUNTS = 1,500,000

LOAD = 1,500,000 - 1

LOAD = 1,499,999
```

The subtraction is necessary because the countdown includes zero.

For example, a five-count timer would use:

```text
LOAD = 4
```

and count:

```text
4
3
2
1
0
```

This represents five timer counts.

Therefore, loading `1,499,999` produces 1,500,000 timer counts.

When SysTick reaches zero, the configured `LOAD` value is used again for the next countdown.

---

### 4.5 Clear the Current Counter (VAL)

Before starting SysTick, the current counter value is cleared:

```c
SysTick->VAL = 0U;
```

Writing to `VAL` clears the current SysTick counter value.

This does not mean that SysTick will count upward from zero.

Instead, it provides a clean starting state. When SysTick begins operating, the timer loads the configured value from `LOAD` and starts counting downward.

Conceptually:

```text
Write to VAL
     ↓
Current count cleared
     ↓
SysTick begins operating
     ↓
Reload from LOAD
     ↓
1,499,999
     ↓
Count downward
```

This ensures that the countdown begins using the reload value configured by the program.

---

### 4.6 Configure and Enable SysTick (CTRL)

SysTick is first disabled while its registers are configured:

```c
SysTick->CTRL = 0U;
```

This allows the reload and current counter values to be configured before the timer begins running.

After `LOAD` and `VAL` are configured, SysTick is started using:

```c
SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk |
                 SysTick_CTRL_ENABLE_Msk);
```

Two control bits are enabled in this example.

#### CLKSOURCE

```c
SysTick_CTRL_CLKSOURCE_Msk
```

This selects the processor clock as the SysTick clock source.

The processor clock frequency therefore determines how quickly the SysTick counter decreases.

#### ENABLE

```c
SysTick_CTRL_ENABLE_Msk
```

This enables the SysTick counter.

Once enabled, SysTick begins counting downward and continues operating automatically.

#### TICKINT

The SysTick control register also contains an interrupt enable bit represented by:

```c
SysTick_CTRL_TICKINT_Msk
```

This bit is intentionally **not enabled** in this example.

Therefore:

```text
CLKSOURCE = 1 → Use the processor clock
TICKINT   = 0 → Do not generate a SysTick interrupt
ENABLE    = 1 → Enable the SysTick counter
```

Instead of using an interrupt, the program determines when SysTick expires by polling `COUNTFLAG`.

---

### 4.7 Poll the COUNTFLAG

The main loop continuously checks the SysTick `COUNTFLAG`:

```c
while (1)
{
    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U)
    {
        P1->OUT ^= BIT0;
    }
}
```

`COUNTFLAG` indicates that SysTick has counted down to zero since the flag was last observed.

The condition:

```c
(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U
```

isolates the `COUNTFLAG` bit from the `CTRL` register.

When the result is nonzero, SysTick has expired.

The LED is then toggled:

```c
P1->OUT ^= BIT0;
```

The polling process can be visualized as:

```text
             main()
               ↓
          Read CTRL
               ↓
       Is COUNTFLAG set?
          /           \
        No             Yes
        ↓               ↓
   Check again      Toggle LED
        \               /
         └───────┬─────┘
                 ↓
               Repeat
```

Reading the SysTick `CTRL` register clears `COUNTFLAG`.

Therefore, after the program detects one timer expiration, `COUNTFLAG` can be set again when SysTick reaches zero during a future countdown.

A useful way to think about `COUNTFLAG` is:

```text
COUNTFLAG → "Has SysTick reached zero since I last checked?"
```

Unlike the GPIO interrupt flags used in the Port Interrupt examples, this program does not manually clear `COUNTFLAG` using a separate register operation.

---

### 4.8 Automatic Reload and Continuous Operation

SysTick does not stop after reaching zero.

As long as SysTick remains enabled, the timer automatically reloads and continues counting.

The complete process is:

```text
LOAD = 1,499,999
        ↓
    Countdown
        ↓
        0
        ↓
 COUNTFLAG set
        ↓
Automatic reload
        ↓
LOAD = 1,499,999
        ↓
    Countdown
        ↓
        0
        ↓
 COUNTFLAG set
        ↓
       ...
```

Meanwhile, the main loop continuously checks `COUNTFLAG`.

This means the application does not need to manually restart SysTick after every timer expiration.

The relationship between the hardware and software is:

```text
          SysTick Hardware                 main()

          Count downward
                ↓
           Reach zero
                ↓
          COUNTFLAG = 1 ───────────────► Check COUNTFLAG
                ↓                              ↓
          Reload LOAD                     Toggle LED
                ↓                              ↓
          Count downward                  Continue polling
                ↓
               ...
```

This is an example of **polling** because the processor repeatedly checks the timer status to determine whether an event occurred.

The next SysTick example will use an interrupt instead. Rather than having `main()` repeatedly check `COUNTFLAG`, SysTick will automatically cause the processor to execute `SysTick_Handler()` when the timer expires.

---

## 5. Expected Result

After programming the MSP432P401R LaunchPad:

1. The onboard red LED begins turned off.
2. SysTick begins counting downward.
3. After approximately 0.5 seconds, SysTick reaches zero.
4. `COUNTFLAG` is detected by the main loop.
5. The onboard red LED toggles on.
6. SysTick automatically reloads and begins another countdown.
7. After approximately another 0.5 seconds, the LED toggles off.
8. The process repeats continuously.

The result is approximately:

```text
0.0 s    LED OFF
0.5 s    LED ON
1.0 s    LED OFF
1.5 s    LED ON
2.0 s    LED OFF
...
```

Therefore, the LED changes state approximately every 0.5 seconds and completes one full ON/OFF cycle approximately every second.

---

## 6. Register Summary

| Register / Bit | Purpose |
| --- | --- |
| `WDT_A->CTL` | Controls the watchdog timer |
| `P1->SEL0`, `P1->SEL1` | Select GPIO operation for P1.0 |
| `P1->DIR` | Configures P1.0 as an output |
| `P1->OUT` | Controls the onboard red LED |
| `SysTick->LOAD` | Stores the SysTick reload value |
| `SysTick->VAL` | Contains the current SysTick counter value |
| `SysTick->CTRL` | Controls and reports SysTick operation |
| `SysTick_CTRL_CLKSOURCE_Msk` | Selects the processor clock |
| `SysTick_CTRL_ENABLE_Msk` | Enables the SysTick counter |
| `SysTick_CTRL_COUNTFLAG_Msk` | Indicates that SysTick reached zero |

---

## 7. Common Problems

### The LED does not blink

Verify that SysTick is enabled:

```c
SysTick_CTRL_ENABLE_Msk
```

and that the processor clock is selected:

```c
SysTick_CTRL_CLKSOURCE_Msk
```

Also verify that `P1.0` is configured as a GPIO output.

### The LED blinks at the wrong speed

The timer interval depends on the clock frequency driving SysTick.

This example assumes the default processor clock of approximately 3 MHz and uses:

```c
#define SYSTICK_HALF_SECOND_COUNTS (1500000U)
```

If the processor clock is changed elsewhere in the program, the reload calculation must also be changed.

The general relationship is:

```text
Timer Counts = Clock Frequency × Desired Time
```

### COUNTFLAG never appears to remain set

`COUNTFLAG` is cleared when the SysTick `CTRL` register is read.

The program therefore checks the flag directly while reading `CTRL`:

```c
if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U)
```

SysTick can set `COUNTFLAG` again the next time the counter reaches zero.

### SysTick generates an interrupt

This example intentionally operates without SysTick interrupts.

Make sure `SysTick_CTRL_TICKINT_Msk` is not included when configuring `CTRL`:

```c
SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk |
                 SysTick_CTRL_ENABLE_Msk);
```

The next example enables `TICKINT` and demonstrates `SysTick_Handler()`.

---

## 8. Next Example

The next example introduces interrupt-driven SysTick operation.

Instead of continuously checking:

```c
SysTick_CTRL_COUNTFLAG_Msk
```

the program will enable the SysTick interrupt using:

```c
SysTick_CTRL_TICKINT_Msk
```

When SysTick reaches zero, the processor will automatically execute:

```c
void SysTick_Handler(void)
```

This provides a direct comparison between polling a hardware timer and responding to the same timer using an interrupt.