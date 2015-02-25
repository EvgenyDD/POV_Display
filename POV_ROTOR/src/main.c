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
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "core_cm3.h"

#include "debug.h"
#include "string.h"
#include "draw.h"
#include "display_HAL.h"
#include "timework.h"
#include "TxRx.h"
#include "UI.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define BitGet(p,m) ((p) & (m))
#define BitSet(p,m) ((p) |= (1<<(m)))
#define BitReset(p,m) ((p) &= ~(1<<(m)))
#define BitToggle(p,m) ((p) ^= (m))
#define BitWrite(c,p,m) ((c) ? BitSet(p,m) : BitReset(p,m))
#define BitIsSet(reg, bit) ((reg & (1<<(bit))) != 0)
#define BitIsReset(reg, bit) ((reg & (1<<(bit))) == 0)

#define abs(x)  ( (x)<0 ) ? (-(x)) : (x)


/* Private variables ---------------------------------------------------------*/
volatile uint16_t delay;					//for delay function
volatile uint16_t lowSpinCounter=0;			//step down counter for under spinning detection

volatile uint8_t step = 0;

volatile bool flagGoodSpeed = FALSE;
volatile bool flagNewLap = FALSE;
volatile bool flagNewStep = FALSE;

	//uint32_t RX_data = 255;

	//uint16_t fmNumReports = 0;
	//uint16_t fmFAILReports=0;

	//char  sss[20]={"123"};

	//uint32_t spinFreq=12000;


/* Extern variables ----------------------------------------------------------*/
extern volatile uint8_t DispBuf[12];
//extern SettingsArray sett;

extern TextDispStruct Text[30];
extern ObjDispStruct Object[30];

extern const char font3x5[];
extern const char font5x8[];

//extern uint8_t TX_counter;
extern volatile uint16_t transCounter;

extern volatile signed int metronomePos;
extern volatile signed int metronomePeriod;


/* Private function prototypes -----------------------------------------------*/
int Dispatcher();


/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : __delay_ms
* Description    : Delay the code on the N cycles of SysTick Timer.
* Input          : N Delay Cycles.
*******************************************************************************/
void __delay_ms( volatile uint32_t nTime )
{
	delay = nTime;
	while(delay);
}


/*******************************************************************************
* Function Name  : SysTick_Handler
* Description    : Handles SysTick Timer Interrupts every 1ms.
*******************************************************************************/
void SysTick_Handler(void)
{
	if(delay) delay--;
	if(transCounter) transCounter--;
	if(lowSpinCounter) lowSpinCounter--;

	if(metronomePeriod)
	{
		static bool metrDir = FALSE;
		if(metrDir)
		{
			if(++metronomePos >= metronomePeriod)
			{
				metronomePos = metronomePeriod;
				metrDir = FALSE;
			}
		}
		else
		{
			if(--metronomePos <= -metronomePeriod)
			{
				metronomePos = -metronomePeriod;
				metrDir = TRUE;
			}
		}
	}
	else
		metronomePos = 0;
}


/*******************************************************************************
* Function Name  : Init
* Description    : Initialize the chip peripheral.
*******************************************************************************/
void Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TimeBaseInitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	RCC_DeInit();

	// Enable HSI
	RCC_HSEConfig(RCC_HSE_OFF);
	RCC_HSICmd(ENABLE);

	// Enable Prefetch Buffer
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	// Flash 2 wait state
	FLASH_SetLatency(FLASH_Latency_2);

	// HCLK = SYSCLK
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	// PCLK2 = HCLK
	RCC_PCLK2Config(RCC_HCLK_Div1);
	// PCLK1 = HCLK
	RCC_PCLK1Config(RCC_HCLK_Div1);
	// PLL Clock
	RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16);
	// Enable PLL
	RCC_PLLCmd(ENABLE);
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
	// Select PLL as system clock source
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	// Wait till PLL is used as system clock source
	while (RCC_GetSYSCLKSource() != 0x08);

	//System Clock = (64Mhz) / 64000 = 1000Hz = 1ms reload
	//System Clock = (64Mhz) / 12800 = 5000Hz = 0.2ms reload
	SysTick_Config(64000);

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable , ENABLE);

//RTC
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Disable Backup Domain write protection */
	PWR->CR |= PWR_CR_DBP;

	/* Enable LSI */
	RCC_BackupResetCmd(ENABLE);
	RCC_BackupResetCmd(DISABLE);

	/* Enable LSE - it starts very long */
	RCC_ITConfig(RCC_IT_LSERDY, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = RCC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(RCC_IRQn);
	RCC_LSEConfig(RCC_LSE_ON);

//Peripheral clocking
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB
			| RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

//GPIO
	//ADC - Vsens
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//PWM
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//SPI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	//FM in
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


//Timer
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); //200 step counting
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //PWM
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //FM capture
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //frame capture

	//TIM1 - 200 step counting
	TimeBaseInitStructure.TIM_Prescaler = 3;
	TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TimeBaseInitStructure.TIM_Period = 10000;
	TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TimeBaseInitStructure);

	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(TIM1_UP_IRQn);

	//TIM2
	TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TimeBaseInitStructure.TIM_Prescaler = 0;
	TimeBaseInitStructure.TIM_Period = 1000;
	TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TimeBaseInitStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_Pulse = 1000;

	TIM_OC3Init(TIM2, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);


	//TIM3 - 18-30 khz capture
	TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TimeBaseInitStructure.TIM_Prescaler = 0;
	TimeBaseInitStructure.TIM_Period = 8000;
	TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TimeBaseInitStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;//TIM_ICPolarity_Falling
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;//TIM_ICSelection_DirectTI
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;//TIM_ICPSC_DIV1
	TIM_ICInitStructure.TIM_ICFilter = 3;//3
	TIM_ICInit(TIM3, &TIM_ICInitStructure);

	TIM_SelectInputTrigger(TIM3, TIM_TS_TI2FP2);
	TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);

	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(TIM3_IRQn);


	//TIM4 - 15-40 Hz frame capture
	/* Hz - Counter value [63800/Hz = Counter value]
 	 * 40 - 1594 * 35 - 1823 * 30 - 2126
 	 * 25 - 2552 * 20 - 3190 * 18 - 3545
	 * 15 - 4254 * 12 - 5318 * 10 - 6380 */
	TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TimeBaseInitStructure.TIM_Prescaler = 999; //64.000 hz count //63800 exact
	TimeBaseInitStructure.TIM_Period = 65000;
	TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &TimeBaseInitStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0;//3
	TIM_ICInit(TIM4, &TIM_ICInitStructure);

	TIM_SelectInputTrigger(TIM4, TIM_TS_TI2FP2);
	TIM_SelectSlaveMode(TIM4, TIM_SlaveMode_Reset);

	TIM_ITConfig(TIM4, TIM_IT_CC2, ENABLE);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 10;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(TIM4_IRQn);

	TIM_Cmd(TIM1, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
	TIM_Cmd(TIM4, ENABLE);

	/* Watchdog - Base frequency 40kHz */
	//DOG-Time(sec.) = reloadValue * prescaler / 40000
	//ReloadValue = DOG-Time(sec.) * 40000 / prescaler
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_8);
	IWDG_SetReload(500); //100ms
	IWDG_ReloadCounter();
	IWDG_Enable();
}


/*******************************************************************************
* Function Name  : main
* Description    : Main cycle
*******************************************************************************/
int main(void)
{
	Init();
	HALDisplayInit();
	DebugInit();
	UI_Init();

	/* wait until rotor spins up */
/*	while(!Dispatcher())
		IWDG_ReloadCounter();

	TxAddMessage(Message_GetTimeDateSett);*/

    while(1)
    {
    	Dispatcher();
    	IWDG_ReloadCounter();

    	/* New Lap */
    	if(flagNewLap == TRUE)
    	{
    		flagNewLap = FALSE;

/*todo
* voltage sensor on GPIOA.0 - ?
* ...
*/

#if 0
    		char s[40];

    		/* spin frequency */
    		itoa_(/*spinFreq*/emp, s);
			DisplayTextAdd(0,s, &font5x8[0], BLUE, 40, 28-8-8, ROUTE_BOTTOM, 3);

    		/* debug FM */
    		/*itoa_(RX_data, s);
			DisplayTextAdd(1, s, &font5x8[0], GREEN, 60+50, 28-8, ROUTE_BOTTOM, 3);

			itoa_(fmFAILReports, s);
			DisplayTextAdd(2, s, &font5x8[0], RED, 30, 28-8, ROUTE_TOP, 3);

			itoa_(fmNumReports, s);
			DisplayTextAdd(3, s, &font5x8[0], MAGENTA, 1, 28-8, ROUTE_TOP, 3);

			if(sss[0]=='G')
				DisplayTextAdd(5, sss, &font5x8[0], BLUE, 150, 28-8, ROUTE_TOP, 3);
			else
				DisplayTextAdd(5, sss, &font5x8[0], RED, 150, 28-8, ROUTE_TOP, 3);*/

    		//GPIOA->BRR = GPIO_Pin_4;
    		//GPIOA->BRR = GPIO_Pin_6;
#endif
    	}
    }
}


/*******************************************************************************
* Function Name  : RCC_IRQHandler
* Description    : "LSE is stable" - ~1.1sec after system start
*******************************************************************************/
void RCC_IRQHandler(void)
{
	if(RCC_GetITStatus(RCC_IT_LSERDY) != RESET)
	{
		RCC_ClearITPendingBit(RCC_IT_LSERDY);

		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);

		RTC_SetPrescaler(0x7FFF); // divide/32768
		RTC_WaitForSynchro();
	}
}


/*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : FM input handler
*******************************************************************************/
void TIM3_IRQHandler(void)
{
	/* New FM period capture */
	if(TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);

		static uint8_t RX_debounce = 0;
		static volatile uint32_t RX_prevCapture = 0;

		/* Debounce (if new frequency appears) */
		if(RX_debounce)
		{
			RX_debounce--;
			return;
		}

		uint32_t RX_currCapture = TIM_GetCapture2(TIM3);

		/* Is Frequency belongs to FM interval */
		if( RX_currCapture > 3100 || RX_currCapture < 2110 ) return;

		/* New Frequency appeared */
		if( abs(RX_currCapture - RX_prevCapture) > 70 )
		{
			RX_debounce = 7; //8
			RX_prevCapture = RX_currCapture;
			return;
		}
		else
			RX_prevCapture = RX_currCapture;

																	//FREQ  					  Counter value
																	//0		- FREQ20 - Default	- 3208-3215
		if(      RX_currCapture > 3020 && RX_currCapture < 3090 )	//I 	- FREQ21 - START	- 3054-3058
			RX_Dispatcher(1);
		else if( RX_currCapture > 2890 && RX_currCapture < 2960 )	//II 	- FREQ22 **->00		- 2914-2925
			RX_Dispatcher(2);
		else if( RX_currCapture > 2760 && RX_currCapture < 2840 )	//III 	- FREQ23 **->01		- 2792-2800
			RX_Dispatcher(3);
		else if( RX_currCapture > 2640 && RX_currCapture < 2705 )	//IV 	- FREQ24 **->10		- 2675-2685
			RX_Dispatcher(4);
		else if( RX_currCapture > 2540 && RX_currCapture < 2590 )	//V 	- FREQ25 **->11		- 2568-2572
			RX_Dispatcher(5);
		else if( RX_currCapture > 2450 && RX_currCapture < 2495 )	//VI 	- FREQ26 **->00		- 2472-2475
			RX_Dispatcher(6);
		else if( RX_currCapture > 2340 && RX_currCapture < 2400 )	//VII 	- FREQ27 **->01		- 2374-2379
			RX_Dispatcher(7);
		else if( RX_currCapture > 2260 && RX_currCapture < 2315 )	//IIX 	- FREQ28 **->10		- 2290-2294
			RX_Dispatcher(8);
		else if( RX_currCapture > 2190 && RX_currCapture < 2240 )	//IX 	- FREQ29 **->11		- 2214-2219
			RX_Dispatcher(9);
		else if( RX_currCapture > 2115 && RX_currCapture < 2160 )	//X 	- FREQ30 - END		- 2140-2145
			RX_Dispatcher(10);
	}

	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{	//No Power+FM transmitting
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}


/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : New lap handler
*******************************************************************************/
void TIM4_IRQHandler(void)
{
	/* New Lap */
	if(TIM_GetITStatus(TIM4, TIM_IT_CC2) == SET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);

		static uint32_t prevCapture = 0;
		uint32_t tempCapture = TIM_GetCapture2(TIM4);

		if(tempCapture >= 16000 || tempCapture < 800)
		{/* Not enough rotation speed */
			flagGoodSpeed = DISABLE;
			TIM1->ARR = 16000;
			TIM1->EGR = TIM_PSCReloadMode_Immediate;
		}
		else
		{/* Enough rotation speed */
			// = 8 000 000 * capture_tim_4 / (Nsteps * 64000)
			TIM1->ARR = (uint16_t)( ((uint32_t)( (prevCapture+tempCapture) *5)) >>3 );// /*5/4
			TIM1->EGR = TIM_PSCReloadMode_Immediate;

			prevCapture = tempCapture;

			step = STEPSHIFT; //compensation for not-12-hour sensor position

//	spinFreq = tempCapture; // ************************ FOR DEBUG

			flagNewLap = TRUE;
			flagGoodSpeed = TRUE;
			lowSpinCounter = 1000;
		}

		return;
	}

	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

		/* Not enough rotation speed */
		if(!lowSpinCounter)
		{
			flagGoodSpeed = FALSE;
			TIM1->ARR = 16000;
			TIM1->EGR = TIM_PSCReloadMode_Immediate;
		}
		return;
	}
}


/*******************************************************************************
* Function Name  : TIM1_UP_IRQHandler
* Description    : Step (200/round) handler
*******************************************************************************/
void TIM1_UP_IRQHandler()
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
	{
		DMA_Cmd(DMA1_Channel3, DISABLE);
		DMA_SetCurrDataCounter(DMA1_Channel3, 12);
		DMA_Cmd(DMA1_Channel3, ENABLE);

		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		flagNewStep = ENABLE;
	}
}


/*******************************************************************************
* Function Name  : Dispatcher
* Description    : Main picture dispatcher
*******************************************************************************/
int Dispatcher()
{
	/* New Step */
	if(flagNewStep == TRUE)
	{
		flagNewStep = FALSE;

		/* Wait until 96 bit DMA transfer complete */
		while(SPI1->SR & SPI_I2S_FLAG_BSY);

		/* Send "LOAD" impulse to 595 registers */
		GPIOA->BSRR = GPIO_Pin_6;
		GPIOA->BRR = GPIO_Pin_6;

		if(++step >= 200) step = 0;

		ColOff();

		if(flagGoodSpeed)
		{
			/* IR Transmitter */
			TX_Process(step);

			//Text dispatcher
			for(uint8_t i=0; i<25; i++)
			{
				if(Text[i].flagEn) DisplayTextProcess(step, i);
			}

			//Object dispatcher
			for(uint8_t i=0; i<30; i++)
			{
				if(Object[i].flagEn) DisplayObjectProcess(step, i);
			}

			static uint8_t backCnt = 0;
			if(++backCnt >= 50)
			{
				backCnt = 0;
				UI_Dispatcher();
			}

			/* if the new second appears... */
			/*static uint32_t prevRTC = 0;
			if(prevRTC != RTC_GetCounter())
			{
				prevRTC = RTC_GetCounter();
			}*/
			return 1;
		}
		else
		{
			/* turn on first and last RED led's */
			uint8_t position = 31*3 + 1;
			BitWrite(1, DispBuf[(11-position/8)], position%8);//31-st
			position = 0*3 + 1;
			BitWrite(1, DispBuf[(11-position/8)], position%8);//0-s

			return 0;
		}

#if 0
//STRAIGHT LINEs
		if(step <100)
		{
			for(uint8_t i=0; i<32; i++)
			{
				signed char x=0, y=0;
				XYfromRa(i, step, &x, &y);

				if(x==10)// && (((k*x+b)-y)<2))//((y<=(k*x+b)))
				{
					PixelWrite(i, GREEN, 1);
				}
			}
		}
#endif
	}

	return 0;
}
