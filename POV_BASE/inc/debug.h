/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DEBUG_H
#define DEBUG_H


/* Includes ------------------------------------------------------------------*/
#include <stm32f0xx.h>
#include "stm32f0xx_usart.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_rcc.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void DebugInit();
void DebugSendString(char*);
void DebugSendChar(char);
void DebugSendNum(uint16_t Num);
void DebugSendNumWDesc(char *string, uint32_t Num);
void DebugSendNum2WDesc(char *string, uint32_t Num, uint32_t Num2);

#endif //DEBUG_H
