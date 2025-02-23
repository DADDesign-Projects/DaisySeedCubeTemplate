/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32h7xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_7
#define LED_GPIO_Port GPIOC
#define RESET_CODEC_Pin GPIO_PIN_11
#define RESET_CODEC_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
/* Sections ---------------------------------------------------------*/
#define SDRAM_SECTION __attribute__((section(".SDRAM_Section")))
#define QFLASH_SECTION __attribute__((section(".QFLASH_Section")))
#define NO_CACHE_RAM __attribute__((section(".RAM_NO_CACHE_Section")))
#define ITCM __attribute__((section(".moveITCM")))

/* Audio ---------------------------------------------------------*/
#define AUDIO_BUFFER_SIZE 4
struct AudioBuffer{
	float Right;
	float Left;
};
extern HAL_StatusTypeDef StartAudio();

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
