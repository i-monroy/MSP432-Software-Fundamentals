# SysTick 4-Bit Binary Counter 🦭

This example uses the SysTick timer and GPIO interrupts to control a 4-bit binary counter with automatic and manual counting modes.

Four external LEDs display values from `0` to `15`, while push buttons allow the user to increment, decrement, or switch between counting modes.

---

## 1. Overview

This project combines the SysTick timer with concepts introduced in the previous GPIO and interrupt examples.

The counter operates in two modes:

- **Automatic Mode** - SysTick increments the counter approximately every 0.5 seconds.
- **Manual Mode** - The onboard buttons increment or decrement the counter.

An external button connected to `P4.4` switches between automatic and manual operation.

Two onboard LEDs indicate the current counting mode:

| LED | Mode |
| --- | --- |
| Green `P2.1` | Automatic |
| Red `P1.0` | Manual |

The current counter value is displayed using four external LEDs connected to `P4.0` through `P4.3`.

This example demonstrates:

- Using SysTick as a periodic application timer
- Combining SysTick and GPIO interrupts
- Automatic and manual operating modes
- Using an `enum` to represent application states
- Using software flags to control button input
- Timer-based button input lockout
- Keeping interrupt service routines short
- Wrapping a 4-bit counter between `0` and `15`
- Displaying a binary value directly through GPIO

---

## 2. Hardware Used

| Item | Description |
| --- | --- |
| Microcontroller Board | MSP432P401R LaunchPad |
| Mode LEDs | Onboard red and green LEDs |
| Binary Display | 4 external LEDs |
| Buttons | 2 onboard buttons and 1 external button |
| IDE | Code Composer Studio 12.8.1 |

The external LEDs should each use an appropriate current-limiting resistor.

---

## 3. Pinout / Wiring

### LEDs

| Component | MSP432P401R Pin | Purpose |
| --- | --- | --- |
| Onboard Red LED | `P1.0` | Manual mode indicator |
| Onboard Green LED | `P2.1` | Automatic mode indicator |
| External LED 0 | `P4.0` | Binary bit 0 (LSB) |
| External LED 1 | `P4.1` | Binary bit 1 |
| External LED 2 | `P4.2` | Binary bit 2 |
| External LED 3 | `P4.3` | Binary bit 3 (MSB) |

The four external LEDs represent the binary counter:

```text
P4.3    P4.2    P4.1    P4.0
 MSB                      LSB
  ↓        ↓       ↓       ↓
Bit 3    Bit 2   Bit 1   Bit 0
```

For example:

```text
Decimal     P4.3 P4.2 P4.1 P4.0

   0          0    0    0    0
   1          0    0    0    1
   2          0    0    1    0
   3          0    0    1    1
   4          0    1    0    0
   ...
  15          1    1    1    1
```

### Buttons

| Button | MSP432P401R Pin | Purpose |
| --- | --- | --- |
| Onboard Button 1 | `P1.1` | Increment counter |
| Onboard Button 2 | `P1.4` | Decrement counter |
| External Button | `P4.4` | Switch counting mode |

All three buttons are configured as active-low inputs using pull-up resistors.

The external button should therefore connect `P4.4` to ground when pressed:

```text
P4.4 ---- Button ---- GND
```

Because the internal pull-up resistor is enabled:

```text
Released = HIGH
Pressed  = LOW
```

---

## 4. Code Walkthrough

### 4.1 Initialize the LEDs

Two groups of LEDs are used in this project.

The onboard LEDs indicate the current counting mode:

```c
P1.0 - Red LED
P2.1 - Green LED
```

They are configured as GPIO outputs:

```c
P1->SEL0 &= ~BIT0;
P1->SEL1 &= ~BIT0;

P2->SEL0 &= ~BIT1;
P2->SEL1 &= ~BIT1;

P1->DIR |= BIT0;
P2->DIR |= BIT1;
```

Both LEDs begin turned off:

```c
P1->OUT &= ~BIT0;
P2->OUT &= ~BIT1;
```

The four external LEDs are configured on `P4.0` through `P4.3`:

```c
P4->SEL0 &= ~(BIT0 | BIT1 | BIT2 | BIT3);
P4->SEL1 &= ~(BIT0 | BIT1 | BIT2 | BIT3);

P4->DIR |= (BIT0 | BIT1 | BIT2 | BIT3);
```

They begin at binary `0000`:

```c
P4->OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);
```

---

### 4.2 Automatic and Manual Counting Modes

The program supports two operating modes:

```c
typedef enum
{
    AUTOMATIC,
    MANUAL
} counting_state_t;
```

An `enum` gives meaningful names to the possible application states instead of representing them using unexplained numeric values.

The current mode is stored in:

```c
volatile counting_state_t counting_state = AUTOMATIC;
```

The program therefore begins in automatic mode.

The green onboard LED is turned on during initialization:

```c
P2->OUT |= BIT1;
```

This gives the user a visual indication that automatic counting is active.

The modes behave differently:

```text
AUTOMATIC MODE

SysTick
   ↓
Increment counter
   ↓
Update binary LEDs


MANUAL MODE

P1.1 / P1.4
     ↓
Port 1 interrupt
     ↓
Increment / decrement counter
     ↓
Next SysTick
     ↓
Update binary LEDs
```

The red and green LEDs indicate which mode is currently selected:

```text
Automatic → Green ON, Red OFF

Manual    → Red ON, Green OFF
```

---

### 4.3 Interrupt-Driven Program Structure

After initialization, the main loop does not poll the buttons or the SysTick timer:

```c
while (1)
{
    /*
     * No polling is required in the main loop.
     * Counter operation is controlled by interrupts.
     */
}
```

Instead, three interrupt handlers control the application:

```c
PORT1_IRQHandler()
PORT4_IRQHandler()
SysTick_Handler()
```

Each handler has a different responsibility:

| Handler | Responsibility |
| --- | --- |
| `PORT1_IRQHandler()` | Manual increment and decrement |
| `PORT4_IRQHandler()` | Switch between automatic and manual modes |
| `SysTick_Handler()` | Automatic counting, button lockout timing, range checking, and binary display updates |

The general program structure is therefore:

```text
                   main()
                     ↓
               Initialization
                     ↓
                 while (1)
                     │
           ┌─────────┼─────────┐
           │         │         │
         P1 IRQ    P4 IRQ    SysTick
           │         │         │
       Manual      Change    Periodic
       counter      mode      control
```

The processor spends most of its normal execution inside the empty main loop and responds when one of these events occurs.

---

### 4.4 Configure the Button Interrupts

All three buttons use internal pull-up resistors:

```c
P1->REN |= (BIT1 | BIT4);
P4->REN |= BIT4;

P1->OUT |= (BIT1 | BIT4);
P4->OUT |= BIT4;
```

Therefore:

```text
Released = HIGH
Pressed  = LOW
```

A button press produces a falling edge:

```text
HIGH → LOW
```

The interrupt edge selection registers are therefore configured using:

```c
P1->IES |= (BIT1 | BIT4);
P4->IES |= BIT4;
```

For these pins:

```text
IES = 1 → HIGH-to-LOW transition
```

Stale interrupt flags are cleared before enabling the interrupts:

```c
P1->IFG &= ~(BIT1 | BIT4);
P4->IFG &= ~BIT4;
```

The button interrupts are then enabled:

```c
P1->IE |= (BIT1 | BIT4);
P4->IE |= BIT4;
```

Because Port 1 and Port 4 are peripheral interrupt sources, they must also be enabled in the Nested Vectored Interrupt Controller:

```c
NVIC_EnableIRQ(PORT1_IRQn);
NVIC_EnableIRQ(PORT4_IRQn);
```

Finally, interrupts are enabled globally:

```c
__enable_irq();
```

This differs from the SysTick exception itself, which does not require a separate `NVIC_EnableIRQ()` call.

---

### 4.5 Configure the SysTick Timer

SysTick provides the periodic timing for the application.

The timer is configured for approximately 0.5 seconds using the default 3 MHz processor clock:

```c
#define SYSTICK_HALF_SECOND_COUNTS (1500000U)
```

The relationship is:

```text
Timer Counts = Clock Frequency × Desired Time
```

Therefore:

```text
3,000,000 Hz × 0.5 s
= 1,500,000 counts
```

SysTick is disabled before configuration:

```c
SysTick->CTRL = 0U;
```

The reload value is then configured:

```c
SysTick->LOAD = SYSTICK_HALF_SECOND_COUNTS - 1U;
```

The current counter is cleared:

```c
SysTick->VAL = 0U;
```

Finally, SysTick is started with the processor clock and its interrupt enabled:

```c
SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk |
                 SysTick_CTRL_TICKINT_Msk   |
                 SysTick_CTRL_ENABLE_Msk);
```

The three control bits provide:

```text
CLKSOURCE → Use the processor clock

TICKINT   → Generate a SysTick exception when
            the counter reaches zero

ENABLE    → Start the SysTick counter
```

When SysTick reaches zero, the processor automatically executes:

```c
SysTick_Handler()
```

No `NVIC_EnableIRQ()` call or peripheral `IFG` clearing is required for the SysTick exception.

---

### 4.6 Timer Flags and Button Input Lockout

This project introduces software flags used to control when button actions are accepted:

```c
#define MANUAL_COUNTER_BUTTONS_ENABLE (0x01U)
#define CHANGE_STATE_BUTTON_ENABLE    (0x02U)
```

The flags are stored together inside:

```c
volatile uint8_t timer_flags = 0U;
```

Each bit has a separate purpose:

```text
timer_flags

Bit 1                         Bit 0
  ↓                             ↓
CHANGE_STATE_BUTTON_ENABLE    MANUAL_COUNTER_BUTTONS_ENABLE
```

The flags can therefore be controlled independently using bitwise operations.

For example:

```c
timer_flags |= MANUAL_COUNTER_BUTTONS_ENABLE;
```

sets the manual-button enable flag.

The program can check it using:

```c
(timer_flags & MANUAL_COUNTER_BUTTONS_ENABLE) != 0U
```

and clear it using:

```c
timer_flags &= ~MANUAL_COUNTER_BUTTONS_ENABLE;
```

These are **software flags created by the program**.

They are different from hardware interrupt flags such as:

```c
P1->IFG
P4->IFG
```

The hardware `IFG` bits indicate that a GPIO interrupt event occurred.

The software flags instead answer questions such as:

```text
MANUAL_COUNTER_BUTTONS_ENABLE

"Should I accept an increment or decrement button press right now?"


CHANGE_STATE_BUTTON_ENABLE

"Should I accept a mode-change button press right now?"
```

When an accepted button press occurs, its software enable flag is cleared.

SysTick later sets the flag again.

The process is:

```text
SysTick
   ↓
Enable button action
   ↓
Button pressed
   ↓
Perform action
   ↓
Disable button action
   ↓
Ignore additional actions
   ↓
Next SysTick
   ↓
Enable button action again
```

This creates a simple timer-based input lockout.

Because SysTick occurs approximately every 0.5 seconds, this lockout is much longer than typical mechanical switch bounce. It is used here as a simple educational technique rather than a precise general-purpose debounce implementation.

---

### 4.7 Manual Increment and Decrement

The onboard buttons are handled by:

```c
void PORT1_IRQHandler(void)
```

The interrupt source is first determined using `IFG`.

For the increment button:

```c
if ((P1->IFG & BIT1) != 0U)
```

The button action is accepted only when:

1. The application is in manual mode.
2. SysTick has enabled the manual counter buttons.

```c
if ((counting_state == MANUAL) &&
    ((timer_flags & MANUAL_COUNTER_BUTTONS_ENABLE) != 0U))
```

If both conditions are true, the software flag is cleared:

```c
timer_flags &= ~MANUAL_COUNTER_BUTTONS_ENABLE;
```

and the counter is incremented:

```c
binary_counter++;
```

The P1.1 hardware interrupt flag is then cleared:

```c
P1->IFG &= ~BIT1;
```

The decrement button follows the same process:

```c
if ((P1->IFG & BIT4) != 0U)
```

followed by:

```c
binary_counter--;
```

and:

```c
P1->IFG &= ~BIT4;
```

The hardware interrupt flag is cleared regardless of whether the software accepts the button action.

This prevents a rejected button event from remaining pending after the ISR finishes.

The overall flow is:

```text
P1.1 pressed
     ↓
P1.1 IFG set
     ↓
PORT1_IRQHandler()
     ↓
Manual mode?
     ↓
Button enabled?
     ↓
    Yes
     ↓
Disable manual buttons
     ↓
binary_counter++
     ↓
Clear P1.1 IFG
     ↓
Return
```

The same process occurs for P1.4, except the counter is decremented.

---

### 4.8 Switch Between Counting Modes

The external `P4.4` button switches between automatic and manual counting.

The interrupt source is checked using:

```c
if ((P4->IFG & BIT4) != 0U)
```

The mode changes only when SysTick has enabled the action:

```c
if ((timer_flags & CHANGE_STATE_BUTTON_ENABLE) != 0U)
```

After an accepted button press, the software flag is cleared:

```c
timer_flags &= ~CHANGE_STATE_BUTTON_ENABLE;
```

If the current mode is automatic:

```c
if (counting_state == AUTOMATIC)
```

the program switches to manual mode:

```c
counting_state = MANUAL;
```

The red LED is turned on and the green LED is turned off:

```c
P1->OUT |= BIT0;
P2->OUT &= ~BIT1;
```

If the application is already in manual mode, it returns to automatic mode:

```c
counting_state = AUTOMATIC;
```

and the indicators are reversed:

```c
P1->OUT &= ~BIT0;
P2->OUT |= BIT1;
```

The mode button therefore behaves as:

```text
            P4.4 press
                ↓
        ┌──── AUTOMATIC ────┐
        │                   │
        ▼                   │
      MANUAL                │
        │                   │
        └──── P4.4 press ───┘
```

The P4.4 hardware interrupt flag is cleared before leaving the ISR:

```c
P4->IFG &= ~BIT4;
```

---

### 4.9 SysTick Handler

The SysTick handler controls the periodic behavior of the application:

```c
void SysTick_Handler(void)
```

Its first action depends on the current counting mode.

#### Automatic Mode

When the application is in automatic mode:

```c
if (counting_state == AUTOMATIC)
{
    binary_counter++;
}
```

SysTick increments the counter approximately every 0.5 seconds.

The counter therefore advances automatically:

```text
0 → 1 → 2 → 3 → ... → 15 → 0 → ...
```

#### Manual Mode

When the application is in manual mode:

```c
else
{
    timer_flags |= MANUAL_COUNTER_BUTTONS_ENABLE;
}
```

SysTick does not increment the counter.

Instead, it enables another manual increment or decrement action.

#### Mode Button

SysTick also re-enables the mode-change button:

```c
timer_flags |= CHANGE_STATE_BUTTON_ENABLE;
```

This means SysTick performs several application tasks depending on the current state:

```text
                   SysTick
                      ↓
             Which counting mode?
                /           \
         AUTOMATIC           MANUAL
             ↓                 ↓
      Increment counter     Enable manual
                            counter buttons
                \           /
                 \         /
                  ↓       ↓
             Enable mode button
                      ↓
             Check counter range
                      ↓
            Update binary display
```

---

### 4.10 Display the 4-Bit Binary Value

A 4-bit unsigned value can represent:

```text
0000 = 0
through
1111 = 15
```

The program therefore keeps `binary_counter` within this range.

If manual decrementing causes the counter to go below zero:

```c
if (binary_counter < 0)
{
    binary_counter = 15;
}
```

the value wraps around to `15`.

If automatic or manual incrementing causes the counter to exceed `15`:

```c
else if (binary_counter > 15)
{
    binary_counter = 0;
}
```

the value wraps around to `0`.

This produces:

```text
Increment:

14 → 15 → 0 → 1


Decrement:

1 → 0 → 15 → 14
```

Before displaying the new value, the previous four output bits are cleared:

```c
P4->OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);
```

The lower four bits of the counter are then written to `P4.0` through `P4.3`:

```c
P4->OUT |= ((uint8_t)binary_counter & 0x0FU);
```

`0x0F` is:

```text
0000 1111
```

The mask therefore keeps only the lowest four bits.

For example, if:

```text
binary_counter = 10
```

then:

```text
Decimal 10 = Binary 1010
```

and:

```text
P4.3 = 1
P4.2 = 0
P4.1 = 1
P4.0 = 0
```

The four LEDs display:

```text
1 0 1 0
```

Because `P4.0` through `P4.3` directly correspond to bits 0 through 3, no individual LED conditions are required.

---

### 4.11 Complete Program Execution Flow

This project combines multiple interrupt sources into one application.

#### Automatic Mode

The application begins in automatic mode:

```text
Program starts
     ↓
Initialize hardware
     ↓
Green mode LED ON
     ↓
Automatic mode
     ↓
SysTick counts ~0.5 seconds
     ↓
SysTick_Handler()
     ↓
binary_counter++
     ↓
Keep counter between 0 and 15
     ↓
Update P4.0–P4.3
     ↓
SysTick counts again
     ↓
Repeat
```

#### Manual Mode

Pressing the mode button changes the application to manual mode:

```text
P4.4 pressed
     ↓
PORT4_IRQHandler()
     ↓
Switch to MANUAL
     ↓
Red mode LED ON
Green mode LED OFF
     ↓
SysTick_Handler()
     ↓
Enable manual buttons
     ↓
P1.1 or P1.4 pressed
     ↓
PORT1_IRQHandler()
     ↓
Increment or decrement counter
     ↓
Disable manual buttons
     ↓
Next SysTick
     ↓
Keep counter between 0 and 15
     ↓
Update P4.0–P4.3
     ↓
Enable manual buttons again
```

Because the binary display is updated by `SysTick_Handler()`, a manual increment or decrement does not necessarily appear on the LEDs immediately.

The display is refreshed at the next SysTick event, so the visible update can occur up to approximately 0.5 seconds after a manual button press.

This behavior is intentional in this example because SysTick acts as the central periodic timing source for the application.

---

## 5. Expected Result

When the program starts:

- The counter begins at `0`.
- The four external LEDs display `0000`.
- The green onboard LED turns on.
- The application begins in automatic mode.

### Automatic Mode

Approximately every 0.5 seconds, the counter increments:

```text
Decimal    Binary

   0       0000
   1       0001
   2       0010
   3       0011
   4       0100
   5       0101
   ...
  15       1111
   0       0000
```

The counter continuously wraps from `15` back to `0`.

### Manual Mode

Pressing the external `P4.4` button switches to manual mode.

The green LED turns off and the red LED turns on.

In manual mode:

```text
P1.1 → Increment counter
P1.4 → Decrement counter
```

The counter wraps in both directions:

```text
Increment:
14 → 15 → 0 → 1

Decrement:
1 → 0 → 15 → 14
```

The binary LEDs are refreshed by SysTick, so a manual counter change may take up to approximately 0.5 seconds to appear.

Pressing `P4.4` again returns the application to automatic mode.

---

## 6. Register Summary

| Register / Function | Purpose |
| --- | --- |
| `WDT_A->CTL` | Controls the watchdog timer |
| `P1->SEL0`, `P1->SEL1` | Configure Port 1 pins for GPIO |
| `P2->SEL0`, `P2->SEL1` | Configure Port 2 pins for GPIO |
| `P4->SEL0`, `P4->SEL1` | Configure Port 4 pins for GPIO |
| `Px->DIR` | Configures GPIO pins as inputs or outputs |
| `Px->REN` | Enables internal pull resistors |
| `Px->OUT` | Controls outputs or selects pull-up/pull-down resistors |
| `Px->IES` | Selects the GPIO interrupt edge |
| `Px->IE` | Enables GPIO interrupts |
| `Px->IFG` | Indicates pending GPIO interrupt events |
| `SysTick->LOAD` | Stores the SysTick reload value |
| `SysTick->VAL` | Contains the current SysTick counter value |
| `SysTick->CTRL` | Controls SysTick operation |
| `SysTick_CTRL_CLKSOURCE_Msk` | Selects the processor clock |
| `SysTick_CTRL_TICKINT_Msk` | Enables the SysTick exception |
| `SysTick_CTRL_ENABLE_Msk` | Enables the SysTick counter |
| `NVIC_EnableIRQ()` | Enables Port interrupts in the NVIC |
| `__enable_irq()` | Enables interrupts globally |

---

## 7. Common Problems

### The binary LEDs do not count automatically

Verify that the application begins in:

```c
AUTOMATIC
```

and that SysTick is configured with:

```c
SysTick_CTRL_CLKSOURCE_Msk
SysTick_CTRL_TICKINT_Msk
SysTick_CTRL_ENABLE_Msk
```

Also verify that the handler is named exactly:

```c
void SysTick_Handler(void)
```

---

### The buttons do not generate interrupts

The buttons use pull-up resistors and are active-low.

A press therefore produces:

```text
HIGH → LOW
```

The interrupt edge should be configured using:

```c
P1->IES |= (BIT1 | BIT4);
P4->IES |= BIT4;
```

Also verify that Port 1 and Port 4 are enabled in the NVIC:

```c
NVIC_EnableIRQ(PORT1_IRQn);
NVIC_EnableIRQ(PORT4_IRQn);
```

---

### The increment and decrement buttons do nothing in automatic mode

This is expected.

The onboard buttons are intentionally accepted only when:

```c
counting_state == MANUAL
```

Automatic mode uses SysTick to increment the counter instead.

---

### A manual button press does not immediately change the LEDs

This is expected in this example.

The Port 1 ISR changes:

```c
binary_counter
```

but the binary LEDs are updated by:

```c
SysTick_Handler()
```

The visible display therefore updates at the next SysTick event, which may take up to approximately 0.5 seconds.

---

### A button does not respond to repeated rapid presses

The program uses SysTick-controlled software flags to create a simple input lockout.

After an accepted button action, its software enable flag is cleared and is not enabled again until a later SysTick event.

This prevents repeated actions during the lockout period.

The approximately 0.5-second lockout is intentionally simple and is longer than typical mechanical switch debounce times.

---

### The counter goes above 15 or below 0

The counter should be wrapped inside `SysTick_Handler()`:

```c
if (binary_counter < 0)
{
    binary_counter = 15;
}
else if (binary_counter > 15)
{
    binary_counter = 0;
}
```

This keeps the displayed value within the 4-bit range of `0` through `15`.

---

### Why are some interrupt flags cleared manually while SysTick is not?

Port 1 and Port 4 are peripheral interrupt sources.

Their GPIO interrupt flags must be cleared manually:

```c
P1->IFG &= ~BIT1;
P1->IFG &= ~BIT4;
P4->IFG &= ~BIT4;
```

SysTick is a Cortex-M core exception and does not use a GPIO-style peripheral `IFG` that must be manually cleared inside `SysTick_Handler()`.

---

## 8. Next Example

This project completes the SysTick timer examples.

The three examples demonstrate progressively more advanced uses of the same Cortex-M timer:

```text
01_systick_delay
        ↓
Poll COUNTFLAG in main()
        ↓
Learn basic SysTick operation


02_systick_interrupt
        ↓
Enable TICKINT
        ↓
Respond using SysTick_Handler()


03_systick_binary_counter
        ↓
Combine SysTick with GPIO interrupts,
application states, software flags,
and a 4-bit binary display
```

The next topic introduces another timer available on the MSP432P401R: **Timer_A**.

Unlike SysTick, which is part of the ARM Cortex-M4 processor core, Timer_A is an MSP432 peripheral with additional timer features that can be used for more advanced timing applications.