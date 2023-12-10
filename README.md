# ESP8266 Temperature Alert System

This project is an IoT-based temperature alert system using an ESP8266. It monitors the temperature inside and provides alerts and real-time data through a web interface. The system uses a buzzer and LED for local alerts and integrates with IFTTT for notifications.

I have been wanting to mess around with ESP microcontrollers for a while now, so I built this to use in my greenhouse and learn how the ESP devices work.

[![ESP8266 Temp Alert](http://img.youtube.com/vi/KhjjuUKmlEc/0.jpg)](https://www.youtube.com/shorts/KhjjuUKmlEc "Greenhouse Temp Alert System Demo")


## Features

- Real-time temperature monitoring
- Web server for real-time data display and control
- Adjustable temperature threshold for alerts
- Local alerts using a buzzer and LED
- Integration with IFTTT for remote notifications
- EEPROM storage for threshold persistence across reboots

## Hardware Components

- Adafruit Feather HUZZAH ESP8266 (any ESP8266 device should work)
- DHT20/AHT20 Temperature and Humidity Sensor
- Piezo Buzzer
- LED
- Resistors for the LED
- Breadboard and connecting wires

## GPIO Pin Configuration

- DHT20/AHT20 Sensor: Connected to any available GPIO (e.g., GPIO4)
- Buzzer: Connected to GPIO12
- LED: Connected to GPIO13 (with a suitable resistor)

## Software Requirements

- Arduino IDE
- ESP8266 Board Package in Arduino IDE
- Libraries: Adafruit_AHTX0, ESP8266WiFi, ESP8266WebServer, EEPROM

## Setup Instructions

1. **Hardware Setup**:
   - Connect the DHT20/AHT20 sensor, buzzer, and LED to the ESP8266 according to the GPIO configuration.
   - Ensure the LED is connected with a current-limiting resistor.
   - ![PXL_20231203_011112851](https://github.com/ludothegreat/ESP8266_temp_alert/assets/131192727/6c6aa859-f013-4abc-b2af-30ceb105accc)


2. **Software Setup**:
   - Open the project code in Arduino IDE.
   - Update the Wi-Fi credentials to connect to your network.
   - Flash the code to the ESP8266.

3. **Using the System**:
   - Once powered, the ESP8266 will start monitoring the temperature.
   - Access the web interface via the ESP8266's IP address for real-time data and controls.
        - ![image](https://github.com/ludothegreat/ESP8266_temp_alert/assets/131192727/5140e09c-7b02-41eb-be35-b25e3738e3ab)
   - Adjust the temperature threshold as needed.
   - The buzzer and LED will alert locally when the temperature falls below the threshold.
   - IFTTT notifications will be sent for remote alerts.

## Notes

- Ensure your network's firewall settings allow you to access the ESP8266's IP address.
- For security reasons, avoid exposing the ESP8266 to the internet without proper security measures.

## Future Enhancements

- Integration with additional sensors (e.g., humidity, light intensity)
- Additional remote alert devices
- Implementing OTA (Over-The-Air) firmware updates
- Enhanced power management for battery operation
- Security improvements for the web interface
