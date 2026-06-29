Here's the updated README with the images integrated:

---

# 🎮 Pathfinder Button Box

A simple, open-source button box built around the **Seeed Studio XIAO RP2040**. This project was designed as a compact learning platform for embedded systems, PCB design, and custom HID applications. It features **3 push buttons** and **3 status LEDs**, making it a great starting point for custom controllers, simulators, macros, or automation projects.

The goal of this project was not only to build a functional button box, but also to gain hands-on experience with the complete hardware development workflow—from schematic design and PCB layout in KiCad to firmware development using the Arduino IDE.

---

## ✨ Features

* ✅ Seeed Studio XIAO RP2040 based
* ✅ Three momentary push buttons
* ✅ Three indicator LEDs with current limiting resistors
* ✅ Compact single-layer PCB design
* ✅ Easy to customize and expand
* ✅ Beginner-friendly hardware project
* ✅ Fully open source

---

## 🛠 Hardware Used

| Component                  | Quantity |
| -------------------------- | -------- |
| Seeed Studio XIAO RP2040   | 1        |
| Push Buttons               | 3        |
| LEDs                       | 3        |
| Current Limiting Resistors | 3        |
| Pin Headers                | 2        |
| Custom PCB                 | 1        |

---

# 📐 Hardware Overview

## PCB Layout

<img width="847" height="837" alt="Screenshot 2026-06-29 115307" src="https://github.com/user-attachments/assets/b28e03de-5013-4ace-ae16-28acd6300dcb" />


The PCB was designed in **KiCad** with simplicity and readability in mind. The XIAO RP2040 sits on the right side of the board, while the three push buttons occupy the lower section for comfortable access. Three LEDs with their respective resistors are positioned at the top as status indicators. The routing was intentionally kept clean to make the board easier to understand and modify.

---

## Schematic

<img width="757" height="346" alt="image" src="https://github.com/user-attachments/assets/cd31feba-68ab-420e-ab51-d4ba301bf879" />


The schematic connects each push button directly to a dedicated GPIO pin on the RP2040. Each LED is driven from its own GPIO through a current-limiting resistor. All switches share a common ground, keeping the circuit straightforward and easy to troubleshoot.

---

## 📂 Repository Structure

```text
.
├── Firmware/
│   └── Pathfinder_Button_Box.ino
├── Hardware/
│   ├── PCB/
│   ├── Schematic/
│   └── Gerbers/
├── Images/
├── README.md
└── LICENSE
```

---

## 🚀 Getting Started

### 1. Clone the repository

```bash
git clone <repository-url>
```

### 2. Open the firmware

Open the Arduino sketch using **Arduino IDE 2.x**.

### 3. Select the board

```
Seeed Studio XIAO RP2040
```

### 4. Upload the code

Connect your board via USB and upload the firmware.

---

## 🔌 GPIO Mapping

| GPIO | Function |
| ---- | -------- |
| GP26 | Button 1 |
| GP27 | Button 2 |
| GP28 | Button 3 |
| GP29 | LED 1    |
| GP6  | LED 2    |
| GP7  | LED 3    |

---

## 🎯 Possible Applications

* Racing Simulator Button Box
* Flight Simulator Controls
* Macro Pad
* Stream Deck Alternative
* Robotics Controller
* Home Automation Interface
* Custom USB HID Device
* Learning Embedded Systems

---

## 📚 Software Used

* KiCad
* Arduino IDE
* Seeed Studio XIAO RP2040 Core

---


---

## 📄 License

This project is released under the **MIT License**.



