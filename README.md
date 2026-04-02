# ESP8266 OTA Uploader
### Course: EECE8041 Engineering Capstone Project

This firmware turns an **ESP8266** into a Wi‑Fi connected OTA uploader for STM32 devices. It connects the ESP to a Wi‑Fi network, hosts an OTA server, and provides a web interface that lets you send firmware images from a browser to an STM32F411RE.

### Features

- **Wi‑Fi OTA Server:** ESP8266 connects to your network and serves an OTA interface via HTTP.  
- **Browser Upload UI:** Upload STM32 firmware binary files through a simple web page hosted on the ESP.  
- **STM32 Packet Transport:** Handles framing and UART communication between the ESP and your STM32 OTA bootloader.  
- **SPIFFS:** Stores the uploaded firmware images, so they can be sent over UART.  
- **Modular Components:** Includes OTA server logic, web page definitions, and STM32 UART packet handling.  

### How it Works

1. **Connects to Wi‑Fi:** ESP attempts to join your configured SSID and password.  
2. **Starts OTA Server:** Once connected, it initializes an `ESP8266WebServer` to listen for incoming firmware upload requests.  
3. **Web UI:** Clients connect via browser to a provided upload page to send new firmware images.  
4. **UART Forwarding:** Received binary data is packaged and forwarded to the STM32 bootloader with the required delimiters and protocol expected by the STM32 side.

### Usage

1. Edit the Wi‑Fi credentials (`ssid` and `password`) in `src/main.cpp`.  
2. Build and flash the ESP firmware using PlatformIO or Arduino IDE.  
3. Connect your ESP8266 board to power and let it join your Wi‑Fi network.  
4. Open the ESP’s IP in a browser to access the OTA upload page and send your `.bin` file destined for the STM32.

### Dependencies

- ESP8266 Arduino Core (for Wi‑Fi/server support)  
- SPIFFS to store and serve static web assets  
- Web server routines to handle file uploads

This tool bridges your PC/browser and the STM32 OTA bootloader, enabling convenient wireless firmware delivery using an ESP8266 as a network‑to‑serial OTA gateway.  