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
#include "sound.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_dac.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_dma.h"

#include "debug.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum {FALSE=0, TRUE=!FALSE} bool;


/* Private define ------------------------------------------------------------*/
#define BUF_SIZE	16 //sound circular buffer size


const uint16_t Sine12bit[32] = {
	2047, 2447, 2831, 3185,
	3498, 3750, 3939, 4056,
	4095, 4056, 3939, 3750,
	3495, 3185, 2831, 2447,
	2047, 1647, 1263,  909,
	 599,  344,  155,   38,
	   0,   38,  155,  344,
	 599,  909, 1263, 1647
};

const uint16_t Meandr12bit[32] = {
	4095, 4095, 4095, 4095,
	4095, 4095, 4095, 4095,
	4095, 4095, 4095, 4095,
	4095, 4095, 4095, 4095,
	   0,    0,    0,    0,
	   0,    0,    0,    0,
	   0,    0,    0,    0,
	   0,    0,    0,    0
};

const uint16_t Saw12bit[32] = {
	   0,  132,  264,  396,
	 528,  660,  792,  924,
	1056, 1188, 1320, 1452,
	1584, 1716, 1848, 1980,
	2112, 2245, 2377, 2509,
	2641, 2773, 2905, 3037,
	3169, 3301, 3433, 3565,
	3697, 3829, 3961, 4095
};

const uint16_t Triangle12bit[32] = {
	   0, 264,  528,   792,
	1056, 1320, 1584, 1848,
	2112, 2377, 2641, 2905,
	3169, 3433, 3697, 3961,
	3965, 3701, 3437, 3173,
	2909, 2645, 2381, 2117,
	1853, 1589, 1324, 1060,
	 796,  532,  268,    4
};

/* Note frequencies (in Hz)
 * from C - low octave
 * till H - 4th octale
 * octave length - 12
 */
const uint16_t NoteMass[12*5] =
{
	131, 139, 148, 156, 165, 175, 185, 196, 207, 220, 233, 247,
	262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
	523, 554, 587, 622, 659, 698, 740, 784, 830, 880, 932, 988,
	1046,1109,1174,1244,1318,1397,1480,1568,1661,1720,1865,1975,
	2093,2217,2349,2489,2637,2794,2960,3136,3332,3440,3729,3951
};


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint16_t DAC12bit[32] = {0}; //current DAC massive
uint8_t currentWave = 0;
uint8_t currentVolume = 1;

volatile uint16_t soundTimer = 0;

bool noteState = FALSE; //playing/not playing

struct SoundCBType SoundCB[BUF_SIZE]; //circular buffer
uint8_t cbStart = 0, cbEnd = 0; //and it's indexes


/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : SoundInit
* Description    : Initialize DAC + TIMER2 + DMA
*******************************************************************************/
void SoundInit()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	DAC_InitTypeDef DAC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	/* RCC */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

	/* GPIO PA4 in analog (output) mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

#define DAC_FREQ 440
	int Period = (SystemCoreClock / (DAC_FREQ*32)); //200 000 = 200 KHz timebase, 6.25 KHz tone
			   //(SystemCoreClock / (20000 * 32)); // 20 KHz Sine, 32 sample wave table

	/* Timer2 */
	TIM_TimeBaseStructure.TIM_Period = Period-1;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* TIM2 TRGO selection: update event is selected as trigger for DAC */
	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
	TIM_Cmd(TIM2, ENABLE);

	/* DAC */
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	DAC_Cmd(DAC_Channel_1, ENABLE);
	DAC_DMACmd(DAC_Channel_1, ENABLE);

	/* DMA */
	DMA_DeInit(DMA1_Channel3);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&DAC->DHR12R1;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&DAC12bit;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 32;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // 16-bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel3, ENABLE);
}


/*******************************************************************************
* Function Name  : SoundSetWave
* Description    : Set wave type for DAC + set volume
*******************************************************************************/
void SoundSetWave(uint8_t wave)
{
	if(currentWave == wave) return; //return if no change
	else
	{
		currentWave = wave;

		switch(currentWave)
		{
		case WAVE_NULL:
			for(uint8_t i=0; i<32; i++) DAC12bit[i] = 0;
			break;
		case WAVE_SIN:
			for(uint8_t i=0; i<32; i++) DAC12bit[i] = Sine12bit[i]/currentVolume;
			break;
		case WAVE_MEANDR:
			for(uint8_t i=0; i<32; i++) DAC12bit[i] = Meandr12bit[i]/currentVolume;
			break;
		case WAVE_SAW:
			for(uint8_t i=0; i<32; i++) DAC12bit[i] = Saw12bit[i]/currentVolume;
			break;
		case WAVE_TRIANGLE:
			for(uint8_t i=0; i<32; i++) DAC12bit[i] = Triangle12bit[i]/currentVolume;
			break;
		}
	}
}


/*******************************************************************************
* Function Name  : SoundSetVolume
* Description    : Set volume
*******************************************************************************/
void SoundSetVolume(uint8_t volume)
{
	//volume - 7 = max sound
	//volume - 0 = min sound = silent

	//if(currentVolume == ((volume==0)?255:8-volume)) return; //return if no change
	//else
	{
		currentVolume = (volume==0)?255:8-volume;
		//SoundSetWave(currentWave);
	}
}


/*******************************************************************************
* Function Name  : SoundSetFreq
* Description    : Set note frequency
*******************************************************************************/
void SoundSetFreq(uint16_t freq)
{
	static uint16_t lastFreq = 0;

	if(lastFreq == freq) return; //return if no change
	else
	{
		lastFreq = freq;
		TIM2->ARR = (SystemCoreClock / (freq*32)) - 1;
		TIM2->EGR = TIM_PSCReloadMode_Immediate;
	}
}


/*******************************************************************************
* Function Name  : SoundPlayNote
* Description    : Add note to circular buffer
*******************************************************************************/
int SoundPlayNote(uint16_t note, uint8_t wave, uint16_t noteLen)
{
	assert_param(note <= 12*5);

	SoundCB[cbEnd].noteFreq = NoteMass[note];
	SoundCB[cbEnd].noteLen = noteLen;
	SoundCB[cbEnd].wave = wave;
	noteState = TRUE;

	if(++cbEnd == BUF_SIZE) cbEnd = 0;
	return 0;
}


/*******************************************************************************
* Function Name  : SoundDispatcher
* Description    : Process sound playing
*******************************************************************************/
void SoundDispatcher()
{
	if(noteState && !soundTimer)
	{
		if(cbStart != cbEnd)
		{ //take new note from buffer
			SoundSetWave(SoundCB[cbStart].wave);
			SoundSetFreq(SoundCB[cbStart].noteFreq);
			soundTimer = SoundCB[cbStart].noteLen;

			if(++cbStart == BUF_SIZE) cbStart = 0;
			noteState = TRUE;
		}
		else
		{ //stop sound playing = no new data in buffer
			noteState = FALSE;
			SoundSetWave(WAVE_NULL);
		}
	}
}
