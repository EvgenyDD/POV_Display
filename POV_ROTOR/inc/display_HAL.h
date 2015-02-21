/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DISPLAY_HAL
#define DISPLAY_HAL

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define STEPSHIFT 4//200-14//200-50;


/* Exported macro ------------------------------------------------------------*/
#define BitWrite(c,p,m) ((c) ? BitSet(p,m) : BitReset(p,m))
#define BitSet(p,m) ((p) |= (1<<(m)))
#define BitReset(p,m) ((p) &= ~(1<<(m)))
#define BitIsSet(reg, bit) ((reg & (1<<(bit))) != 0)
#define BitIsReset(reg, bit) ((reg & (1<<(bit))) == 0)


/* Exported define -----------------------------------------------------------*/
enum {GREEN, RED, BLUE, YELLOW, MAGENTA, CYAN, WHITE};


/* Exported functions ------------------------------------------------------- */
void HALDisplayInit();
void ColOff();
void ColOn();
void PixelWrite(uint8_t, uint8_t, uint8_t);
void ColDisplay(uint32_t*);
void ColDisplayLayer(uint32_t*);

#endif //DISPLAY_HAL
