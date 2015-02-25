/**
  ******************************************************************************
  * @file    RC5.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    03/16/2010
  * @brief   This file contains the RC5 functions prototypes.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * COPYRIGHT 2010 STMicroelectronics
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef RC5_H
#define RC5_H


/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_rcc.h"


/* Exported types ------------------------------------------------------------*/
typedef enum { NOTOK = 0, OK = !NOTOK} BitTimigStatus;
typedef enum { NO = 0, YES = !NO} StatusYesOrNo;

typedef struct
{
  __IO uint8_t ToggleBit;  /* Toggle bit field */
  __IO uint8_t Address;    /* Address field */
  __IO uint8_t Command;    /* Command field */
} RC5Frame_TypeDef;


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void RC5Init();
uint32_t RC5_TIM_GetCounterCLKValue(void);
void RC5_WaitForNextFallingEdge(void);
RC5Frame_TypeDef RC5_Decode(void);


#endif //RC5_H
