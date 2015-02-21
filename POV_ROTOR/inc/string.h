/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STRING_H
#define STRING_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int strlen(char *);
void itoa_(int n, char s[]);
void reverse(char s[]);
void strcat_(char first[], char second[]);
void ftoa_(float num, char str[], char precision);
float log10_(int v);
float pow_(float x, float y);

#endif //STRING_H
