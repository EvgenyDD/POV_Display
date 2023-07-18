;************************************************************************
; Following the example of Bob Blick's original propeler clock I made 	*
; my own version of this fancy clock and have re-written the code from	*
; scratch. I used a bit more powerfull version of PIC cpu and added 	*
; these features:							*		
;  - the clock keeps time AND DATE			           	*
;  - can display analog hands or digital time and mixed mode where	*
;    time is displayed as analog hands and digital date			*
;  - setting the time/date or switch mode is done by RC5 remote control	*
;  - possibility to display a message programmed in ROM or EEPROM	*		
;                                                                 	*
;************************************************************************
;                                                               	*
;    Filename:	    prop.asm                                        	*
;    StartDate:     02/12/2001                                        	*
;    LastUpdate:    09/02/2003						*
;    File Version:  1.06                                       	   	*
;                                                                     	*
;    Author:        Soubry Henk                                       	*
;    Company:       Soubry Software Service                           	*
;                                                                     	*	 
;                                                                     	*
;************************************************************************
;                                                                    	*
;    Files required: CharGen.asm                                      	*
;                    Keys.asm  	                                      	*
;                                                                     	*
;                                                                     	*
;************************************************************************
;                                                                     	*
;    Notes:                                                           	*
;    	Pin assignment                                                	*
;      		Port A                                             	*
;		0 = inner display led (because RB0 is index)		*						
;               1 = analog outer led				   	*	                     
;               2 = Ir receiver                                       	*
;               3 = OSC Calibration output                            	*
;               4 = unused	                  		       	*
;      		Port B 							*
;		0 = index                                     	       	*
;               1..6 = Display led's (1=inner, 6=outer                	*
;               7 = analog inner led's  				*
;                                                                     	*
;************************************************************************


	list	p=16f628            ; list directive to define processor
	#include <p16f628.inc>        ; processor specific variable definitions
	#include "CharGen.asm"
	#include "Keys.asm"
	
	__CONFIG _CP_OFF & _WDT_OFF & _BODEN_ON & _PWRTE_ON & _HS_OSC & _MCLRE_OFF & _LVP_OFF


;***** VARIABLE DEFINITIONS

	 #define	MotorCounterClockWise	; un-comment this line if motor is running counter clockwise
; #define 	ReversTextScroll	; un-comment this line if your text must scroll from left to right (e.g. Hebrew)

;Display memory
DispI		EQU     0x20	  ; 40 bytes from 0x20 -> 0x48
DispII		EQU     0xA0	  ; 80 bytes from 0xA0 -> 0xF0

;Vars in shared memory. This memory is available from any bank
  cblock	0x77
	Scratch		; memory locations for general use
	Scratch2	;
	Scratch3	;
	Scratch4	;					
	w_temp		; variable used for context saving 
	status_temp	; variable used for context saving
	fsr_temp	; variable used for context saving
	flags		; all sorts of flags 
	flags2		; 
  endc
	
;Vars for display timing routines and interrupts
  cblock	0x48
	PeriodCnt_H	; 16 bit counter counts number of interrupt
	PeriodCnt_L	; between index-puls (one revolution)
	StorePCnt1_H	; first 16 bit storage for PeriodeCounter
	StorePCnt1_L	;
	StorePCnt2_H	; second 16 bit storage for PeriodeCounter
	StorePCnt2_L	; 
	SubSec_H	; 16 bit sub-second counter
	SubSec_L	; there are 2500 sub-seconds in 1/2 second

	PixelWidth	; Pixel-width = number of interrupts in one ON pixel
	PixelOff	; 8 bit down-counter to time PixelWidth
			
	PixelPitch_H	; PixelPitch = 16 bit number of interrupts between two pixel
	PixelPitch_L	; 
	PixelPitch_F	; Fraction of PixelPitch needed for accuracy and to avoid jitter

	NewPPitch_H	; New pixel pitch calculated by main programm, will be
	NewPPitch_L	; copied to PixelPithc by interrupt routine
	NewPPitch_F	; 

	NextPixel_H	; Next pixel @ PeriodCnt = NextPixel
	NextPixel_L	;
	NextPixel_F	;
	RoundedNext_L	; RoundedNext = Round(NextPixel)
	RoundedNext_H	;
		
	iFSR		; copy of FSR used by interrupt routine
	DispOffset	; Display offset compared to indexpuls

	TmrScroll	; Count Down timer for scroll delay
	DemoCnt		; Counter for demo mode

;Vars for time-keeping			
	Hour		; looks obvious, doesn't it !
	Minute		;
	Second2		; 0..119 counting half seconds
	Day		;
	Month		;
	Year		;
	DMon		; days in current Month		
			
;Vars for building display-content
	timeticks	; index for 5min time tick
	BCD		; Binary coded decimal
	digitindex	; index digit to display
	dotindex	; index dot to display
	dpi		; 0..119 display index for main program
	tdd		; temporary display data
	PrevIndex	; stores state of index

;Vars for RC5 decoding		
	RC5_flags	;
	RC5_Tmr		;
	RC5_BitCnt	;
	RC5_Addr	;
	RC5_Cmd		;
	RC5_Cmd2	; storage for previous cmd
			
  endc

	IF RC5_Cmd2 >= Scratch
		ERROR "To many variables used"
	ENDIF

; start a new block of vars in bank2
; these are vars for the char printing routines
  cblock	0x120
	ch_dot_point_H	; pointer the the current dot# in a character	
	ch_dot_point_L	; points the the chargenerator table
	ch_dot_index	; index of dot# in a char, 0..5
	ch_num		; char number
	ch_blanking	; counter for blanking the display when scrolling text
  endc

;**********************************************************************
; give meaningfull names to the scratch locations

; for the 16bit/8bit divide routine
DIV_HI		EQU	Scratch	; 16 bit Divisor
DIV_LO		EQU	Scratch2; needed for 16 bit / 8 bit division
DIV_Q		EQU	Scratch3; 
DIVISOR		EQU	Scratch4; Divisor

;**********************************************************************
; define Port bits
bIndexPuls	EQU	0
bOuterLED	EQU	1
bRC5inp		EQU	2
bCalibration	EQU	3

; define flag bits
bDigitStart	EQU	0
bNewPCnt	EQU	1
bNewTime	EQU	2
bNewPPitch	EQU	3
bShowHand	EQU	4
bShowDTime	EQU	5
bShowDDate	EQU	6
bShowTicks	EQU	7

; define flag2 bits
bText		EQU	4
bScrollOn	EQU	5
bDemo		EQU	6

;**********************************************************************

#define 	IndexPuls	PORTB,bIndexPuls
#define 	OuterLED	PORTA,bOuterLED
#define 	RC5inp		PORTA,bRC5inp
#define		Calibration	PORTA,bCalibration

#define		digitstart	flags,bDigitStart
#define		NewPCnt		flags,bNewPCnt
#define		NewTime		flags,bNewTime
#define		NewPPitch	flags,bNewPPitch
#define		ShowHand	flags,bShowHand
#define		ShowDTime	flags,bShowDTime
#define		ShowDDate	flags,bShowDDate
#define		ShowTicks	flags,bShowTicks

#define 	fOuterLED	flags2,bOuterLED
#define		fText		flags2,bText
#define		fScrollOn	flags2,bScrollOn
#define		fDemo		flags2,bDemo

#define		RC5_WaitStart	RC5_flags,0
#define		RC5_DataReady	RC5_flags,1
#define		RC5_prev_inp	RC5_flags,2
#define		RC5_Idle	RC5_flags,3
#define		RC5_HalfBit	RC5_flags,4
#define		RC5_ReSynced	RC5_flags,5
#define		PStorage	RC5_flags,6
#define		reserved	RC5_flags,7


;******************************************************************************
;	Define macro's to Save w and Status register during interrupt
;******************************************************************************
push	macro
	movwf	w_temp
	swapf	w_temp, f
	swapf	STATUS, w
	movwf	status_temp
	endm
;
;******************************************************************************
;	Define macro's to Reload w and Status register after interrupt
;******************************************************************************
pop	macro
	swapf	status_temp, w
	movwf	STATUS
	swapf	w_temp, w
	endm
;
;******************************************************************************
;	Define macro's for a Select - Case 
;******************************************************************************
select_w	macro 
sel_last	set 0                     ;setup variable only 
         	endm 
case     	macro   val
	        xorlw   val ^ sel_last 
		btfsc	STATUS,Z
sel_last set val 
         	endm 

;
;******************************************************************************
;	Define EEPROM content for initial message
;******************************************************************************

	ORG     0x2100           ; Start off EEPROM
	de	" À Á Â Ã Ä d   ",0x83,0x84,"<===              "
	de	0x00


;**********************************************************************
		ORG     0x000           ; processor reset vector
		goto    main            ; go to beginning of program

		ORG     0x004           ; interrupt vector location		
		push			; context switching		
		bcf	STATUS,RP0	; select bank 0 for interrupt stuff
		bcf	STATUS,RP1	; the 'pop' will restore current bank status
		
;**********************************************************************
		
INT_RB0		btfss	INTCON,INTF	; interrupt on RB0?
		goto	INT_TMR0	; nope, TMR0 is next

;----------	RB0 interrupt ! == Index Sensor
		movlw	0xff		;
		movwf	TMR0		; force TMR0 to 0xFF

		btfss	PStorage	;
		goto	Store2		; goto second storage
		movf	PeriodCnt_H,w	; 
		movwf	StorePCnt1_H	; StorePCnt = PeriodCnt
		movf	PeriodCnt_L,w	;
		movwf	StorePCnt1_L	;
		bcf	PStorage	;
		goto	StoreDone	;
Store2
		movf	PeriodCnt_H,w	; 
		movwf	StorePCnt2_H	; StorePCnt = PeriodCnt
		movf	PeriodCnt_L,w	;
		movwf	StorePCnt2_L	;
		bsf	PStorage	;
StoreDone:	
		bsf	NewPCnt		; signal new stored PeriodCount to main program
		clrf	PeriodCnt_H	; PeriodCnt = 0
		clrf	PeriodCnt_L	;
		clrf	NextPixel_H	; Next_Pixel = 0 (thus now)
		clrf	NextPixel_L	;
		clrf	NextPixel_F	;
		clrf	RoundedNext_L	; RoundedNext = 0
		clrf	RoundedNext_H	;
		clrf	PixelOff	; display next pixel no matter what.
		movf 	DispOffset,w	; iDispIndex = DispOffset
		movwf	iFSR		;
		btfss	NewPPitch	; is there a new value calculated by main prg?
		goto	lINT_RB0	; no, continue
		movf	NewPPitch_H,w	; PixelPitch = NewPPitch
		movwf	PixelPitch_H	; 
		movf	NewPPitch_L,w	; 
		movwf	PixelPitch_L	; 
		movf	NewPPitch_F,w	; 
		movwf	PixelPitch_F	; 
		bcf	NewPPitch	;
lINT_RB0
		bcf	INTCON,INTF	; clear RB0 interrupt flag		
		goto	lDisp_1		; do display routine but skip Period increment
;--------
INT_TMR0	btfss	INTCON,T0IF	; Test if a TMR0 interrupt occured
		goto	INT_TMR2	; nope, TMR2 is next
		
		bsf	TMR0,7		; TMR0 = TMR0 + 128 => Test to double TMR0 speed

		incfsz	PeriodCnt_L,f	; Increment 16-bit period counter
		goto	lDisp_1		;
		incfsz	PeriodCnt_H,f	;
		goto	lDisp_1		; if overflow in MSB period counter = speed to low
		bsf	STATUS,RP0	; Bank1
		movlw	0xFF		; disable LED outputs
		movwf	TRISB		;
		movlw	b'11110111'	; leave PORTA.3 as output
		movwf	TRISA		;
		bcf 	STATUS,RP0	; Bank0

lDisp_1		movf	RoundedNext_L,w	; PeriodCnt = RoundedNextPixel ?
		xorwf	PeriodCnt_L,w	;
		btfss	STATUS,Z	;
		goto	lPixelOn	; no, check if there is a pixel on
		movf	RoundedNext_H,w	;
		xorwf	PeriodCnt_H,w	;
		btfss	STATUS,Z	;
		goto	lPixelOn	; no,  check if there is a pixel on
					; yes, display next pixeldata
		movf	FSR,w		; context saving
		movwf	fsr_temp	;
		
		movf	iFSR,w		; load new memory pointer in w
		movwf	FSR		; load File Select register with Display Index
		movf	INDF,w		; PortB = DisplayData(index)
		movwf	PORTB		; 
		bcf	PORTA, 0	; dispatch bit 0 to PORTA
		btfsc	INDF, 0
		bsf 	PORTA, 0	
		
#ifdef MotorCounterClockWise
		decf	iFSR,w		; increment i(nterrupt)FSR
		call	CheckDecrement	; check correct progress of diplay memory pointer
#else
		incf	iFSR,w		; increment i(nterrupt)FSR
		call	CheckIncrement	; check correct progress of diplay memory pointer
#endif
		movwf	iFSR		;				

		movf	fsr_temp,w	; context recall
		movwf	FSR		;

		movf	PixelPitch_F,w 	; NextPixel = NextPixel + PixelPitch
		addwf	NextPixel_F,f	;
		btfss	STATUS,C	;
		goto	no_overflow	;
		incf	NextPixel_L,f	; fraction overflow, add 1 to Low byte
		btfsc	STATUS,Z	;
		incf	NextPixel_H,f	; low byte roll-over, add 1 to High byte
no_overflow:	movf	PixelPitch_L,w	;
		addwf	NextPixel_L,f	;
		btfsc	STATUS,C	;
		incf	NextPixel_H,f	; low byte overflow, add 1 to High byte
		
		movf	NextPixel_L,w	; RoundedNext = NextPixel
		movwf	RoundedNext_L	;
		movf	NextPixel_H,w	;
		movwf	RoundedNext_H	;
		btfss	NextPixel_F,7	; IF NextPixel_Fraction >= 128 then
		goto	NotRoundUp		;		
		incf	RoundedNext_L,f	; ROUND UP
		btfsc	STATUS,Z	; 
		incf	RoundedNext_H,f	;
NotRoundUp:
		movf	PixelWidth,w	;
		movwf	PixelOff	; PixelOff = PixelWidth	

		goto	lDispDone	;	

lPixelOn	movf	PixelOff,w	; Is there a pixel on?
		btfsc	STATUS,Z	; PixelOff = 0 ?
		goto	lDispDone	; no, jump

		decfsz	PixelOff,f	; pixel is on, countdown
		goto	lDispDone	;
		movlw	0x00
		movwf	PORTB		; turn off display LED's
		bcf	PORTA,0		; also turn of PortA LED
	
lDispDone		
		bcf	INTCON,T0IF	; clear TMR0 interrupt flag before return

;--------
INT_TMR2	btfss	PIR1,TMR2IF	; interrupt on TMR2?
		goto	INT_EXIT	; nope, interrupt is done!		
				
		; do the TMR2 stuff, we get here every 200uSec

		; toggle PORTA.3, use scope or freq counter to calibrate to 2.5 kHz
		movf	PORTA,w		;
		xorlw   1 << bCalibration	;
		andlw	1 << bCalibration	;
		bcf	Calibration	; Toggle PORTA.3
		btfss	STATUS,Z	; 
		bsf	Calibration	;

		; Text-scroll timer
		movf	TmrScroll,f	; TmrScroll == 0 ?
		btfss	STATUS,Z	;
		decf	TmrScroll,f	; no, TmrScroll --

		; real time sub-second counter
		incfsz	SubSec_L,f	; Increment 16-bit Sub Second counter
		goto	lTime_1		; stil counting
		incfsz	SubSec_H,f	;
		goto	lTime_1		; stil counting
		
		incf	Second2,f	; 1/2 second has passed
		bsf	NewTime		; signal new time to main program
		movlw	0x3C		; reload counter SubSecond = 0x10000 - .2500 = 0xF63C
		movwf	SubSec_L	;
		movlw	0xF6		;
		movwf	SubSec_H	;
		
lTime_1		; start RC5 stuff here
		btfsc	RC5_DataReady	;
		goto	lRC5_Exit	;
		btfss	RC5_Idle	;
		goto	lRC5_Not_Idle	;
		decfsz	RC5_Tmr	,f	;
		goto	lRC5_Exit	;		
		btfsc	RC5inp		; test input
		bcf	RC5_Idle	; input = high, cancel Idle state
		incf	RC5_Tmr,f	; continue Idle state until input = high
		goto	lRC5_Exit	;		
lRC5_Not_Idle	
		btfss	RC5_WaitStart	;
		goto	lRC5_ReSync	;
lRC5WaitStart	btfsc	RC5inp		; test input
		goto	lRC5_Exit	; no startbit
		bcf	RC5_WaitStart	; start received
		movlw	.6		;
		movwf	RC5_Tmr		;
		movlw	.13		; 13 bits to receive
		movwf	RC5_BitCnt	;
		clrf 	RC5_Addr	;
		clrf	RC5_Cmd		;
		goto	lRC5_Exit	;
lRC5_ReSync				;
		movf	PORTA,w		;
		xorwf	RC5_flags,w	;
		andlw	0x04		;
		btfsc	STATUS,Z	;
		goto	lRC5_no_sync	;
		bsf	RC5_ReSynced	;
		movlw	.6		; re-sync the timer
		btfss	RC5_HalfBit	;
		movlw	.2		;
		movwf	RC5_Tmr		;
		bcf	RC5_prev_inp	; clear previous input
		btfsc	RC5inp		;
		bsf	RC5_prev_inp	;
		
lRC5_no_sync	btfsc	RC5_HalfBit	;
		goto	lRC5_2nd_Half	;

lRC5_1st_Half	decfsz	RC5_Tmr,f	;
		goto	lRC5_Exit	;	
		bcf	STATUS,C	;
		btfsc	RC5inp		;
		bsf	STATUS,C	; C = RC5inp
		rlf	RC5_Cmd,f	;
		rlf	RC5_Addr,f	; 
		bsf	RC5_HalfBit	; indicate that the first half bit is received
		bcf	RC5_ReSynced	;
		movlw	.4		;
		movwf	RC5_Tmr		; reload timer 
		goto	lRC5_Exit
lRC5_2nd_Half   
		btfsc	RC5_ReSynced	;
		goto	lReSyncOK	;
		decfsz	RC5_Tmr,f	;
		goto	lRC5_Exit	; 
lRC5_Error	clrf	RC5_flags	;
		bsf	RC5_WaitStart	;
		bsf	RC5_Idle	;
		movlw	.128		;
		movwf	RC5_Tmr		;
		goto	lRC5_Exit	;		
lReSyncOK	
		; test second bit half
		bcf	RC5_HalfBit	;
		decfsz	RC5_BitCnt,f	;
		goto	lRC5_Exit	;
		rlf	RC5_Cmd,f	; Shift bit right to compleet Addr
		rlf	RC5_Addr,f	;
		rlf	RC5_Cmd,f	;
		rlf	RC5_Addr,f	;
		bcf	STATUS,C	;
		btfss	RC5_Addr,6	;
		bsf	STATUS,C	; C = Inv Bit6 in RC5_Addr
		rrf	RC5_Cmd,f	;
		bcf	STATUS,C	;
		btfsc	RC5_Addr,5	;
		bsf	STATUS,C	; C = ToggleBit in RC5_Addr
		rrf	RC5_Cmd,f	;
		movlw	0x1F		;
		andwf	RC5_Addr,f	;
		bsf	RC5_DataReady	;
lRC5_Exit			
		bcf	PIR1,TMR2IF	; clear TMR2 interrupt flag
;--------
INT_EXIT		
		pop			; restore conext for main program
					; this will also restore bank selection
		retfie                  ; return from interrupt

;**********************************************************************

div16X8		; DIV_HI and DIV_LO / DIVISOR.  result to DIV_Q
		; remainder in DIV_LO
		; does not deal with divide by 0 case

		clrf 	DIV_Q
div_1
		movf 	DIVISOR, W
		subwf 	DIV_LO, F
		btfss 	STATUS, C	; if positive skip
		goto 	div_borrow
		goto 	div_2
div_borrow
		movlw 	.1
		subwf 	DIV_HI, F	; DIV_HI = DIV_HI - 1
		btfss 	STATUS, C	; if no borrow occurred
		goto	div_done	
div_2
		incf DIV_Q, F
		goto div_1
div_done
		movf DIVISOR, W		; re-add DIVISOR to DIV_LO to get
		addwf DIV_LO, F		; remainder in DIV_LO
	
		retlw	0

CalcPixelPitch	; thats a hard one. We have to divide by 120 !!!
		; PixelPitch = PeriodCnt / 120
		
		movf 	StorePCnt1_H,w	; check if two period counters
		xorwf	StorePCnt2_H,w	; are the same. If not, don't
		btfss	STATUS,Z	; calculate new pixelpitch because
		goto	CalcDone	; the rotation speed was not stable
		movf	StorePCnt1_L,w	;
		xorwf	StorePCnt2_L,w	;
		btfss	STATUS,Z	;
		goto	CalcDone	;		

		clrf	DIV_HI		; Divisor = PeriodCount_H
		movf	StorePCnt1_H,w	;
		movwf	DIV_LO		; 
		movlw	.120
		movwf	DIVISOR
		
		call 	div16X8
		
		movf	DIV_Q,w		; store result in High byte
		movwf	NewPPitch_H	;
		movf	DIV_LO,w	; shift remainder to DIV_HI
		movwf	DIV_HI		;
		movf	StorePCnt1_L,w	;
		movwf	DIV_LO		;
		
		call	div16X8
		
		movf	DIV_Q,w		; store result in Low byte
		movwf	NewPPitch_L	;
		movf	DIV_LO,w	; shift remainder to DIV_HI
		movwf	DIV_HI		;
		clrf	DIV_LO		;
		
		call	div16X8

		movf	DIV_Q,w		; store result in Fraction byte
		movwf	NewPPitch_F	;

		; start calculation for pixel width
		clrc			; Clear Carry
		rrf	NewPPitch_L,w	;
		movwf	PixelWidth	; PixelWidth = NewPPitch_L / 2
		movwf	Scratch		;
		clrc
		rrf	Scratch,f	;
		clrc	
		rrf	Scratch,w	; w = NewPPitch / 8
		addwf	PixelWidth,f	; PixelWidth = NewPPitch * 5 / 8

		bsf	STATUS,RP0	; Bank1
		movlw	0x01		; 
		movwf	TRISB		; PortB1..7 as output, PortB0 as input
		movlw	b'11110100'	; Port A bit 0, 1 and 3 as output
		movwf	TRISA		; Port A bit 2, 3, 4 as input
		bcf	STATUS,RP0	; Bank0

		btfss	fOuterLED	;
		bcf	OuterLED	; set Port for Outerled equal to fOuterLED
		btfsc	fOuterLED	;
		bsf	OuterLED	;

NotON:
		bsf	NewPPitch	; signal new Pitch value to interrupt routine
CalcDone:
		bcf	NewPCnt		; clear flag		

		retlw 	0

;******************************************************************************
;	Check correct progress of sec,min,hour,day,month,year
;******************************************************************************

TimeCheck	; second variable is changed, check if an overflow
		; is generated to minute, hour, day,...

		movlw	.120		; 
		subwf	Second2,w	; Second2 < 120 ? (counting 120 half seconds)
		btfss	STATUS,C        ; if second2 < 120 then c=0 (borrow)
		goto	lMinute		; 
					; yes, Second >= 120
		movwf	Second2		; Second = Second - 120
		incf	Minute,f	; Minute = Minute + 1
lMinute		
		movlw	.60		; 
		subwf	Minute,w	; Minute < 60 ?
		btfss	STATUS,C        ; if Minute < 60 then c=0 (borrow)
		goto	lHour		; 
					; yes, Minute >= 60
		movwf	Minute		; Minute = Minute - 60
		incf	Hour,f		; Hour = Hour + 1
lHour
		movlw	.24		; 
		subwf	Hour,w		; Hour < 24 ?
		btfss	STATUS,C        ; if Hour < 24 then c=1 (borrow)
		goto	lDay		; 
					; yes, Hour >= 24
		movwf	Hour		; Hour = Hour - 24
		incf	Day,f		; Day = Day + 1
lDay		
		movlw	.32		; DMon = 32
		movwf	DMon		
		movf	Month,w		; 
		xorlw	.4		; test for april
		btfsc	STATUS,Z	;
		decf	DMon,f		; Dmon = Dmon - 1 
		xorlw	.6 ^ .4		; test for juni
		btfsc	STATUS,Z	;
		decf	DMon,f		; Dmon = Dmon - 1 
		xorlw	.9 ^ .6		; test for september
		btfsc	STATUS,Z	;
		decf	DMon,f		; Dmon = Dmon - 1 
		xorlw	.11 ^ .9	; test for november
		btfsc	STATUS,Z	;
		decf	DMon,f		; Dmon = Dmon - 1 
	
		xorlw	.2 ^ .11	; Month = Februari ?
		btfss	STATUS,Z	; 
		goto	lNotFeb		; continue
		movlw	.29		
		movwf	DMon		;
		btfsc	Year,0		;
		goto	lNotFeb		;
		btfsc	Year,1		;
		goto	lNotFeb		;
		incf	DMon,f		; Year is LeapYear, inc DMon for Feb			
	
lNotFeb
		movf	DMon,w		; w = DMon
		subwf	Day,w		; Day < DMon ?
		btfss	STATUS,C        ; if Day < DMon then c=1 (borrow)
		goto	lMonth		; 
		
		movwf	Day		; Day = (Day - DMon)+1
		incf	Day,f		; 
		incf	Month,f		; Month = Month + 1
		
lMonth
		movlw	.13		; 
		subwf	Month,w		; Month < 13 ?
		btfss	STATUS,C        ; if Month < 13 then c=0 (borrow)
		goto	lYear		; 
					
		movwf	Month		; Month = Month - 12
		incf	Month,f		;
		incf	Year,f		; Year = Year + 1
	
lYear		
		movlw	.100		;
		subwf	Year,w		; Year < 100 ?
		btfsc	STATUS,C	; 
		clrf	Year		;		

		bcf	NewTime		;
		retlw 	0


;******************************************************************************
;	Routines to display Time and Date
;******************************************************************************

; CHARACTER LOOKUP TABLE
; set=LED on, clear=LED off
Char_tbl1
   	addwf   PCL,f

	DT  0x3E , 0x41 , 0x41 , 0x41 , 0x3E ; "0"
;	DT  0x3E , 0x45 , 0x49 , 0x51 , 0x3E ; "0"
	DT  0x00 , 0x21 , 0x7F , 0x01 , 0x00 ; "1"
	DT  0x21 , 0x43 , 0x45 , 0x49 , 0x31 ; "2"
	DT  0x42 , 0x41 , 0x51 , 0x69 , 0x46 ; "3"
	DT  0x0C , 0x14 , 0x24 , 0x7F , 0x04 ; "4"
	DT  0x72 , 0x51 , 0x51 , 0x51 , 0x4E ; "5"
	DT  0x1E , 0x29 , 0x49 , 0x49 , 0x06 ; "6"
	DT  0x40 , 0x47 , 0x48 , 0x50 , 0x60 ; "7"
	DT  0x36 , 0x49 , 0x49 , 0x49 , 0x36 ; "8"
	DT  0x30 , 0x49 , 0x49 , 0x4A , 0x3C ; "9"
	DT  0x00 , 0x36 , 0x36 , 0x00 , 0x00 ; ":"
	DT  0x00 , 0x00 , 0x00 , 0x00 , 0x00 ; " "
Char_tbl1_end

;-flipped digits upside down and mirrored
;-needed for the bottom display
Char_tbl2
   	addwf   PCL,f
	DT 0x3E , 0x41 , 0x41 , 0x41 , 0x3E ; "0"	
	DT 0x00 , 0x40 , 0x7F , 0x42 , 0x00 ; "1"
	DT 0x46 , 0x49 , 0x51 , 0x61 , 0x42 ; "2"
	DT 0x31 , 0x4B , 0x45 , 0x41 , 0x21 ; "3"
	DT 0x10 , 0x7F , 0x12 , 0x14 , 0x18 ; "4"
	DT 0x39 , 0x45 , 0x45 , 0x45 , 0x27 ; "5"
	DT 0x30 , 0x49 , 0x49 , 0x4A , 0x3C ; "6"
	DT 0x03 , 0x05 , 0x09 , 0x71 , 0x01 ; "7"
	DT 0x36 , 0x49 , 0x49 , 0x49 , 0x36 ; "8"
	DT 0x1E , 0x29 , 0x49 , 0x49 , 0x06 ; "9"
	DT 0x00 , 0x08 , 0x08 , 0x08 , 0x00 ; "-"
	DT 0x00 , 0x00 , 0x00 , 0x00 , 0x00 ; " "
Char_tbl2_end

	IF Char_tbl1 < 0x100
		ERROR "Tabel1 pages error"
        ENDIF

	IF Char_tbl2_end >= 0x200
		ERROR "Tabel2 pages error"
        ENDIF

;-------- Update Display memory with Hands

Hands		movlw	.30		; difference in offset for hands and digital display
		subwf	dpi,w		; scratch4 = dpi - 30
		btfss	STATUS,C	; if scratch4 < 0 then 
		addlw	.120		;    scratch4 = scartch4 + 120
		movwf	Scratch4	; 

		movf	Second2,w	;
		andlw	0xFE		; filter out 1/2 second
		xorwf	Scratch4,w	;
		btfsc	STATUS,Z	;
		bsf	tdd,5	;
		
		bcf	STATUS,C	;
		rlf	Minute,w	;
		xorwf	Scratch4,w	;
		btfss	STATUS,Z	;
		goto	lhour		;
		movlw	B'10011111'	;
		iorwf	tdd,f	;	
		
lhour		movf	Hour,w		;
		movwf	Scratch2	; 
		bcf	STATUS,C	; clear carry
		rlf	Scratch2,f	; Scratch2 * 2
		rlf	Scratch2,f	; Scratch2 * 2
		addwf	Scratch2,f	; Scratch2 = 5 * hour
		rlf	Scratch2,f	; Scratch2 = 10 * Hour
		movf	Minute,w	; 
		movwf	Scratch		; Scratch = minute
		movlw	.6		;
l_hour_div6	subwf	Scratch,f	;
		btfss	STATUS,C	;
		goto	l_div6_done	;
		incf	Scratch2,f	; advance hour abit
		goto	l_hour_div6	;
l_div6_done	movlw	.120		; Scratch2 = (10 * Hour) + (Minute / 6)
l_120		subwf	Scratch2,f	; 
		btfsc	STATUS,C	; result > 0 ?
		goto	l_120
		addwf	Scratch2,w	; result was negatif, re-add 120 and load it in w
		xorwf	Scratch4,w	;
		btfss	STATUS,Z	;
		return			;
		movlw	B'10000001'	;
		iorwf	tdd,f	;	
		return

;-------- Update Display memory with Digital Time Display

DTime		movlw	.6		;
		subwf	dpi,w		;
		btfss	STATUS,C	;
		return			; display index < 6
		btfss	STATUS,Z	;
		goto	l_DTime_1	; display index > 6

		clrf	digitindex	; display index = 6, load start parameters for
		clrf	dotindex	; digit displaying

l_DTime_1	movlw	.54		;
		subwf	dpi,w		;
		btfsc	STATUS,C	;
		return			; display index >= 54 
		
		movf	dotindex,w	;
		btfss	STATUS,Z	;
		goto	l_DTime_3
		call	LoadBCDTime	; dotindex = 0, load new digit
		movf	digitindex,w	;
		btfss	STATUS,Z	;
		goto	l_DTime_3
		movf    BCD,w		; digit index = 0, 10 hour digit
		btfss 	STATUS,Z	;
		goto	l_DTime_3	;
		movlw	0x37		; 10 hour digit is zero, remove leading zero
		movwf	BCD		; BCD = " "
l_DTime_3
		incf	dotindex,f	;
		movlw	.6		;
		xorwf	dotindex,w	;
		btfss	STATUS,Z	;
		goto	l_DTime_2	; dotindex < 6, display data
		clrf	dotindex	; dotindex = 6, reset dotindex and don't display 
		incf	digitindex,f	; select next digit
		return			; to get a gap between digits
l_DTime_2
		movf	BCD,w		;
		bcf	PCLATH,1	; set to page 0x100 for lookup
		bsf	PCLATH,0	
                call    Char_tbl1       ; get the dot pattern for this column
		movwf	tdd		;
		incf	BCD,f		;
		return			;

;-------- Update Display memory with Digital Date Display

DDate		movlw	.66		;
		subwf	dpi,w		;
		btfss	STATUS,C	;
		return			; display index < 66
		btfss	STATUS,Z	;
		goto	l_DDate_1	; display index > 66

		clrf	digitindex	; display index = 66, load start parameters for
		clrf	dotindex	; digit displaying

l_DDate_1	movlw	.114		;
		subwf	dpi,w		;
		btfsc	STATUS,C	;
		return			; display index >= 114 
		
		movf	dotindex,w	;
		btfsc	STATUS,Z	;
		call	LoadBCDDate	; dotindex = 0, load new digit

		incf	dotindex,f	;
		movlw	.6		;
		xorwf	dotindex,w	;
		btfss	STATUS,Z	;
		goto	l_DDate_2	; dotindex < 6, display data
		clrf	dotindex	; dotindex = 6, reset dotindex and don't display 
		incf	digitindex,f	; select next digit
		return			; to get a gap between digits
l_DDate_2
		movf	BCD,w		;
		bcf	PCLATH,1	; set to page 0x100 for lookup
		bsf	PCLATH,0	
                call    Char_tbl2       ; get the dot pattern for this column
		movwf	tdd		;
		incf	BCD,f		;
		return			;
;--------
Conv2BCD	movwf	Scratch		;
		clrf	Scratch2	;
		movlw	.10		;
l_bcd_1		subwf	Scratch,f	;
		btfss	STATUS,C	;
		goto	l_bcd_2
		incf	Scratch2,f	;
		goto	l_bcd_1
l_bcd_2		addwf	Scratch,f	;
		swapf	Scratch2,w	;
		addwf	Scratch,w	;
		return
;--------
LoadBCDDate	; load BCD with date digits
		movf	digitindex,w	;
		clrf	PCLATH
		bsf	PCLATH,1	; set to page 0x200 for lookup
		addwf	PCL,f		;
		goto	l_1year		;
		goto	l_10year	;
		goto	l_dot		;
		goto	l_1mon		;
		goto	l_10mon		;
		goto	l_dot		;
		goto	l_1day		;
		goto	l_10day		;

LoadBCDTime	; load BCD with time digits
		movf	digitindex,w	;
		clrf	PCLATH
		bsf	PCLATH,1	; set to page 0x200 for lookup
		addwf	PCL,f		;
		goto	l_10hour	;
		goto	l_1hour		;
		goto	l_dot		;
		goto	l_10min		;
		goto	l_1min		;
		goto	l_dot		;
		goto	l_10sec		;
;		goto	l_1sec		;

l_1sec		bcf	STATUS,C	; Second2 / 2 = Second
		rrf	Second2,w	;
		goto	l_convert	;
l_1min		movf	Minute,w
		goto	l_convert	;
l_1hour		movf	Hour,w
		goto	l_convert	;
l_1day		movf	Day,w
		goto	l_convert	;
l_1mon		movf	Month,w
		goto	l_convert	;
l_1year		movf	Year,w
l_convert	call	Conv2BCD
		andlw	0x0F		;
		movwf	BCD		;		
		goto	l_mult5		; multiply BCD by 5

l_10sec		bcf	STATUS,C	; Second2 / 2 = Second
		rrf	Second2,w	;
		goto	l_convert_swap	;
l_10min		movf	Minute,w	;
		goto	l_convert_swap	;
l_10hour	movf	Hour,w
		goto	l_convert_swap	;
l_10day		movf	Day,w
		goto	l_convert_swap	;
l_10mon		movf	Month,w
		goto	l_convert_swap	;
l_10year	movf	Year,w
l_convert_swap	call	Conv2BCD
		andlw	0xF0		;		
		movwf	BCD		;
		swapf	BCD,f		;
		movf	BCD,w		;
l_mult5		rlf	BCD,f		; BCD x 2
		rlf	BCD,f		; BCD x 2
		addwf	BCD,f		; + BCD
		return		
				
l_dot		movlw	0x32		; 0x0A * 5 = 0x32
		movwf	BCD		;		
		return
		
;-------- Update Display memory with Time Ticks every 5 minutes

Ticks		movf	dpi,w		;
		xorwf	timeticks,w	;
		btfss	STATUS,Z	;
		return
		bsf	tdd,6
		movlw	.10
		addwf	timeticks,f	; set next tick @
		return

;******************************************************************************
;	Processing of RC5 command's, called by main program if a command is received
;******************************************************************************

ProcessRC5
		movf	RC5_Addr,w		;		
		xorlw	RemoteAddr		; test if RC5_Addr = RemoteAddr
		btfss	STATUS,Z		; 
		goto	ProcessRC5Done		;

		movf	RC5_Cmd,w		;
		andlw	0x7F			;	
		
		select_w
		case	NUM_0			;
		  clrf	Second2			; Zero pressed => Seconds = 0
		case	SEC_UP			;
		  goto	IncSecond		; Adjust time : Increment Seconds
		case	SEC_DN			;
		  goto	DecSecond		; Adjust time : Decrement Seconds
		case	MIN_UP			;
		  incf	Minute,f		; Adjust time : Increment Minutes
		case	MIN_DN			;
		  goto	DecMinute		; Adjust time : Decrement Minutes
		case	HOUR_UP			;
		  goto	IncHour			; Adjust time : Increment Hours
		case	HOUR_DN			;
		  goto	DecHour			; Adjust time : Decrement Hours
		case	DAY_UP			;
		  goto	IncDay			; Adjust Date : Increment Days
		case	DAY_DN			;
		  goto	DecDay			; Adjust Date : Decrement Days
		case	MON_UP			;
		  goto	IncMonth		; Adjust Date : Increment Month
		case	MON_DN			;
		  goto	DecMonth		; Adjust Date : Decrement Month
		case	YEAR_UP			;
		  incf	Year,f			; Adjust Date : Increment Year
		case	YEAR_DN
		  goto	DecYear			; Adjust Date : Decrement Year
		case	INDEX_UP		;
		  goto	DecDispOffset		; Adjust index sensor Offset, rotate display left
		case	INDEX_DN		;
		  goto	IncDispOffset		; Adjust index sensor Offset, rotate display right
		
		; Toggle functions
		movf	RC5_Cmd,w		;
		xorwf	RC5_Cmd2,f		;
		btfsc	STATUS,Z		;
		goto	ProcessRC5Done		;
		andlw	0x7F			;	

		select_w
		case	BlueLine 		;
		  goto	ToggleOuterLed		; Blue Line
		case	DigiTime		;
		  goto	ToggleTime		; Digital Time
		case	DigiDate		;
		  goto	ToggleDate		; Digital Date
		case	AnaTime			;
		  goto	ToggleAnalog		; Analog Time
		case	TTicks			;
		  goto	ToggleTick		; Analog Time Ticks
		case	DM			;
		  goto	ToggleDemo		; Demo Mode
		case	TextMode		;
		  goto	ToggleText		; Text Mode

ProcessRC5Done		
		movf	RC5_Cmd,w	;
		movwf	RC5_Cmd2	; 
		bsf	NewTime		; force display update
		bcf	RC5_DataReady	;
		return			;
		
ToggleOuterLed	movlw	1 << bOuterLED	;
		xorwf	flags2,f	;
		bcf	OuterLED	;
		btfsc	fOuterLED	;
		bsf	OuterLED	;
		goto	ProcessRC5Done	;
		
ToggleTime	movlw	1 << bShowDTime	;
		xorwf	flags,f		;
		goto	ProcessRC5Done	;
		
ToggleDate	movlw	1 << bShowDDate	;
		xorwf	flags,f		;
		goto	ProcessRC5Done	;
		
ToggleAnalog	movlw	1 << bShowHand	;
		xorwf	flags,f		;
		goto	ProcessRC5Done	;
		
ToggleTick	movlw	1 << bShowTicks	;
		xorwf	flags,f		;
		goto	ProcessRC5Done	;
		
IncDispOffset	incf	DispOffset,w	;
		call 	CheckIncrement 	;
		movwf	DispOffset	;				
		goto	ProcessRC5Done	;

DecDispOffset	decf	DispOffset,w	;
		call 	CheckDecrement	;
		movwf	DispOffset	;				
		goto	ProcessRC5Done	;
		
IncSecond	incf	Second2,f	;
		incf	Second2,f	;
		goto	ProcessRC5Done	;
		
DecSecond	movlw	.2		;
		subwf	Second2,f	;
		btfsc	STATUS,C	;	
		goto	ProcessRC5Done	;
		movlw	.120		;
		addwf	Second2,f	;
DecMinute
		movlw	.1
		subwf	Minute,f	;
		btfsc	STATUS,C	;
		goto	ProcessRC5Done	;
		movlw	.60		;
		addwf	Minute,f	;
DecHour
		movlw	.1
		subwf	Hour,f		;
		btfsc	STATUS,C	;
		goto	ProcessRC5Done	;
		movlw	.24		;
		addwf	Hour,f		;
		goto	ProcessRC5Done	;

IncHour		incf	Hour,f		;
		movlw	.24		;
		xorwf	Hour,w		;
		btfsc	STATUS,Z	;
		clrf	Hour		;
		goto	ProcessRC5Done	;

IncDay		decf	DMon,w		;
		xorwf	Day,w		;
 	        btfsc	STATUS,Z	;
		clrf	Day		;
		incf	Day,f		;		
		goto	ProcessRC5Done	;

DecDay		decfsz	Day,f		;
		goto	ProcessRC5Done	;
		decf	DMon,w		;
		movwf	Day		;
		goto	ProcessRC5Done	;

IncMonth	movlw	.12		;
		xorwf	Month,w		;
		btfsc	STATUS,Z	;
		clrf	Month		;
		incf	Month,f		;
		goto	ProcessRC5Done	;

DecMonth	decfsz	Month,f		;
		goto	ProcessRC5Done	;
		movlw	.12		;
		movwf	Month		;
		goto	ProcessRC5Done	;

DecYear		movlw	.1		;
		subwf	Year,f		;
		btfsc	STATUS,C	;
		goto	ProcessRC5Done	;
		movlw	.99		;
		movwf	Year		;
		goto	ProcessRC5Done	;

ToggleText	call	TextON_OFF	; Toggel Text mode
		goto	ProcessRC5Done	;

ToggleDemo	movlw	1 << bDemo	;
		xorwf	flags2,f	;
		goto	ProcessRC5Done	;

TextON_OFF	bcf	fScrollOn	; Scrolling OFF
		movlw	1 << bText	; toggle Text flag
		xorwf	flags2,f	;
		btfss	fText		; test Text flag
		goto	ProcessRC5Done	;
		call	ClearDisplay	;
		bsf	fScrollOn	; Scrolling ON
		return
			
;******************************************************************************
;	Initialisation routines, called by main program
;******************************************************************************

InitRam
		clrf	PixelWidth	;
		clrf	PixelOff	;
		movlw	0x37		; Initial Display Offset for my hardware !!!
		movwf	DispOffset	;
		movwf	iFSR		;

		movlw	0x3C		; load counter SubSecond = 0x10000 - 2500 = 0xF63C
		movwf	SubSec_L	;
		movlw	0xF6		;
		movwf	SubSec_H	;
		clrf	flags		; clear all flags
		clrf	flags2		;

		movlw	.12		;why do clocks always start
		movwf	Hour		;at 12:00 ?
		clrf	Minute
                clrf	Second2
		movlw	0x01
		movwf	Day
		movwf	Month
		movlw	Year
				
		clrf	RC5_flags	; clear flags
		clrf	RC5_Tmr		;
		bsf	RC5_WaitStart	; RC5 is waiting for startbit
		return
		
;--------		
InitIO
		clrf	PORTA		; all LED's OFF
		clrf	PORTB

		bcf	STATUS,IRP	; Bank0.1 for FSR/INDF operations
		bcf	STATUS,RP1	; 
		bsf	STATUS,RP0	; Bank1
		movlw	0xFF		;
		movwf	TRISA		; set all is input
 		bcf	TRISA, bCalibration	; Set Calibration pin as output
 		movwf	TRISB		; outputs will be turned on if rotation is stable
		bcf	STATUS,RP0	; Bank0

		movlw	0x07		;
		movwf	CMCON		; comparators OFF
		
		retlw	0
;--------		
InitTmr 
		clrf	INTCON		; clear and dissable all possible interrupt flag
		clrf	PIR1		; 

		movlw	b'00000101'	; TMR2=ON, Prescaler = 1/4
		movwf	T2CON		; 

		bsf	STATUS,RP0	; Bank1
		clrf	PIE1		; dissable all possible interrupt flag
		movlw	b'10011000'	; set up timer. prescaler(bit3)bypassed, Int on falling Edge RB0
		movwf	OPTION_REG	; send w to option.
		movlw	.249		; TMR2 PR2=249
		movwf	PR2		;
		bsf	PIE1,TMR2IE	; enable Peripheral interrupt from TMR2
		bcf	STATUS,RP0	; Bank0

		bsf	INTCON,T0IE	; enable TMR0 interrupt
		bsf	INTCON,INTE	; enable RB0 interrupt
		bsf	INTCON,PEIE	; enable Peripheral interrupts
		bsf	INTCON,GIE	; enable global interrupts

		clrf	TMR0		; start timer
		return

;--------		
ClearDisplay
		movlw	0x20		; set FSR to 0x20
		movwf	FSR		;
cldisp
		clrf	INDF		; clear display memory
		incf	FSR,f		; 
		movlw	0x48
		xorwf	FSR,w		; FSR = 0x48
		btfss	STATUS,Z	; ??
		goto	cldisp_2	; nope, jump
		movlw	0x58		; w = 0x58
		addwf	FSR,f		; FSR = FSR + 0x58 = 0xA0
cldisp_2
		movlw	0xF0		; 
		xorwf	FSR,w		; FSR = 0xF0
		btfss	STATUS,Z	; ??
		goto	cldisp		; nope, jump

		; init stuff needed for Text Scroll function
		bsf	STATUS,RP1	; jump to bank 2
      		clrf	ch_dot_index	; 
      		clrf	ch_blanking	;
      		bcf	STATUS,RP1	; return to bank 0
		bsf	STATUS,RP0	; goto bank 1
		clrf	EEADR		; inc address for next read
		bcf	STATUS,RP0	; goto bank 0

		return

;******************************************************************************
;	Main program 
;******************************************************************************

main

		call InitIO		; Initialise Ports
		call InitRam		; Initialise Ram and flags
		call ClearDisplay	; clear display content
		call InitTmr		; Initialise Timers and interrupt

		bsf	ShowHand	; turn analog hands on
		bsf	ShowDTime	;
		bsf	ShowDDate	;
		bsf	ShowTicks	;
		bsf	OuterLED	;
MainLoop
		btfsc	RC5_DataReady	;
		call	ProcessRC5	;

		btfss	fScrollOn	;
		goto	NoScrolling	;
		movf	TmrScroll,w	;
		btfss	STATUS,Z	;
		goto	NoScrolling	;
		decf	TmrScroll,f	; TmrScroll-- => 0 - 1 = 0xFF 
		call	ScrollText	;

NoScrolling
		btfsc	NewPCnt		; test Period counter flag
		call	CalcPixelPitch  ; calculate new pixel pitch

		btfss	NewTime		; test new time flag
		goto	MainLoop	;

; one second past. Do timecheck and if text=off, also a display update.
		call	TimeCheck	;

		btfsc	fDemo		;
		call	DemoMode	;

		btfsc	fText		; test Image flag
		goto  	MainLoop	; if Image flag SET, leave display-content unchanged

; start display memory update
		clrf	dpi		; dpi = 0
		clrf	timeticks	; keep index for 5 min timeticks
		bcf	digitstart	; 
		movlw	0x20		; start of display memory
		movwf	FSR
		bcf	digitstart	;

lUpdateDisplay	
		
		clrf	tdd		;

		btfsc	ShowDTime	;
		call	DTime		; Display digital Time
		
		btfsc	ShowDDate	;
		call	DDate		; Display digital Date
		
		btfsc	ShowHand	;
		call	Hands	
		btfsc	ShowTicks	;
		call	Ticks		;
		
		movf	tdd,w		; store data in 
		movwf	INDF		; display memory
		
		incf	dpi,f		;
		movlw	.120		;
		xorwf	dpi,w		;
		btfsc	STATUS,Z	;
		goto  	MainLoop	;
		
		incf	FSR,f		;
		movlw	0x48		; for memory gap
		xorwf	FSR,w		; FSR = 0x48
		btfss	STATUS,Z	; ??
		goto	lUpdateDisplay	; nope, jump
		movlw	0x58		; w = 0x58
		addwf	FSR,f		; FSR = FSR + 0x58 = 0xA0		
		goto	lUpdateDisplay
		
;******************************************************************************
;	Text display functions
;******************************************************************************

CharOffset	movwf	ch_dot_point_L		; store ASC code
		clrf	ch_dot_point_H		; clear high byte
		movlw	.32			; asc code - 32 
		subwf	ch_dot_point_L,f	; 
		btfss	STATUS,C		; ASC < 32 ? Invalid code, 
		clrf	ch_dot_point_L		; relpace with CHR(32) = " "
		movlw	.127			;
		subwf	ch_dot_point_L,w	;
		btfsc	STATUS,C		; ASC >= 127 ? Invalid code,
		movwf	ch_dot_point_L		; replace with last valid ASC code
	
		bcf 	STATUS,C		; Clear Carry flag
		rlf	ch_dot_point_L,f	; asc code * 2
		rlf	ch_dot_point_H,f	; 		

		bcf 	STATUS,C		; Clear Carry flag		
		rlf	ch_dot_point_L,w	; 
		movwf	Scratch			; Scratch = (asc code * 2) *2
		rlf	ch_dot_point_H,w	;
		movwf	Scratch2		; 
		
		movf	Scratch,w		; ch_dot_point (16bit) = (asc code - 32) * 6
		addwf	ch_dot_point_L,f	; 
		btfsc	STATUS,C		; 
		incf	Scratch2,f		;
		movf	Scratch2,w		;
		addwf	ch_dot_point_H,f	; ch_dot_point = ch_dot_point + 0x500 start of char gen table
		movlw	0x05			; this is the entry in the char gen table for the
		addwf	ch_dot_point_H,f	; first pixel data of the requested char
		
		movlw	.6
		movwf	ch_dot_index		; indicate start of new char
		return

;--------		
LoadChrData	movf	ch_dot_point_H,w	;
		movwf	PCLATH			;
		movf	ch_dot_point_L,w	;
		call	CharGen			;
		incf	ch_dot_point_L,f	; pointer++
		btfsc	STATUS,Z		;
		incf	ch_dot_point_H,f	;
		decf	ch_dot_index,f		; dec dot-index
		return							

;--------		
PrintDisp	call	ClearDisplay		; clear display content
		bsf	STATUS, RP0		; Bank1
		clrf	EEADR			; start of EE-memory
		movlw	0xE4			; Start for first char a 7 o'clock
		movwf	FSR			;
		movlw	.14			; 14 char to display
		movwf	Scratch3		;
pdisp_1				
		bsf	EECON1, RD		; read character from EEProm
		incf	EEADR,f			; inc address for next read
		movf	EEDATA,w		;

		bsf	STATUS,RP1		; goto Bank2
		bcf	STATUS,RP0		;

		call	CharOffset		; call character data
pdisp_2		
		call	LoadChrData		; load pixel data from CharGen
		movwf	INDF			; store char pixel data in display memory
		incf	FSR,w		; increment FSR
		call 	CheckIncrement	;
		movwf	FSR		;				
		
		decfsz	ch_dot_index,f		; 6 dot's in one character, all displayed ?
		goto	pdisp_2			;

		bcf	STATUS,RP1		; leave bank 2, back to bank1
		bsf	STATUS,RP0		;
			
		decfsz	Scratch3,f		; 						
		goto	pdisp_1				
		
		bcf	STATUS,RP0		; go back to bank 0
		return	
		
;--------
ScrollText	bsf	STATUS,RP1		; goto Bank2
		bcf	STATUS,RP0		;
      		
      		movf	ch_dot_index,w		;
		btfss	STATUS,Z		; dot_index == 0 ?
		goto	Scroll_0 
		
		movf	ch_blanking,w		;
		btfsc	STATUS,Z		;
		goto	Scroll_read_ee		;
		decfsz	ch_blanking,f		;
		goto	Scroll_2		; insert one more " "
		btfss	fDemo			; in demo mode?
		goto	Scroll_read_ee		; re-read char
		
		bcf	STATUS,RP1		; to bank 0
		call	TextON_OFF		; at end of line, turn text off!
		return				; stop scrolling
		
		
Scroll_read_ee	bcf	STATUS,RP1		; leave bank 2, to bank1
		bsf	STATUS,RP0		;
		bsf	EECON1, RD		; read character from EEProm
		incf	EEADR,f			; inc address for next read
		bcf	EEADR,7			; roll over after 0x80 => 0x00
		movf	EEDATA,w		;

		btfsc	STATUS,Z		; Chr$(0) indicates end of line
		clrf	EEADR			; start of EE-memory
		bsf	STATUS,RP1		; go back to Bank2
		bcf	STATUS,RP0		;

		btfss	STATUS,Z		; if Z was 1, it wil still be 1 here 
		goto	Scroll_1		;
		movlw	0x0F			; insert 15 " " at end of string to clear display
		movwf	ch_blanking		;
Scroll_2
		movlw	" "			; w = " "
Scroll_1

		call	CharOffset		; call character data
Scroll_0
		call	LoadChrData		; load pixel data from CharGen
		movwf	Scratch			;		

#ifdef ReversTextScroll
		movlw	0xE4			; Start at begin of 7 o'clock character
#else
		movlw	0xBF			; Start at end of 5 o'clock character
#endif
		movwf	FSR			;
ScrollLoop
		movf	INDF,w			; load current data
		movwf	Scratch2		; temp store
		movf	Scratch,w		; 
		movwf	INDF			; 
		movf	Scratch2,w		; shift data
		movwf	Scratch			;
		
#ifdef ReversTextScroll
		incf	FSR,w			; 
		call	CheckIncrement
		movwf	FSR			;						
		movlw	.14			; check end of 5 o'clock character
#else
		decf	FSR,w			; 
		call	CheckDecrement
		movwf	FSR			;						
		movlw	0xE3			; check begin of 7 o'clock character
#endif
		subwf	FSR,w			;
		btfss 	STATUS,Z		; FSR = 0xE4 ? (or 0xC0)
		goto	ScrollLoop		;

		bcf	STATUS,RP0		; goto Bank0
		bcf	STATUS,RP1		; 

		return				;

;DemoMode	btfsc	fScrollOn		; in scroll mode?
;		goto	Demo_0			; yes, already in scroll mode
;		
;		movf	Second,w		;
;		btfsc	STATUS,Z		;
;		bsf	fSCrollOn		;	
;		return
;		
;Demo_0		
;
;		return				;

DemoMode	movf	Second2,w		;
		btfsc	STATUS,Z		;
		call	TextON_OFF
		return
		
				
;******************************************************************************
;	Some general functions
;******************************************************************************
				
;- check correct decrement of Display memory pointer
CheckDecrement
		xorlw	0x1F		;
		btfsc	STATUS,Z	;
		movlw	0xEF ^ 0x1F	;
		xorlw	0x9F ^ 0x1F	;
		btfsc	STATUS,Z	;
		movlw	0x47 ^ 0x9F	;
		xorlw	0x9F		;
		return			;

;- check correct increment of Display memory pointer
CheckIncrement
		xorlw	0x48		;
		btfsc	STATUS,Z	;
		movlw	0xA0 ^ 0x48	;
		xorlw	0xF0 ^ 0x48	;
		btfsc	STATUS,Z	;
		movlw	0x20 ^ 0xF0	;
		xorlw	0xF0		;
		return			;
		
				
	END    ; directive 'end of program'
