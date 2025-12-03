/**
 ******************************************************************************
 * @file    vl53l0x_app.c
 * @brief   VL53L0X Application wrapper using Pololu library
 ******************************************************************************
 */

#include "vl53l0x_app.h"
#include "main.h"
#include <stdio.h>

/* Include Pololu VL53L0X implementation directly */
#include "VL53L0X.c"

/* Global sensor data */
static VL53L0X_AppData_t sensors[VL53_NUM_SENSORS] = {0};
static I2C_HandleTypeDef *g_hi2c = NULL;

/**
 * @brief  Control XSHUT pin for a specific sensor
 * @param  sensor: Sensor identifier (FRONT, LEFT, RIGHT)
 * @param  state: 0=LOW (shutdown), 1=HIGH (active)
 */
void VL53L0X_App_SetXSHUT(VL53L0X_Sensor_t sensor, uint8_t state)
{
    GPIO_PinState pin_state = state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    
    switch(sensor) {
        case VL53_FRONT:
            HAL_GPIO_WritePin(VL53_FRONT_XSHUT_GPIO_Port, VL53_FRONT_XSHUT_Pin, pin_state);
            break;
        case VL53_LEFT:
            HAL_GPIO_WritePin(VL53_LEFT_XSHUT_GPIO_Port, VL53_LEFT_XSHUT_Pin, pin_state);
            break;
        case VL53_RIGHT:
            HAL_GPIO_WritePin(VL53_RIGHT_XSHUT_GPIO_Port, VL53_RIGHT_XSHUT_Pin, pin_state);
            break;
        default:
            break;
    }
}

/**
 * @brief  Initialize all three VL53L0X sensors with address reassignment
 * @param  hi2c: Pointer to I2C handle
 * @retval true if all sensors initialized successfully, false otherwise
 */
bool VL53L0X_App_InitAll(I2C_HandleTypeDef *hi2c)
{
    g_hi2c = hi2c;
    
    /* Set timeout for sensor operations */
    setTimeout(500);
    
    /* Shut down all sensors first */
    VL53L0X_App_SetXSHUT(VL53_FRONT, 0);
    VL53L0X_App_SetXSHUT(VL53_LEFT, 0);
    VL53L0X_App_SetXSHUT(VL53_RIGHT, 0);
    HAL_Delay(100);
    
    /* Initialize FRONT sensor with unique address */
    printf("F...");
    VL53L0X_App_SetXSHUT(VL53_FRONT, 1);
    HAL_Delay(50);
    
    if (!initVL53L0X(true, hi2c)) {
        printf("FAIL ");
        sensors[VL53_FRONT].initialized = 0;
        sensors[VL53_FRONT].status = 0xFF;
    } else {
        setAddress_VL53L0X(VL53L0X_FRONT_ADDR);
        sensors[VL53_FRONT].i2c_address = VL53L0X_FRONT_ADDR;
        startContinuous(0);
        sensors[VL53_FRONT].initialized = 1;
        sensors[VL53_FRONT].distance_mm = 0;
        sensors[VL53_FRONT].status = 0;
        printf("OK ");
    }
    
    /* Initialize LEFT sensor with unique address */
    printf("L...");
    VL53L0X_App_SetXSHUT(VL53_LEFT, 1);
    HAL_Delay(50);
    
    if (!initVL53L0X(true, hi2c)) {
        printf("FAIL ");
        sensors[VL53_LEFT].initialized = 0;
        sensors[VL53_LEFT].status = 0xFF;
    } else {
        setAddress_VL53L0X(VL53L0X_LEFT_ADDR);
        sensors[VL53_LEFT].i2c_address = VL53L0X_LEFT_ADDR;
        startContinuous(0);
        sensors[VL53_LEFT].initialized = 1;
        sensors[VL53_LEFT].distance_mm = 0;
        sensors[VL53_LEFT].status = 0;
        printf("OK ");
    }
    
    /* Initialize RIGHT sensor with unique address */
    printf("R...");
    VL53L0X_App_SetXSHUT(VL53_RIGHT, 1);
    HAL_Delay(50);
    
    if (!initVL53L0X(true, hi2c)) {
        printf("FAIL ");
        sensors[VL53_RIGHT].initialized = 0;
        sensors[VL53_RIGHT].status = 0xFF;
    } else {
        setAddress_VL53L0X(VL53L0X_RIGHT_ADDR);
        sensors[VL53_RIGHT].i2c_address = VL53L0X_RIGHT_ADDR;
        startContinuous(0);
        sensors[VL53_RIGHT].initialized = 1;
        sensors[VL53_RIGHT].distance_mm = 0;
        sensors[VL53_RIGHT].status = 0;
        printf("OK ");
    }
    
    
    printf("\r\n");
    return true;
}

/**
 * @brief  Update distances from all initialized sensors
 */
void VL53L0X_App_UpdateAllDistances(void)
{
    for (int i = 0; i < VL53_NUM_SENSORS; i++) {
        if (!sensors[i].initialized) {
            sensors[i].distance_mm = 0;
            sensors[i].status = 0xFF;
            continue;
        }
        
        /* Set current sensor address */
        setAddress_VL53L0X(sensors[i].i2c_address);
        
        /* Read distance (non-blocking, reads latest available measurement) */
        uint16_t distance = readRangeContinuousMillimeters(NULL);
        
        /* Check for timeout */
        if (timeoutOccurred()) {
            sensors[i].status = 0xFE;  // Timeout error
        } else {
            sensors[i].distance_mm = distance;
            sensors[i].status = 0;  // Success
            sensors[i].last_update = HAL_GetTick();
        }
    }
}

/**
 * @brief  Get pointer to sensor data
 * @param  sensor: Sensor identifier
 * @retval Pointer to sensor data structure
 */
VL53L0X_AppData_t* VL53L0X_App_GetData(VL53L0X_Sensor_t sensor)
{
    if (sensor < VL53_NUM_SENSORS) {
        return &sensors[sensor];
    }
    return NULL;
}
