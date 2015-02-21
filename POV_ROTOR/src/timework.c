/* Includes ------------------------------------------------------------------*/
#include "timework.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DAYS_OFFSET 2451911 	//days from 0 to 01.01.2001


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : RTC_TimeDateToCnt
* Description    : Translate Time&Date to counter value
* Input			 : pointer to TimeDate struct
*******************************************************************************/
uint32_t RTC_TimeDateToCnt(RTC_TimeDateTypeDef *TD)
{
	if(TD->year < 2000) TD->year = 2000 + TD->year%100;
	uint8_t a = (14 - TD->month)/12;
	uint16_t y = TD->year + 4800 - a;
	uint8_t m = TD->month + (12*a) - 3;

	uint32_t CNT = TD->date;
	CNT += (153*m+2)/5 + 365*y + y/4 -y/100 + y/400 - 32045 - DAYS_OFFSET;
	CNT *= 86400;    //to seconds
	CNT += TD->hour*3600 + TD->minute*60 + TD->second;

	return CNT; 	//seconds from 00-00 01.01.2001
}


/*******************************************************************************
* Function Name  : RTC_CntToTimeDate
* Description    : Translate counter to Time&Date value
* Input			 : counter value, pointer to TimeDate struct
*******************************************************************************/
void RTC_CntToTimeDate(uint32_t counter, RTC_TimeDateTypeDef *TD)
{
	uint32_t ace = counter/86400 + 32044 + DAYS_OFFSET;
	uint8_t b = (4*ace+3)/146097;
	ace -= (146097*b)/4;
	uint8_t d = (4*ace+3)/1461;
	ace -= (1461*d)/4;
	uint8_t m = (5*ace+2)/153;

	TD->date = ace - (153*m+2)/5 + 1;
	TD->month = m + 3 - 12*(m/10);
	TD->year = 100*b + d - 4800 + m/10;
	TD->hour = counter/3600%24;
	TD->minute = counter/60%60;
	TD->second = counter%60;
}


/*******************************************************************************
* Function Name  : TimeToString
* Description    : Convert time to formatted string
* Input		 	 : pointer to string, pointer to TimeDate struct
*******************************************************************************/
void TimeToString(char s[], RTC_TimeDateTypeDef *RTCmass)
{
	*(s+2) = *(s+5) = ':';

	*(s+0) = RTCmass->hour/10 +'0';
	*(s+1) = RTCmass->hour%10 +'0';

	*(s+3) = RTCmass->minute/10 +'0';
	*(s+4) = RTCmass->minute%10 +'0';
	*(s+6) = RTCmass->second/10 +'0';
	*(s+7) = RTCmass->second%10 +'0';
	*(s+8) = '\0';
}


/*******************************************************************************
* Function Name  : DateToString
* Description    : Convert date to formatted string
* Input		 	 : pointer to string, pointer to TimeDate struct
*******************************************************************************/
void DateToString(char s[], RTC_TimeDateTypeDef *RTCmass)
{
	*(s+2) = *(s+5) = '/';

	*(s+0) = RTCmass->date/10 +'0';
	*(s+1) = RTCmass->date%10 +'0';

	*(s+3) = RTCmass->month/10 +'0';
	*(s+4) = RTCmass->month%10 +'0';

	*(s+6) = RTCmass->year/1000 +'0';
	uint16_t left = RTCmass->year%1000;
	*(s+7) = left/100 +'0';
	left %= 100;
	*(s+8) = left/10 +'0';
	*(s+9) = left%10 +'0';
	*(s+10) = '\0';
}
