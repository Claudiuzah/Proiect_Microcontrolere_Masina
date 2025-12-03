/**
 ******************************************************************************
 * @file    vl53l0x_app.h
 * @brief   VL53L0X Application wrapper for STM32 (Pololu library)
 ******************************************************************************
 */

#ifndef VL53L0X_APP_H
#define VL53L0X_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "VL53L0X.h"
#include "stm32f4xx_hal.h"

/* VL53L0X I2C addresses */
#define VL53L0X_DEFAULT_ADDRESS    0x52  // 7-bit address (0x29 << 1)

/* Sensor positions */
typedef enum {
    VL53_FRONT = 0,
    VL53_LEFT = 1,
    VL53_RIGHT = 2,
    VL53_NUM_SENSORS = 3
} VL53L0X_Sensor_t;

/* New I2C addresses for each sensor */
#define VL53L0X_FRONT_ADDR     0x54  // 0x2A << 1
#define VL53L0X_LEFT_ADDR      0x56  // 0x2B << 1
#define VL53L0X_RIGHT_ADDR     0x58  // 0x2C << 1

/* Sensor data structure */
typedef struct {
    uint8_t i2c_address;       // Current I2C address
    uint16_t distance_mm;      // Distance in millimeters
    uint8_t status;            // Measurement status
    uint32_t last_update;      // Timestamp of last update
    uint8_t initialized;       // Initialization flag
} VL53L0X_AppData_t;

/* Function prototypes */
void VL53L0X_App_SetXSHUT(VL53L0X_Sensor_t sensor, uint8_t state);
bool VL53L0X_App_InitAll(I2C_HandleTypeDef *hi2c);
void VL53L0X_App_UpdateAllDistances(void);
VL53L0X_AppData_t* VL53L0X_App_GetData(VL53L0X_Sensor_t sensor);

#ifdef __cplusplus
}
#endif

#endif /* VL53L0X_APP_H */
