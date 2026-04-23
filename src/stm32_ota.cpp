#include "stm32_ota.h"
#include "stm32_crc.h"
#include <FS.h>

void stm32_start_ota() {
  extern ESP8266WebServer server;
  File firmware = SPIFFS.open("/F411RE_Binaries.bin", "r"); // TODO: Needs to be different then a static name  
  if (!firmware) {
    server.send(500, "text/plain", "Firmware file not found");
    return;
  }
  size_t fileSize = firmware.size();
  uint16_t totalPackets = fileSize / TX_DATA_SIZE;
  uint8_t buffer[TX_DATA_SIZE];
  uint16_t packetNumber = 0;
  Serial.println("Starting OTA transfer to STM32...");
  // First we need to tell the STM we are about to send an update and that we expect a response saying we are in the bootloader
  // START delimiter
  Serial.write(TX_START_DELIM_1);
  Serial.write(TX_START_DELIM_2);
  // Send UPDATE INCOMING to stm32
  Serial.write(TX_START_OTA_HEX);
  Serial.write(TX_START_OTA_HEX);
  Serial.write(TX_START_OTA_HEX);
  Serial.write(TX_START_OTA_HEX);
  Serial.write(TX_START_OTA_HEX);
  Serial.write(TX_START_OTA_HEX);
  Serial.write(TX_START_OTA_HEX);
  // END delimiter
  Serial.write(TX_END_DELIM_1);
  Serial.write(TX_END_DELIM_2);
  // Wait for STM32 response
  unsigned long timeout = millis();
  bool ready = false;
  while (millis() - timeout < MAX_RX_TIMEOUT) {
    if (Serial.available()) {
      uint8_t resp = Serial.read();
      if (resp == RX_RESPONSE_READY) {
        ready = true;
      }
      break;
    }
  }
  if (!ready) {
    firmware.close();
    server.send(500, "text/plain", "STM32 not responding to OTA start");
    return;
  }
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
    bool success = send_packet_with_retry(packetNumber, totalPackets, buffer, crc);
    if (!success) {
      firmware.close();
      server.send(500, "text/plain", "Packet failed after retries");
      return;
    }
    packetNumber++; 
    // Now safely continue to next chunk
  }
  packetNumber = 0;
  firmware.close();
  server.send(200, "text/plain", "OTA completed successfully");
}

bool send_packet_with_retry(uint16_t packetNumber, uint8_t totalPackets, uint8_t *buffer, uint32_t crc){
  for (int attempt = 0; attempt < MAX_RETRIES; attempt++){
    // Clear RX buffer BEFORE sending
    while (Serial.available()) {
      Serial.read();
    }
    // START delimiter
    Serial.write(TX_START_DELIM_1);
    Serial.write(TX_START_DELIM_2);
    // HEADER
    Serial.write((uint8_t*)&packetNumber, sizeof(packetNumber));
    Serial.write(totalPackets);
    // CHUNK
    Serial.write(buffer, TX_DATA_SIZE);
    // CRC
    Serial.write((uint8_t*)&crc, sizeof(crc));
    // END delimiter
    Serial.write(TX_END_DELIM_1);
    Serial.write(TX_END_DELIM_2);
    Serial.flush();
    // Wait for ACK/NACK
    unsigned long timeout = millis();
    while (millis() - timeout < MAX_RX_TIMEOUT){
      if (Serial.available()){
        uint8_t resp = Serial.read();
        if (resp == RX_RESPONSE_ACK){
          return true; 
        }
        else if (resp == RX_RESPONSE_NACK){
          break; // Retry
        } else if (resp == RX_STOP_SEND_ACK){
          return false;
        }
      }
    }
  }
  return false;
}


void stm32_load_bootloader(void){
  extern ESP8266WebServer server;
  // START delimiter
  Serial.write(TX_START_DELIM_1);
  Serial.write(TX_START_DELIM_2);
  // Send UPDATE INCOMING to stm32
  Serial.write(TX_START_OTA_HEX);
  Serial.write(TX_START_OTA_HEX);
  Serial.write(TX_START_OTA_HEX);
  Serial.write(TX_START_OTA_HEX);
  Serial.write(TX_START_OTA_HEX);
  Serial.write(TX_START_OTA_HEX);
  Serial.write(TX_START_OTA_HEX);
  // END delimiter
  Serial.write(TX_END_DELIM_1);
  Serial.write(TX_END_DELIM_2);
  Serial.flush();
  // Wait for STM32 to ACK bootloader entry
  unsigned long timeout = millis();
  bool ready = false;
  while (millis() - timeout < MAX_RX_TIMEOUT) {
    if (Serial.available()) {
      uint8_t resp = Serial.read();
      if (resp == RX_RESPONSE_READY) { // or RX_BOOTLOADER_ACK if defined
        ready = true;
        break;
      }
    }
  }
  if (ready) {
    server.send(200, "text/plain", "STM32 bootloader ready");
  } else {
    server.send(500, "text/plain", "STM32 failed to enter bootloader");
  }
}

void stm32_load_main(void){
  extern ESP8266WebServer server;
  // START delimiter
  Serial.write(TX_START_DELIM_1);
  Serial.write(TX_START_DELIM_2);
  // Send UPDATE INCOMING to stm32
  Serial.write(TX_START_BOOTLOADER_HEX);
  Serial.write(TX_START_BOOTLOADER_HEX);
  Serial.write(TX_START_BOOTLOADER_HEX);
  Serial.write(TX_START_BOOTLOADER_HEX);
  Serial.write(TX_START_BOOTLOADER_HEX);
  Serial.write(TX_START_BOOTLOADER_HEX);
  Serial.write(TX_START_BOOTLOADER_HEX);
  // END delimiter
  Serial.write(TX_END_DELIM_1);
  Serial.write(TX_END_DELIM_2);
  Serial.flush();
  // Wait for STM32 to ACK bootloader entry
  unsigned long timeout = millis();
  bool ready = false;
  while (millis() - timeout < MAX_RX_TIMEOUT) {
    if (Serial.available()) {
      uint8_t resp = Serial.read();
      if (resp == RX_RESPONSE_READY) { // or RX_BOOTLOADER_ACK if defined
        ready = true;
        break;
      }
    }
  }
  if (ready) {
    server.send(200, "text/plain", "STM32 main ready");
  } else {
    server.send(500, "text/plain", "STM32 failed to enter main");
  }
}