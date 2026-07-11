# GPIO Blink LED ⚡

This example blinks the onboard red LED connected to `P1.0` on the MSP432P401R LaunchPad.

This is the first GPIO example because it shows the basic structure used in embedded codes:

1. Stop the watchdog timer.
2. Configure a pin for GPIO.
3. Configure the pin as an output.
4. Control the output pin inside an infinite loop.

---

## 1. Overview

The MSP432P401R LaunchPad has an onboard red LED connected to pin `P1.0`.

In this example, `P1.0` is configured as a GPIO output. The program turns the LED on, waits for a short delay, turns the LED off, waits again, and repeats forever.

---

## 2. Hardware Used

| Item                  | Description           |
| --------------------- | --------------------- |
| Microcontroller Board | MSP432P401R LaunchPad |
| LED                   | Onboard red LED       |
| LED Pin               | `P1.0`                |
| IDE                   | Code Composer Studio  |

No external components are required.

---

## 3. Pinout / Wiring

The red LED is already connected on the MSP432P401R LaunchPad.

| Signal  | MSP432P401R Pin | Description     |
| ------- | --------------- | --------------- |
| Red LED | `P1.0`          | Onboard red LED |

Since the LED is already built into the board, no wiring is needed.

---

## 4. Code Walkthrough

### 4.1 Stop the Watchdog Timer

```c
WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
```

The watchdog timer is enabled by default after reset.

If the watchdog timer is not stopped or serviced, it can reset the microcontroller periodically. Since this example does not use the watchdog timer, the first step is to stop it.

---

### 4.2 Select GPIO Mode

```c
P1->SEL0 &= ~BIT0;
P1->SEL1 &= ~BIT0;
```

Pins on the MSP432P401R can have multiple functions. A pin can be used as GPIO, UART, SPI, I2C, timer output, ADC input, or another peripheral function depending on the pin.

The `SEL0` and `SEL1` registers select the function of the pin.

For this example, `P1.0` must be used as a normal GPIO pin, so both selection bits for `P1.0` are cleared, `&= ~`. Other examples show a different configuration for `SEL0` and `SEL1`.

---

### 4.3 Configure P1.0 as an Output

```c
P1->DIR |= BIT0;
```

The `DIR` register controls whether a GPIO pin is an input or output.

For GPIO direction:

| Bit Value | Pin Direction |
| --------- | ------------- |
| `0`       | Input         |
| `1`       | Output        |

Since the red LED is connected to `P1.0`, the program sets, `|=`, bit 0 in `P1DIR` to configure `P1.0` as an output.

This allows the MSP432P401R to drive the LED pin high or low.

---

### 4.4 Initialize the LED State

```c
P1->OUT &= ~BIT0;
```

The `OUT` register controls the output value of a GPIO pin when that pin is configured as an output.

This line clears, `&= ~`, bit 0 in `P1OUT`, which turns the red LED off before the main loop begins.

Starting with a known output state is good practice because it makes the program behavior predictable.

---

### 4.5 Create the Software Delay

```c
#define LED_BLINK_DELAY_CYCLES (1000000U)
```

This macro defines the number of CPU cycles used for the delay.

```c
__delay_cycles(LED_BLINK_DELAY_CYCLES);
```

The `__delay_cycles()` function pauses the program for the specified number of CPU cycles.

In this example, the delay is approximately 1 second when the main clock is running at 1 MHz.

This is a simple software delay. It is useful for beginner examples, but timer peripherals should be used for more accurate timing in larger projects.

---

### 4.6 Blink the LED

```c
P1->OUT |= BIT0;
```

This line sets bit 0 in `P1OUT`, which turns the red LED on.

```c
P1->OUT &= ~BIT0;
```

This line clears bit 0 in `P1OUT`, which turns the red LED off.

These two operations are repeated forever inside the `while (1)` loop.

---

## 5. Expected Result

After programming the MSP432P401R LaunchPad:

* The onboard red LED turns on.
* The LED stays on for approximately 1 second.
* The LED turns off.
* The LED stays off for approximately 1 second.
* The pattern repeats forever.

---

## 6. Register Summary

| Register     | Purpose                                     |
| ------------ | ------------------------------------------- |
| `WDT_A->CTL` | Controls the watchdog timer                 |
| `P1->SEL0`   | Selects the pin function for Port 1 pins    |
| `P1->SEL1`   | Selects the pin function for Port 1 pins    |
| `P1->DIR`    | Configures Port 1 pins as inputs or outputs |
| `P1->OUT`    | Controls the output value of Port 1 pins    |

---

## 7. Common Problems

### LED does not blink

Check that:

* The correct board is selected in Code Composer Studio.
* The project builds successfully.
* The MSP432P401R LaunchPad is connected correctly.
* The debugger/programmer is detected by Code Composer Studio.
* The code is using `P1.0`, which is the onboard red LED.

### LED blinks too fast or too slow

The delay depends on the main clock frequency.

This example assumes the main clock is approximately 1 MHz. If the clock configuration changes, the delay time will also change.

---

## 8. Next Example

Next, continue with a GPIO input example using one of the onboard push buttons.