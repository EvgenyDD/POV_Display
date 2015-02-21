/* Includes ------------------------------------------------------------------*/
#include "UI.h"
#include "draw.h"
#include "timework.h"
#include "stm32f10x_rtc.h"
#include "string.h"

/* text and graphic objects
 * 3 temp menu strings	 	10, 11, 12
 * 1 string for time		13
 * 1 string to date			14
 * 1 string to metronome	15
 *
 * 12 for arrows			10-21
 * 3 for arrows				22, 23, 24
 * 2 for metronome			25, 26
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//#define LANG_EN


/* Private macro -------------------------------------------------------------*/
#define IsInMenu		BitIsSet(currModeNum, 7)
#define IsInColorMenu	(((CurrMenuItem->ID&224)==224) && inParamSet && BitIsReset(CurrMenuItem->ID, 4))


/* Private variables ---------------------------------------------------------*/
Menu_Item*       CurrMenuItem;
WriteFuncPtr*    WriteFunc;

volatile uint8_t currModeNum = 0;//, lastModeNum = 0;

#ifdef LANG_EN
/* Display screens */
  //Name			Next			Previous, 		Parent, 	Child      	ID	Text
M_M(MAnalogClock, 	MDigitalClock, 	MDAClock, 		NULL_MENU, 	NULL_MENU,	0, 	NULL_TEXT);
M_M(MDigitalClock, 	MDAClock, 		MAnalogClock, 	NULL_MENU, 	NULL_MENU,  32,	NULL_TEXT);
M_M(MDAClock, 		MAnalogClock, 	MDigitalClock, 	NULL_MENU, 	NULL_MENU, 	64,	NULL_TEXT);
M_M(MMetronome, 	NULL_MENU, 		NULL_MENU, 		NULL_MENU, 	NULL_MENU,  96,	NULL_TEXT);

/* Main menus */
//Name			//Next		//Previous, //Parent, 	//Child 	//ID	Text
M_M(MMENU, 		NULL_MENU, 	NULL_MENU, 	NULL_MENU, 	SetClock, 	128,	"MENU");

//Name			//Next		//Previous, //Parent, 	//Child 	 		//Text
M_M(SetClock, 	SetDate, 	NULL_MENU, 	MMENU, 		Set_Hour, 	144, 	"Clock");
M_M(SetDate, 	SetVol, 	SetClock, 	MMENU, 		Set_Date, 	160,	"Date");
M_M(SetVol, 	SetTone, 	SetDate, 	MMENU, 		Set_VClick, 176,	"Volume");
M_M(SetTone, 	SetMetr, 	SetVol, 	MMENU, 		Set_TClick, 192,	"Tone");
M_M(SetMetr, 	SetColors, 	SetTone, 	MMENU, 		Set_4Strk, 	208,	"Metronome");
M_M(SetColors, 	SetWork, 	SetMetr, 	MMENU, 		Set_ColArr, 224,	"Colors");
M_M(SetWork, 	NULL_MENU, 	SetColors, 	MMENU, 		Set_OnOff, 	240,	"Auto ON/OFF");

//Name			//Next		//Previous, //Parent, 	//Child 	//ID	//Text
M_M(Set_Hour, 	Set_Min, 	NULL_MENU, 	SetClock, 	NULL_MENU, 	145, 	"Hr");
M_M(Set_Min, 	Set_Sec, 	Set_Hour, 	SetClock, 	NULL_MENU, 	146,	"Min");
M_M(Set_Sec, 	Set_Corr, 	Set_Min, 	SetClock, 	NULL_MENU,  147,	"Sec");
M_M(Set_Corr, 	NULL_MENU,	Set_Sec, 	SetClock, 	NULL_MENU,	148,	"Corr");//..151

M_M(Set_Date, 	Set_Mon, 	NULL_MENU, 	SetDate, 	NULL_MENU, 	161, 	"Date");
M_M(Set_Mon, 	Set_Year, 	Set_Date, 	SetDate, 	NULL_MENU, 	162,	"Mon");
M_M(Set_Year, 	NULL_MENU,	Set_Mon, 	SetDate, 	NULL_MENU, 	163,	"Year");//..167

M_M(Set_VClick,	Set_VMetr, 	NULL_MENU, 	SetVol, 	NULL_MENU,	177, 	"Click");
M_M(Set_VMetr, 	Set_VHrStr, Set_VClick,	SetVol, 	NULL_MENU, 	178,	"Metr");
M_M(Set_VHrStr, NULL_MENU,	Set_VMetr, 	SetVol, 	NULL_MENU,	179,	"HrStrk");//..183

M_M(Set_TClick,	Set_TMtr1, 	NULL_MENU, 	SetTone, 	NULL_MENU,	193, 	"Click");
M_M(Set_TMtr1, 	Set_TMtr2, 	Set_TClick, SetTone, 	NULL_MENU, 	194,	"Metr1");
M_M(Set_TMtr2, 	Set_THrStr, Set_TMtr1, 	SetTone, 	NULL_MENU, 	195,	"Metr2");
M_M(Set_THrStr,	NULL_MENU,	Set_TMtr2, 	SetTone, 	NULL_MENU,	196,	"HrStrk");//..199

M_M(Set_4Strk, 	NULL_MENU, 	NULL_MENU, 	SetMetr, 	NULL_MENU, 	209,	"4thStike");//..215

M_M(Set_ColArr, Set_ColMrk, NULL_MENU, 	SetColors, 	NULL_MENU,	225, 	"Arrow");
M_M(Set_ColMrk, Set_ColTim, Set_ColArr, SetColors, 	NULL_MENU,	226,	"Marks");
M_M(Set_ColTim, Set_ColDat, Set_ColMrk, SetColors, 	NULL_MENU,	227,	"Time");
M_M(Set_ColDat, Set_ColPend,Set_ColTim, SetColors, 	NULL_MENU,	228,	"Date");
M_M(Set_ColPend,NULL_MENU, 	Set_ColDat, SetColors, 	NULL_MENU,	229,	"Pendulum");//..231

M_M(Set_OnOff, 	Set_OnTim, 	NULL_MENU, 	SetWork, 	NULL_MENU,	241, 	"On:1/Off:0");
M_M(Set_OnTim, 	Set_OffTim,	Set_OnOff, 	SetWork, 	NULL_MENU,	242, 	"OnTime");
M_M(Set_OffTim,	NULL_MENU, 	Set_OnTim, 	SetWork, 	NULL_MENU,	243, 	"OffTime");//!244,!245,!246,!247
#else
/* Display screens */
  //Name			Next			Previous, 		Parent, 	Child      	ID	Text
M_M(MAnalogClock, 	MDigitalClock, 	MDAClock, 		NULL_MENU, 	NULL_MENU,	0, 	NULL_TEXT);
M_M(MDigitalClock, 	MDAClock, 		MAnalogClock, 	NULL_MENU, 	NULL_MENU,  32,	NULL_TEXT);
M_M(MDAClock, 		MAnalogClock, 	MDigitalClock, 	NULL_MENU, 	NULL_MENU, 	64,	NULL_TEXT);
M_M(MMetronome, 	NULL_MENU, 		NULL_MENU, 		NULL_MENU, 	NULL_MENU,  96,	NULL_TEXT);

/* Main menus */
//Name			//Next		//Previous, //Parent, 	//Child 	//ID	Text
M_M(MMENU, 		NULL_MENU, 	NULL_MENU, 	NULL_MENU, 	SetClock, 	128,	"\x8c\xa5\xad\xee" /*Меню*/);

//Name			//Next		//Previous, //Parent, 	//Child 	 		//Text
M_M(SetClock, 	SetDate, 	NULL_MENU, 	MMENU, 		Set_Hour, 	144, 	"\x82\xe0\xa5\xac\xef" /*Время*/);
M_M(SetDate, 	SetVol, 	SetClock, 	MMENU, 		Set_Date, 	160,	"\x84\xa0\xe2\xa0" /*Дата*/);
M_M(SetVol, 	SetTone, 	SetDate, 	MMENU, 		Set_VClick, 176,	"\x83\xe0\xae\xac\xaa\xae\xe1\xe2\xec" /*Громкость*/);
M_M(SetTone, 	SetMetr, 	SetVol, 	MMENU, 		Set_TClick, 192,	"\x87\xa2\xe3\xaa. \x92\xae\xad" /*Звук. Тон*/);
M_M(SetMetr, 	SetColors, 	SetTone, 	MMENU, 		Set_4Strk, 	208,	"\x8c\xa5\xe2\xe0\xae\xad\xae\xac" /*Метроном*/);
M_M(SetColors, 	SetWork, 	SetMetr, 	MMENU, 		Set_ColArr, 224,	"\x96\xa2\xa5\xe2" /*Цвет*/);
M_M(SetWork, 	NULL_MENU, 	SetColors, 	MMENU, 		Set_OnOff, 	240,	"\x80\xa2\xe2\xae \x82\xaa\xab/\x82\xeb\xaa\xab" /*Авто Вкл/Выкл*/);

//Name			//Next		//Previous, //Parent, 	//Child 	//ID	//Text
M_M(Set_Hour, 	Set_Min, 	NULL_MENU, 	SetClock, 	NULL_MENU, 	145, 	"\x97\xa0\xe1" /*Час*/);
M_M(Set_Min, 	Set_Sec, 	Set_Hour, 	SetClock, 	NULL_MENU, 	146,	"\x8c\xa8\xad\xe3\xe2\xa0" /*Минута*/);
M_M(Set_Sec, 	Set_Corr, 	Set_Min, 	SetClock, 	NULL_MENU,  147,	"\x91\xa5\xaa\xe3\xad\xa4\xa0" /*Секунда*/);
M_M(Set_Corr, 	NULL_MENU,	Set_Sec, 	SetClock, 	NULL_MENU,	148,	"\x8a\xae\xe0\xe0\xa5\xaa\xe6\xa8\xef" /*Коррекция*/);//..151

M_M(Set_Date, 	Set_Mon, 	NULL_MENU, 	SetDate, 	NULL_MENU, 	161, 	"\x84\xa0\xe2\xa0" /*Дата*/);
M_M(Set_Mon, 	Set_Year, 	Set_Date, 	SetDate, 	NULL_MENU, 	162,	"\x8c\xa5\xe1\xef\xe6" /*Месяц*/);
M_M(Set_Year, 	NULL_MENU,	Set_Mon, 	SetDate, 	NULL_MENU, 	163,	"\x83\xae\xa4" /*Год*/);//..167

M_M(Set_VClick,	Set_VMetr, 	NULL_MENU, 	SetVol, 	NULL_MENU,	177, 	"\x99\xa5\xab\xe7\xae\xaa" /*Щелчок*/);
M_M(Set_VMetr, 	Set_VHrStr, Set_VClick,	SetVol, 	NULL_MENU, 	178,	"\x8c\xa5\xe2\xe0\xae\xad\xae\xac" /*Метроном*/);
M_M(Set_VHrStr, NULL_MENU,	Set_VMetr, 	SetVol, 	NULL_MENU,	179,	"\x97\xa0\xe1\xae\xa2\xae\xa9 \xe3\xa4\xa0\xe0" /*Часовой удар*/);//..183

M_M(Set_TClick,	Set_TMtr1, 	NULL_MENU, 	SetTone, 	NULL_MENU,	193, 	"\x99\xa5\xab\xe7\xae\xaa" /*Щелчок*/);
M_M(Set_TMtr1, 	Set_TMtr2, 	Set_TClick, SetTone, 	NULL_MENU, 	194,	"1-3 \xe3\xa4\xa0\xe0" /*1-3 удар*/);
M_M(Set_TMtr2, 	Set_THrStr, Set_TMtr1, 	SetTone, 	NULL_MENU, 	195,	"4\xa9 \xe3\xa4\xa0\xe0" /*4й удар*/);
M_M(Set_THrStr,	NULL_MENU,	Set_TMtr2, 	SetTone, 	NULL_MENU,	196,	"\x97\xa0\xe1\xae\xa2\xae\xa9 \xe3\xa4\xa0\xe0" /*Часовой удар*/);//..199

M_M(Set_4Strk, 	NULL_MENU, 	NULL_MENU, 	SetMetr, 	NULL_MENU, 	209,	"4\xa9 \xe3\xa4\xa0\xe0" /*4й удар*/);//..215

M_M(Set_ColArr, Set_ColMrk, NULL_MENU, 	SetColors, 	NULL_MENU,	225, 	"\x91\xe2\xe0\xa5\xab\xaa\xa8" /*Стрелки*/);
M_M(Set_ColMrk, Set_ColTim, Set_ColArr, SetColors, 	NULL_MENU,	226,	"\x97\xa0\xe1. \xae\xe2\xac\xa5\xe2\xaa\xa8" /*Час. отметки*/);
M_M(Set_ColTim, Set_ColDat, Set_ColMrk, SetColors, 	NULL_MENU,	227,	"\x82\xe0\xa5\xac\xef" /*Время*/);
M_M(Set_ColDat, Set_ColPend,Set_ColTim, SetColors, 	NULL_MENU,	228,	"\x84\xa0\xe2\xa0" /*Дата*/);
M_M(Set_ColPend,NULL_MENU, 	Set_ColDat, SetColors, 	NULL_MENU,	229,	"\x8c\xa0\xef\xe2\xad\xa8\xaa" /*Маятник*/);//..231

M_M(Set_OnOff, 	Set_OnTim, 	NULL_MENU, 	SetWork, 	NULL_MENU,	241, 	"\x82\xaa\xab:1/\x82\xeb\xaa\xab:0" /*Вкл:1/Выкл:0*/);
M_M(Set_OnTim, 	Set_OffTim,	Set_OnOff, 	SetWork, 	NULL_MENU,	242, 	"\x97\xa0\xe1 \x82\xaa\xab." /*Час Вкл.*/);
M_M(Set_OffTim,	NULL_MENU, 	Set_OnTim, 	SetWork, 	NULL_MENU,	243, 	"\x97\xa0\xe1 \x82\xeb\xaa\xab." /*Час Выкл.*/);//!244,!245,!246,!247
#endif

Menu_Item    Null_Menu = {(void*)&NULL_MENU, (void*)&NULL_MENU, (void*)&NULL_MENU, (void*)&NULL_MENU, 0, NULL_TEXT};

Menu_Item *pMenu[] = {
		&MAnalogClock, &MDigitalClock, &MDAClock, &MMetronome,
		&MMENU,
		&SetClock, 	&SetDate, 	&SetVol, 	&SetTone, 	&SetMetr, 	&SetColors, &SetWork,
		&Set_Hour, 	&Set_Min, 	&Set_Sec, 	&Set_Corr,
		&Set_Date, 	&Set_Mon, 	&Set_Year,
		&Set_VClick,&Set_VMetr, &Set_VHrStr,
		&Set_TClick,&Set_TMtr1, &Set_TMtr2, &Set_THrStr,
		&Set_4Strk,
		&Set_ColArr,&Set_ColMrk,&Set_ColTim,&Set_ColDat,&Set_ColPend,
		&Set_OnOff, &Set_OnTim,	&Set_OffTim
};

char Time[10], Date[12];

RTC_TimeDateTypeDef RTC_TimeDate;

volatile uint16_t transCounter=0;
uint8_t transDir;

char tempStr[30]="";
char temp[30]="";

uint16_t menuParamValue;

uint8_t inMenu=0, inParamSet=0;

volatile signed int metronomePos = 0;
volatile signed int metronomePeriod = 468/2;

SettingsArray sett;

/* Extern variables ----------------------------------------------------------*/
extern const char font3x5[];
extern const char font5x8[];


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : UI_Init
* Description    :
*******************************************************************************/
void UI_Init()
{
	/* Initialize menu strings, as they take part in transitions */
	for(uint8_t i=0; i<3; i++)
	{
		DisplayTextAdd(10+i, "\x7F", &font5x8[0], GREEN, 100, 10, ROUTE_BOTTOM, 2);
		DisplayTextDisable(10+i);
	}

	UI_NewMenuMode(0); //32
}


/*******************************************************************************
* Function Name  : UI_Dispatcher
* Description    :
*******************************************************************************/
void UI_Dispatcher()
{
#define TRANS_DELAY 800
#define TEN (TRANS_DELAY/10)

	if(IsInMenu)
	{
		Menu_Item *tmp;

		/* if transition is in progress */
		if(transCounter)
		{
			switch(transDir)
			{
			case DIR_DOWN://0
				DisplayTextModifyPos(10, sett.colTime, 255, 28-8+10-transCounter/TEN);
				tmp = PARENT;
				UI_MakeMenuString(tempStr, (char*)TEXT, menuParamValue);
				DisplayTextAdd(11, tempStr, &font5x8[0], sett.colTime, 199-7*(strlen(tempStr)/2), 28-8-transCounter/TEN, ROUTE_TOP, 2);
				DisplayTextAdd(12, (char*)tmp->Text, &font5x8[0], sett.colTime, 199-7*(strlen((char*)tmp->Text)/2), (28-8-10)-transCounter/TEN, ROUTE_TOP, 2);
				break;

			case DIR_UP://1
				UI_MakeMenuString(tempStr, (char*)TEXT, menuParamValue);
				DisplayTextAdd(12, tempStr, &font5x8[0], sett.colTime, 197-7*(strlen(tempStr)/2), 28-8+transCounter/TEN, ROUTE_TOP, 2);
				tmp = PARENT;
				DisplayTextAdd(10, (char*)tmp->Text, &font5x8[0], sett.colTime, 199-7*(strlen((char*)tmp->Text)/2), 28-8-10+transCounter/TEN, ROUTE_TOP, 2);
				DisplayTextModifyPos(11, sett.colTime, 255, (28-8-10)-10+transCounter/TEN);
				break;

			case DIR_LEFT://2
				UI_MakeMenuString(tempStr, (char*)TEXT, menuParamValue);
				DisplayTextAdd(10, tempStr, &font5x8[0], sett.colTime, 199-7*(strlen(tempStr)/2)+transCounter/50, 28-8, ROUTE_TOP, 2);
				tmp = PARENT;
				DisplayTextAdd(11, (char*)tmp->Text, &font5x8[0], sett.colTime, 199-7*(strlen((char*)tmp->Text)/2), 28-8-10, ROUTE_TOP, 2);
				break;

			case DIR_RIGHT://3
				UI_MakeMenuString(tempStr, (char*)TEXT, menuParamValue);
				DisplayTextAdd(10, tempStr, &font5x8[0], sett.colTime, 199-7*(strlen(tempStr)/2)-transCounter/50, 28-8, ROUTE_TOP, 2);
				tmp = PARENT;
				DisplayTextAdd(11, (char*)tmp->Text, &font5x8[0], sett.colTime, 199-7*(strlen((char*)tmp->Text)/2), 28-8-10, ROUTE_TOP, 2);
				break;
			}

			return;
		}

		/* Settings mode */
		UI_MakeMenuString(tempStr, (char*)TEXT, menuParamValue);
		DisplayTextAdd(10, tempStr, &font5x8[0],  IsInColorMenu?menuParamValue:sett.colTime,
				199-7*(strlen(tempStr)/2), 28-8, ROUTE_TOP, 2);
		tmp = PARENT;
		DisplayTextAdd(11, (char*)tmp->Text, &font5x8[0], sett.colTime,
				199-7*(strlen((char*)tmp->Text)/2), 28-8-10, ROUTE_TOP, 2);
		DisplayTextDisable(12);
	}
	else
	{
		uint8_t pos;
		signed int normal;

		/* Normal work mode */
		switch(currModeNum)
		{
		case 0:
			RTC_CntToTimeDate(RTC_GetCounter(), &RTC_TimeDate);
			TimeToString(Time, &RTC_TimeDate);		DateToString(Date, &RTC_TimeDate);
			DisplayTextAdd(13, Time, &font5x8[0], sett.colTime, 174, 28-8, ROUTE_TOP, 2);
			DisplayTextAdd(14, Date, &font5x8[0], sett.colDate, 66, 28-8, ROUTE_BOTTOM, 2);
			break;

		case 32:
			RTC_CntToTimeDate(RTC_GetCounter(), &RTC_TimeDate);

			uint8_t hourArrow = (200-5* (10*(RTC_TimeDate.hour%12) + RTC_TimeDate.minute/6) /3) %200;
			DisplayObjectAdd(22, TYPE_ARROW, sett.colArrow, 0, 15,	(hourArrow>=3)?(hourArrow-3):(200-3+hourArrow), (hourArrow+3)%200);

			uint8_t minuteArrow = (200-10*(RTC_TimeDate.minute)/3) %200;
			DisplayObjectAdd(23, TYPE_ARROW, sett.colArrow, 0, 22,	(minuteArrow>=1)?(minuteArrow-1):(200-1+minuteArrow), (minuteArrow+1)%200);

			DisplayObjectAdd(24, TYPE_LINE, sett.colArrow, 0, 30, (200-200*(RTC_TimeDate.second)/60) %200, 0);
			break;

		case 64:
			RTC_CntToTimeDate(RTC_GetCounter(), &RTC_TimeDate);

			hourArrow = (200-5* (10*(RTC_TimeDate.hour%12) + RTC_TimeDate.minute/6) /3) %200;
			DisplayObjectAdd(22, TYPE_ARROW, sett.colArrow, 0, 15,	(hourArrow>=3)?(hourArrow-3):(200-3+hourArrow), (hourArrow+3)%200);

			minuteArrow = (200-10*(RTC_TimeDate.minute)/3) %200;
			DisplayObjectAdd(23, TYPE_ARROW, sett.colArrow, 0, 22,	(minuteArrow>=1)?(minuteArrow-1):(200-1+minuteArrow), (minuteArrow+1)%200);

			DisplayObjectAdd(24, TYPE_LINE, sett.colArrow, 0, 30, (200-200*(RTC_TimeDate.second)/60) %200, 0);

			TimeToString(Time, &RTC_TimeDate);	DateToString(Date, &RTC_TimeDate);
			DisplayTextAdd(13, Time, &font5x8[0], sett.colTime, 174, 28-8, ROUTE_TOP, 2);
			DisplayTextAdd(14, Date, &font5x8[0], sett.colDate, 66, 28-8, ROUTE_BOTTOM, 2);
			break;

		case 96:
			normal = 50*metronomePos/metronomePeriod;
			pos = (metronomePos > 0)? 200-normal*(1- (50 - normal)/(3*50)) : normal*-1*(1-(50 + normal)/(3*50));

			DisplayObjectAdd(25, TYPE_LINE, sett.colArrow, 0, 28, pos, 0);
			DisplayObjectAdd(26, TYPE_POINT, sett.colArrow, 28, 0, pos, 0);

			temp[0] = '\0';
			strcat_(temp, "\x92\xa5\xac\xaf: ");
			char u[10];
			itoa_(30000/metronomePeriod, u);
			strcat_(temp, u);
			DisplayTextAdd(15, temp, &font5x8[0], sett.colTime, 65, 28-8, ROUTE_BOTTOM, 2);
			break;
		}
	}
}


/*******************************************************************************
* Function Name  : UI_NewMenuMode
* Description    :
*******************************************************************************/
void UI_NewMenuMode(uint8_t modeID)
{
	/* if user press key during transition */
	transCounter = 0;
	UI_Dispatcher();

	/* disable old mode graphics */
	for(uint8_t i=0; i<17; i++)	DisplayObjectDisable(10+i);
	for(uint8_t i=0; i<6; i++)
	{
		DisplayTextAdd(10+i, "\x7F", &font5x8[0], sett.colTime, 0, 28, ROUTE_TOP, 2);
		DisplayTextDisable(10+i);
	}

	inMenu = IsInMenu?1:0;
	inParamSet = BitIsSet(modeID, 3);
	BitReset(modeID,3);

	transCounter = inParamSet?0:TRANS_DELAY;

	currModeNum = modeID;

	/* Find exact menu item which equal to ID Number */
	for(uint8_t i=0; i < sizeof(pMenu)/4; i++)
	{
		if(currModeNum == pMenu[i]->ID)
		{
			Menu_Item *L=PARENT, /**R=CHILD,*/ *U=NEXT, *D=PREVIOUS;

			if(pMenu[i]->ID == L->ID) transDir = DIR_DOWN;
			else if(pMenu[i]->ID == U->ID) transDir = DIR_LEFT;
			else if(pMenu[i]->ID == D->ID) transDir = DIR_RIGHT;
			//if(pMenu[i]->ID == R->ID) transDir = DIR_UP;
			else transDir = DIR_UP;

			CurrMenuItem = pMenu[i];
			break;
		}
	}

	if(currModeNum == 32 || currModeNum == 64)
	{
		/* Hour marks */
		for(uint8_t i=0; i<12; i++)
			DisplayObjectAdd(10+i, TYPE_LINE, sett.colHMrk, 31-5- (((i)%3)?0:4), 31, 200*i/12, 200*i/12);
	}

	metronomePeriod = 0;
}


/*******************************************************************************
* Function Name  : UI_NewParameter
* Description    :
*******************************************************************************/
void UI_NewParameter(uint16_t paramValue)
{
	if(inMenu)
	{
		menuParamValue = paramValue;
		UI_MakeMenuString(tempStr, (char*)TEXT, menuParamValue);
	}
	else
	{	/* not in menu */
		if(currModeNum == 96)
		{
			metronomePos = 0;
			metronomePeriod = paramValue;
		}
	}
}


/*******************************************************************************
* Function Name  : UI_MakeMenuString
* Description    :
*******************************************************************************/
void UI_MakeMenuString(char *string, char *source, uint16_t paramValue)
{
	uint16_t index=0;

	if(CurrMenuItem->Next != &NULL_MENU && !inParamSet)
		string[index++] = '<';

	for(uint8_t i=0; i<strlen(source); i++)
		string[index++] = source[i];

	if(CurrMenuItem->Previous != &NULL_MENU && !inParamSet)
		string[index++] = '>';

	string[index] = '\0';

	if(inParamSet)
	{
		string[index++] = '=';
		string[index++] = '\0';
		char t[12];

		if(inParamSet && currModeNum == 148)
		{
			if(paramValue < 65000)
				ftoa_((float)paramValue/32, t, 2);
			else
				ftoa_(-(float)(65536-paramValue)/32, t, 2);


			strcat_(string, t);
			strcat_(string, "sec");
		}

		else
		{
			itoa_(paramValue, t);
			strcat_(string, t);
		}


	}
}


/*******************************************************************************
* Function Name  : BkpToSett
* Description    :
*******************************************************************************/
void MakeSettFromNumber(uint8_t bkpNum, uint32_t bkpValue)
{
const uint16_t numOfBits[] = {0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095};

	switch(bkpNum)
	{
	case 0:
		sett.prevMode 	= (bkpValue) 				& numOfBits[2];
		sett.metr4Strk	= (bkpValue >> (2))			& numOfBits[1];
		sett.toneM1		= (bkpValue >> (2+1))		& numOfBits[6];
		sett.toneM2		= (bkpValue >> (2+1+6))		& numOfBits[6];
		sett.toneHrStrk	= (bkpValue >> (2+1+6+6))	& numOfBits[6];
		sett.corr		= (bkpValue >> (2+1+6+6+6))	& numOfBits[10];
		break;

	case 1:
		sett.toneClick 	= (bkpValue) 				& numOfBits[6];
		sett.volClick	= (bkpValue >> (6))			& numOfBits[3];
		sett.volMetr	= (bkpValue >> (6+3))		& numOfBits[3];
		sett.volHrStrk	= (bkpValue >> (6+3+3))		& numOfBits[3];
		break;

	case 2:
		sett.colDate	= (bkpValue)				& numOfBits[3];
		sett.colTime	= (bkpValue >> (3))			& numOfBits[3];
		sett.colArrow	= (bkpValue >> (3+3))		& numOfBits[3];
		sett.colHMrk	= (bkpValue >> (3+3+3))		& numOfBits[3];
		sett.colPend	= (bkpValue >> (3+3+3+3))	& numOfBits[3];
		break;

	default:
		break;
	}
}
