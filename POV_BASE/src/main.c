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
#include "stm32f0xx.h"
#include "stm32f0xx_adc.h"
#include "stm32f0xx_dac.h"
#include "stm32f0xx_dma.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_flash.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_iwdg.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_pwr.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_rtc.h"
#include "stm32f0xx_syscfg.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_usart.h"
//#include "stm32f0xx_wwdg.h"

#include "motor.h"
#include "RC5.h"
#include "FM.h"
#include "debug.h"
#include "string.h"
#include "timework.h"
#include "sound.h"
#include "UI.h"


/* Private typedef -----------------------------------------------------------*/
typedef enum {FALSE=0, TRUE=!FALSE} bool;

/* Private define ------------------------------------------------------------*/
#define NO_ROTATION_PERIOD 1500//ms

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
volatile uint16_t delay;
RC5Frame_TypeDef RC5_Frame;
volatile uint16_t noRotationCounter = 0;

bool PowerFlag = FALSE;

volatile uint16_t IrDebounce = 0;
volatile bool RX_complete = FALSE;
MessageType RX_data;

RTC_TimeDateTypeDef RTC_TD_Temp;
RTC_TimeTypeDef TimeTemp;
RTC_DateTypeDef DateTemp;

volatile uint16_t AdcData[4];

volatile uint16_t currentSpinFreq = 0;
volatile uint16_t spinTimer = 0;


/* Extern variables ----------------------------------------------------------*/
extern StatusYesOrNo RC5_FrameReceived;
extern SettingsArray sett;

//extern volatile uint8_t powMul;
extern volatile uint16_t soundTimer;

extern volatile uint16_t metrCounter;
extern volatile bool metrFlag;

extern uint8_t currMenuID;

/* Private function prototypes -----------------------------------------------*/
void Debug_OutSysTimeDate();


/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : __delay_ms.
* Description    : Delay the code on the N cycles of SysTick Timer.
* Input          : N Delay Cycles.
*******************************************************************************/
void delay_ms( volatile uint32_t nTime )
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
	if(IrDebounce) IrDebounce--;
	if(soundTimer) soundTimer--;
	if(noRotationCounter) noRotationCounter--;
	if(spinTimer < 5000) spinTimer++;

	if(metrFlag && metrCounter)
	{
		if(--metrCounter == 0)
			MetronomeSound();
	}
}


/*******************************************************************************
* Function Name  : Init
* Description    : Initialize everything
*******************************************************************************/
void Init()
{
//RCC Peripheral
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);

//RCC
	RCC_HSICmd(ENABLE);
	while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

	// Init PLL (8MHZ/2)*12 = 48MHZ
	RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_12);
	RCC_PLLCmd(ENABLE);
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLKConfig(RCC_HCLK_Div1);

	FLASH_PrefetchBufferCmd(ENABLE);
	FLASH_SetLatency(FLASH_Latency_1);

//48MHz / 48000 = 1000 Hz interrupt
	SysTick_Config(48000);

//RTC
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	PWR_BackupAccessCmd(ENABLE);

	RCC_LSEConfig(RCC_LSE_ON);
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	RCC_RTCCLKCmd(ENABLE);

	RTC_InitTypeDef RTC_InitStructure;
	RTC_InitStructure.RTC_AsynchPrediv = 0x7f;//7f		0
	RTC_InitStructure.RTC_SynchPrediv = 0xFF;//ff  0x7fff
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);

/* GPIO */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 |   GPIO_Pin_11| GPIO_Pin_12; //GPIO_Pin_12|11-is tsc
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//touch// a+b

	//ADC + real speed input capture
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource6);

	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line6;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

/* ADC */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div2);

	/* Time base configuration */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1; // 1 MHz, from 48 MHz
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1; // 1 KHz
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	/* Output Compare PWM Mode configuration */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; /* low edge by default */
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0x01;
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);

	/* TIM1 enable counter */
	TIM_Cmd(TIM1, ENABLE);
	/* Main Output Enable */
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

	/* DMA1 Channel1 Config */
	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&AdcData[0];
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 4;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1,ENABLE);

	 /* ADC DMA request in circular mode */
	ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);
	ADC_DMACmd(ADC1, ENABLE);

	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
	ADC_InitStructure.ADC_ExternalTrigConv =  ADC_ExternalTrigConv_T1_CC4;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
	ADC_Init(ADC1, &ADC_InitStructure);

	// channels selection and it's sampling time config
	ADC_ChannelConfig(ADC1, ADC_Channel_8, ADC_SampleTime_55_5Cycles);
	ADC_ChannelConfig(ADC1, ADC_Channel_9, ADC_SampleTime_55_5Cycles);
	ADC_ChannelConfig(ADC1,ADC_Channel_Vbat,ADC_SampleTime_55_5Cycles);
	ADC_VbatCmd(ENABLE);
	ADC_ChannelConfig(ADC1, ADC_Channel_TempSensor , ADC_SampleTime_55_5Cycles);
	ADC_TempSensorCmd(ENABLE);

	/* ADC Calibration */
	ADC_GetCalibrationFactor(ADC1);
	/* Enable ADC1 */
	ADC_Cmd(ADC1,ENABLE);
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN));
	/* ADC1 regular Software Start Conversion */
	ADC_StartOfConversion(ADC1);

/* IWDG */
	//DOG-Time(sec.) = reloadValue * prescaler / 40000
	//ReloadValue = DOG-Time(sec.) * 40000 / prescaler
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_16);
	IWDG_SetReload(2500); //1000ms
	IWDG_ReloadCounter();
	IWDG_Enable();

#if 1
/* MCO */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_MCOConfig(RCC_MCOSource_LSE); // Put on MCO pin the: System clock selected
	//RCC_MCOConfig(RCC_MCOSource_HSE); // Put on MCO pin the: freq. of external crystal
	//RCC_MCOConfig(RCC_MCOSource_PLLCLK_Div2); // Put on MCO pin the: System clock selected
#endif
}


/*******************************************************************************
* Function Name  : main
* Description    : Main Cycle
*******************************************************************************/
int main(void)
{
	/* RC5 		- tim17
	 * FM 		- tim3
	 * Motor 	- tim16
	 * Sound 	- tim2 */
	Init();
	DebugInit();

	MotorInit();
	FMInit();
	RC5Init();

	SoundInit();

	DebugSendString("$ System Started!");
	Debug_OutSysTimeDate();


	UIInit();

#if 0
	MotorSpeedSet(120);
	FMSetFreq(FREQ20K, FREQ20K*2/9);
#else
	FMOff();
#endif


#if 0
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;

	RTC_TimeStructure.RTC_H12 = RTC_H12_AM;
	RTC_TimeStructure.RTC_Hours = 12;
	RTC_TimeStructure.RTC_Minutes = 05;
	RTC_TimeStructure.RTC_Seconds = 0;
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);

	RTC_DateStructure.RTC_Date = 20;
	RTC_DateStructure.RTC_Month = 12;
	RTC_DateStructure.RTC_Year = 14;
	RTC_DateStructure.RTC_WeekDay = 5;
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
#endif



	//SoundPlayNote(25, WAVE_SIN, 200); //startup sound

	//Forever loop__
    while(1)
    {
    	IWDG_ReloadCounter();


    	static uint32_t lastRTCReg = 0;
    	/* code block that executed every second */
    	if(lastRTCReg != RTC->TR)
    	{
    		lastRTCReg = RTC->TR;

    		static uint32_t light = 0;
    		light += ((uint32_t)(AdcData[2]*125))>>9;  //[0;1000]

    		uint16_t voltage = (AdcData[1]>>2)*27; //in mV

			static uint8_t counter = 0;
			if(++counter >= 8)
			{
				counter = 0;
				light = light >> 3;
#if 0
				DebugSendChar('\n');
				DebugSendNumWDesc("Battery: ", (AdcData[0]<<3)/5); //in mV
				DebugSendNumWDesc("Voltage: ", voltage);
				//DebugSendNumWDesc("Temp: ", AdcData[3]);
				DebugSendNumWDesc("Light: ", light);
				DebugSendChar('\n');
#endif
				/* Send luminosity */
				if(PowerFlag) SendPWM(light<20?20:light);
				light = 0;
			}

			/* read current time and date */
    		GetTimeDate(&TimeTemp, &DateTemp);

    		/* Auto power ON/OFF */
			if(sett.WorkOnOff)
			{
				/* Power ON clock if morning come */
				if(TimeTemp.RTC_Hours==sett.OnTime && TimeTemp.RTC_Minutes==0 && TimeTemp.RTC_Seconds<5 && !PowerFlag)
				{
					DebugSendString(" $ Wake-up time!");
					PowerFlag = TRUE;
					FMSetFreq(FREQ20K, FREQ20K*2/9);
					MotorSpeedSet(100);

					delay_ms(200);
						GetTimeDate(&TimeTemp, &DateTemp);
						TimeDateToStruct(&TimeTemp, &DateTemp, &RTC_TD_Temp);
						FMSendData(RTC_TimeDateToCnt(&RTC_TD_Temp), 32);

						SendSettArray();
						FMSendData(currMenuID, 8);

					noRotationCounter = NO_ROTATION_PERIOD;
					spinTimer = 0;
				}
			}

    		/* Make sound at the start of each hour */
			if(TimeTemp.RTC_Minutes==0 && TimeTemp.RTC_Seconds==0 && PowerFlag)
			{
				SoundPlayNote(sett.toneHrStrk, WAVE_TRIANGLE, 500, sett.volHrStrk);

				GetTimeDate(&TimeTemp, &DateTemp);
				TimeDateToStruct(&TimeTemp, &DateTemp, &RTC_TD_Temp);
				FMSendData(RTC_TimeDateToCnt(&RTC_TD_Temp), 32);
			}

    		/* Auto power ON/OFF */
    		if(sett.WorkOnOff)
    		{
				/* Power OFF clock if night come */
				if(TimeTemp.RTC_Hours==sett.OffTime && TimeTemp.RTC_Minutes==0 && TimeTemp.RTC_Seconds<5 && PowerFlag)
				{
					DebugSendString(" $ Sleep Time!");
					PowerFlag = FALSE;
					FMOff();
					MotorSpeedSet(0);
				}
    		}

    		if(PowerFlag)
    		{
    			MotorSpeedSet((78258 - 4*voltage)/337); //for currentSpinFreq = 90ms = 11.1Hz
    			//MotorSpeedSet((79805 - 4*voltage)/422); //for currentSpinFreq = 100ms = 10Hz
    		}
#if 1
    		/* send debug message every second */
    		//DebugSendNumWDesc("spin:", currentSpinFreq);
    		//DebugSendNumWDesc("spd:", MotorGetSpeed());
    		//DebugSendChar('\n');
    		Debug_OutSysTimeDate();
#endif
    	}

    	//RC5__
		if(RC5_FrameReceived == YES)
		{
			RC5_Frame = RC5_Decode();

			static uint8_t RC5_TogglePrevious=0;
			if(RC5_Frame.ToggleBit != RC5_TogglePrevious)
			{
				RC5_TogglePrevious = RC5_Frame.ToggleBit;

				/*char rc5s[50], out[50]={'\0'};
				itoa_(RC5_Frame.Address, rc5s);strcat_(out, " Address: ");strcat_(out, rc5s);strcat_(out, " Command: ");
				itoa_(RC5_Frame.Command, rc5s);strcat_(out, rc5s);strcat_(out, " Toggle bit: ");
				itoa_(RC5_Frame.ToggleBit, rc5s);strcat_(out, rc5s);
				DebugSendString(out);*/

				//if(RC5_Frame.Address < 2)
				{
					/* make sound if button is pressed */
					SoundPlayNote(sett.toneClick, WAVE_SIN, 100, sett.volClick);
					DebugSendChar('\n');DebugSendChar('*'); //button is pressed

					switch(RC5_Frame.Command)
					{
					//ON/OFF RED Button
					case 19:
					case 51:
						if(PowerFlag)
						{
							PowerFlag = FALSE;
							metrFlag = FALSE;
							DebugSendString("--- ROTOR OFF");
							FMOff();
							MotorSpeedSet(0);
							//SoundSetVolume(sett.volClick);
							//SoundPlayNote(D+12*2, WAVE_SAW, 200);SoundPlayNote(G+12*1, WAVE_SAW, 200);
						}
						else
						{
							PowerFlag = TRUE;
							DebugSendString("+++ ROTOR ON");
							FMSetFreq(FREQ20K, FREQ20K*2/9);
							MotorSpeedSet(110);
							//SoundSetVolume(sett.volClick);
							//SoundPlayNote(G+12*1, WAVE_SAW, 200);SoundPlayNote(D+12*2, WAVE_SAW, 200);

								delay_ms(200);
								GetTimeDate(&TimeTemp, &DateTemp);
								TimeDateToStruct(&TimeTemp, &DateTemp, &RTC_TD_Temp);
								FMSendData(RTC_TimeDateToCnt(&RTC_TD_Temp), 32);

								SendSettArray();
								FMSendData(currMenuID, 8);

							noRotationCounter = NO_ROTATION_PERIOD;
							spinTimer = 0;

							UIDispatcher(B_ONOFF);
						}
						break;

					//MUTE Button
					case 18:
					case 50:
						if(PowerFlag) UIDispatcher(B_MUTE);
						break;

					//AV/TV Button
					case 20:
					case 52:
						if(PowerFlag) UIDispatcher(B_AVTV);
						break;

					//RIGHT Button
					case 15:
					case 47:
						if(PowerFlag) UIDispatcher(B_RIGHT);
						break;

					//LEFT Button
					case 14:
					case 46:
						if(PowerFlag) UIDispatcher(B_LEFT);
						break;

					//UP Button
					case 31:
						if(PowerFlag) UIDispatcher(B_UP);
						break;

					//DOWN Button
					case 30:
						if(PowerFlag) UIDispatcher(B_DOWN);
						break;

					default:
						DebugSendNumWDesc("->FAIL RC5 command: ", RC5_Frame.Command);
						SoundPlayNote(12*3+E, WAVE_SIN, 40, sett.volClick);
					}
				}
			}
		}
		//__RC5

		SoundDispatcher();

		/* if rotor was physically stopped - to prevent motor drive burn */
		if(!noRotationCounter && PowerFlag)
		{
			PowerFlag = FALSE;
			DebugSendString("--- ROTOR HALTED!");
			FMOff();
			MotorSpeedSet(0);

			/* triple peak*/
			SoundPlayNote(Gd+12*2, WAVE_SAW, 150, !sett.volClick?2:sett.volClick);
				SoundPlayNote(Gd+12*2, WAVE_NULL, 150, !sett.volClick?2:sett.volClick);
			SoundPlayNote(Gd+12*2, WAVE_SAW, 150, !sett.volClick?2:sett.volClick);
				SoundPlayNote(Gd+12*2, WAVE_NULL, 150, !sett.volClick?2:sett.volClick);
			SoundPlayNote(Gd+12*2, WAVE_SAW, 150, !sett.volClick?2:sett.volClick);
		}

		//Rotor data IR receiver__
		if(RX_complete == TRUE)
		{
			//SoundSetVolume(4);
			//SoundPlayNote(A+12*2, WAVE_TRIANGLE, 100);
			DebugSendNumWDesc("< Rotor send: ", RX_data);

			switch(RX_data)
			{
			case Message_FAIL_RX:
				break;

			case Message_GetTimeDate:
				/* Send TimeDate */
				GetTimeDate(&TimeTemp, &DateTemp);
				TimeDateToStruct(&TimeTemp, &DateTemp, &RTC_TD_Temp);
				FMSendData(RTC_TimeDateToCnt(&RTC_TD_Temp), 32);

				SendSettArray();
				break;

			case Message_2:
				break;

			case Message_3:
				break;

			case Message_4:
				break;

			case Message_5:
				break;
			}

			RX_data = 0;
			RX_complete = FALSE;
		}
		//__rotor data receiver

/* OLD DEBUG FUNCTIONS
 *     	if(!tempDelay) {
    		tempDelay = 150;
    		static uint16_t ggg = 20000;
    		//FMSendByte(ggg++);//202
    		//FMSendData(ggg++, 32);
    	}

		static uint8_t tempp=0;
		if(++tempp >=2)
		{
			static uint8_t fr=0;
			fr++;
			tempp = 0;
			//FMSendData(fr, 16);
			static uint8_t hyy[3]={8,16,32};
			FMSendData(fr++, hyy[(fr/2)%3]);
			DebugSendNumWDesc("$SendDumb : ", fr);

		}

		static uint8_t uu=0;
		if(++uu%20==0)
		{
			static uint8_t ghj=0;
			SoundChangeSource((++ghj)%4, VOL);
			cntcnt++;
		}
		else
		{
			SoundChangeSource(5, VOL);
		}

		GPIOA->ODR ^= GPIO_Pin_11;
		GPIOA->ODR ^= GPIO_Pin_12;


		switch(RC5_Frame.Command)
		{
		//ON/OFF RED Button
		case 51:
				if(MotorGetSpeed()) MotorSpeedSet(0);
				else				MotorSpeedSet(120);
				DebugSendNumWDesc("Motor ON/OFF: ", MotorGetSpeed());
			break;

		//MUTE Button
		case 50:

			break;

		//AV/TV Button
		case 52:
				if(FMGetFreq()) FMOff();
				else			FMSetFreq(FMFreq[freqTemp], FMFreq[freqTemp]*powMul/64);
			break;

		//RIGHT Button
		case 47:
				if(MotorGetSpeed()<490) MotorSpeedSet(MotorGetSpeed()+10);
				DebugSendNumWDesc("Motor ++: ", MotorGetSpeed());
			break;

		//LEFT Button
		case 46:
				if(MotorGetSpeed()>10) MotorSpeedSet(MotorGetSpeed()-10);
				DebugSendNumWDesc("Motor --: ", MotorGetSpeed());
			break;

		//UP Button
		case 31:
				if(VOL < 7) VOL++;
				DebugSendNumWDesc("Volume ++: ", VOL);
			break;

		//DOWN Button
		case 30:
				if(VOL > 1) VOL--;
				DebugSendNumWDesc("Volume --: ", VOL);
			break;
		}
 */
    }
    //__forever loop
}


/*******************************************************************************
* Function Name  : Debug_OutSysTimeDate
* Description    : Send time and date to debug interface
*******************************************************************************/
void Debug_OutSysTimeDate()
{
	char s[50], d[20];

	GetTimeDate(&TimeTemp, &DateTemp);
	s[0] = '\0';
	strcat_(s, "\0System date: ");
	itoa_(TimeTemp.RTC_Hours, d);strcat_(s, d);strcat_(s, ":");
	itoa_(TimeTemp.RTC_Minutes, d);strcat_(s, d);strcat_(s, ":");
	itoa_(TimeTemp.RTC_Seconds, d);strcat_(s, d);
	strcat_(s, "   ");
	itoa_(DateTemp.RTC_Date, d);strcat_(s, d);strcat_(s, "-");
	itoa_(DateTemp.RTC_Month, d);strcat_(s, d);strcat_(s, "-");
	itoa_(DateTemp.RTC_Year, d);strcat_(s, d);strcat_(s, "; Day:");
	itoa_(DateTemp.RTC_WeekDay, d);strcat_(s, d);
	DebugSendString(s);
}


/*******************************************************************************
* Function Name  : NewLapHandler
* Description    : Called every time IR position sensor detect edge
*******************************************************************************/
void NewLapHandler()
{
/* Commands (*-HI, 0-LOW)	- numbers are inverted 	-	what actually send rotor
 * *** 		- FAIL PACKET	- 7							0x07
 * *00** 	- 1				- 25						0x13
 * *0*0*	- 2				- 21						0x15
 * *0***	- 3				- 29						0x17
 * **00*	- 4				- 19						0x19
 * *000*	- 5				- 17						0x11
 * **0**	- 6				- 27						0x1B
 */
	noRotationCounter = NO_ROTATION_PERIOD;
	currentSpinFreq = spinTimer;
	spinTimer = 0;

	static uint8_t RX_counter = 0;
	static uint8_t zeroPacketCounter = 0;

	if(IrDebounce == 0)
	{
		IrDebounce = 22; //ms

		if(RX_complete == FALSE)
		{
			/* if IR data sensor detect "1" */
			if((GPIOA->IDR & GPIO_Pin_5) == 0)
			{
				if(zeroPacketCounter >= 5)
				{
					RX_complete = FALSE;
					RX_counter = 0;
					zeroPacketCounter = 0;
					RX_data = 0;
				}

				BitSet(RX_data, RX_counter);

				if(RX_data == 7 && RX_counter == 2)
				{
					RX_complete = TRUE;
					RX_counter = 0;
					zeroPacketCounter = 0;
				}
				else if(RX_counter == 4)
				{
					RX_complete = TRUE;
					RX_counter = 0;
					zeroPacketCounter = 0;
				}
				else
				{
					RX_counter++;
				}
			}
			/* if IR data sensor detect "0" */
			else
			{
				BitReset(RX_data, RX_counter);
				RX_counter++;

				if(zeroPacketCounter < 5)
					zeroPacketCounter++;
			}
		}
	}
}
