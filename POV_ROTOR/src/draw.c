/*******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2015 Evgeny Dolgalev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "draw.h"
#include "string.h"
#include "display_HAL.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define abs(x)  				( (x)<0 ) ? (-(x)) : (x)
#define GetDiff(stat,move)		((move)>=(stat))?((move)-(stat)):(200-(stat)+(move))


/* Private variables ---------------------------------------------------------*/
TextDispStruct Text[30];
ObjDispStruct Object[30];


/* Extern variables ----------------------------------------------------------*/
extern const char font3x5[];
extern const char font5x8[];


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : DisplayTextAdd
* Description    : Add text to text display channel
* Input		 	 : Channel, pointer to text, pointer to font,
* 				 : text color, angle, raduis,
* 				 : route_direction, text spacing
*******************************************************************************/
void DisplayTextAdd(uint16_t channel, char *pNewTextDisp, const char *font, uint8_t color, uint16_t angle, uint16_t raduis, uint16_t route, uint16_t spacing)
{
	Text[channel].pFont = font;

	Text[channel].len = strlen(pNewTextDisp) * (Text[channel].pFont[0] + spacing) - spacing;
	Text[channel].spacing = spacing;

	if(route == ROUTE_BOTTOM)
	{
		for(uint8_t i=0; i<strlen(pNewTextDisp); i++)
			Text[channel].text[i] = pNewTextDisp[i];
	}
	else
	{	//ROUTE_TOP
		uint8_t tempLen = strlen(pNewTextDisp);

		for(uint8_t i=0; i<tempLen; i++)
			Text[channel].text[tempLen-1-i] = pNewTextDisp[i];
	}

	Text[channel].angle = angle%200;
	Text[channel].radius = raduis/*%32*/;
	Text[channel].route = route;
	Text[channel].color = color;

	Text[channel].flagEn = ENABLE;
}


/*******************************************************************************
* Function Name  : DisplayTextModifyPos
* Description    : Modify text object position
* Input		 	 : Channel, text color, angle, raduis
*******************************************************************************/
void DisplayTextModifyPos(uint16_t channel, uint8_t color, uint8_t angle, uint8_t raduis)
{
	if(angle != 255) Text[channel].angle = angle%200; //255 means that angle wouldn't be changed
	if(raduis < 40)	Text[channel].radius = raduis/*%32*/;
	Text[channel].color = color;

	Text[channel].flagEn = ENABLE;
}


/*******************************************************************************
* Function Name  : DisplayTextDisable
* Description    : Disable text channel displaying
*******************************************************************************/
void DisplayTextDisable(uint8_t channel) {Text[channel].flagEn = DISABLE;}


/*******************************************************************************
* Function Name  : DisplayTextEnable
* Description    : Enable text channel displaying
*******************************************************************************/
void DisplayTextEnable(uint8_t channel) {Text[channel].flagEn = ENABLE;}


/*******************************************************************************
* Function Name  : DisplayTextProcess
* Description    : Text display dispatcher
* Input		 	 : step(rotor position), channel
*******************************************************************************/
void DisplayTextProcess(uint16_t step, uint8_t channel)
{
	uint32_t Column[3] = {0};
	uint8_t Line;

#define ANGLE 	(Text[channel].angle)
#define S 		(Text[channel].spacing)
#define W 		(Text[channel].pFont[0])
#define H 		(Text[channel].pFont[1])

	if(step < ANGLE) step += 200;

	if(step < ANGLE || (step-ANGLE) > Text[channel].len || (step-ANGLE)%(W+S) >= W)
		return;

	if(Text[channel].route == ROUTE_BOTTOM)
	{
		Line = Text[channel].pFont[2+ W*(Text[channel].text[ ((step-ANGLE)%200)/(W+S) ] - 32) +
							   ((step-ANGLE)%200)%(W+S)    ];
	}
	else
	{	//ROUTE_TOP
		Line = ReverseByte( Text[channel].pFont[2+ W*(Text[channel].text[ ((step-ANGLE)%200)/(W+S) ] - 32) +
							   W-1-((step-ANGLE)%200)%(W+S)]     );
	}

	uint32_t temp = Line << Text[channel].radius;

	if(Text[channel].color < 3) //single color mode
	{
		Column[Text[channel].color] = temp;
	}
	else						//multi color mode
	{
		switch(Text[channel].color)
		{
		case YELLOW:
				Column[RED] = Column[GREEN] = temp;
			break;
		case MAGENTA:
				Column[RED] = Column[BLUE] = temp;
			break;
		case CYAN:
				Column[BLUE] = Column[GREEN] = temp;
			break;
		case WHITE:
				Column[RED] = Column[GREEN] = Column[BLUE] = temp;
			break;
		}
	}
	ColDisplayLayer(&Column[0]);
}


/*******************************************************************************
* Function Name  : ReverseByte
* Description    : Reverse byte
*******************************************************************************/
uint8_t ReverseByte(uint8_t byte)
{
	const uint8_t revTable[] = {0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15};
	return (revTable[byte >> 4]) | (revTable[byte & 0x0F] << 4);
}


/*******************************************************************************
* Function Name  : DisplayObjectAdd
* Description    : Add object to object display channel
* Input		 	 : Channel, type, color, radius, radiusEnd, start, end
*******************************************************************************/
void DisplayObjectAdd(	uint8_t channel, uint8_t type, uint8_t color,
						uint16_t radius, uint16_t radiusEnd,
						uint16_t start,	uint16_t end)
{
	Object[channel].type = type;

	Object[channel].color = color;
	Object[channel].radius = radius;
	Object[channel].radiusEnd = radiusEnd;

	Object[channel].start = start;
	Object[channel].end = end;

	Object[channel].flagEn = ENABLE;
}


/*******************************************************************************
* Function Name  : DisplayObjectDisable
* Description    : Disable text channel displaying
* Input		 	 : Channel
*******************************************************************************/
void DisplayObjectDisable(uint8_t channel) {Object[channel].flagEn = DISABLE;}


/*******************************************************************************
* Function Name  : DisplayObjectEnable
* Description    : Enable text channel displaying
* Input		 	 : Channel
*******************************************************************************/
void DisplayObjectEnable(uint8_t channel) {Object[channel].flagEn = ENABLE;}


/*******************************************************************************
* Function Name  : DisplayObjectProcess
* Description    : Object display dispatcher
* Input		 	 : step(rotor position), channel
*******************************************************************************/
void DisplayObjectProcess(uint16_t step, uint8_t channel)
{
	switch(Object[channel].type)
	{
	case TYPE_CIRCLE:
		PixelWrite(Object[channel].radius, Object[channel].color, ENABLE);
		break;

	case TYPE_ARC:
		if(Object[channel].start > Object[channel].end)
		{
			if( (step > Object[channel].start) || (step < Object[channel].end) )
				PixelWrite(Object[channel].radius, Object[channel].color, ENABLE);
		}
		else
		{
			if(step > Object[channel].start && step < Object[channel].end)
				PixelWrite(Object[channel].radius, Object[channel].color, ENABLE);
		}
		break;

	case TYPE_LINE:
		if(step == Object[channel].start)
		{
			for(uint8_t i=Object[channel].radius; i<=Object[channel].radiusEnd; i++)
				PixelWrite(i, Object[channel].color, 1);
		}
		break;

	case TYPE_ROUNDBOX:
		if(Object[channel].start == step || Object[channel].end == step)
		{
			for(uint8_t i=Object[channel].radius; i<=Object[channel].radiusEnd; i++)
				PixelWrite(i, Object[channel].color, 1);
		}
		if(Object[channel].start > Object[channel].end)
		{
			if( (step > Object[channel].start && step <= 200) || (step < Object[channel].start) )
			{
				PixelWrite(Object[channel].radius, Object[channel].color, ENABLE);
				PixelWrite(Object[channel].radiusEnd, Object[channel].color, ENABLE);
			}
		}
		else
		{
			if(step > Object[channel].start && step < Object[channel].end)
			{
				PixelWrite(Object[channel].radius, Object[channel].color, ENABLE);
				PixelWrite(Object[channel].radiusEnd, Object[channel].color, ENABLE);
			}
		}
		break;

	case TYPE_SOLIDROUNDBOX:
		if(Object[channel].start > Object[channel].end)
		{
			if( (step > Object[channel].start && step <= 200) || (step < Object[channel].start) )
			{
				for(uint8_t i=Object[channel].radius; i<=Object[channel].radiusEnd; i++)
					PixelWrite(i, Object[channel].color, 1);
			}
		}
		else
		{
			if(step >= Object[channel].start && step <= Object[channel].end)
			{
				for(uint8_t i=Object[channel].radius; i<=Object[channel].radiusEnd; i++)
					PixelWrite(i, Object[channel].color, 1);
			}
		}
		break;

	case TYPE_ARROW:
		if(isInBound(Object[channel].start, Object[channel].end, step))
		{
			signed char limit = (Object[channel].start <= Object[channel].end)?
						abs( (step-Object[channel].start)-(Object[channel].end - step) ):
						abs( ((step + 30)%200 - ((Object[channel].start + 30)%200)) - (((Object[channel].end + 30)%200) - (step + 30)%200));

			for(uint8_t i=Object[channel].radius; i<=Object[channel].radiusEnd-limit; i++)
				PixelWrite(i, Object[channel].color, 1);
		}
		break;

	case TYPE_POINT:
		if( (Object[channel].start - step) == 1 || (step - Object[channel].start)==1 ||
		   (Object[channel].start==0 && step==199 ) || (step==0 && Object[channel].start==199) )
		{
			PixelWrite(Object[channel].radius, Object[channel].color, 1);
			PixelWrite(Object[channel].radius-1, Object[channel].color, 1); //optional
			PixelWrite(Object[channel].radius-2, Object[channel].color, 1); //optional
		}
		if(Object[channel].start == step)
		{
			for(uint8_t i=Object[channel].radius-1; i<=Object[channel].radius+1; i++)
				PixelWrite(i, Object[channel].color, 1);
		}
		break;
	}
}


/*******************************************************************************
* Function Name  : isInBound
* Description    : define is the current step belong to bounds
* Input		 	 : bounds, current step
*******************************************************************************/
bool isInBound(uint8_t begin, uint8_t end, uint8_t curStep)
{
	if(begin == end)
	{
		if(curStep == begin) return 1;
	}
	else if(begin < end)
	{
		if(curStep >= begin && curStep <= end) 	return 1;
		else							 		return 0;
	}
	else if(begin > end)
	{
		if(curStep >= begin || curStep <= end) 	return 1;
		else							 		return 0;
	}

	return 0;
}


/*
 **
const uint16_t SINTable[50] = {
0,		32,		64,		96,		128,	160,	192,	223,	255,	286,
316,	347,	377,	407,	436,	465,	493,	521,	549,	576,
602,	628,	653,	677,	701,	724,	746,	768,	789,	809,
828,	847,	865,	881,	897,	912,	927,	940,	952,	963,
974,	983,	992,	999,	1006,	1011,	1016,	1019,	1022,	1023
};*/
/*
//STRAIGHT LINEs
void XYfromRa(uint8_t len, uint8_t angle, signed char *x, signed char *y)
{
	if(angle >=0 && angle < 50)
	{
		*x = (signed char)round(   (len * SINTable[angle-1]) /1024 );
		*y = (signed char)round(  (len * (1024 - SINTable[angle-1])) /1024  );
	}

	if(angle >= 50 && angle < 100)
	{
		*y = (signed char)round(  (len * (1023 - SINTable[angle-50-1])) /1024  );
		*x = (signed char)round(  (len * SINTable[100-angle-1]) /1024  );
	}

	if(angle >=100 && angle < 150)
	{

	}

	if(angle >= 150)
	{

	}
}
*/
