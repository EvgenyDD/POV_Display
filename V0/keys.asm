; Define RC5 address of Remote Control

RemoteAddr	EQU	0x00

; Definition of Keys and key-codes on my remote control

NUM_0	EQU	0x07	;обнуление секунд
NUM_1	EQU	0x08	;sec+
NUM_2	EQU	0x00	;sec-
NUM_3	EQU	0x01	;day+
NUM_4	EQU	0x04	;day-
NUM_5	EQU	0x02	;month+
NUM_6	EQU	0x05	;month-
NUM_7	EQU	0x03	;year+
NUM_8	EQU	0x06	;year-
NUM_9	EQU	0x32	;--база-- отключение ИК
FREEZE	EQU	0x5D
PAUSE	EQU	0x30
STANDBY	EQU	0x36	;11-es gomb, kikapcsolбs (kйszenlйt)
MUTE	EQU	0x0D
HELP	EQU	0x2F
TEXT	EQU	0x3C    ;+;Running string
VOL_UP	EQU	0x20	;Min +
VOL_DN	EQU	0x21	;Min -
BRI_UP	EQU	0x1E	;14-es gomb
BRI_DN	EQU	0x34	;15-цs gomb
SAT_UP	EQU	0x0A	;Индикация вправо
SAT_DN	EQU	0x0E    ;Индикация влево
BAS_UP	EQU	0x16
BAS_DN	EQU	0x17
TRE_UP	EQU	0x18
TRE_DN	EQU	0x19
BAL_RI	EQU	0x1A
BAL_LE	EQU	0x1B
CON_UP	EQU	0x1C
CON_DN	EQU	0x1E
HUE_UP	EQU	0x27
HUE_DN	EQU	0x1F
SHA_UP	EQU	0x4D
SHA_DN	EQU	0x4E
PHA_UP	EQU	0x48
PHA_DN	EQU	0x49
ADJUST	EQU	0x52
EXIT	EQU	0x53
ADDR	EQU	0x54
ASTRSK 	EQU	0x77
ENTER	EQU	0x3B    ;OK;Demo mode
RED	EQU	0x0C    ;Кольцо
GREEN	EQU	0x3F    ;Digital clock
YELOW	EQU	0x38    ;Analog clock
BLUE	EQU	0x0D    ;Дата
PP	EQU	0x26    ;5 sec points
CUR_UP	EQU	0x10    ;Hour +
CUR_RI	EQU	0x56    
CUR_DN	EQU	0x11    ;Hour -
CUR_LE	EQU	0x55

; Definition of Functions on the Propeller clock

SEC_UP		EQU	NUM_1   ;sec+
SEC_DN		EQU	NUM_2   ;sec-
MIN_UP		EQU	VOL_UP  ;Min +
MIN_DN		EQU	VOL_DN  ;Min -
HOUR_UP		EQU	CUR_UP  ;Hour +
HOUR_DN		EQU	CUR_DN  ;Hour -
DAY_UP		EQU	NUM_3   ;day+
DAY_DN		EQU	NUM_4   ;day-
MON_UP		EQU	NUM_5   ;month+
MON_DN		EQU	NUM_6   ;month-
YEAR_UP		EQU	NUM_7   ;year+
YEAR_DN		EQU	NUM_8   ;year-
INDEX_UP	EQU	SAT_UP  ;Индикация вправо
INDEX_DN	EQU	SAT_DN  ;Индикация влево

BlueLine	EQU	RED     ;Кольцо
DigiTime	EQU	GREEN   ;Digital clock
DigiDate	EQU	BLUE    ;Дата
AnaTime		EQU	YELOW   ;Analog clock
TTicks		EQU	PP      ;5 sec points
WEEK_DN     EQU	MUTE

DM		EQU	ENTER	;Demo Mode
TextMode	EQU	TEXT                  