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
#include "FM.h"
#include "stm32f0xx_tim.h"
#include "debug.h"
#include "string.h"
#include "stm32f0xx_rtc.h"
#include "timework.h"
#include "sound.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
const uint16_t FMFreq[12] ={FREQ19K, FREQ20K, FREQ21K, FREQ22K, FREQ23K, FREQ24K,
							FREQ25K, FREQ26K, FREQ27K, FREQ28K, FREQ29K, FREQ30K};


/* Private macro -------------------------------------------------------------*/
#define BitSet(p,m) ((p) |= (1<<(m)))
#define BitReset(p,m) ((p) &= ~(1<<(m)))
#define BitFlip(p,m) ((p) ^= (m))
#define BitWrite(c,p,m) ((c) ? BitSet(p,m) : BitReset(p,m))
#define BitIsSet(reg, bit) (((reg) & (1<<(bit))) != 0)
#define BitIsReset(reg, bit) (((reg) & (1<<(bit))) == 0)


/* Private variables ---------------------------------------------------------*/
volatile uint8_t powMul = 14; //from 8 to 25(?); higher = more power to rotor

RTC_TimeTypeDef TimeTemp;
RTC_DateTypeDef DateTemp;


/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void delay_ms(volatile uint32_t);


/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : FMInit
* Description    : Initialize frequency modulation & power module
*******************************************************************************/
void FMInit()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_1); 	//TIM3_CH2 - FM

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//Timer 3 - FM
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = 3;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = FREQ20K;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);

	TIM_Cmd(TIM3, ENABLE);

	FMOff();
}


/*******************************************************************************
* Function Name  : FMOff
* Description    : Turn off frequency generator
*******************************************************************************/
void FMOff()
{
	//FREQ = 12800 / kHz
	TIM3->ARR = FREQ20K;
	TIM3->CCR2 = 0;
	TIM3->EGR = TIM_PSCReloadMode_Immediate;
}


/*******************************************************************************
* Function Name  : FMSetFreq
* Description    : Turn off frequency generator
*******************************************************************************/
void FMSetFreq(uint16_t freq, uint16_t period)
{
	assert_param(freq >= FREQ30K && freq <= FREQ19K);

	//FREQ = 12800 / kHz
	TIM3->ARR = freq;
	TIM3->CCR2 = period;
	TIM3->EGR = TIM_PSCReloadMode_Immediate;

	//GPIOA->ODR ^= GPIO_Pin_12;
}


/*******************************************************************************
* Function Name  : FMGetFreq
* Description    : Get FM frequency
*******************************************************************************/
uint16_t FMGetFreq(){
	return TIM3->CCR2;
}


/*******************************************************************************
* Function Name  : FMSendData
* Description    : Send data to rotor via frequency modulation
* Input			 : data [32bit], amount [8,16,32]
********************************************************************************/
void FMSendData(uint32_t inData, uint8_t amount)
{
	/* FM FRAME FORMAT
	 * 0: I frequency  			start of packet 21
	 * 1: II+III+IV+V frequency		data 8-32 bit
	 * 2: VI+VII+IIX+IX frequency	data 8-32 bit
	 * n: X frequency			end of packet
	 */
	assert_param(amount==8 || amount==16 || amount==32);

#define FM_DELAY 3  //ms

	/* start */
	FMSetFreq(FREQ21K, FREQ21K*powMul/64);
	//FM_DebugSendChar('1');
	delay_ms(FM_DELAY);

	uint8_t prevBit = 0;

	for(uint8_t i=0; i<amount/2; i++)
	{
		uint8_t data = 0;

		BitWrite(BitIsSet(inData, i*2), data, 0);
		BitWrite(BitIsSet(inData, i*2+1), data, 1);

		if(prevBit>=4 && (prevBit%4)==data)
		{
			FMSetFreq(FMFreq[data+3], FMFreq[data+3]*powMul/64);
			//FM_DebugSendChar('0'+data+2);
			prevBit = data;
		}
		else
		{
			FMSetFreq(FMFreq[data+3+4], FMFreq[data+3+4]*powMul/64);
			//FM_DebugSendChar('0'+data+2+4);
			prevBit = data + 4;
		}

		delay_ms(FM_DELAY);
	}

	/* end */
	FMSetFreq(FREQ30K, FREQ30K*powMul/64);
	//FM_DebugSendChar('A');
	delay_ms(FM_DELAY);
	FMSetFreq(FREQ20K, FREQ20K*powMul/64);

	/* Send debug message */
	switch(amount)
	{
	case 8:
		DebugSendNumWDesc("> Send command = ", inData);
		break;

	case 16:
		DebugSendNumWDesc("> Send num16(int) = ", inData);
		break;

	case 32:
		DebugSendNumWDesc("> Send num32(double) = ", inData);
		break;
	}
}

