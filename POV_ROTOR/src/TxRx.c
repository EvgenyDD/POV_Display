/* Includes ------------------------------------------------------------------*/
#include "TxRx.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rtc.h"
#include "UI.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
const uint8_t Message[8] = {
		0x07,	0x13, 	0x15,	0x17,	0x19,	0x11, 	0x1B
};

//#define DEBUG_MODE

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t TX_counter = 0;
uint8_t TX_data;
bool TX_LedFlag = 0;


/* Extern variables ----------------------------------------------------------*/
#ifdef DEBUG_MODE
	extern char  sss[20];
	extern uint16_t fmFAILReports;
	extern uint16_t fmNumReports;
	extern uint32_t RX_data;
#endif


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : TxAddMessage
* Description    : send message through IR Transmitter
* Input			 : message number(0-8)
*******************************************************************************/
int TxAddMessage(MessageType message)
{
	if(message > 8) return 2; //no such message
	if(TX_counter) return 1; //an old message is still being sending

	for(uint8_t i=0; i<8; i++)
	{
		if(BitIsSet(Message[message], i))
			TX_counter = i+3;		//message length
	}
	TX_data = Message[message];

	TX_LedFlag = TRUE;

	return 0;
}


/*******************************************************************************
* Function Name  : TX_Process
* Description    : IR Transmitter Handler
*******************************************************************************/
void TX_Process(uint8_t step)
{
	/* Turn on/off Tx IR LED */
	//if((step >= 120 || step <= 40) && TX_LedFlag)
	if(step == 120 && TX_LedFlag)
	{
		if(BitIsSet(TX_data, TX_counter-1))
			TX_LED_SET;
		else
			TX_LED_RESET;

		TX_counter--;

		TX_LedFlag = FALSE;
		return;
	}

	/* It's time to turn off Tx IR LED - it can cause BASE position-transistor triggering */
	//if(step >= 40 && step < 120)
	if(step == 40)
	{
		TX_LED_RESET;
		if(TX_counter)	TX_LedFlag = TRUE;
	}
}


/*******************************************************************************
* Function Name  : RX_Dispatcher
* Description    : Receive message from EM FM receiver
//I 	- FREQ21k - START PACKET
//II 	- FREQ22k **->00
//III 	- FREQ23k **->01
//IV 	- FREQ24k **->10
//V 	- FREQ25k **->11
//VI 	- FREQ26k **->00
//VII 	- FREQ27k **->01
//IIX 	- FREQ28k **->10
//IX 	- FREQ29k **->11
//X 	- FREQ30k - END PACKET
*******************************************************************************/
void RX_Dispatcher(uint8_t newFreqNum)
{
	static uint8_t prevFreqNum = 0;

	static volatile uint8_t RX_pointer = 255;
	static uint32_t RX_temp = 0;

	static uint8_t isBkpRxMode = 0;

	/* Start packet condition */
	if(newFreqNum == 1)
	{
		RX_pointer = 0;
		RX_temp = 0;
		return;
	}

	/* End packet condition */
	if(newFreqNum == 10)
	{
		if(newFreqNum != prevFreqNum)
		{
			prevFreqNum = newFreqNum;

#ifdef DEBUG_MODE
			fmNumReports ++;
#endif
			if(RX_pointer == 16 || RX_pointer == 8 || RX_pointer == 4)
			{	//Received good packed
#ifdef DEBUG_MODE
				sss[0] = 'G';sss[1] = 'O';sss[2] = 'O';sss[3] = 'D';sss[4] = '0'+RX_pointer/4;sss[5]='\0';
				RX_data = RX_temp;
#endif
				if(RX_pointer == 16)
				{
					if(isBkpRxMode > 1)
						MakeSettFromNumber(isBkpRxMode-2, RX_temp);
					else if(isBkpRxMode == 1)
						TIM2->CCR3 = RX_temp; //set PWM brightness
					else//isBkpRxMode==0
							RTC_SetCounter(RX_temp);
				}
				if(RX_pointer == 8)	UI_NewParameter(RX_temp);
				if(RX_pointer == 4)
				{
					if(RX_temp < 252)
					{
						isBkpRxMode = 0;
						UI_NewMenuMode(RX_temp);
					}
					else
						isBkpRxMode = RX_temp - 251;
				}

				RX_temp = 0;
			}
			else
			{	//Received fail packed (length mismatch)
#ifdef DEBUG_MODE
				sss[0] = 'F';sss[1] = 'A';sss[2] = 'I';sss[3] = 'L';sss[4] = '\0';
				RX_data = 222;
				fmFAILReports++;
#endif
				RX_pointer = 0;
				RX_temp = 0;
			}
		}
		return;
	}

	/* Data */
	if(newFreqNum != prevFreqNum)
	{
		prevFreqNum = newFreqNum;

		BitWrite(BitIsSet((newFreqNum-2), 0),  RX_temp, RX_pointer*2);
		BitWrite(BitIsSet((newFreqNum-2), 1),  RX_temp, RX_pointer*2+1);
		RX_pointer++;
		return;
	}
}
