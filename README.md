🐾 Smart Cat Litter Environmental Monitor

A sensor-based system for monitoring cat litter box usage and odor levels.

📌 Overview

This project is a smart environmental monitoring system designed for indoor cats. It combines behavior tracking and air-quality sensing to:

Monitor how many times a cat uses the litter box (full entry + full exit)

Automatically detect odor levels using an MQ135 gas sensor

Control a ventilation fan to reduce ammonia and unpleasant smells

Trigger an alarm when odor becomes severe

Display only the daily usage count on a small OLED screen

Output full system data to the serial monitor for debugging

This system improves feline welfare by monitoring behavioral patterns (which are linked to urinary tract health) and maintaining a cleaner, healthier indoor environment.

🎯 Motivation

Cat litter odor is produced by the bacterial breakdown of urea and other nitrogen compounds, including ammonia, which affects indoor air quality (Trujillo & Dimas-Correa, 2025).
Changes in urination patterns—such as increased frequency (pollakiuria)—are common clinical signs of feline lower urinary tract disease (FLUTD) (Little, 2011).

Monitoring both litter box usage and odor levels therefore supports early detection of urinary issues and improves environmental hygiene.

🛠 Hardware Components
Component	Purpose
Arduino MKR WiFi 1010	Microcontroller
IR Break-Beam Sensor	Detects entry & exit cycles
MQ135 Gas Sensor	Detects ammonia/VOC odor levels
Relay Module (CW-019, HIGH-trigger)	Switches fan power
5V DC Fan	Ventilation
Active Buzzer	High-odor alarm
SSD1306 0.96" OLED	Displays daily usage count
Jumper wires & breadboard	Connections

🔌 Wiring
<img width="1981" height="2156" alt="CatLitter_bb" src="https://github.com/user-attachments/assets/a37dd710-8966-45bc-847f-5328febbc929" />

🔍 System Logic
✔ Cat Usage Detection (Two-Cycle Method)

A full toilet use requires:

Entry cycle: CLEAR → BLOCKED → CLEAR  
Exit cycle:  CLEAR → BLOCKED → CLEAR  


Only when both cycles complete does the system increment the daily usage counter.
This prevents false triggers from tail movement, hesitation, or partial entry.

✔ Odor-Based Fan & Alarm Control
MQ135 Value	Fan	Buzzer
≤ 300	OFF	OFF
301–499	ON	OFF
≥ 500	ON	ON

All detailed sensor values are printed to the serial monitor.
