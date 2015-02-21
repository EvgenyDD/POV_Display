/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TIMEWORK_H
#define TIMEWORK_H

/* Includes ------------------------------------------------------------------*/
#include <stm32f0xx.h>
#include "stm32f0xx_rtc.h"


/* Exported types ------------------------------------------------------------*/
typedef struct
{
	uint16_t year;
	uint8_t month;
	uint8_t date;

	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}RTC_TimeDateTypeDef;


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint32_t RTC_TimeDateToCnt(RTC_TimeDateTypeDef*);
void RTC_CntToTimeDate(uint32_t, RTC_TimeDateTypeDef*);
void TimeToString(char[], RTC_TimeDateTypeDef*);
void DateToString(char[], RTC_TimeDateTypeDef*);
void TimeDateToStruct(RTC_TimeTypeDef*, RTC_DateTypeDef*, RTC_TimeDateTypeDef*);
uint32_t TimeDateToCounter(RTC_TimeTypeDef*, RTC_DateTypeDef*);
void GetTimeDate(RTC_TimeTypeDef*, RTC_DateTypeDef*);


#endif //TIMEWORK_H
