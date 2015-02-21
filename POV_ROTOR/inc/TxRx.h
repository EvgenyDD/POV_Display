/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TXRX_H
#define TXRX_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
typedef enum{
	Message_FAIL_RX,
	Message_GetTimeDateSett,
	Message_VLow6,
	Message_VLow8,
	Message_4,
	Message_5,
	Message_6
}MessageType;


/* Exported macro ------------------------------------------------------------*/
#define BitIsSet(reg, bit) ((reg & (1<<(bit))) != 0)
#define BitIsReset(reg, bit) ((reg & (1<<(bit))) == 0)
#define BitSet(p,m) ((p) |= (1<<(m)))
#define BitReset(p,m) ((p) &= ~(1<<(m)))
#define BitWrite(c,p,m) ((c) ? BitSet(p,m) : BitReset(p,m))

#define TX_LED_SET		GPIOA->BSRR = GPIO_Pin_4
#define TX_LED_RESET 	GPIOA->BRR = GPIO_Pin_4


/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int TxAddMessage(MessageType);
void TX_Process(uint8_t);

void RX_Dispatcher(uint8_t newFreqNum);

#endif //TXRX_H
