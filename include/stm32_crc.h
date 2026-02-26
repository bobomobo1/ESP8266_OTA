#ifndef STM32_CRC_H
#define STM32_CRC_H

#include <stdint.h>
#include <stddef.h>

#define STM32_CRC_POLY 0x04C11DB7 // CRC polynomial used by STM32F411RE (Page 67 in the RM0383)

uint32_t stm32_crc32(const uint8_t *data, size_t length);

#endif