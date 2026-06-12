# MSP432 Software Fundamentals

A collection of MSP432P401R software examples intended to serve as a reference for common embedded systems peripherals and functionality.

Examples are implemented using register-level programming and focus on readability, code organization, and practical usage.

## Hardware

* MSP432P401R LaunchPad
* Code Composer Studio (CCS) 12.8.1

## Repository Contents

### GPIO

* Blink LED
* Onboard LEDs
* Button Polling
* Button LED Control

### Port Interrupts

* Button Interrupts
* Pull-Up Interrupts
* Pull-Down Interrupts

### SysTick Timer

* Simple Delay
* Timer Interrupts
* Stopwatch

### TimerA

* Basic Timer
* Multiple Timer Instances
* Timer Interrupts

### PWM

* LED Dimming
* Multiple PWM Outputs
* SG90 Servo Control
* Sine Wave Generation

### ADC

* Polling Examples
* Interrupt Examples

### UART

* UART Echo
* External Serial Communication
* UART Interrupts

### LCD

* LCD Output
* LCD with Interrupts

## Repository Standards

Each example is intended to:

* Compile without warnings
* Include clear project documentation
* Document hardware pin usage
* Serve as a reusable software reference

## Disclaimer

These examples are provided as reference material. They are intended to demonstrate peripheral configuration and basic firmware structure. Users are encouraged to consult the MSP432 Technical Reference Manual and device datasheet for complete peripheral documentation and device-specific behavior.