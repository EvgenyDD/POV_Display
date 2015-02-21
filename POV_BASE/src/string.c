/* Includes ------------------------------------------------------------------*/
#include "string.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/*******************************************************************************
* Function Name  : strlen
* Description    : calculating length of the string
* Input          : pointer to text string
* Return         : string length
*******************************************************************************/
int strlen(char *pText)
{
	int len = 0;
	for(; *pText != '\0'; pText++, len++);
	return len;
}


/*******************************************************************************
* Function Name  : itoa
* Description    : Convert int to char
* Input          : int number (signed/unsigned)
* Return         : pointer to text string
*******************************************************************************/
void itoa_(uint32_t n, char s[])
{
int i, sign;

	if ((sign = n) < 0)
		n = -n;

	i = 0;

	do {
		s[i++] = n % 10 + '0';
	}
	while ((n /= 10) > 0);

	if (sign < 0) s[i++] = '-';

	s[i] = '\0';
	reverse(s);
}


/*******************************************************************************
* Function Name  : reverse
* Description    : Reverses string
* Input          : pointer to string
*******************************************************************************/
void reverse(char s[])
{
	int c, i, j;
	for (i = 0, j = strlen(s)-1; i < j; i++, j--)
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}


/*******************************************************************************
* Function Name  : strcat
* Description    : add string 2 to the end of string 1
* Input          : pointer to strings
*******************************************************************************/
void strcat_(char first[], char second[])
{
	int i=0, j=0;

	while (first[i] != '\0') i++;
	while ((first[i++] = second[j++]) != '\0');
}


/*******************************************************************************
* Function Name  : strcatWRev
* Description    : add reversed string 2 to the end of string 1
* Input          : pointer to strings
*******************************************************************************/
void strcatWRev(char first[], char second[])
{
	int i=0, j=0;
	char temp[strlen(second)+2];
	for(uint8_t i=0; i<=strlen(second); i++) temp[i]=second[i];
	reverse(temp);

	while (first[i] != '\0') i++;
	while ((first[i++] = temp[j++]) != '\0');
}
