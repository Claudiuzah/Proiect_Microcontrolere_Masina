/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
/* Motor control functions */
void Motor_Init(void);
void Motor_SetSpeed(char motor, int16_t speed);
void Motor_Stop(void);
void Motor_Forward(uint16_t speed);
void Motor_Backward(uint16_t speed);
void Motor_TurnLeft(uint16_t speed);
void Motor_TurnRight(uint16_t speed);
void Motor_RotateLeft(uint16_t speed);
void Motor_RotateRight(uint16_t speed);

/* VL53L0X Sensor functions */
void Setup_Multi_Sensors(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define VSYNC_FREQ_Pin GPIO_PIN_2
#define VSYNC_FREQ_GPIO_Port GPIOE
#define RENDER_TIME_Pin GPIO_PIN_3
#define RENDER_TIME_GPIO_Port GPIOE
#define FRAME_RATE_Pin GPIO_PIN_4
#define FRAME_RATE_GPIO_Port GPIOE
#define MCU_ACTIVE_Pin GPIO_PIN_5
#define MCU_ACTIVE_GPIO_Port GPIOE
#define SPI5_NCS_Pin GPIO_PIN_1
#define SPI5_NCS_GPIO_Port GPIOC
#define XSHUT_LEFT_Pin GPIO_PIN_3
#define XSHUT_LEFT_GPIO_Port GPIOC
#define USER_Pin GPIO_PIN_0
#define USER_GPIO_Port GPIOA
#define XSHUT_FRONT_Pin GPIO_PIN_1
#define XSHUT_FRONT_GPIO_Port GPIOA
#define XSHUT_RIGHT_Pin GPIO_PIN_5
#define XSHUT_RIGHT_GPIO_Port GPIOA
#define MOTOR_A_DIR_Pin GPIO_PIN_10
#define MOTOR_A_DIR_GPIO_Port GPIOC
#define MOTOR_B_DIR_Pin GPIO_PIN_11
#define MOTOR_B_DIR_GPIO_Port GPIOC
#define LD3_Pin GPIO_PIN_13
#define LD3_GPIO_Port GPIOG
#define LD4_Pin GPIO_PIN_14
#define LD4_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */
/* Motor control defines */
#define MOTOR_MAX_SPEED 999
#define MOTOR_MIN_SPEED 0

/* Motor configuration */
#define MOTOR_A_PWM_CHANNEL TIM_CHANNEL_1  // PB4
#define MOTOR_B_PWM_CHANNEL TIM_CHANNEL_2  // PA7

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
