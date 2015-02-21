/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_spi.h"

#include "display_HAL.h"
#include "draw.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint8_t DispBuf[12] = {0};


/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : HALDisplayInit
* Description    : Initialize SPI display interface
*******************************************************************************/
void HALDisplayInit()
{
	//DMA+SPI
	SPI_I2S_DeInit(SPI1);
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS  = SPI_NSS_Soft;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);

	DMA_DeInit(DMA1_Channel3);
	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&DispBuf[0];
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 12;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);

	DMA_Cmd(DMA1_Channel3, ENABLE);
	SPI_Cmd(SPI1, ENABLE);
}


/*******************************************************************************
* Function Name  : ColOff
* Description    : Turn ALL column OFF
*******************************************************************************/
void ColOff()
{
	for(uint8_t i=0; i<12; i++)
		DispBuf[i] = 0;
}


/*******************************************************************************
* Function Name  : ColOn
* Description    : Turn ALL column ON
*******************************************************************************/
void ColOn()
{
	for(uint8_t i=0; i<12; i++)
		DispBuf[i] = 0xFF;
}


/*******************************************************************************
* Function Name  : ColDisplay
* Description    : OverWrite column
* Input			 : Mass[3] (R, G, B) to write to column
*******************************************************************************/
void ColDisplay(uint32_t *pData)
{
	for(uint8_t k=0; k<3; k++)
	{
		for(uint8_t i=0; i<32; i++)
		{
			uint8_t position = i*3 + k;
			BitWrite(BitIsSet(*(pData+k), i), DispBuf[(11-position/8)], position%8);
		}
	}
}


/*******************************************************************************
* Function Name  : ColDisplayLayer
* Description    : Write column
* Input			 : Mass[3] (G, R, B) to write to column
*******************************************************************************/
void ColDisplayLayer(uint32_t *pData)
{
	for(uint8_t k=0; k<3; k++)
	{
		for(uint8_t i=0; i<32; i++)
		{
			uint8_t position = i*3 + k;
			if( BitIsSet(*(pData+k), i) )
			{
				BitSet(DispBuf[(11-position/8)], position%8);
			}
		}
	}
}


/*******************************************************************************
* Function Name  : PixelWrite
* Description    : Turn ON/OFF pixel in the column
* Input			 : Pixel number
* 				 : Pixel color
* 				 : State: ON/OFF
*******************************************************************************/
void PixelWrite(uint8_t number, uint8_t color, uint8_t state)
{
	if(number > 31) return;

	if(color < 3)	//single color
	{
		uint8_t position = number*3 + color;
		BitWrite(state, DispBuf[(11-position/8)], position%8);
	}
	else			//multi color
	{
		uint8_t position;

		switch(color)
		{
		case YELLOW:
				position = number*3 + RED;
				BitWrite(state, DispBuf[(11-position/8)], position%8);
				position = number*3 + GREEN;
				BitWrite(state, DispBuf[(11-position/8)], position%8);
			break;

		case MAGENTA:
				position = number*3 + RED;
				BitWrite(state, DispBuf[(11-position/8)], position%8);
				position = number*3 + BLUE;
				BitWrite(state, DispBuf[(11-position/8)], position%8);
			break;

		case CYAN:
				position = number*3 + GREEN;
				BitWrite(state, DispBuf[(11-position/8)], position%8);
				position = number*3 + BLUE;
				BitWrite(state, DispBuf[(11-position/8)], position%8);
			break;

		case WHITE:
				position = number*3 + RED;
				BitWrite(state, DispBuf[(11-position/8)], position%8);
				position = number*3 + GREEN;
				BitWrite(state, DispBuf[(11-position/8)], position%8);
				position = number*3 + BLUE;
				BitWrite(state, DispBuf[(11-position/8)], position%8);
			break;
		}
	}
}
