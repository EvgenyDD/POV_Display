
; Define RC5 address of Remote Control

RemoteAddr	EQU	0x00

; Definition of Keys and key-codes on my remote control

NUM_0	EQU	0x00	;1-es gomb, m�sodperc 0-ra
NUM_1	EQU	0x01	;2-es gomb
NUM_2	EQU	0x02	;3-as gomb
NUM_3	EQU	0x03	;4-es gomb
NUM_4	EQU	0x04	;5-�s gomb
NUM_5	EQU	0x05	;6-os gomb
NUM_6	EQU	0x06	;7-es gomb
NUM_7	EQU	0x07	;8-as gomb
NUM_8	EQU	0x08	;9-es gomb
NUM_9	EQU	0x09	;10-es gomb
FREEZE	EQU	0x5D
PAUSE	EQU	0x30
STANDBY	EQU	0x26	;11-es gomb, kikapcsol�s (k�szenl�t)
MUTE	EQU	0x0D
HELP	EQU	0x22
TEXT	EQU	0xAF
VOL_UP	EQU	0x10	;12-es gomb
VOL_DN	EQU	0x11	;13-as gomb
BRI_UP	EQU	0x20	;14-es gomb
BRI_DN	EQU	0x21	;15-�s gomb
SAT_UP	EQU	0x0A	;16-os gomb
SAT_DN	EQU	0x0B
BAS_UP	EQU	0x15
BAS_DN	EQU	0x16
TRE_UP	EQU	0x0F
TRE_DN	EQU	0x38
BAL_RI	EQU	0x1A
BAL_LE	EQU	0x1B
CON_UP	EQU	0x0F
CON_DN	EQU	0x38
HUE_UP	EQU	0x27
HUE_DN	EQU	0x1F
SHA_UP	EQU	0x4D
SHA_DN	EQU	0x4E
PHA_UP	EQU	0x48
PHA_DN	EQU	0x49
ADJUST	EQU	0x0C
EXIT	EQU	0x53
ADDR	EQU	0x54
ASTRSK 	EQU	0x77
ENTER	EQU	0x57
F1	EQU	0x37
F2	EQU	0x36
F3	EQU	0x32
F4	EQU	0x34
F5	EQU	0x2E
CUR_UP  EQU     0x50
CUR_RI	EQU	0x56
CUR_DN	EQU	0x51
CUR_LE	EQU	0x55

; Definition of Functions on the Propeller clock

SEC_UP		EQU	NUM_1
SEC_DN		EQU	NUM_2
MIN_UP		EQU	NUM_4
MIN_DN		EQU	NUM_5
HOUR_UP		EQU	NUM_7
HOUR_DN		EQU	NUM_8
DAY_UP		EQU	NUM_3
DAY_DN		EQU	NUM_6
MON_UP		EQU	TRE_UP
MON_DN		EQU	TRE_DN
YEAR_UP		EQU	HELP
YEAR_DN		EQU	MUTE
INDEX_UP	EQU	BRI_UP
INDEX_DN	EQU	BRI_DN

BlueLine	EQU	F4
DigiTime	EQU	F3
DigiDate	EQU	F2
AnaTime		EQU	F1
TTicks		EQU	F5

DM		EQU	VOL_UP	;Demo Mode
TextMode	EQU	VOL_DN
