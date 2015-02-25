/**
  ******************************************************************************
  * @file  	 RC5.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    03/16/2010
  * @brief   This file provides all the RC5 firmware functions.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * COPYRIGHT 2010 STMicroelectronics
  */


/* Includes ------------------------------------------------------------------*/
#include "RC5.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_exti.h"

#include "debug.h"
#include "string.h"

#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_syscfg.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#if 0
#define   NOMINAL_HALF_BIT_TIME_US   889  /* Nominal half bit time in 탎 */
#define   MIN_HALF_BIT_TIME_US       640  /* Minimum half bit time in 탎 */
#define   MAX_HALF_BIT_TIME_US       1140 /* Maximum half bit time in 탎 */

#define   NOMINAL_FULL_BIT_TIME_US   1778 /* Nominal Full bit time in 탎 */
#define   MIN_FULL_BIT_TIME_US       1340 /* Minimum Full bit time in 탎 */
#define   MAX_FULL_BIT_TIME_US       2220 /* Maximum Full bit time in 탎 */
#else
#define   NOMINAL_HALF_BIT_TIME_US   680  /* Nominal half bit time in 탎 */
#define   MIN_HALF_BIT_TIME_US       610  /* Minimum half bit time in 탎 */
#define   MAX_HALF_BIT_TIME_US       1500 /* Maximum half bit time in 탎 */

#define   NOMINAL_FULL_BIT_TIME_US   1778 /* Nominal Full bit time in 탎 */
#define   MIN_FULL_BIT_TIME_US       1550 /* Minimum Full bit time in 탎 */
#define   MAX_FULL_BIT_TIME_US       2500 /* Maximum Full bit time in 탎 */
#endif
#define   RC5_TIM_PRESCALER          2


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint8_t EdgesNumber = 0; /* To count the first edges of the frame (max 3 edges) */
StatusYesOrNo Bit4_HasBeen_Sampled = NO; /* To know if the next sampling will be after 3/4 bit time or one bit time */
StatusYesOrNo RC5_FrameReceived = NO; /* Will be used by the user to check if an RC5 has been received or not */
BitTimigStatus BitTimeStatus = OK; /* Variable to store the timing status of the first low duration */
__IO uint8_t FieldBit = 0; /* Bit field value (command between 0-63 or 64-127 */
__IO uint16_t RC5_data[13]; /* Table that contains the value of the GPIOx_IDR register each sampling */
uint8_t RC5_Indexdata = 0; /* Variable that increments each time a RC5 bit is sampled */
__IO uint16_t LowDuration = 0; /* Contains the first low duration in in TIM count */
__IO uint32_t RC5_TIM_CLKValuekHz = 0; /* Contains the frequency input of RC5_TIM in Khz */
uint16_t HalfBitDurationCount_Min = 0; /* Minimum Half bit duration in TIM count */
uint16_t HalfBitDurationCount_Max = 0; /* Maximum Half bit duration in TIM count*/
uint16_t NominalHalfBitDurationCount  = 0; /* Nominal Half bit duration in TIM count */
uint16_t FullBitDurationCount_Min = 0; /* Minimum Full bit duration in TIM count */
uint16_t FullBitDurationCount_Max = 0; /* Maximum Full bit duration in TIM count */
uint16_t NominalBitDurationCount  = 0; /* Nominal Full bit duration in TIM count */
uint16_t NominalBitDurationCount_3div4 = 0; /* 3/4 of nominal bit time in TIM count */
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;


/* Extern variables ----------------------------------------------------------*/
extern volatile uint16_t IrDebounce;


/* Private function prototypes -----------------------------------------------*/
void NewLapHandler(); //in "main.c"


/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : RC5Init
* Description    : Init IR remote command decoding
*******************************************************************************/
void RC5Init()
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	//RC5 input pin
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* System Clocks Configuration for RC5 reception */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Get frequency input of RC5_TIM in Khz */
	RC5_TIM_CLKValuekHz = RC5_TIM_GetCounterCLKValue()/1000; //==16000

	/* Compute the different timing tolerences in TIM count */
	NominalBitDurationCount =  (RC5_TIM_CLKValuekHz * NOMINAL_FULL_BIT_TIME_US)/1000;
	NominalHalfBitDurationCount = (RC5_TIM_CLKValuekHz * NOMINAL_HALF_BIT_TIME_US)/1000;
	HalfBitDurationCount_Min = (RC5_TIM_CLKValuekHz * MIN_HALF_BIT_TIME_US)/1000;
	HalfBitDurationCount_Max = (RC5_TIM_CLKValuekHz * MAX_HALF_BIT_TIME_US)/1000;
	FullBitDurationCount_Min = (RC5_TIM_CLKValuekHz * MIN_FULL_BIT_TIME_US)/1000;
	FullBitDurationCount_Max = (RC5_TIM_CLKValuekHz * MAX_FULL_BIT_TIME_US)/1000;

	/* Compute the 3/4 bit time duration in TIM count */
	NominalBitDurationCount_3div4 = (NominalBitDurationCount * 3)/4;

	/* Enable EXTIx to detect the start bit of the RC5 frame */
	EXTI_ClearITPendingBit(EXTI_Line7);
	EXTI_InitStructure.EXTI_Line = EXTI_Line7;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Connect EXTI Line x to RC5 pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource7);

	/* Enable the EXTIx global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the RC5_TIM global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM17_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Wait for next falling edge of RC5 frame */
	RC5_WaitForNextFallingEdge();
}


/*
* Compute the RC5_TIM frequency input in Hz.
* Return	 : RC5_TIM Frequency value in Hz
*/
uint32_t RC5_TIM_GetCounterCLKValue(void)
{
	uint32_t apbprescaler = 0, apbfrequency = 0;

	RCC_ClocksTypeDef RCC_ClockFreq;

	/* This function fills the RCC_ClockFreq structure with the current
	frequencies of different on chip clocks */
	RCC_GetClocksFreq(&RCC_ClockFreq);

	/* Get the clock prescaler of APB2 */
	apbprescaler = ((RCC->CFGR >> 11) & 0x7);
	apbfrequency = RCC_ClockFreq.PCLK_Frequency;

	/* If APBx clock div >= 4 */
	if (apbprescaler >= 4)
		return ((apbfrequency * 2)/(RC5_TIM_PRESCALER + 1));
	else
		return (apbfrequency/(RC5_TIM_PRESCALER + 1));
}


// Configure the system to receive the next RC5 frame
void RC5_WaitForNextFallingEdge(void)
{
	/* Enable EXTI line x */
	EXTI->IMR |= EXTI_Line7;

	/* RC5_TIM Configuration ------------------------------------------------------*/
	TIM_DeInit(TIM17);
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler = RC5_TIM_PRESCALER;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM17, &TIM_TimeBaseStructure);

	/* Clear RC5_TIM Capture compare interrupt pending bit */
	TIM_ClearITPendingBit(TIM17, TIM_IT_Update);
}


// Sample the RC5 data bits
void TIM17_IRQHandler()
{
	if (TIM_GetITStatus(TIM17, TIM_IT_Update) != RESET) /* Update event occured */
	{
		/* Sample RC5 GPIO input */
		RC5_data[RC5_Indexdata++] = GPIOB->IDR & GPIO_Pin_7;

		if(Bit4_HasBeen_Sampled == NO) /* RC5 bit 4 has been sampled? */
		{                              /*(to know if the next sampling will be after 3/4 or 1 bit time) */

			/* If NO: Set the RC5_TIM auto-reload register to allow the next sampling at 1bit time */
			TIM17->ARR = NominalBitDurationCount;

			/* Set the variable to yes */
			Bit4_HasBeen_Sampled = YES;
		}

		/* If the number of data reaches 13 (without start bit and field bit and extra bit is sampled: (14-2) + 1) */
		if(RC5_Indexdata == 13)
		{
			/* Initialize the RC5 data index */
			RC5_Indexdata = 0;

			/* Set this flag software to inform the user that a RC5 frame is ready to be read */
			RC5_FrameReceived = YES;

			/* Initialize  Bit4_HasBeen_Sampled variable for next reception */
			Bit4_HasBeen_Sampled = NO;

			/* Disable RC5_TIM Update event Interrupt Request */
			TIM_ITConfig(TIM17, TIM_IT_Update, DISABLE);

			/* Disable RC5_TIM counter */
			TIM_Cmd(TIM17, DISABLE);
		}

		/* Clear RC5_TIM Update event interrupt pending bit */
		TIM_ClearITPendingBit(TIM17, TIM_IT_Update);
	}
}


// Measure the first low duration of the RC5 frame
void EXTI4_15_IRQHandler()
{
	/* If an edge is detected on GPIOA.6 input pin */
	if(EXTI_GetITStatus(EXTI_Line6) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line6);

		if(IrDebounce == 0)
			NewLapHandler();

		return;
	}

	/* If an edge is detected on RC5 input pin */
	if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	{
		/* Increment the edges number */
		EdgesNumber++;

		/* If it was the first edge */
		if(EdgesNumber == 1)
		{
			/* Reset the RC5_TIM counter */
			TIM17->CNT = 0;

			/* Enable RC5_TIM counter */
			TIM_Cmd(TIM17, ENABLE);
		}
		/* If it was the 2nd edge */
		else if(EdgesNumber == 2)
		{
			/* Disable RC5_TIM counter */
			TIM_Cmd(TIM17, DISABLE);

			/* Read RC5_TIM counter to get the first low duration */
			LowDuration = TIM17->CNT;

			/* Reset RC5_TIM counter */
			TIM17->CNT = 0;

			/* If low duration is between 640탎*TimClk and 1140탎*TimClk (min and max half bit time) */
			if ((LowDuration >= HalfBitDurationCount_Min) && (LowDuration <= HalfBitDurationCount_Max))
			{
				/* Validate the frame, by setting this variable to OK */
				BitTimeStatus = OK;
			}
			/* If low duration is between 1340탎*TimClk and 2220탎*TimClk (min and max full bit time)*/
			else if ((LowDuration >= FullBitDurationCount_Min) && (LowDuration <= FullBitDurationCount_Max))
			{
				/* Disable EXTI line x to avoid jumping to this interrupt while receiving
				the RC5 data bits (it will be reenabled in the next RC5 data reception */
				EXTI->IMR &= ((uint32_t)~EXTI_Line7);

				/* Enable RC5_TIM counter */
				TIM_Cmd(TIM17, ENABLE);

				/* Enable RC5_TIM Update Event Interrupt Request */
				TIM_ITConfig(TIM17, TIM_IT_Update, ENABLE);

				/* Validate the frame, by setting this variable to OK */
				BitTimeStatus = OK;

				/* Set the RC5_TIM auto-reload register to allow the next sampling at 3/4 bit time */
				TIM17->ARR = NominalBitDurationCount_3div4;

				/* The bit field value is equal to 0 */
				FieldBit = 0;

				/* Initialize edges detected number */
				EdgesNumber = 0;
			}
			else /* If the first low duration is not in the right timing range */
			{
				/* Set the Bit timing to NOTOK */
				BitTimeStatus = NOTOK;

				/* Reset RC5_TIM counter */
				TIM17->CNT = 0;

				/* Initialize the number of glitches detected */
				EdgesNumber = 0;
			}
		}
		else if(EdgesNumber == 3) /* If the number of edges is equal to 3 */
		{
			/* Disable EXTI line x to avoid jumping to this interrupt while receiving
			the RC5 data bits (it will be reenabled in the next RC5 data reception */
			EXTI->IMR &= ((uint32_t)~EXTI_Line7);

			/* Enable RC5_TIM counter */
			TIM_Cmd(TIM17, ENABLE);

			/* Enable RC5_TIM Update Event Interrupt Request */
			TIM_ITConfig(TIM17, TIM_IT_Update, ENABLE);

			/* Validate the frame, by setting this variable to OK */
			BitTimeStatus = OK;

			/* Set the RC5_TIM auto-reload register to allow the next sampling at 3/4 bit time */
			TIM17->ARR = NominalBitDurationCount_3div4;

			/* The bit field value is equal to 1 */
			FieldBit = 1;

			/* Initialize the number of glitches detected */
			EdgesNumber = 0;
		}

		/* Clear the RC5 EXTI line pending bit */
		EXTI_ClearITPendingBit(EXTI_Line7);
	}
}


// Decode the RC5 frame
RC5Frame_TypeDef RC5_Decode(void)
{
	RC5Frame_TypeDef RC5_frame_struct;
	uint32_t RC5_dataIndex = 0;

	/* Initialize the different fields of the RC5 structure */
	RC5_frame_struct.ToggleBit = 0;
	RC5_frame_struct.Address = 0;
	RC5_frame_struct.Command = 0;

	/* Get the Toggle bit value */
	if(RC5_data[0] & GPIO_Pin_7)
		RC5_frame_struct.ToggleBit = 1;
	else
		RC5_frame_struct.ToggleBit = 0;

	/* Get RC5 Address value */
	for(RC5_dataIndex=1; RC5_dataIndex<6; RC5_dataIndex++)
	{
		RC5_frame_struct.Address <<= 1 ;

		if(RC5_data[RC5_dataIndex] & GPIO_Pin_7)
			RC5_frame_struct.Address |= 1;
	}

	/* Get RC5 Command value */
	for(RC5_dataIndex=6; RC5_dataIndex<12; RC5_dataIndex++)
	{
		RC5_frame_struct.Command <<= 1 ;

		if(RC5_data[RC5_dataIndex] & GPIO_Pin_7)
			RC5_frame_struct.Command |= 1;
	}

	/* Set the 6th bit of the command regarding of the filed bit value */
	if(FieldBit == 0) /* logic 0 = command from 64 to 127 */
		RC5_frame_struct.Command |= 0x40;

	/* Initialize RC5_FrameReceived for next RC5 reception */
	RC5_FrameReceived = NO;

	/* Wait for next falling edge of RC5 frame*/
	RC5_WaitForNextFallingEdge();

	/* Return the RC5 struct */
	return RC5_frame_struct;
}
