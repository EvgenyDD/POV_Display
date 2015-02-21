/* Includes ------------------------------------------------------------------*/
#include "debug.h"
#include "string.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : DebugInit
* Description    : Initialize debug (via USART1)
*******************************************************************************/
void DebugInit()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_0);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_Cmd(USART1, ENABLE);
}


/*******************************************************************************
* Function Name  : DebugSendString
* Description    : Sends debug information (via USART1)
* Input          : pointer to text massive
*******************************************************************************/
void DebugSendString(char *pText)
{
	while(!(USART1->ISR & USART_FLAG_TXE));
	USART1->TDR = (uint16_t)'\n' & (uint16_t)0x01FF;

	for(; *pText != '\0'; pText++)
	{
		while(!(USART1->ISR & USART_FLAG_TXE));
		USART1->TDR = (uint16_t)*pText & (uint16_t)0x01FF;
	}
	//while((USART1->ISR & USART_FLAG_TXE) == RESET);
	//USART_SendData(USART1, '\n');
}


/*******************************************************************************
* Function Name  : DebugSendChar
* Description    : Sends debug information (via USART1)
* Input          : char to send
*******************************************************************************/
void DebugSendChar(char charTx)
{
	while(!(USART1->ISR & USART_FLAG_TXE));
	USART1->TDR = (uint16_t)charTx & (uint16_t)0x01FF;

	/*while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, '\n');*/
}


/*******************************************************************************
* Function Name  : DebugSendNum
* Description    : Send number
*******************************************************************************/
void DebugSendNum(uint16_t Num)
{
	char str[50];
	itoa_(Num, str);
	DebugSendString(str);
}


/*******************************************************************************
* Function Name  : DebugSendNumWDesc
* Description    : Send text + number
*******************************************************************************/
void DebugSendNumWDesc(char *string, uint32_t Num)
{
	char str[strlen(string)+20], number[25];
	str[0]='\0';
	strcat_(str, string);
	itoa_(Num, number);
	strcat_(str, number);
	DebugSendString(str);
}
