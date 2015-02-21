/* Includes ------------------------------------------------------------------*/
#include "motor.h"
#include "UI.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/*const uint16_t spinFrequency[] ={
		125, 111, //8
		100, 91, 83, 77, 71, //10
		67,	62, 59, 56, 53, //15
		50, 48, 45, 43, 42, //20
		40, 38, 37, 35, 34, //25
		33};//30
*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//volatile uint16_t currentSpinFreq = 0;

/* Extern variables ----------------------------------------------------------*/
//extern SettingsArray sett;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : MotorInit
* Description    : Initialize PWM motor speed control
*******************************************************************************/
void MotorInit()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_2);	//TIM16_CH1 - PWM 2motor

	//PWM - spin
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//Timer 16 - PWM
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = 4;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 500;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM16, &TIM_TimeBaseStructure);

	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
	TIM_OC1Init(TIM16, &TIM_OCInitStructure);

	TIM_CtrlPWMOutputs(TIM16, ENABLE);
	TIM_Cmd(TIM16, ENABLE);
}


/*******************************************************************************
* Function Name  : SpeedSet
* Description    : Set the motor speed (PWM counter)
* Input			 : 0-500 PWM value
*******************************************************************************/
void MotorSpeedSet(uint16_t speed)
{
	if(speed > 500) speed = 500;
	TIM16->CCR1 = speed;
}


/*******************************************************************************
* Function Name  : MotorGetSpeed
* Return 		 : PWM counter value
*******************************************************************************/
uint16_t MotorGetSpeed(){
	return TIM16->CCR1;
}

#if 0
/*******************************************************************************
* Function Name  : MotorSetCurrSpin
* Return 		 : Input real spin frequency
*******************************************************************************/
void MotorSetCurrSpin(uint16_t value)
{
	currentSpinFreq = value;
}


/*******************************************************************************
* Function Name  : MotorSpinDispatcher
* Return 		 : Make motor spin frequency stable
*******************************************************************************/
void MotorSpinDispatcher()
{
	uint8_t temp;

	if(abs(currentSpinFreq - spinFrequency[sett.spinFreq-8])>15)
		temp = 3;
	else if(abs(currentSpinFreq - spinFrequency[sett.spinFreq-8])>5)
		temp = 1;
	else
		temp = 0;

	if(currentSpinFreq > spinFrequency[sett.spinFreq-8])
		MotorSpeedSet(MotorGetSpeed()+temp);
	else
		MotorSpeedSet(MotorGetSpeed()-temp);
}
#endif
