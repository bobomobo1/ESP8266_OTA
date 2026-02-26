#ifndef STM32_OTA_H
#define STM32_OTA_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define MAX_RX_TIMEOUT        3000
#define TX_START_DELIM_1      0x7E
#define TX_START_DELIM_2      0x7E
#define TX_END_DELIM_1        0x7F
#define TX_END_DELIM_2        0x7F
#define TX_DELIMITER_SIZE     2
#define TX_HEADER_SIZE        3 // 2 Bytes(uint16) for packet # and 1 byte(uint8) for packet size
#define TX_CRC_SIZE           4
#define TX_DATA_SIZE          128
#define TX_CHUNK_SIZE         TX_DELIMITER_SIZE + TX_HEADER_SIZE + TX_DATA_SIZE + TX_CRC_SIZE + TX_DELIMITER_SIZE
#define RX_RESPONSE_ACK       0x79

void stm32_start_ota();

#endif