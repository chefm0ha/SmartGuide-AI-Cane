# Required Libraries

## Core Libraries
- [Arduino Core for ESP32](https://github.com/espressif/arduino-esp32) - Base ESP32 functionality
- [TensorFlow Lite for Microcontrollers](https://github.com/tensorflow/tflite-micro) - For running the AI model
- [TinyGPS++](https://github.com/mikalhart/TinyGPSPlus) - For processing GPS data
- [ArduinoJson](https://arduinojson.org/) - For handling JSON data in the mapping system
- [SD](https://www.arduino.cc/en/Reference/SD) - For SD card operations to store map and waypoint data

## Installation
1. Open Arduino IDE
2. Go to Tools > Manage Libraries...
3. Search for and install each of the required libraries

## Notes
- TensorFlow Lite for Microcontrollers may require additional setup. Follow the official [TensorFlow documentation](https://www.tensorflow.org/lite/microcontrollers) for installation.
- For ESP32, ensure you have the board definition installed through Boards Manager