#include "stm32_crc.h"

uint32_t stm32_crc32(const uint8_t *data, size_t length)
{
  uint32_t crc = 0xFFFFFFFF; // Initial val
  for (size_t i = 0; i < length; i += 4)
  {
    // Swap the endianness to little endian
    uint32_t word =
          ((uint32_t)data[i])
        | ((uint32_t)data[i+1] << 8)
        | ((uint32_t)data[i+2] << 16)
        | ((uint32_t)data[i+3] << 24);
    crc ^= word; // XOR word with 0xFFFFFFFF
    for (uint8_t bit = 0; bit < 32; bit++)
    {
      // Check if MSB == 1 
      if (crc & 0x80000000) 
        crc = (crc << 1) ^ STM32_CRC_POLY; // CRC polynomial used by STM32F411RE (Page 67 in the RM0383)
      else
        crc <<= 1; // Else shift 1 
    }
  }
  return crc;
}