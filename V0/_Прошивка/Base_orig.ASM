;************************************************************************
; Following the example of Bob Blick's original propeler clock I made 	*
; my own version of this fancy clock and have re-written the code from	*
; scratch. 								*
; This 16f628 code is for the stationary part of the clock and will 	*
; drive the primary transformer coil, and use RC5 to switch the motor	*
; on and off (and adjust the speed)					*
;									*
;************************************************************************
;                                                               	*
;    Filename:	    base.asm                                        	*
;    Date:          20/08/2002                                        	*
;    Last Modified: 07/02/2003                                        	*
;    File Version:  1.04                                        	*
;                                                                     	*
;    Author:        Soubry Henk                                       	*
;    Company:       Soubry Software Service                           	*
;                                                                     	*	 
;                                                                     	*
;************************************************************************
;                                                                    	*
;    Files required:                                                  	*
;                                                                     	*
;                                                                     	*
;                                                                     	*
;************************************************************************
;                                                                     	*
;    Notes:                                                           	*
;    	Pin assignment                                                	*
;      		Port A                                                	*
;               2 = Ir receiver                                       	*
;									*
;      		Port B							*
;		RB0 = unused
;		RB1 = Index LED						*
;		RB2 = FET GATE for motor				*
;		CCP1(RB3) = FET GATE for primary transformer coil	*
;		RB4 = Motor Shutdown					*
;		RB5 = DCF77 input					*
;		RB6..7 = unused						*
;                                                                     	*
;************************************************************************


	list	p=16f628		; list directive to define processor
	#include <p16f628.inc>        	; processor specific variable definitions
	#include "keys.asm"
	
;	__CONFIG _CP_OFF & _WDT_ON & _BODEN_ON & _PWRTE_ON & _HS_OSC & _MCLRE_OFF & _LVP_OFF
	__CONFIG _CP_OFF & _WDT_OFF & _BODEN_ON & _PWRTE_ON & _HS_OSC & _MCLRE_OFF & _LVP_OFF

;***** VARIABLE DEFINITIONS

;Vars in shared memory. This memory is available from any bank
w_temp		EQU	0x7D        ; variable used for context saving 
status_temp	EQU	0x7E        ; variable used for context saving
fsr_temp	EQU	0x7F	    ; variable used for context saving

  cblock	0x20
	SubSec_L
	SubSec_H
	SleepTimer
	flags
			
	PWM_Tmr		; Timer counter for motor PWM
	PWM_Duty	; counter for PWM duty cycle			
	PWM_T_Pre	; preset value for PWM_Tmr
	PWM_D_Pre	; preset value for PWM_Duty

	RC5_flags	;
	RC5_Tmr		;
	RC5_BitCnt	;
	RC5_Addr	;
	RC5_Cmd		;
	RC5_Cmd2	; storage for previous cmd
			
	Scratch		; memory locations for general use
	Scratch2	;
	Scratch3	;
	Scratch4	;					
  endc

  IF Scratch4 >= 0x7D
	ERROR "To many variables used"
  ENDIF

;**********************************************************************

bMotorOFF	EQU	4
bTrafo		EQU	3
bMotor		EQU	2
bIndexLed	EQU	1
bRC5inp		EQU	2
bDCF77		equ	5

PR2Sleep	EQU	0x6D
PR2RUN		EQU	0xB4
CCPR1LSleep	EQU	0x10
CCPR1LRUN	EQU	0x55 ;0X25

;**********************************************************************

#define 	Trafo		PORTB,bTrafo
#define 	Motor		PORTB,bMotor
#define 	RC5inp		PORTA,bRC5inp
#define 	MotorOFF	PORTB,bMotorOFF
#define		IndexLed	PORTB,bIndexLed
#define		DCF77		PORTB,bDCF77

#define		RC5_WaitStart	RC5_flags,0
#define		RC5_DataReady	RC5_flags,1
#define		RC5_prev_inp	RC5_flags,2
#define		RC5_Idle	RC5_flags,3
#define		RC5_HalfBit	RC5_flags,4
#define		RC5_ReSynced	RC5_flags,5
#define		PStorage	RC5_flags,6
#define		reserved	RC5_flags,7

#define 	StandBy		flags,0
#define		Sleeping	flags,1


#define		Bank0		bcf STATUS,RP0
#define		Bank1		bsf STATUS,RP0


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

;**********************************************************************
		ORG     0x000           ; processor reset vector
		goto    main            ; go to beginning of program


		ORG     0x004           ; interrupt vector location
		
		push			; context switching
		Bank0			; select bank 0 for interrupt stuff

;**********************************************************************

INT_TMR0	btfss	INTCON,T0IF	; Test if a TMR0 interrupt occured
		goto	INT_RB		; nope, goto RB port change interrupt
;--------						
		; do the TMR0 stuff, we get here every 200uSec
		nop
		movlw	.9		; TMR0 = 9
		movwf	TMR0		;

		btfss	StandBy		; In standby? 
		goto	lTime_1		; no, skip sleep timer
		movf	SleepTimer,w	;
		btfsc	STATUS,Z	; SleepTimer = 0 ?
		goto	lTime_1		; yes, stop countdown
		
		incfsz	SubSec_L,f	; Increment 16-bit Sub Second counter
		goto	lTime_1		; stil counting
		incfsz	SubSec_H,f	;
		goto	lTime_1		; stil counting
		
		decf	SleepTimer,f	; count down sleeptimer in 1/2 seconcs steps
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
		; do stuff for s/w PWM 
		btfsc	StandBy		; motor ON ?
		goto	lPWM_Exit	; no, shutdown motor.	
		; yes, run PWM to spin motor
		decfsz	PWM_Duty,f	; dec Dutycycle counter
		goto	Duty_not_Zero	;
		bcf	Motor		; if zero, turn OFF motor output
Duty_not_Zero
		decfsz	PWM_Tmr,f	; dec PWM freq counter
		goto	lPWM_Exit	;
		
		movf	PWM_D_Pre,w	; reload timer and duty cycle register
		movwf	PWM_Duty	;
		movf	PWM_T_Pre,w	;
		movwf	PWM_Tmr		;
		bsf	Motor		; turn ON motor output
lPWM_Exit
		bcf	INTCON,T0IF	; clear TMR0 interrupt flag before return
;--------
INT_RB		btfss	INTCON,RBIF	; Test if a port change interrupt occured
		goto	INT_EXIT	; nope, Int_Exit	

		btfss	DCF77		; 
		goto	l_dcf_on	;
		
		btfss	StandBy		;
		bcf	IndexLed	;
		goto	l_dcf_done
		
l_dcf_on	bsf	IndexLed
l_dcf_done	bcf	INTCON,RBIF	; clear RB port change interrupt flag
;--------
INT_EXIT		
		pop			; restore conext for main program
					; this will also restore bank selection
		retfie                  ; return from interrupt


;**********************************************************************

ProcessRC5
		movf	RC5_Addr,w	;		
		xorlw	RemoteAddr	; test if RC5_Addr = RemoteAddr
		btfss	STATUS,Z	; 
		goto	ProcessRC5Done	;

		movf	RC5_Cmd,w	;
		andlw	0x7F		;	
		
		select_w
;		case	NUM_1		;
;		  decf	PWM_D_Pre,f	; Num 1
;		case	NUM_2		;
;		  incf	PWM_D_Pre,f	; Num 2
;		case	NUM_3		;
;		  decf	PWM_T_Pre,f	; Num 3
;		case	NUM_4		;
;		  incf	PWM_T_Pre,f	; Num 4
;		case	NUM_5		;
;		  decf	CCPR1L,f	; Num 5
;		case	NUM_6		;
;		  incf	CCPR1L,f	; Num 6
;		case	NUM_7		;
;		  goto	DecFreq		; Num 7
;		case	NUM_8		;
;		  goto	IncFreq		; Num 8		
		case	ADJUST		;
		  goto	PowerUpCoil	; ADJUST
		
		; Toggle functions
		movf	RC5_Cmd,w	;
		xorwf	RC5_Cmd2,f	;
		btfsc	STATUS,Z	;
		goto	ProcessRC5Done	;
		andlw	0x7F		;	
		
		select_w
		case	NUM_9	 	;
		  goto	ToggleIndex	; NUM 9
		case	STANDBY		;
		  goto	ToggleStndBy	; StandBy
		goto	ProcessRC5Done	;
		
ToggleIndex	movlw	1 << bIndexLed	;
		xorwf	PORTB,f		;
		goto	ProcessRC5Done	;
		
IncFreq		Bank1	
		decf	PR2,f		
		Bank0
		goto	ProcessRC5Done	;
		
DecFreq		Bank1	
		incf	PR2,f
		Bank0
		goto	ProcessRC5Done	;

ToggleStndBy	btfsc	StandBy		;
		goto	NotStandby	;
	
		movlw	.10		;
		movwf	SleepTimer	; SleepTimer = 10 * 1/2 sec
		
		bsf	IndexLed	; turn off index led
		bsf	StandBy		; motor in standby
		bcf	Motor		; turn OFF motor FET
		bcf	MotorOFF	; shutdonw MIC 2940A
		
		goto	ProcessRC5Done	;
NotStandby	
		; high power drive to trafo coil
		Bank1
		movlw	PR2RUN		;
		movwf	PR2		; PR2 set for high power
		Bank0

		movlw	CCPR1LRUN	;
		movwf	CCPR1L		; CCPR1L = Run

		bcf	IndexLed	; turn on index led
		bcf	StandBy		; spin motor
		bcf	Sleeping	; stop sleeping 
		bsf	MotorOFF	; startup MIC 2940A

		goto	ProcessRC5Done	;
PowerUpCoil
		; high power drive to trafo coil
		Bank1
		movlw	PR2RUN		;
		movwf	PR2		; PR2 set for high power
		Bank0

		movlw	CCPR1LRUN	;
		movwf	CCPR1L		; CCPR1L = Run
		

;		goto	ProcessRC5Done	;
	
ProcessRC5Done		
		movf	RC5_Cmd,w	;
		movwf	RC5_Cmd2	; 
		bcf	RC5_DataReady	;
		return			;


;**********************************************************************

InitPORRam

		movlw	0x3C		; load counter SubSecond = 0x10000 - 2500 = 0xF63C
		movwf	SubSec_L	;
		movlw	0xF6		;
		movwf	SubSec_H	;
		retlw	0
		
InitRam
		clrf	RC5_flags	; clear flags
		clrf	RC5_Tmr		;
		bsf	RC5_WaitStart	; RC5 is waiting for startbit

		movlw	0x55		;
		movwf	PWM_D_Pre	;
		movwf	PWM_Duty	;

		movlw	0x70		;
		movwf	PWM_T_Pre	;
		movwf	PWM_Tmr		;

		retlw	0
		
InitIO
		clrf	PORTA		; all outputs OFF
		clrf	PORTB

		Bank1
		movlw	0xFF		;
		movwf	TRISA		; set 0..7 as input
 		movlw   0xE1		;
 		movwf	TRISB		; set 1..4 as output, rest as input 
		Bank0
		
		movlw	0x07		;
		movwf	CMCON		; comparators OFF

		retlw	0
;--------		
InitTmr 
		clrf	INTCON		; clear and dissable all possible interrupt flag
		clrf	PIR1		; 

		clrf	T2CON		; TMR2=OFF

		Bank1
		clrf	PIE1		; dissable all possible interrupt flag
		movlw	b'00000001'	; set up timer. prescaler(bit3)bypassed
		movwf	OPTION_REG	; send w to option.
		Bank0

		bsf	INTCON,T0IE	; TMR0 interrupt enable
		bsf	INTCON,RBIE	; RB Port Change interrupt enable
		bsf	INTCON,GIE	; enable global interrupts

		clrf	TMR0		; start timer
		retlw	0

InitPWM		Bank1			;
		movlw	PR2Sleep	; set frequentie for sleep mode
		movwf	PR2		;
		Bank0			;

		movlw	CCPR1LSleep	; Set dutycycle for sleep mode
		movwf	CCPR1L		;
		
		movlw	b'00000101'	;
		movwf	T2CON		; Timer2=ON, Prescaler = 1/4
		
		movlw	0x0C		; Set CCP in PWM mode
		movwf	CCP1CON		;

		retlw 	0


;**********************************************************************

main

		call InitIO		; Initialise Ports
		call InitPORRam		; Init Powerup Reset Ram
		call InitRam		; Init Brownout Reset Ram
		call InitTmr		; Initialise Timers and interrupt
		call InitPWM		; start the PWM output to drive primary coil

		bsf	StandBy		; startup in SLEEP mode
		bsf	Sleeping	;
		bcf	Motor		; turn OFF motor FET
		bsf	IndexLed	; Turn OFF index LED	
		bcf	MotorOFF	; Shutdown MIC2940A 

MainLoop
			
		btfsc	RC5_DataReady	;
		call	ProcessRC5	;
		
		btfss	StandBy		; in standby?
		goto	MainLoop	; no, continue main loop
		btfsc	Sleeping	; yes, in standby: already sleeping?
		goto	MainLoop	; yes, sleeping, continue main loop
					
		movf	SleepTimer,w	;
		btfss	STATUS, Z	; SleepTimer = 0
		goto	MainLoop	; no, continue main loop
		
		; lower power drive to trafo coil
		Bank1
		movlw	PR2Sleep	;
		movwf	PR2		; PR2 set for low power
		Bank0

		movlw	CCPR1LSleep	;
		movwf	CCPR1L		; CCPR1L = Sleep
		
		bsf	Sleeping	;
		goto	MainLoop	;

		END                     ; directive 'end of program'
