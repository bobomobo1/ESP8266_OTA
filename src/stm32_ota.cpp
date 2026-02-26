#include "stm32_ota.h"
#include "stm32_crc.h"
#include <FS.h>

void stm32_start_ota() {
  extern ESP8266WebServer server;
  File firmware = SPIFFS.open("/firmware.bin", "r"); // TODO: Needs to be different then a static name  
  if (!firmware) {
    server.send(500, "text/plain", "Firmware file not found");
    return;
  }
  uint8_t buffer[TX_DATA_SIZE];
  uint16_t packetNumber = 0;
  Serial.println("Starting OTA transfer to STM32...");
  while (firmware.available()) {
    size_t len = firmware.read(buffer, TX_DATA_SIZE);
    // If last packet is smaller than 128 bytes
    if (len < TX_DATA_SIZE) {
        memset(&buffer[len], 0x00, TX_DATA_SIZE - len); // Fill with trailing 0x00
    }
    uint32_t crc = stm32_crc32(buffer, TX_DATA_SIZE); 
    // Clear RX buffer BEFORE sending
    while (Serial.available()) {
      Serial.read();
    }
    /*
      Start of packet code
    */
    // START delimiter
    Serial.write(TX_START_DELIM_1);
    Serial.write(TX_START_DELIM_2);
    // HEADER
    Serial.write((uint8_t*)&packetNumber, sizeof(packetNumber));
    uint8_t dataLen = len;
    Serial.write(dataLen);
    // CHUNK
    Serial.write(buffer, TX_DATA_SIZE);
    // CRC
    Serial.write((uint8_t*)&crc, sizeof(crc));
    // END delimiter
    Serial.write(TX_END_DELIM_1);
    Serial.write(TX_END_DELIM_2);
    Serial.flush();   // Ensure transmission finished
    packetNumber++;
    // Wait for 1-byte ACK from STM32
    unsigned long timeout = millis();
    bool ackReceived = false;
    while (millis() - timeout < MAX_RX_TIMEOUT) {
      if (Serial.available()) {
        uint8_t ack = Serial.read();
        if (ack == RX_RESPONSE_ACK) {
          ackReceived = true;
        }
        break;
      }
    }
    if (!ackReceived) {
      firmware.close();
      server.send(500, "text/plain", "ACK timeout or invalid ACK");
      return;
    }
    // Now safely continue to next chunk
  }
  packetNumber = 0;
  firmware.close();
  server.send(200, "text/plain", "OTA completed successfully");
}