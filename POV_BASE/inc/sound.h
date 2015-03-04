/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SOUND_H
#define SOUND_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"


/* Exported types ------------------------------------------------------------*/
struct SoundCBType{
	uint16_t noteFreq;
	uint8_t wave;
	uint16_t noteLen;
	uint8_t volume;
};

/* Exported constants --------------------------------------------------------*/
enum Wave{WAVE_NULL, WAVE_SIN, WAVE_MEANDR, WAVE_SAW, WAVE_TRIANGLE};
enum Notes{C, Cd, D, Dd, E, F, Fd, G, Gd, A, B, H};

/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void SoundInit();

void SoundSetWave(uint8_t, uint8_t);
void SoundSetFreq(uint16_t freq);

int SoundPlayNote(uint16_t, uint8_t, uint16_t, uint8_t);
void SoundDispatcher();

#endif //SOUND_H
