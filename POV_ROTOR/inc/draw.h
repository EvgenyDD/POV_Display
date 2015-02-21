/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DRAW_H
#define DRAW_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"


/* Exported types ------------------------------------------------------------*/
typedef struct {
	uint8_t flagEn;

	char text[200];
	uint16_t len;		//text length

	const char *pFont; 	//pointer to font
	uint8_t spacing; 	//font spacing

	uint8_t route;		//text displayed on upper display part or lower display part
	uint16_t angle; 	//start display angle
	uint16_t radius;	//distance from center to text beginning
	uint8_t color;
}TextDispStruct;

typedef struct {
	uint8_t flagEn;
	uint8_t type;

	uint8_t start; 		// start angle or start radius
	uint8_t end;		// end angle or end radius

	uint8_t radius;		// radius of arc
	uint8_t radiusEnd;
	uint8_t color;
}ObjDispStruct;


/* Exported constants --------------------------------------------------------*/
enum {ROUTE_BOTTOM, ROUTE_TOP};
enum {TYPE_ARC, TYPE_CIRCLE, TYPE_LINE, TYPE_ROUNDBOX, TYPE_SOLIDROUNDBOX, TYPE_ARROW, TYPE_POINT};


/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void DisplayTextAdd(uint16_t, char*, const char*, uint8_t, uint16_t, uint16_t, uint16_t, uint16_t);
void DisplayTextModifyPos(uint16_t, uint8_t, uint8_t, uint8_t);
void DisplayTextProcess(uint16_t, uint8_t);
void DisplayTextDisable(uint8_t);
void DisplayTextEnable(uint8_t);
uint8_t ReverseByte(uint8_t d);

void DisplayObjectAdd(uint8_t, uint8_t, uint8_t, uint16_t, uint16_t, uint16_t, uint16_t);
void DisplayObjectProcess(uint16_t step, uint8_t channel);
void DisplayObjectDisable(uint8_t);
void DisplayObjectEnable(uint8_t);

bool isInBound(uint8_t begin, uint8_t end, uint8_t step);

//void XYfromRa(uint8_t len, uint8_t angle, signed char *x, signed char *y);

#endif //DRAW_H
