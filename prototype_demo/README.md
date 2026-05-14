# Smart Pill Dispenser Prototype Demo

This prototype demonstrates the core functionality of the smart pill dispenser system designed for elderly users, especially dementia patients.

## Prototype Features

- 7 LED indicators representing weekly medication sections
- LCD display showing medication status
- Countdown system for next medication intake
- Buzzer alarm notification
- Push button acknowledgement system
- ESP32 based embedded control system

## Prototype Demonstration Logic

### Initial State

LCD displays:

Breakfast done
Lunch in 20 sec

The LEDs continuously blink in sequence every 3 seconds to simulate weekly medication compartments.

---

### Alarm State

After 20 seconds:

- LCD changes to:
  
  Time for lunch
  Press button

- Buzzer alarm activates periodically

---

### Confirmation State

When the push button is pressed:

- Alarm stops
- Medication is marked as taken
- LCD updates to:

  Lunch pill taken
  Next pill:1 hr

This demonstrates how the system helps dementia patients avoid double medication intake.

---

## Hardware Used

- ESP32 Dev Module
- DS1302 RTC Module
- I2C 16x2 LCD
- Active/Passive Buzzer
- Push Button
- 7 LEDs
- Breadboards and jumper wires

---

## Prototype Purpose

This prototype validates the user interaction flow and embedded control logic before full mechanical integration into the final cylindrical pill dispenser design.
