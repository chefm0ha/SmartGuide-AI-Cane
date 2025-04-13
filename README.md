# SmartGuide: AI-Assisted Navigation Cane

<p align="center">
  <img src="docs/images/cane_prototype.jpg" alt="SmartGuide Prototype" width="400"/>
</p>

## Overview
SmartGuide is an intelligent assistive device that transforms the traditional white cane into a sophisticated navigation system for visually impaired individuals. By integrating AI algorithms with ultrasonic sensors and GPS, the device detects and classifies obstacles, builds environmental maps, and provides intuitive navigation guidance.

This project was developed for the MARC Hackathon 2025 at ENSIAS, Rabat, focusing on the theme "AI and Robots in Human's Service."

## Key Features
- **Dual Ultrasonic Sensor System**: Detects obstacles at different heights with 92% accuracy
- **AI-powered Obstacle Classification**: Identifies common obstacle types (walls, people, furniture, etc.)
- **Intelligent Path Mapping**: Creates and remembers routes for improved navigation
- **Intuitive Feedback System**: Provides haptic and audio guidance
- **GPS Navigation Assistance**: Offers turn-by-turn directions to destinations

## Repository Structure
- `/docs`: Technical documentation and user manuals
- `/hardware`: Schematics and parts list
- `/src`: Source code for the Arduino project
- `/src/ai_model`: TensorFlow Lite model and training code
- `/test`: Test cases for different system components

## Hardware Requirements
- ESP32 Microcontroller
- HC-SR04 Ultrasonic Sensors (2x)
- NEO-6M GPS Module
- Vibration Motors (3x)
- 3.7V 2000mAh Li-ion Battery
- Speaker for Audio Feedback

## Software Dependencies
- Arduino IDE 2.0+
- TensorFlow Lite for Microcontrollers
- TinyGPS++ Library
- ArduinoJson Library
- SD Card Library

## Getting Started
1. Clone this repository
2. Install the required libraries (see `src/libraries.md`)
3. Connect the hardware components according to the schematics
4. Upload the code to your ESP32
5. Follow the user manual for calibration and usage instructions

## Team
- Mohammed Fadlouallah
- Yassir Amraoui

## License
This project is licensed under the MIT License - see the LICENSE file for details.