/**
 ******************************************************************************
 * @file    vl53l0x_i2c_platform.c
 * @brief   VL53L0X I2C Platform implementation for STM32 HAL
 ******************************************************************************
 */

#include "vl53l0x_i2c_platform.h"
#include "vl53l0x_def.h"
#include "vl53l0x_platform_log.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include <string.h>

/* External I2C handle - defined in main.c */
extern I2C_HandleTypeDef hi2c3;

#define VL53L0X_I2C_TIMEOUT 100  /* I2C timeout in milliseconds */
#define VL53L0X_MAX_I2C_XFER_SIZE 64  /* Maximum I2C transfer size */

char debug_string[VL53L0X_MAX_STRING_LENGTH_PLT];

/**
 * @brief  Writes a single byte to VL53L0X device
 */
int32_t VL53L0X_write_byte(uint8_t address, uint8_t index, uint8_t data)
{
    uint8_t buffer[2];
    buffer[0] = index;
    buffer[1] = data;
    
    if (HAL_I2C_Master_Transmit(&hi2c3, address, buffer, 2, VL53L0X_I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

/**
 * @brief  Writes a 16-bit word to VL53L0X device
 */
int32_t VL53L0X_write_word(uint8_t address, uint8_t index, uint16_t data)
{
    uint8_t buffer[3];
    buffer[0] = index;
    buffer[1] = (data >> 8) & 0xFF;  // MSB first
    buffer[2] = data & 0xFF;
    
    if (HAL_I2C_Master_Transmit(&hi2c3, address, buffer, 3, VL53L0X_I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

/**
 * @brief  Writes a 32-bit double word to VL53L0X device
 */
int32_t VL53L0X_write_dword(uint8_t address, uint8_t index, uint32_t data)
{
    uint8_t buffer[5];
    buffer[0] = index;
    buffer[1] = (data >> 24) & 0xFF;
    buffer[2] = (data >> 16) & 0xFF;
    buffer[3] = (data >> 8) & 0xFF;
    buffer[4] = data & 0xFF;
    
    if (HAL_I2C_Master_Transmit(&hi2c3, address, buffer, 5, VL53L0X_I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

/**
 * @brief  Writes multiple bytes to VL53L0X device
 */
int32_t VL53L0X_write_multi(uint8_t address, uint8_t index, uint8_t *pdata, int32_t count)
{
    uint8_t buffer[VL53L0X_MAX_I2C_XFER_SIZE + 1];
    
    if (count > VL53L0X_MAX_I2C_XFER_SIZE) {
        return -1;  // Buffer overflow protection
    }
    
    buffer[0] = index;
    memcpy(&buffer[1], pdata, count);
    
    if (HAL_I2C_Master_Transmit(&hi2c3, address, buffer, count + 1, VL53L0X_I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

/**
 * @brief  Reads a single byte from VL53L0X device
 */
int32_t VL53L0X_read_byte(uint8_t address, uint8_t index, uint8_t *pdata)
{
    if (HAL_I2C_Master_Transmit(&hi2c3, address, &index, 1, VL53L0X_I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    
    if (HAL_I2C_Master_Receive(&hi2c3, address, pdata, 1, VL53L0X_I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

/**
 * @brief  Reads a 16-bit word from VL53L0X device
 */
int32_t VL53L0X_read_word(uint8_t address, uint8_t index, uint16_t *pdata)
{
    uint8_t buffer[2];
    
    if (HAL_I2C_Master_Transmit(&hi2c3, address, &index, 1, VL53L0X_I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    
    if (HAL_I2C_Master_Receive(&hi2c3, address, buffer, 2, VL53L0X_I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    
    *pdata = (buffer[0] << 8) | buffer[1];  // MSB first
    return 0;
}

/**
 * @brief  Reads a 32-bit double word from VL53L0X device
 */
int32_t VL53L0X_read_dword(uint8_t address, uint8_t index, uint32_t *pdata)
{
    uint8_t buffer[4];
    
    if (HAL_I2C_Master_Transmit(&hi2c3, address, &index, 1, VL53L0X_I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    
    if (HAL_I2C_Master_Receive(&hi2c3, address, buffer, 4, VL53L0X_I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    
    *pdata = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) | 
             ((uint32_t)buffer[2] << 8) | buffer[3];
    return 0;
}

/**
 * @brief  Reads multiple bytes from VL53L0X device
 */
int32_t VL53L0X_read_multi(uint8_t address, uint8_t index, uint8_t *pdata, int32_t count)
{
    if (count > VL53L0X_MAX_I2C_XFER_SIZE) {
        return -1;  // Buffer overflow protection
    }
    
    if (HAL_I2C_Master_Transmit(&hi2c3, address, &index, 1, VL53L0X_I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    
    if (HAL_I2C_Master_Receive(&hi2c3, address, pdata, count, VL53L0X_I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}
