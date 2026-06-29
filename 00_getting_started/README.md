# Getting Started with the MSP432P401R

This folder explains the basic setup needed before running the examples in this repository.

---

## 1. Install Code Composer Studio

This repository was created using:

**Code Composer Studio:** 12.8.1  
**Board:** MSP432P401R LaunchPad  

Download CCS here:

https://www.ti.com/tool/download/CCSTUDIO/12.8.1

For Windows, download:

**Windows single file (offline) installer for Code Composer Studio IDE (all features, devices)**

After downloading the `.zip` file:

1. Extract the folder.
2. Open the extracted folder.
3. Run `ccs_setup_12.8.1.00005`.
4. Follow the installation steps.

---

## 2. Create a New CCS Project

This section will explain how to create a new MSP432P401R project in Code Composer Studio.

1. Launch Code Composer Studio.
2. Select your workspace.
3. Navigate to File → New → CCS Project.
4. Select the MSP432P401R device.
5. Enter your project name.
6. Choose Empty Project (with main.c).
7. Click Finish.

---

## 3. Basic MSP432P401R Notes

Before working through the examples, keep the following in mind:

- The MSP432P401R is a 32-bit ARM Cortex-M4F microcontroller.
- Most examples in this repository use direct register-level programming.
- The watchdog timer is enabled after every reset and is disabled at the beginning of most examples.
- The onboard red LED is connected to P1.0.
- The RGB LED is connected to P2.0, P2.1, and P2.2.
- The onboard push buttons are connected to P1.1 and P1.4.

---

## 4. main.c Template

A starter `main.c` template is included in this folder.

Use it when creating new examples or when organizing your own MSP432P401R code.