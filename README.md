# MSP432P401R Reference 💡🧑‍💻

A beginner-friendly reference repository for learning the Texas Instruments MSP432P401R LaunchPad using C and Code Composer Studio.

This repository contains examples for common MSP432 peripherals such as GPIO, timers, UART, ADC, PWM, interrupts, LCDs, I2C, and SPI.

![C](https://img.shields.io/badge/Language-C-blue) ![Code Composer Studio](https://img.shields.io/badge/IDE-Code%20Composer%20Studio-blue) ![MSP432P401R](https://img.shields.io/badge/Board-MSP432P401R-green) ![MIT License](https://img.shields.io/badge/License-MIT-orange)

---

## Repository Goals

This repository is designed to help you:

* Understand how MSP432P401R peripheral examples are structured
* Copy example code into Code Composer Studio and run it
* Learn what each important register/configuration step does
* Connect the C code with the README explanations
* Build a foundation for larger embedded systems projects

Each example includes beginner-focused C code and a README file that explains what the code does, why it works, and what to expect when running it.

---

## Hardware and Software Used

**Microcontroller Board:**
MSP432P401R LaunchPad

**Programming Language:**
C

**IDE:**
Code Composer Studio (CCS) 12.8.1

**Programming Style:**
Direct register-level programming (Bare Metal)

---

## Repository Contents

| Folder | Description |
| ------ | ----------- |
| `00_Getting_Started` | Code Composer Studio setup, project creation, and basic MSP432P401R notes |
| `01_GPIO` | Digital input/output examples such as blinking LEDs and reading buttons |
| `02_SysTick_Timer` | Basic delay and timing examples using the SysTick timer |
| `03_Timer_A` | Timer_A examples using interrupts and periodic timing |
| `04_Port_Interrupts` | GPIO interrupt examples using buttons and external signals |
| `05_UART` | Serial communication examples |
| `06_ADC` | Analog-to-digital conversion examples |
| `07_PWM` | Pulse-width modulation examples for LEDs, servos, and signals |
| `08_LCD` | LCD examples for displaying text and values |
| `09_I2C` | I2C communication examples |
| `10_SPI` | SPI communication examples |
| `11_Wireless_Communication` | Wireless communication examples using external modules such as the ESP8266 |
| `12_Cryptography` | Basic embedded cryptography examples such as AES |
| `docs` | Additional notes, references, and repository documentation |
| `images` | Pinouts, wiring diagrams, and example images |

---

## How to Use This Repository

1. Open an example project.
2. Read the example README.
3. Open `main.c` in Code Composer Studio.
4. When a comment in the code points to a README section, use that section for a more thorough explanation on that piece of code.

Example:

```c
/* README 3.1 - Stop the watchdog timer */
WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
```

---

## Documentation Style

Each example follows the same documentation structure:

1. Overview
2. Hardware Used
3. Pinout / Wiring
4. Code Walkthrough
5. Expected Result
6. Register Summary
7. Common Problems
8. Next Example

---

## Important Note

These examples intentionally favor readability over optimization.

Some examples may use simple delays, direct register writes, or beginner-friendly structure instead of more advanced embedded design patterns. This is intentional. The purpose of this repository is to explain what the code is doing, why it works, and how it communicates with the MSP432P401R before moving into more complex implementations.

---

## Additional References

Texas Instruments provides official documentation for the MSP432P401R, including the Technical Reference Manual and device datasheet.

These documents are useful for deeper study, but they are not required to understand the examples in this repository.

---

## Acknowledgments

This repository was inspired by a fellow UTEP (The University of Texas at El Paso) classmate and friend, **Jesus Minjares**, who created one of the first MSP432 reference repositories that I came across during college.

His work demonstrated the value of sharing embedded systems examples with the community and inspired me to create my own MSP432P401R reference repository.

Thank you for the inspiration, Jesus!

* [MSP432-Example-Codes](https://github.com/jminjares4/MSP432-Example-Codes)