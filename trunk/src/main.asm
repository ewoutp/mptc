; ================================================
; $Id: main.asm,v 1.1 2002/11/27 09:22:25 epr Exp $
;
; Multi Protocol Train Controller
; Copyright (C) 2002, Ewout Prangsma <ewout at prangsma.net>
;
; This library is free software; you can redistribute it and/or
; modify it under the terms of the GNU Lesser General Public
; License as published by the Free Software Foundation; either
; version 2.1 of the License, or (at your option) any later version.
;
; This library is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
; Lesser General Public License for more details.
;
; You should have received a copy of the GNU Lesser General Public
; License along with this library; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
; ================================================

	IFDEF __16C84
	include "p16c84.inc"
#define __16x84
	ENDIF
	IFDEF __16F84
	include "p16f84.inc"
#define __16x84
	ENDIF
	IFDEF __16F628
	include "p16f628.inc"
	ENDIF

; ================================================
; Defines
; ================================================

#define _carry	STATUS, C
#define _zero	STATUS, Z
#define TRISINC	(TRISA - PORTA)

	IFDEF __16x84
RegBase	equ 0x0c
Page0	MACRO
	ERRORLEVEL +302
	bcf	STATUS, RP0
	ENDM
Page1	MACRO
	bsf	STATUS, RP0
	ERRORLEVEL -302
	ENDM
	ENDIF
	IFDEF __16F628
RegBase	equ 0x20
Page0	MACRO
	ERRORLEVEL +302
	bcf	STATUS, RP0
        bcf	STATUS, RP1
	ENDM
Page1	MACRO
	bsf	STATUS, RP0
        bcf	STATUS, RP1
	ERRORLEVEL -302
	ENDM
Page2	MACRO
	bcf	STATUS, RP0
        bsf	STATUS, RP1
	ERRORLEVEL -302
	ENDM
Page3	MACRO
	bsf	STATUS, RP0
        bsf	STATUS, RP1
	ERRORLEVEL -302
	ENDM
	ENDIF

; Leds for debugging purposes
#define LED1_PORT	PORTB
#define LED1_BIT	3
#define LED2_PORT	PORTA
#define LED2_BIT	2
#define LED3_PORT	PORTA
#define LED3_BIT	1
; CTS towards RS232 on PC
#define CTS_PORT	PORTA
#define CTS_BIT		3
; Rail signal
#define RDATA_PORT	PORTA
#define RDATA_BIT	0
; RTS towards RS232 on PC
#define RTS_PORT	PORTA
#define RTS_BIT		4
; RxD towards RS232 on PC
#define RXD_PORT	PORTB
#define RXD_BIT		1
; TxD towards RS232 on PC
#define TXD_PORT	PORTB
#define TXD_BIT		2
; Feedback clock pulse
#define FBCLK_PORT	PORTB
#define FBCLK_BIT	5
; Feedback data input
#define FBDATA_PORT	PORTB
#define FBDATA_BIT	4
; Feedback reset pulse
#define FBPS_PORT	PORTB
#define FBPS_BIT	0

; ================================================
; Registers
; ================================================

	IFDEF __16C84
FB_MOD_COUNT		equ 8		; The number of feedback modules
	ENDIF
	IFDEF __16F84
FB_MOD_COUNT		equ 16		; The number of feedback modules
	ENDIF
	IFDEF __16F628
FB_MOD_COUNT		equ 32		; The number of feedback modules
	ENDIF

DELAY_TMP1		equ RegBase + 0x00
RS232_DATA		equ RegBase + 0x01
RS232_CNT		equ RegBase + 0x02
DCC_TXCOUNT		equ RS232_CNT
FB_CNT			equ RS232_CNT
M2_TXCOUNT		equ RS232_CNT
PH_CNT			equ RS232_CNT
DCC_ADDR		equ RegBase + 0x03
DCC_DATA1		equ RegBase + 0x04
DCC_DATA2		equ RegBase + 0x05
DCC_DATA3		equ RegBase + 0x06
DCC_CTRL		equ RegBase + 0x07
DCC_TXBYTE		equ RegBase + 0x08
M2_TXBYTE		equ DCC_TXBYTE
DCC_TXBYTE1		equ RegBase + 0x09
DCC_TXBYTE2		equ RegBase + 0x0A
DCC_TXBYTE3		equ RegBase + 0x0B
DCC_TXBYTE4		equ RegBase + 0x0C
DCC_TMP1		equ RegBase + 0x0D
MAIN_STATE		equ RegBase + 0x0E
MAIN_BYTE1		equ RegBase + 0x0F
MAIN_BYTE2		equ RegBase + 0x10
MAIN_BYTE3		equ RegBase + 0x11
MAIN_BYTE4		equ RegBase + 0x12
MAIN_BYTE5		equ RegBase + 0x13
MAIN_CTRL		equ RegBase + 0x14
M2_ADDR			equ RegBase + 0x15
M2_DATA			equ RegBase + 0x16
FB_TMP			equ RegBase + 0x17
PH_TMP			equ FB_TMP
FB_CNT2			equ RegBase + 0x18
FB_MOD0			equ RegBase + 0x19
FB_MOD_MAX		equ FB_MOD0 + (FB_MOD_COUNT - 1)
FB_CHANGED0		equ FB_MOD_MAX + 1
FB_CHANGED_MAX		equ FB_CHANGED0 + ((FB_MOD_COUNT / 8) - 1)
REG_MAX			equ FB_CHANGED_MAX	; Just a dummy label for compile time checks

#define DCC_DATA2_BIT	0
#define DCC_DATA3_BIT	1
#define DCC_LONGPR_BIT	2

        include "ph-state.inc"

; ================================================
; General macro's
; ================================================

#define LED1_On  bsf LED1_PORT, LED1_BIT
#define LED2_On  bsf LED2_PORT, LED2_BIT
#define LED3_On  bsf LED3_PORT, LED3_BIT

#define LED1_Off bcf LED1_PORT, LED1_BIT
#define LED2_Off bcf LED2_PORT, LED2_BIT
#define LED3_Off bcf LED3_PORT, LED3_BIT

; ================================================
; Startup
; ================================================

Start:
	clrf	REG_MAX		; Just a compile time check for maximum register usage
	movlw	ST_BEGIN
        movwf	MAIN_STATE
	call	InitPorts
        call	RS232_Init
        call	DCC_Reset
        call	M2_Init
        call	FB_Init

        goto	ML_Begin

; ================================================
; Subroutine includes
; ================================================

	include "delay.inc"
	include "rs232.inc"
        include "dcc.inc"
        include "m2.inc"
        include "feedback.inc"
        include "ph.inc"

; ================================================
; Main loop
; ================================================
ML_Begin:
	clrwdt
        RS232_JumpOnNoData ML_NoRS232	; Handle RS232 data if any
	call	RS232_Read		; Read the RS232 data
        call	PH_Handler		; Handle upon the data
        call	DCC_Packet		; Send a DCC packet
        call	RS232_Write		; Write the correct byte back
ML_NoRS232:
        call	DCC_Packet	        ; Send the DCC loc packet
        call	M2_Packet	        ; Send the Motorola II packet
        call	FB_Read		        ; Read the feedback unit
        call	DCC_Packet	        ; Send the DCC loc packet (again)
	goto	ML_Begin	        ; And loop again


; ================================================
; Sub routines
; ================================================

; Set the port into the initial state
InitPorts:
	Page1
        bcf	CTS_PORT + TRISINC, CTS_BIT	; Set CTS port to output
        bcf	RDATA_PORT + TRISINC, RDATA_BIT	; Set Rail DATA port to output
        bcf	TXD_PORT + TRISINC, TXD_BIT	; Set TXD port to output
        bcf	LED1_PORT + TRISINC, LED1_BIT	; Set LED1 port to output
        bcf	LED2_PORT + TRISINC, LED2_BIT	; Set LED2 port to output
        bcf	LED3_PORT + TRISINC, LED3_BIT	; Set LED3 port to output
        bcf	FBCLK_PORT + TRISINC, FBCLK_BIT	; Set feedback clock to output
        bcf	FBPS_PORT + TRISINC, FBPS_BIT	; Set feedback P/S to output
        Page0
        bsf	RDATA_PORT, RDATA_BIT		; Set +18V output on booster, when -18V the booster would switch on without a signal
        bsf	CTS_PORT, CTS_BIT		; De-activate CTS
        bsf	TXD_PORT, TXD_BIT
        bcf	LED1_PORT, LED1_BIT
        bcf	LED2_PORT, LED2_BIT
        bsf	LED3_PORT, LED3_BIT
        return

end

