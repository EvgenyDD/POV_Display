/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MOTOR_H
#define MOTOR_H

/* Includes ------------------------------------------------------------------*/
#include <stm32f0xx.h>
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_rcc.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define abs(x)  ( (x)<0 ) ? (-(x)) : (x)


/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void MotorInit();
void MotorSpeedSet(uint16_t);
uint16_t MotorGetSpeed();
void MotorSetCurrSpin(uint16_t);
void MotorSpinDispatcher();

#endif //MOTOR_H
