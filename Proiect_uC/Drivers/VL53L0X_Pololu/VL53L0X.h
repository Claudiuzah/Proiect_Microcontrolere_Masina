#ifndef VL53L0X_h
#define VL53L0X_h

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Register addresses
#define SYSRANGE_START                              0x00
#define SYSTEM_SEQUENCE_CONFIG                      0x01
#define SYSTEM_INTERMEASUREMENT_PERIOD              0x04
#define SYSTEM_INTERRUPT_CONFIG_GPIO                0x0A
#define GPIO_HV_MUX_ACTIVE_HIGH                     0x84
#define SYSTEM_INTERRUPT_CLEAR                      0x0B
#define RESULT_INTERRUPT_STATUS                     0x13
#define RESULT_RANGE_STATUS                         0x14
#define I2C_SLAVE_DEVICE_ADDRESS                    0x8A
#define MSRC_CONFIG_CONTROL                         0x60
#define PRE_RANGE_CONFIG_VCSEL_PERIOD               0x50
#define PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI          0x51
#define PRE_RANGE_CONFIG_VALID_PHASE_LOW            0x56
#define PRE_RANGE_CONFIG_VALID_PHASE_HIGH           0x57
#define FINAL_RANGE_CONFIG_VCSEL_PERIOD             0x70
#define FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI        0x71
#define FINAL_RANGE_CONFIG_VALID_PHASE_LOW          0x47
#define FINAL_RANGE_CONFIG_VALID_PHASE_HIGH         0x48
#define FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT 0x44
#define GLOBAL_CONFIG_VCSEL_WIDTH                   0x32
#define GLOBAL_CONFIG_SPAD_ENABLES_REF_0            0xB0
#define GLOBAL_CONFIG_REF_EN_START_SELECT           0xB6
#define DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD         0x4E
#define DYNAMIC_SPAD_REF_EN_START_OFFSET            0x4F
#define POWER_MANAGEMENT_GO1_POWER_FORCE            0x80
#define VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV           0x89
#define ALGO_PHASECAL_LIM                           0x30
#define ALGO_PHASECAL_CONFIG_TIMEOUT                0x30
#define MSRC_CONFIG_TIMEOUT_MACROP                  0x46
#define OSC_CALIBRATE_VAL                           0xF8

#define ADDRESS_DEFAULT 0x52 // 0x29<<1

typedef enum { VcselPeriodPreRange, VcselPeriodFinalRange } vcselPeriodType;

typedef struct {
  uint8_t rangeStatus;
  uint16_t spadCnt;
  uint16_t signalCnt;
  uint16_t ambientCnt;
  uint16_t rawDistance;
} statInfo_t_VL53L0X;

typedef struct
{
  bool tcc, msrc, dss, pre_range, final_range;
} SequenceStepEnables;

typedef struct
{
  uint16_t pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks;
  uint16_t msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks;
  uint32_t msrc_dss_tcc_us,    pre_range_us,    final_range_us;
} SequenceStepTimeouts;

// Public functions
void setAddress_VL53L0X(uint8_t new_addr);
uint8_t getAddress_VL53L0X(void);
bool initVL53L0X(bool io_2v8, I2C_HandleTypeDef *handler);
bool setSignalRateLimit(float limit_Mcps);
float getSignalRateLimit(void);
bool setMeasurementTimingBudget(uint32_t budget_us);
uint32_t getMeasurementTimingBudget(void);
bool setVcselPulsePeriod(vcselPeriodType type, uint8_t period_pclks);
uint8_t getVcselPulsePeriod(vcselPeriodType type);
void startContinuous(uint32_t period_ms);
void stopContinuous(void);
uint16_t readRangeContinuousMillimeters(statInfo_t_VL53L0X *extraStats);
uint16_t readRangeSingleMillimeters(statInfo_t_VL53L0X *extraStats);
bool timeoutOccurred(void);
void setTimeout(uint16_t timeout);
uint16_t getTimeout(void);

// Helper functions
#define calcMacroPeriod(vcsel_period_pclks) ((((uint32_t)2304 * (vcsel_period_pclks) * 1655) + 500) / 1000)
#define decodeVcselPeriod(reg_val)      (((reg_val) + 1) << 1)
#define encodeVcselPeriod(period_pclks) (((period_pclks) >> 1) - 1)
#define startTimeout() (g_timeoutStartMs = HAL_GetTick())
#define checkTimeoutExpired() (g_ioTimeout > 0 && ((uint16_t)(HAL_GetTick() - g_timeoutStartMs) > g_ioTimeout))

// Global variables (extern declaration)
extern uint8_t g_i2cAddr;
extern uint16_t g_ioTimeout;
extern uint8_t g_isTimeout;
extern uint16_t g_timeoutStartMs;
extern uint8_t g_stopVariable;
extern uint32_t g_measTimBudUs;

#ifdef __cplusplus
}
#endif

#endif
