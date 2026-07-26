# GPIO Button Input

This example uses two external push buttons to control two onboard LEDs on the MSP432P401R LaunchPad.

Button 1 uses an internal pull-up resistor and controls an LED while the button is held. Button 2 uses an internal pull-down resistor and toggles a second LED once for each confirmed button press.

---

## 1. Overview

This demonstrates how to configure GPIO pins as digital inputs and use the MSP432P401R's internal pull-up and pull-down resistors.

The two buttons demonstrate different input behaviors:

* Button 1 is continuously polled and directly controls an LED while held.
* Button 2 toggles an LED once for each button press.
* Basic software debouncing is used for Button 2.
* The program waits for Button 2 to be released before accepting another press.

---

## 2. Hardware Used

| Item                  | Description                        |
| --------------------- | ---------------------------------- |
| Microcontroller Board | MSP432P401R LaunchPad              |
| Button 1              | External push button               |
| Button 2              | External push button               |
| LED 1                 | Onboard red LED                    |
| LED 2                 | Red channel of the onboard RGB LED |
| IDE                   | Code Composer Studio 12.8.1        |

---

## 3. Pinout / Wiring

### Pin Assignments

| Component | MSP432P401R Pin | Configuration                      |
| --------- | --------------- | ---------------------------------- |
| LED 1     | `P1.0`          | GPIO output                        |
| LED 2     | `P2.0`          | GPIO output                        |
| Button 1  | `P4.1`          | GPIO input with internal pull-up   |
| Button 2  | `P4.6`          | GPIO input with internal pull-down |

### Button 1: Pull-Up Wiring

Connect Button 1 between `P4.1` and ground.

```text
P4.1 ─── Push Button ─── GND
```

The MSP432P401R's internal pull-up resistor holds `P4.1` high while the button is released.

| Button State | Input Value |
| ------------ | ----------- |
| Released     | Logic `1`   |
| Pressed      | Logic `0`   |

Because the input becomes low when pressed, Button 1 is described as **active-low**.

### Button 2: Pull-Down Wiring

Connect Button 2 between `P4.6` and `3.3 V`.

```text
3.3 V ─── Push Button ─── P4.6
```

The MSP432P401R's internal pull-down resistor holds `P4.6` low while the button is released.

| Button State | Input Value |
| ------------ | ----------- |
| Released     | Logic `0`   |
| Pressed      | Logic `1`   |

Because the input becomes high when pressed, Button 2 is described as **active-high**.

> Do not connect either GPIO input directly between `3.3 V` and ground. The push buttons should connect the GPIO pin to only one supply rail as shown above.

---

## 4. Code Walkthrough

### 4.1 Select GPIO Operation

```c
P1->SEL0 &= ~BIT0;
P1->SEL1 &= ~BIT0;

P2->SEL0 &= ~BIT0;
P2->SEL1 &= ~BIT0;

P4->SEL0 &= ~(BIT1 | BIT6);
P4->SEL1 &= ~(BIT1 | BIT6);
```

MSP432P401R pins can support GPIO and several alternate peripheral functions.

The `SEL0` and `SEL1` registers select which function is assigned to each pin. Clearing both selection bits configures the selected pins for normal GPIO operation.

This project configures:

* `P1.0` and `P2.0` as GPIO outputs for the LEDs.
* `P4.1` and `P4.6` as GPIO inputs for the buttons.

---

### 4.2 Configure the GPIO Directions

```c
P1->DIR |= BIT0;
P2->DIR |= BIT0;
```

Setting a bit in the `DIR` register configures that pin as an output.

Therefore:

* `P1.0` drives the onboard red LED.
* `P2.0` drives the red channel of the onboard RGB LED.

```c
P4->DIR &= ~(BIT1 | BIT6);
```

Clearing a bit in the `DIR` register configures that pin as an input.

Therefore, `P4.1` and `P4.6` can read the electrical state created by the external push buttons.

| `DIR` Bit | Pin Direction |
| --------- | ------------- |
| `0`       | Input         |
| `1`       | Output        |

---

### 4.3 Enable and Select the Internal Resistors

```c
P4->REN |= BIT1 | BIT6;
```

The `REN` register enables the internal resistor for each selected GPIO input.

Enabling the resistor prevents an unpressed button input from floating between unpredictable logic levels.

The corresponding `OUT` bit determines whether the enabled resistor acts as a pull-up or pull-down resistor.

```c
P4->OUT |= BIT1;
```

Setting the `P4.1` output bit selects an internal pull-up resistor.

```c
P4->OUT &= ~BIT6;
```

Clearing the `P4.6` output bit selects an internal pull-down resistor.

| `REN` Bit | `OUT` Bit    | Resistor Configuration     |
| --------- | ------------ | -------------------------- |
| `0`       | Either value | Internal resistor disabled |
| `1`       | `1`          | Internal pull-up enabled   |
| `1`       | `0`          | Internal pull-down enabled |

Although `OUT` normally controls an output pin's voltage, it selects the resistor direction when the pin is configured as an input and its internal resistor is enabled.

---

### 4.4 Continuously Poll an Active-Low Input

```c
if ((P4->IN & BIT1) == 0U)
{
    P1->OUT |= BIT0;
}
else
{
    P1->OUT &= ~BIT0;
}
```

The program continuously reads the `P4.1` input inside the main loop.

Because Button 1 uses a pull-up resistor:

* A logic `0` means the button is pressed.
* A logic `1` means the button is released.

While the button is pressed, `P1.0` is set and the onboard red LED turns on. When the button is released, `P1.0` is cleared and the LED turns off.

This demonstrates **continuous polling** because the program repeatedly checks the button state and immediately updates the LED to match it.

---

### 4.5 Detect and Toggle Using an Active-High Input

```c
if ((P4->IN & BIT6) != 0U)
```

Because Button 2 uses a pull-down resistor, a nonzero result means that the button is pressed.

After the press is confirmed, the program toggles the red channel of the onboard RGB LED:

```c
P2->OUT ^= BIT0;
```

The XOR operator reverses the current state of `P2.0`:

| Previous LED State | New LED State |
| ------------------ | ------------- |
| Off                | On            |
| On                 | Off           |

The output register retains the LED state after the button is released. The next confirmed press reverses it again.

---

### 4.6 Reduce Mechanical Button Bounce

```c
#define BUTTON_DEBOUNCE_DELAY_CYCLES (50000U)
```

Mechanical push buttons do not always transition cleanly between open and closed states. Their electrical contacts may rapidly alternate between high and low for a brief period when pressed or released.

This behavior is called **button bounce**.

When Button 2 is first detected, the program waits for a short cycle-based delay:

```c
__delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);
```

It then checks the input again:

```c
if ((P4->IN & BIT6) != 0U)
```

The LED is toggled only if the button is still pressed after the delay.

The exact delay duration depends on the active CPU clock frequency. The macro represents CPU cycles rather than a guaranteed number of milliseconds.

This is a simple software-debounce technique intended for introductory examples.

---

### 4.7 Wait for Button Release

```c
while ((P4->IN & BIT6) != 0U)
{
    /* Intentionally empty. */
}
```

Without this loop, the program could toggle the LED repeatedly while Button 2 remains held because the main loop executes much faster than a person can release the button.

The empty `while` loop waits until the input returns low, indicating that Button 2 has been released.

After release, another short delay allows the signal to settle before the program accepts another press.

Debouncing and waiting for release solve different problems:

* Debouncing reduces false transitions caused by the mechanical contacts.
* Waiting for release prevents one long press from being counted multiple times.

---

## 5. Expected Result

After programming the MSP432P401R LaunchPad:

### Button 1 — `P4.1`

* Press and hold Button 1.
* The onboard red LED connected to `P1.0` turns on.
* Release Button 1.
* The LED turns off.

### Button 2 — `P4.6`

* Press Button 2 once.
* The red channel of the onboard RGB LED connected to `P2.0` changes state.
* Release the button.
* Press it again.
* The LED changes back to its previous state.

Both buttons operate independently and control separate LEDs.

---

## 6. Register Summary

| Register               | Purpose                                                   |
| ---------------------- | --------------------------------------------------------- |
| `WDT_A->CTL`           | Controls the watchdog timer                               |
| `P1->SEL0`, `P1->SEL1` | Select the function assigned to `P1.0`                    |
| `P2->SEL0`, `P2->SEL1` | Select the function assigned to `P2.0`                    |
| `P4->SEL0`, `P4->SEL1` | Select the function assigned to `P4.1` and `P4.6`         |
| `P1->DIR`              | Configures the `P1.0` LED pin as an output                |
| `P2->DIR`              | Configures the `P2.0` LED pin as an output                |
| `P4->DIR`              | Configures the button pins as inputs                      |
| `P4->REN`              | Enables the internal resistors                            |
| `P4->OUT`              | Selects pull-up or pull-down operation for the input pins |
| `P4->IN`               | Reads the current logic level on the button pins          |
| `P1->OUT`              | Controls the onboard red LED                              |
| `P2->OUT`              | Controls the red channel of the onboard RGB LED           |

---

## 7. Common Problems

### Button 1 behavior is reversed

Button 1 uses an active-low pull-up configuration.

The input reads low when pressed and high when released. Verify that the button is connected between `P4.1` and ground.

### Button 2 always reads low

Verify that Button 2 is connected between `P4.6` and `3.3 V`.

A pull-down input remains low until an external connection drives it high.

### Button 2 always reads high

Check for an incorrect connection between `P4.6` and `3.3 V`, or verify that `P4.6` is configured with a pull-down rather than a pull-up resistor.

### The RGB LED toggles more than once

Confirm that the debounce delay, second input check, and wait-for-release loop are present.

Mechanical button bounce or removing the wait-for-release logic can cause multiple toggles from one press.

### The LEDs do not respond

Check that:

* The project builds and programs successfully.
* The correct MSP432P401R target is selected.
* The external buttons share a common ground with the LaunchPad.
* Button 1 is connected to `P4.1` and ground.
* Button 2 is connected to `P4.6` and `3.3 V`.
* The code uses `P1.0` and `P2.0` for the onboard LEDs.

---

## 8. Next Example

Continue with another GPIO polling example that builds on digital inputs and outputs.