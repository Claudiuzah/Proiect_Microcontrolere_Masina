/**
 ******************************************************************************
 * @file    vl53l0x_app.c
 * @brief   VL53L0X Application wrapper for STM32
 ******************************************************************************
 */

#include "vl53l0x_app.h"
#include "main.h"
#include <string.h>

/* Sensor data storage */
static VL53L0X_AppData_t sensor_data[VL53_NUM_SENSORS];

/**
 * @brief  Control XSHUT pin for sensor enable/disable
 */
void VL53L0X_App_SetXSHUT(VL53L0X_Sensor_t sensor, uint8_t state)
{
    GPIO_PinState pin_state = state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    
    switch (sensor) {
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
 * @brief  Initialize single VL53L0X sensor
 */
static VL53L0X_Error VL53L0X_App_InitSensor(VL53L0X_AppData_t *sensor, uint8_t new_address)
{
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    VL53L0X_Dev_t *pDev = &sensor->device;
    uint32_t refSpadCount;
    uint8_t isApertureSpads;
    uint8_t VhvSettings;
    uint8_t PhaseCal;
    
    /* Initialize device structure */
    pDev->I2cDevAddr = VL53L0X_DEFAULT_ADDRESS;
    pDev->comms_type = 1;  // I2C
    pDev->comms_speed_khz = 400;  // 400kHz
    
    /* Data initialization */
    status = VL53L0X_DataInit(pDev);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    /* Get device info */
    VL53L0X_DeviceInfo_t DeviceInfo;
    status = VL53L0X_GetDeviceInfo(pDev, &DeviceInfo);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    /* Static init */
    status = VL53L0X_StaticInit(pDev);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    /* Change I2C address */
    status = VL53L0X_SetDeviceAddress(pDev, new_address);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    pDev->I2cDevAddr = new_address;
    
    /* SPADs calibration */
    status = VL53L0X_PerformRefSpadManagement(pDev, &refSpadCount, &isApertureSpads);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    /* Reference calibration */
    status = VL53L0X_PerformRefCalibration(pDev, &VhvSettings, &PhaseCal);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    /* Set device mode */
    status = VL53L0X_SetDeviceMode(pDev, VL53L0X_DEVICEMODE_SINGLE_RANGING);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    /* Set measurement timing budget (30ms) */
    status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(pDev, 30000);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    /* Set VCSEL pulse period */
    status = VL53L0X_SetVcselPulsePeriod(pDev, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    status = VL53L0X_SetVcselPulsePeriod(pDev, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);
    if (status != VL53L0X_ERROR_NONE) return status;
    
    sensor->initialized = 1;
    return VL53L0X_ERROR_NONE;
}

/**
 * @brief  Initialize all three VL53L0X sensors
 */
VL53L0X_Error VL53L0X_App_InitAll(I2C_HandleTypeDef *hi2c)
{
    VL53L0X_Error status;
    
    /* Initialize sensor data */
    memset(sensor_data, 0, sizeof(sensor_data));
    
    /* Step 1: Disable all sensors */
    VL53L0X_App_SetXSHUT(VL53_FRONT, 0);
    VL53L0X_App_SetXSHUT(VL53_LEFT, 0);
    VL53L0X_App_SetXSHUT(VL53_RIGHT, 0);
    HAL_Delay(10);
    
    /* Step 2: Initialize FRONT sensor */
    VL53L0X_App_SetXSHUT(VL53_FRONT, 1);
    HAL_Delay(10);
    status = VL53L0X_App_InitSensor(&sensor_data[VL53_FRONT], VL53L0X_FRONT_ADDR);
    if (status != VL53L0X_ERROR_NONE) {
        return status;
    }
    
    /* Step 3: Initialize LEFT sensor */
    VL53L0X_App_SetXSHUT(VL53_LEFT, 1);
    HAL_Delay(10);
    status = VL53L0X_App_InitSensor(&sensor_data[VL53_LEFT], VL53L0X_LEFT_ADDR);
    if (status != VL53L0X_ERROR_NONE) {
        return status;
    }
    
    /* Step 4: Initialize RIGHT sensor */
    VL53L0X_App_SetXSHUT(VL53_RIGHT, 1);
    HAL_Delay(10);
    status = VL53L0X_App_InitSensor(&sensor_data[VL53_RIGHT], VL53L0X_RIGHT_ADDR);
    if (status != VL53L0X_ERROR_NONE) {
        return status;
    }
    
    return VL53L0X_ERROR_NONE;
}

/**
 * @brief  Perform single ranging measurement
 */
static VL53L0X_Error VL53L0X_App_ReadDistance(VL53L0X_AppData_t *sensor)
{
    VL53L0X_Error status;
    VL53L0X_RangingMeasurementData_t RangingData;
    
    if (!sensor->initialized) {
        return VL53L0X_ERROR_CONTROL_INTERFACE;
    }
    
    /* Start measurement */
    status = VL53L0X_PerformSingleRangingMeasurement(&sensor->device, &RangingData);
    
    if (status == VL53L0X_ERROR_NONE) {
        sensor->distance_mm = RangingData.RangeMilliMeter;
        sensor->status = RangingData.RangeStatus;
        sensor->last_update = HAL_GetTick();
    }
    
    return status;
}

/**
 * @brief  Update all sensor distances
 */
VL53L0X_Error VL53L0X_App_UpdateAllDistances(void)
{
    VL53L0X_Error status;
    
    for (uint8_t i = 0; i < VL53_NUM_SENSORS; i++) {
        status = VL53L0X_App_ReadDistance(&sensor_data[i]);
        if (status != VL53L0X_ERROR_NONE) {
            sensor_data[i].status = 0xFF;  // Error indicator
        }
    }
    
    return VL53L0X_ERROR_NONE;
}

/**
 * @brief  Get sensor data pointer
 */
VL53L0X_AppData_t* VL53L0X_App_GetData(VL53L0X_Sensor_t sensor)
{
    if (sensor >= VL53_NUM_SENSORS) return NULL;
    return &sensor_data[sensor];
}
