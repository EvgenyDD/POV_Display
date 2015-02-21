/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FM_H
#define FM_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_rcc.h"


/* Exported types ------------------------------------------------------------*/
typedef enum{
	Message_FAIL_RX=7, Message_GetTimeDate=25, Message_2=21,
	Message_3=19, Message_4=17, Message_5=27
} MessageType;


/* Exported constants --------------------------------------------------------*/
enum FreqValue{ //counter period that matches frequency values
	FREQ19K=632, FREQ20K=600, FREQ21K=571, FREQ22K=545,
    FREQ23K=522, FREQ24K=500, FREQ25K=480, FREQ26K=462,
    FREQ27K=444, FREQ28K=428, FREQ29K=414, FREQ30K=400
};


/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void FMInit();

void FMOff();
void FMSetFreq(uint16_t, uint16_t);
uint16_t FMGetFreq();

void FMSendData(uint32_t, uint8_t);

#endif //FM_H
