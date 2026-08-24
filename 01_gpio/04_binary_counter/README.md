# GPIO 4-Bit Binary Counter 🦌

This example displays a 4-bit binary counter using four external LEDs connected to the MSP432P401R LaunchPad.

The counter can operate automatically or be controlled manually using the onboard push buttons. An external push button switches between the two counting modes.

---

## 1. Overview

This project demonstrates how multiple GPIO outputs can represent a binary value.

Four external LEDs are connected to `P4.0` through `P4.3`, allowing values from decimal `0` through `15` to be displayed as a 4-bit binary number.

The program supports two operating modes:

- **Automatic Mode:** The counter automatically increments from `0` to `15` and repeats.
- **Manual Mode:** The onboard push buttons increment or decrement the counter one value at a time.

The onboard LEDs indicate the current operating mode:

- Green LED → Automatic Mode
- Red LED → Manual Mode

An external push button connected to `P4.4` switches between the two modes.

---

## 2. Hardware Used

| Item | Description |
| --- | --- |
| Microcontroller Board | MSP432P401R LaunchPad |
| Binary LEDs | 4 external LEDs |
| Mode Button | 1 external push button |
| Increment Button | Onboard S1 push button |
| Decrement Button | Onboard S2 push button |
| Automatic Mode Indicator | Green channel of onboard RGB LED |
| Manual Mode Indicator | Onboard red LED |
| IDE | Code Composer Studio 12.8.1 |

Four current-limiting resistors are also required for the external LEDs.

---

## 3. Pinout / Wiring

### Pin Assignments

| Component | MSP432P401R Pin | Purpose |
| --- | --- | --- |
| Red LED | `P1.0` | Manual mode indicator |
| Green LED | `P2.1` | Automatic mode indicator |
| Onboard S1 | `P1.1` | Increment counter |
| Onboard S2 | `P1.4` | Decrement counter |
| External LED 0 | `P4.0` | Bit 0 - LSB |
| External LED 1 | `P4.1` | Bit 1 |
| External LED 2 | `P4.2` | Bit 2 |
| External LED 3 | `P4.3` | Bit 3 - MSB |
| External Button | `P4.4` | Switch counting mode |

### Binary LED Wiring

Each external LED represents one bit of the 4-bit binary number.

```text
P4.3    P4.2    P4.1    P4.0
 MSB                     LSB
Bit 3   Bit 2   Bit 1   Bit 0
```

Each GPIO output should connect to its LED through an appropriate current-limiting resistor, with the other side of the LED connected to ground.

### Mode Button Wiring

The external mode button uses the MSP432P401R's internal pull-up resistor.

Connect the button between `P4.4` and ground:

```text
GND ─── Push Button ─── P4.4
```

This makes the button active-low:

| Button State | P4.4 Input |
| --- | --- |
| Released | Logic `1` |
| Pressed | Logic `0` |

The onboard `P1.1` and `P1.4` buttons also operate as active-low inputs using internal pull-up resistors.

---

## 4. Code Walkthrough

### 4.1 Stop the Watchdog Timer

```c
WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
```

The watchdog timer is enabled after reset.

Since this example does not use the watchdog timer, it is stopped before configuring the GPIO peripherals.

---

### 4.2 Configure the GPIO Pins

The program configures the onboard LEDs, onboard buttons, external LEDs, and external mode button for GPIO operation.

```c
P1->SEL0 &= ~(BIT0 | BIT1 | BIT4);
P1->SEL1 &= ~(BIT0 | BIT1 | BIT4);

P2->SEL0 &= ~BIT1;
P2->SEL1 &= ~BIT1;

P4->SEL0 &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4);
P4->SEL1 &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4);
```

The LED pins are then configured as outputs:

```c
P1->DIR |= BIT0;
P2->DIR |= BIT1;
P4->DIR |= BIT0 | BIT1 | BIT2 | BIT3;
```

The three button pins are configured as inputs:

```c
P1->DIR &= ~(BIT1 | BIT4);
P4->DIR &= ~BIT4;
```

---

### 4.3 Configure the Button Pull-Up Resistors

The internal resistors are enabled for all three button inputs:

```c
P1->REN |= BIT1 | BIT4;
P4->REN |= BIT4;
```

The corresponding `OUT` bits are then set:

```c
P1->OUT |= BIT1 | BIT4;
P4->OUT |= BIT4;
```

When a GPIO pin is configured as an input with its internal resistor enabled, setting its corresponding `OUT` bit selects an internal pull-up resistor.

Therefore, all three buttons are active-low:

```text
Released → Logic 1
Pressed  → Logic 0
```

---

### 4.4 Counting Modes

The program uses an enumeration to represent the two operating modes:

```c
typedef enum
{
    AUTOMATIC,
    MANUAL
} counting_state_t;
```

The current mode is stored using:

```c
counting_state_t counting_state = AUTOMATIC;
```

Using named states makes the program easier to understand than assigning arbitrary numeric values to each operating mode.

The program starts in automatic mode.

---

### 4.5 Switch Between Automatic and Manual Mode

The external button connected to `P4.4` switches between the two operating modes.

```c
if ((P4->IN & BIT4) == 0U)
```

Because `P4.4` uses an internal pull-up resistor, a logic `0` indicates that the button has been pressed.

When the program changes to manual mode:

```c
counting_state = MANUAL;
P1->OUT |= BIT0;
P2->OUT &= ~BIT1;
```

the red LED turns on and the green LED turns off.

When the program returns to automatic mode:

```c
counting_state = AUTOMATIC;
P1->OUT &= ~BIT0;
P2->OUT |= BIT1;
```

the red LED turns off and the green LED turns on.

This provides a visual indication of the current counting mode.

---

### 4.6 Display a 4-Bit Binary Value

The counter is stored using:

```c
uint8_t binary_counter = 0U;
```

Although `uint8_t` can represent values from `0` through `255`, this project intentionally limits the counter to the range `0` through `15`.

Four bits can represent:

```text
2⁴ = 16 values
```

Those values range from:

```text
0000 = 0
```

through:

```text
1111 = 15
```

Before displaying a new value, the previous four LED bits are cleared:

```c
P4->OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);
```

The counter is then mapped directly to the four GPIO outputs:

```c
P4->OUT |= binary_counter;
```

Because the external LEDs are connected to the lowest four bits of Port 4, the lower four bits of `binary_counter` directly control `P4.3:P4.0`.

For example:

```text
binary_counter = 9

Decimal 9 = Binary 1001
```

Therefore:

```text
P4.3    P4.2    P4.1    P4.0
  1       0       0       1
  ON     OFF     OFF      ON
```

The LEDs physically display the binary representation of the counter value.

### 4-Bit Counter Table

| Decimal | P4.3 | P4.2 | P4.1 | P4.0 | Binary |
| ---: | ---: | ---: | ---: | ---: | :---: |
| 0 | 0 | 0 | 0 | 0 | `0000` |
| 1 | 0 | 0 | 0 | 1 | `0001` |
| 2 | 0 | 0 | 1 | 0 | `0010` |
| 3 | 0 | 0 | 1 | 1 | `0011` |
| 4 | 0 | 1 | 0 | 0 | `0100` |
| 5 | 0 | 1 | 0 | 1 | `0101` |
| 6 | 0 | 1 | 1 | 0 | `0110` |
| 7 | 0 | 1 | 1 | 1 | `0111` |
| 8 | 1 | 0 | 0 | 0 | `1000` |
| 9 | 1 | 0 | 0 | 1 | `1001` |
| 10 | 1 | 0 | 1 | 0 | `1010` |
| 11 | 1 | 0 | 1 | 1 | `1011` |
| 12 | 1 | 1 | 0 | 0 | `1100` |
| 13 | 1 | 1 | 0 | 1 | `1101` |
| 14 | 1 | 1 | 1 | 0 | `1110` |
| 15 | 1 | 1 | 1 | 1 | `1111` |

---

### 4.7 Automatic Counting

When the program is in automatic mode:

```c
if (counting_state == AUTOMATIC)
```

the current binary value remains visible for a short delay:

```c
__delay_cycles(BIT_LED_DELAY_CYCLES);
```

The counter is then incremented:

```c
binary_counter++;
```

Once the value moves beyond `15`, it returns to `0`:

```c
if (binary_counter >= 16U)
{
    binary_counter = 0U;
}
```

The automatic sequence therefore repeats continuously:

```text
0 → 1 → 2 → ... → 14 → 15 → 0 → 1 → ...
```

---

### 4.8 Manual Counting

Manual mode allows the counter to be changed using the two onboard push buttons.

#### Increment

Pressing the button connected to `P1.1` increments the counter.

```c
if (binary_counter >= 15U)
{
    binary_counter = 0U;
}
else
{
    binary_counter++;
}
```

If the current value is `15`, incrementing causes the counter to wrap back to `0`.

```text
14 → 15 → 0 → 1
```

#### Decrement

Pressing the button connected to `P1.4` decrements the counter.

```c
if (binary_counter == 0U)
{
    binary_counter = 15U;
}
else
{
    binary_counter--;
}
```

If the current value is `0`, decrementing causes the counter to wrap back to `15`.

```text
1 → 0 → 15 → 14
```

This keeps the counter within the valid 4-bit range of `0` through `15`.

---

### 4.9 Button Debouncing and Release Detection

All button presses use a short delay to reduce mechanical button bounce:

```c
__delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);
```

The input is checked again after the delay to confirm that the button remains pressed.

For example:

```c
if ((P4->IN & BIT4) == 0U)
{
    __delay_cycles(BUTTON_DEBOUNCE_DELAY_CYCLES);

    if ((P4->IN & BIT4) == 0U)
    {
        /* Confirmed button press */
    }
}
```

The program also waits for the button to be released before accepting another press.

This prevents a single long press from being interpreted as several separate button presses.

---

### 4.10 Polling and Blocking Delay Limitation

This example intentionally uses polling and a simple blocking delay to keep the implementation easy to follow.

During automatic mode, the program executes:

```c
__delay_cycles(BIT_LED_DELAY_CYCLES);
```

While this delay is running, the processor is not checking the mode button.

A very short button press that occurs entirely during this delay could therefore be missed.

This is an important limitation of using blocking delays with polling. Later timer and interrupt examples can avoid this limitation by allowing the processor to respond to events without waiting inside a long software delay.

---

## 5. Expected Result

When the program starts:

- The counter begins at `0000`.
- The green onboard LED is on.
- The red onboard LED is off.
- The program automatically counts from `0` through `15`.
- The four external LEDs display each value in binary.

### Switching to Manual Mode

Press the external button connected to `P4.4`.

The program enters manual mode:

- Automatic counting stops.
- The green LED turns off.
- The red LED turns on.
- The current binary value remains available for manual control.

While in manual mode:

- Press `P1.1` to increment the counter.
- Press `P1.4` to decrement the counter.

Press the external `P4.4` button again to return to automatic mode.

The automatic counter resumes from the current counter value.

---

## 6. Register Summary

| Register | Purpose |
| --- | --- |
| `WDT_A->CTL` | Controls the watchdog timer |
| `P1->SEL0`, `P1->SEL1` | Select GPIO operation for the onboard red LED and buttons |
| `P2->SEL0`, `P2->SEL1` | Select GPIO operation for the green mode LED |
| `P4->SEL0`, `P4->SEL1` | Select GPIO operation for the external LEDs and mode button |
| `P1->DIR` | Configures the red LED as an output and onboard buttons as inputs |
| `P2->DIR` | Configures the green LED as an output |
| `P4->DIR` | Configures the binary LEDs as outputs and mode button as an input |
| `P1->REN` | Enables the internal resistors for the onboard buttons |
| `P4->REN` | Enables the internal resistor for the external mode button |
| `P1->IN` | Reads the onboard push buttons |
| `P4->IN` | Reads the external mode button |
| `P1->OUT` | Controls the red mode LED and selects onboard button pull-ups |
| `P2->OUT` | Controls the green mode LED |
| `P4->OUT` | Displays the binary value and selects the mode-button pull-up |

---

## 7. Common Problems

### The binary LEDs display the wrong number

Verify that the LEDs are connected in the correct bit order:

```text
P4.3 → Bit 3 (MSB)
P4.2 → Bit 2
P4.1 → Bit 1
P4.0 → Bit 0 (LSB)
```

Swapping LED connections will cause the displayed binary value to appear incorrect.

### One or more binary LEDs do not turn on

Check that:

- Each LED is connected with the correct polarity.
- Each LED uses a current-limiting resistor.
- `P4.0` through `P4.3` are configured as GPIO outputs.
- All connections share the LaunchPad ground.

### The mode button does not work

Verify that:

- The external button is connected between `P4.4` and ground.
- `P4.4` is configured as an input.
- The internal resistor is enabled.
- The internal resistor is configured as a pull-up.

The mode button is active-low, so pressing it should cause `P4.4` to read logic `0`.

### The increment or decrement buttons behave incorrectly

The onboard `P1.1` and `P1.4` buttons are active-low.

Verify that both pins are configured as inputs with their internal pull-up resistors enabled.

### A button press is occasionally missed in automatic mode

The automatic counter uses a blocking software delay.

The processor cannot poll the mode button while it is inside this delay, so a sufficiently short press may not be detected.

Hold the mode button until the program detects the press. This limitation will be addressed by later timer and interrupt examples.

### The counter does not wrap correctly

The valid range for a 4-bit value is `0` through `15`.

The expected behavior is:

```text
Increment: 15 → 0
Decrement: 0 → 15
```

Verify that both boundary conditions are present in the manual counting logic.

---

## 8. Next Example

This completes the introductory GPIO polling examples.

Continue to the next section to explore GPIO interrupts and responding to input events without continuously polling the pins.