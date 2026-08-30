# Dual Button Interrupt (Pull-Up and Pull-Down) 🐎

This example controls two onboard LEDs using two external push buttons configured as GPIO interrupts.

The first button uses an internal pull-up resistor and controls an LED while the button is held. The second button uses an internal pull-down resistor and toggles an LED each time the button is pressed.

Unlike the previous polling-based GPIO example, button events are detected using Port 4 interrupts.

---

## 1. Overview

This project demonstrates two different GPIO interrupt configurations on the MSP432P401R.

Button 1 is connected to `P4.1` and uses an internal pull-up resistor. Pressing the button turns the onboard red LED on, while releasing the button turns it off.

Button 2 is connected to `P4.6` and uses an internal pull-down resistor. Each button press toggles the red channel of the onboard RGB LED.

Both buttons generate Port 4 interrupts and are handled by the same `PORT4_IRQHandler()` interrupt service routine.

This example demonstrates:

* GPIO interrupts
* Pull-up and pull-down button configurations
* Falling-edge and rising-edge interrupts
* Interrupt flags
* Reading the current input state
* Dynamically changing interrupt edge detection
* Multiple interrupt sources sharing one interrupt service routine
* Basic interrupt debouncing

---

## 2. Hardware Used

| Item                  | Description                    |
| --------------------- | ------------------------------ |
| Microcontroller Board | MSP432P401R LaunchPad          |
| LED 1                 | Onboard red LED                |
| LED 2                 | Red channel of onboard RGB LED |
| Button 1              | External push button           |
| Button 2              | External push button           |
| IDE                   | Code Composer Studio 12.8.1    |

---

## 3. Pinout / Wiring

### LEDs

| Component       | MSP432P401R Pin | Configuration |
| --------------- | --------------- | ------------- |
| Onboard Red LED | `P1.0`          | GPIO output   |
| RGB Red Channel | `P2.0`          | GPIO output   |

### Buttons

| Button   | MSP432P401R Pin | Configuration      | Connection                      |
| -------- | --------------- | ------------------ | ------------------------------- |
| Button 1 | `P4.1`          | Internal pull-up   | Button between `P4.1` and GND   |
| Button 2 | `P4.6`          | Internal pull-down | Button between `P4.6` and 3.3 V |

Button 1 is active-low:

| Button State | P4.1 |
| ------------ | ---- |
| Released     | HIGH |
| Pressed      | LOW  |

Button 2 is active-high:

| Button State | P4.6 |
| ------------ | ---- |
| Released     | LOW  |
| Pressed      | HIGH |

Both external buttons must share the LaunchPad's electrical reference through the appropriate GND and 3.3 V connections.

---

## 4. Code Walkthrough

### 4.1 Initialize the LEDs

The onboard red LED and the red channel of the RGB LED are initialized using:

```c
LED_redLEDsInit();
```

`P1.0` and `P2.0` are configured for GPIO operation and set as outputs.

Both LEDs begin turned off.

```c
P1->OUT &= ~BIT0;
P2->OUT &= ~BIT0;
```

The two LEDs are controlled independently by the two external button interrupts.

---

### 4.2 Configure the Pull-Up Button

Button 1 is connected to `P4.1` and configured as an input:

```c
P4->DIR &= ~BIT1;
```

The internal resistor is enabled using:

```c
P4->REN |= BIT1;
```

The corresponding bit in the `OUT` register is then set:

```c
P4->OUT |= BIT1;
```

When the resistor is enabled for an input pin, setting its `OUT` bit selects an internal pull-up resistor.

Therefore:

```text
Released = HIGH
Pressed  = LOW
```

The button is active-low.

---

### 4.3 Configure the Pull-Down Button

Button 2 is connected to `P4.6` and configured as an input:

```c
P4->DIR &= ~BIT6;
```

Its internal resistor is enabled:

```c
P4->REN |= BIT6;
```

The corresponding `OUT` bit is cleared:

```c
P4->OUT &= ~BIT6;
```

Clearing the `OUT` bit while the resistor is enabled selects an internal pull-down resistor.

Therefore:

```text
Released = LOW
Pressed  = HIGH
```

The button is active-high.

---

### 4.4 Pull-Up vs. Pull-Down Interrupt Edges

The `IES` register selects which signal transition generates an interrupt.

| IES Bit | Interrupt Edge |
| ------- | -------------- |
| `0`     | Low-to-high    |
| `1`     | High-to-low    |

Button 1 uses a pull-up resistor.

Pressing the button causes:

```text
HIGH → LOW
```

Therefore, its initial interrupt edge is configured using:

```c
P4->IES |= BIT1;
```

Button 2 uses a pull-down resistor.

Pressing the button causes:

```text
LOW → HIGH
```

Therefore:

```c
P4->IES &= ~BIT6;
```

The electrical configuration of the button determines which edge represents a button press.

---

### 4.5 Enable Port 4 and Global Interrupts

After both buttons are configured, Port 4 interrupts are enabled in the Nested Vectored Interrupt Controller:

```c
NVIC_EnableIRQ(PORT4_IRQn);
```

The NVIC allows the ARM Cortex-M4F processor to receive interrupt requests generated by Port 4.

Interrupts are then enabled globally:

```c
__enable_irq();
```

The interrupt path can be viewed as:

```text
GPIO signal changes
        ↓
Configured edge detected
        ↓
P4->IFG flag set
        ↓
P4->IE allows pin interrupt
        ↓
NVIC accepts Port 4 interrupt
        ↓
CPU executes PORT4_IRQHandler()
```

The main loop does not need to continuously poll either button.

```c
while (1)
{
    /* Button events are handled by interrupts. */
}
```

---

### 4.6 Port 4 Interrupt Service Routine

Both buttons are connected to Port 4.

Therefore, they share the same interrupt service routine:

```c
void PORT4_IRQHandler(void)
```

The ISR does not automatically represent one specific Port 4 pin. Multiple interrupt-capable pins on Port 4 can cause the processor to enter the same handler.

The program must determine which pin generated the interrupt.

In this example, both `P4.1` and `P4.6` can generate a Port 4 interrupt.

---

### 4.7 Determine the Interrupt Source Using IFG

The `IFG` register contains the interrupt flags for the port.

When the configured edge occurs on an interrupt-enabled pin, its corresponding interrupt flag is set.

The P4.1 flag is checked using:

```c
if ((P4->IFG & BIT1) != 0U)
```

The P4.6 flag is checked using:

```c
if ((P4->IFG & BIT6) != 0U)
```

For either button:

```text
Configured edge occurs
        ↓
Corresponding IFG bit becomes 1
        ↓
Interrupt request
        ↓
PORT4_IRQHandler()
```

The pull-up or pull-down configuration does not change how the interrupt flag itself is checked.

`IFG` answers the question:

> Which pin generated an interrupt event?

After the event is handled, the corresponding flag is cleared:

```c
P4->IFG &= ~BIT1;
```

or:

```c
P4->IFG &= ~BIT6;
```

---

### 4.8 Read the Current Button State Using IN

The `IN` register contains the current logic level present on the GPIO pins.

For the pull-up button:

```c
if ((P4->IN & BIT1) == 0U)
```

checks whether `P4.1` is currently LOW.

Because the button is active-low:

```text
P4.1 = LOW → Button pressed
P4.1 = HIGH → Button released
```

For the pull-down button:

```c
if ((P4->IN & BIT6) != 0U)
```

checks whether `P4.6` is currently HIGH.

Because the button is active-high:

```text
P4.6 = LOW  → Button released
P4.6 = HIGH → Button pressed
```

`IFG` and `IN` therefore provide different information:

```text
IFG → Which pin generated an interrupt event?

IN  → What is the current logic level of the pin?
```

This distinction becomes particularly useful when the same interrupt handler needs to respond differently depending on the current state of an input.

---

### 4.9 Dynamically Change the Interrupt Edge

Button 1 demonstrates that interrupt edge selection can be changed while the program is running.

Initially:

```c
P4->IES |= BIT1;
```

configures `P4.1` for a high-to-low interrupt.

Because the button uses a pull-up resistor, this detects the button press:

```text
Released
HIGH
  ↓
PRESS
  ↓
LOW
  ↓
Interrupt
```

Inside the interrupt handler, the program confirms that the input is LOW:

```c
if ((P4->IN & BIT1) == 0U)
```

The red LED is turned on:

```c
P1->OUT |= BIT0;
```

The interrupt edge is then changed:

```c
P4->IES &= ~BIT1;
```

The program is now waiting for a low-to-high transition.

While the button remains held, the input remains LOW:

```text
LOW → LOW → LOW → LOW
```

No edge occurs, so another interrupt is not generated.

When the button is released:

```text
LOW → HIGH
```

the rising edge generates another interrupt.

The ISR now reads a HIGH input, turns the LED off, and restores falling-edge detection:

```c
P1->OUT &= ~BIT0;
P4->IES |= BIT1;
```

The complete cycle is:

```text
Button Released
P4.1 = HIGH
      │
      │ IES = 1
      │ Wait for HIGH → LOW
      ▼
Button Pressed
P4.1 = LOW
      │
      ├── Interrupt
      ├── LED ON
      └── IES = 0
              │
              │ Wait for LOW → HIGH
              ▼
       Button Released
       P4.1 = HIGH
              │
              ├── Interrupt
              ├── LED OFF
              └── IES = 1
                      │
                      ▼
              Wait for next press
```

This allows one interrupt-enabled button to detect both its press and release without continuously polling the input.

It is important that the edge selection remains synchronized with the expected button state. An incorrect `IES` configuration could cause the program to wait for an edge that does not represent the intended event.

---

### 4.10 Handle Multiple Interrupt Sources

The Port 4 ISR uses two independent `if` statements:

```c
if ((P4->IFG & BIT1) != 0U)
{
    /* Handle P4.1 */
}

if ((P4->IFG & BIT6) != 0U)
{
    /* Handle P4.6 */
}
```

This is intentional.

Both pins share the same Port 4 interrupt handler, and more than one interrupt flag could be pending when the ISR executes.

Using two independent conditions allows both flags to be checked and serviced.

If the second condition were written as:

```c
else if ((P4->IFG & BIT6) != 0U)
```

then handling `P4.1` would prevent `P4.6` from being checked during that pass through the handler.

Each interrupt source is therefore checked independently.

---

### 4.11 Button Debouncing Inside the ISR

Mechanical push buttons can rapidly transition between high and low when pressed or released.

This behavior is known as button bounce.

A short delay is used after detecting an interrupt:

```c
__delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);
```

The input state is then checked before the button action is performed.

This provides a simple software debounce method suitable for this introductory example.

However, the processor remains inside the interrupt service routine during `__delay_cycles()`.

In larger or timing-sensitive embedded systems, interrupt handlers should generally remain short so other processing and interrupts are not unnecessarily delayed.

More advanced implementations can use timers or state-based debounce techniques instead of waiting inside the ISR.

---

## 5. Expected Result

After programming the MSP432P401R LaunchPad:

### Button 1 — P4.1 Pull-Up

1. The onboard red LED begins turned off.
2. Press and hold Button 1.
3. The onboard red LED turns on.
4. Continue holding the button.
5. The LED remains on.
6. Release the button.
7. The LED turns off.

### Button 2 — P4.6 Pull-Down

1. The red channel of the onboard RGB LED begins turned off.
2. Press Button 2.
3. The RGB LED's red channel toggles on.
4. Release the button.
5. Press Button 2 again.
6. The RGB LED's red channel toggles off.

Both buttons operate using Port 4 interrupts rather than continuous polling inside the main loop.

---

## 6. Register Summary

| Register / Function    | Purpose                                        |
| ---------------------- | ---------------------------------------------- |
| `WDT_A->CTL`           | Controls the watchdog timer                    |
| `P1->SEL0`, `P1->SEL1` | Select GPIO operation for P1.0                 |
| `P2->SEL0`, `P2->SEL1` | Select GPIO operation for P2.0                 |
| `P4->SEL0`, `P4->SEL1` | Select GPIO operation for the external buttons |
| `P1->DIR`, `P2->DIR`   | Configure the onboard LEDs as outputs          |
| `P4->DIR`              | Configure the external buttons as inputs       |
| `P4->REN`              | Enable the internal button resistors           |
| `P4->OUT`              | Select pull-up or pull-down operation          |
| `P4->IN`               | Read the current button input states           |
| `P4->IES`              | Select the interrupt edge                      |
| `P4->IFG`              | Store Port 4 interrupt flags                   |
| `P4->IE`               | Enable individual Port 4 pin interrupts        |
| `NVIC_EnableIRQ()`     | Enable Port 4 interrupts in the NVIC           |
| `__enable_irq()`       | Enable interrupts globally                     |
| `PORT4_IRQHandler()`   | Handle Port 4 interrupt requests               |

---

## 7. Common Problems

### Button 1 behavior is reversed

Button 1 uses an active-low pull-up configuration.

When released, `P4.1` should read HIGH. Pressing the button should connect `P4.1` to ground and produce a LOW input.

Verify that Button 1 is connected between `P4.1` and GND.

### Button 2 does not detect a press

Button 2 uses an active-high pull-down configuration.

When released, `P4.6` should read LOW. Pressing the button should connect `P4.6` to `3.3 V` and produce a HIGH input.

Verify that Button 2 is connected between `P4.6` and `3.3 V`.

### Button 1 turns the LED on but never turns it off

Verify that the interrupt edge is changed after detecting the button press:

```c
P4->IES &= ~BIT1;
```

This configures the next interrupt for the low-to-high transition generated when the button is released.

After the release is detected, falling-edge detection should be restored:

```c
P4->IES |= BIT1;
```

### The interrupt handler never executes

Verify that:

* The corresponding bit in `P4->IE` is enabled.
* Port 4 is enabled using `NVIC_EnableIRQ(PORT4_IRQn)`.
* Global interrupts are enabled using `__enable_irq()`.
* The interrupt handler is named `PORT4_IRQHandler()`.

### The LEDs toggle or change unexpectedly

Mechanical button bounce may generate additional transitions.

Verify that the debounce delay and input-state checks are present.

### One button works but the other does not

Check the corresponding `P4->IFG` flag and verify that each button is configured for the correct interrupt edge.

Remember:

```text
Pull-up press   → HIGH → LOW → IES = 1
Pull-down press → LOW → HIGH → IES = 0
```

Also verify that the two external buttons are connected to the correct supply or ground connection.

---

## 8. Next Example

This completes the introductory Port Interrupt examples.

The previous example introduced a single GPIO interrupt, while this example demonstrated multiple interrupt sources, pull-up and pull-down configurations, shared interrupt handling, and dynamic interrupt edge selection.