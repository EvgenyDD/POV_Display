/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UI_H
#define UI_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "display_HAL.h"


/* Exported types ------------------------------------------------------------*/
typedef void (*FuncPtr)(void);
typedef void (*WriteFuncPtr)(const char*);

typedef struct {
	void       *Previous;
	void       *Next;
	void       *Parent;
	void       *Child;
	uint8_t 	ID;
	const char  Text[];
}Menu_Item;

#define M_M(Name,Previous, Next, Parent, Child, ID, Text) \
    extern Menu_Item Next;     \
	extern Menu_Item Previous; \
	extern Menu_Item Parent;   \
	extern Menu_Item Child;  \
	Menu_Item Name = {(void*)&Previous, (void*)&Next, (void*)&Parent, (void*)&Child, ID, {Text}}

#define NULL_MENU 	Null_Menu
#define NULL_TEXT	"\x7F"

#define PREVIOUS   	( CurrMenuItem->Previous )
#define NEXT       	( CurrMenuItem->Next )
#define PARENT     	( CurrMenuItem->Parent )
#define CHILD      	( CurrMenuItem->Child )

#define TEXT	    ( CurrMenuItem->Text )

typedef struct {
#if 1
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
#else
	uint8_t prevMode	;	//0s Reg
	uint8_t metr4Strk	;	//0s Reg

	uint8_t toneM1		;	//0s Reg
	uint8_t toneM2		;	//0s Reg
	uint8_t toneHrStrk	;	//0s Reg

	uint16_t corr		;//0s Reg


	/* second */
	uint8_t toneClick;	//1st Reg

	uint8_t volClick;	//1st Reg
	uint8_t volMetr	;	//1st Reg
	uint8_t volHrStrk;	//1st Reg

	uint8_t metrBPM	; //1st Reg


	/* third ... */
	uint8_t colDate	;	//2nd Reg
	uint8_t colTime	;	//2nd Reg
	uint8_t colArrow;	//2nd Reg
	uint8_t colHMrk	;	//2nd Reg
	uint8_t colPend	;	//2nd Reg

#endif

}SettingsArray;

/* Exported constants --------------------------------------------------------*/
enum TRANS_DIR{DIR_DOWN, DIR_UP, DIR_LEFT, DIR_RIGHT};


/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void UI_Init();
void UI_Dispatcher();
void UI_NewMenuMode(uint8_t);
void UI_NewParameter(uint16_t);
void UI_MakeMenuString(char*, char*, uint16_t);
void MakeSettFromNumber(uint8_t, uint32_t);

#endif //UI_H
