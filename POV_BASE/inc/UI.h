/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UI_H
#define UI_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
typedef struct {
	void       *Next;
	void       *Previous;
	void       *Parent;
	void       *Child;
	uint8_t 	number;
	const char  Text[];
}MenuItem;

typedef struct {
	/* first */
	unsigned prevMode	:2;	//0s Reg
	unsigned metr4Strk	:1;	//0s Reg

	unsigned toneM1		:6;	//0s Reg
	unsigned toneM2		:6;	//0s Reg
	unsigned toneHrStrk	:6;	//0s Reg

	signed corr			:10;//0s Reg


	/* second */
	unsigned toneClick	:6;	//1st Reg

	unsigned volClick	:3;	//1st Reg
	unsigned volMetr	:3;	//1st Reg
	unsigned volHrStrk	:3;	//1st Reg

	unsigned metrBPM	:7; //1st Reg


	/* third ... */
	unsigned colDate	:3;	//2nd Reg
	unsigned colTime	:3;	//2nd Reg
	unsigned colArrow	:3;	//2nd Reg
	unsigned colHMrk	:3;	//2nd Reg
	unsigned colPend	:3;	//2nd Reg

	unsigned OnTime		:5; //2nd Reg
	unsigned OffTime	:5;	//2nd Reg
	unsigned WorkOnOff	:1; //2nd Reg

}SettingsArray;


/* Exported constants --------------------------------------------------------*/
enum RC5_BUTTONS{B_ONOFF=1, B_LEFT, B_RIGHT, B_UP, B_DOWN, B_AVTV, B_MUTE};


/* Exported macro ------------------------------------------------------------*/
#define M_M(Name,   Next, Previous, Parent, Child, number, Text) \
    extern MenuItem Next;     \
	extern MenuItem Previous; \
	extern MenuItem Parent;   \
	extern MenuItem Child;  \
	MenuItem Name = {(void*)&Next, (void*)&Previous, (void*)&Parent, (void*)&Child, number, { Text }}


/* Exported define -----------------------------------------------------------*/
#define NULL_MENU 	Null_Menu

#define PREVIOUS   ( CurrMenuItem->Previous )
#define NEXT       ( CurrMenuItem->Next )
#define PARENT     ( CurrMenuItem->Parent )
#define CHILD      ( CurrMenuItem->Child )


/* Exported functions ------------------------------------------------------- */
void MenuChange(MenuItem* NewMenu);

void UIDispatcher(uint8_t button);
void UIInit();

void SetUp();
void SetDown();
void SendSett();

void MetrEnter();
void MetrUp(uint8_t);
void MetrDown(uint8_t);
void MetronomeSound();

void BkpToSett(uint8_t);
void SettingsToBkp(uint8_t);
void SendSettArray();
void SendPWM(uint16_t);

void RTCCalibrate();

#endif //UI_H
