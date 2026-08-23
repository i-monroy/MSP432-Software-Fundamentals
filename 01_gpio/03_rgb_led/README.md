# GPIO RGB LED Colors 🪼

This example cycles through the available colors of the onboard RGB LED on the MSP432P401R LaunchPad.

The three RGB LED channels are controlled using `P2.0`, `P2.1`, and `P2.2`. Different combinations of these three GPIO outputs produce seven visible colors.

---

## 1. Overview

The onboard RGB LED contains three independently controlled color channels:

* Red
* Green
* Blue

Each channel is connected to a separate GPIO pin. By turning different channels on at the same time, additional colors can be created.

This example uses a 3-bit counter to cycle through each possible color combination while skipping the all-off state.

---

## 2. Hardware Used

| Item                  | Description                 |
| --------------------- | --------------------------- |
| Microcontroller Board | MSP432P401R LaunchPad       |
| LED                   | Onboard RGB LED             |
| Red Channel           | `P2.0`                      |
| Green Channel         | `P2.1`                      |
| Blue Channel          | `P2.2`                      |
| IDE                   | Code Composer Studio 12.8.1 |

No external components or wiring are required.

---

## 3. Pinout / Wiring

The RGB LED is already connected to the MSP432P401R LaunchPad.

| RGB Channel | MSP432P401R Pin | Description                    |
| ----------- | --------------- | ------------------------------ |
| Red         | `P2.0`          | Controls the red LED channel   |
| Green       | `P2.1`          | Controls the green LED channel |
| Blue        | `P2.2`          | Controls the blue LED channel  |

Since all three LED channels are built into the LaunchPad, no external wiring is needed.

---

## 4. Code Walkthrough

### 4.1 Stop the Watchdog Timer

```c
WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
```

The watchdog timer is enabled after reset.

If it is not stopped or periodically serviced, it can reset the microcontroller. This example does not use the watchdog timer, so it is stopped before configuring the GPIO pins.

---

### 4.2 Select GPIO Operation

```c
P2->SEL0 &= ~(BIT0 | BIT1 | BIT2);
P2->SEL1 &= ~(BIT0 | BIT1 | BIT2);
```

The `SEL0` and `SEL1` registers determine which function is assigned to each pin.

Clearing the corresponding selection bits configures `P2.0`, `P2.1`, and `P2.2` for normal GPIO operation.

---

### 4.3 Configure the RGB LED Outputs

```c
P2->DIR |= BIT0 | BIT1 | BIT2;
```

Setting the corresponding bits in the `DIR` register configures all three RGB LED pins as outputs.

This allows the MSP432P401R to independently control the red, green, and blue channels.

The program then starts with all three channels turned off:

```c
P2->OUT &= ~(BIT0 | BIT1 | BIT2);
```

---

### 4.4 RGB LED Color Combinations

The RGB LED uses three GPIO outputs, giving a total of:

```text
2³ = 8 possible output combinations
```

Each bit represents one RGB LED channel:

```text
P2.2    P2.1    P2.0
Blue    Green   Red
```

The eight possible combinations are:

| P2.2 | P2.1 | P2.0 | Color   |
| ---: | ---: | ---: | ------- |
|  `0` |  `0` |  `0` | Off     |
|  `0` |  `0` |  `1` | Red     |
|  `0` |  `1` |  `0` | Green   |
|  `0` |  `1` |  `1` | Yellow  |
|  `1` |  `0` |  `0` | Blue    |
|  `1` |  `0` |  `1` | Magenta |
|  `1` |  `1` |  `0` | Cyan    |
|  `1` |  `1` |  `1` | White   |

The program cycles from binary `001` through `111`, so the all-off `000` state is skipped during the repeating color sequence.

---

### 4.5 Map the Counter to the RGB LED

```c
uint8_t rgb_led_counter = 1U;
```

The counter begins at `1`, which is binary `001`.

Inside the main loop, the previous RGB LED color is first cleared:

```c
P2->OUT &= ~(BIT0 | BIT1 | BIT2);
```

The counter value is then written to the three RGB LED output bits:

```c
P2->OUT |= rgb_led_counter;
```

Because the RGB LED is connected to the lowest three bits of Port 2, the three lowest bits of `rgb_led_counter` map directly to `P2.2`, `P2.1`, and `P2.0`.

For example, if:

```text
rgb_led_counter = 5
```

Decimal `5` is binary:

```text
101
```

Mapping those bits to the RGB LED gives:

```text
P2.2    P2.1    P2.0
Blue    Green   Red
  1       0       1
```

The blue and red channels are therefore turned on together, producing **magenta**.

This same relationship allows the counter values from `1` through `7` to directly produce all seven visible RGB color combinations.

---

### 4.6 Delay Between Colors

```c
__delay_cycles(RGB_LED_DELAY_CYCLES);
```

The program waits before changing to the next color so that each RGB combination remains visible.

The delay is defined using:

```c
#define RGB_LED_DELAY_CYCLES (2000000U)
```

This value represents CPU cycles rather than a fixed amount of time. The actual delay duration therefore depends on the active CPU clock frequency.

---

### 4.7 Advance and Reset the Counter

After displaying the current color, the counter is incremented:

```c
rgb_led_counter++;
```

Once the counter reaches `8`, it is reset to `1`:

```c
if (rgb_led_counter >= 8U)
{
    rgb_led_counter = 1U;
}
```

Only three bits are needed for the RGB LED, so the desired visible combinations range from:

```text
001 → 111
```

Resetting the counter to `1` causes the sequence to continuously repeat without displaying the all-off state between cycles.

---

## 5. Expected Result

After programming the MSP432P401R LaunchPad, the onboard RGB LED cycles through the following colors:

```text
Red
 ↓
Green
 ↓
Yellow
 ↓
Blue
 ↓
Magenta
 ↓
Cyan
 ↓
White
 ↓
Repeat
```

Each color remains visible for the delay specified by `RGB_LED_DELAY_CYCLES`.

---

## 6. Register Summary

| Register     | Purpose                                        |
| ------------ | ---------------------------------------------- |
| `WDT_A->CTL` | Controls the watchdog timer                    |
| `P2->SEL0`   | Selects the pin function for the RGB LED pins  |
| `P2->SEL1`   | Selects the pin function for the RGB LED pins  |
| `P2->DIR`    | Configures the RGB LED pins as outputs         |
| `P2->OUT`    | Controls the red, green, and blue LED channels |

---

## 7. Common Problems

### The RGB LED does not change colors

Check that:

* The project builds and programs successfully.
* The correct MSP432P401R target is selected.
* `P2.0`, `P2.1`, and `P2.2` are configured for GPIO operation.
* All three pins are configured as outputs.

### Some colors do not appear

Verify that all three RGB LED channels are included in the GPIO configuration:

* `P2.0` for red
* `P2.1` for green
* `P2.2` for blue

A missing channel will prevent any color combination that requires that channel from displaying correctly.

### The colors change too quickly or too slowly

The delay depends on `RGB_LED_DELAY_CYCLES` and the active CPU clock frequency.

Increase `RGB_LED_DELAY_CYCLES` for a longer delay between colors or decrease it for a shorter delay.

### The LED turns off between colors

The program clears the previous RGB combination immediately before setting the next one. Under normal operation, this transition happens too quickly to be noticeable.

If an additional delay is placed between clearing `P2->OUT` and setting the new color, the off state may become visible.

---

## 8. Next Example

Continue with the next GPIO example to explore controlling multiple GPIO outputs using bit patterns.
