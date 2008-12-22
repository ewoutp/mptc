; ================================================
; $Id: feedback.inc,v 1.1 2002/11/27 09:22:25 epr Exp $
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

; ================================================
;
; Feedback protocol routines
;
; These routines assume the following:
;   - 4Mhz processor
;   - delay.inc is included before this file.
; The following constants must be set before including
; this source file.
;   - _carry        Defined as STATUS, 0
;   - FBCLK_PORT    The output port where the feedback CLK line is connected.
;   - FBCLK_BIT     The output bit where the feedback CLK line is connected.
;   - FBDATA_PORT   The output port where the feedback DATA line is connected.
;   - FBDATA_BIT    The output bit where the feedback DATA line is connected.
;   - FBPS_PORT     The output port where the feedback P/S line is connected.
;   - FBPS_BIT      The output bit where the feedback P/S line is connected.
;   - FB_CNT2       A temporary register
;   - FB_TMP        A temporary register
;   - FB_MOD0       First feedback data register (FB_MOD0 .. FB_MOD0 + FB_MOD_COUNT - 1)
;                     must be reserved as feedback data registers
;   - FB_MOD_COUNT  The number of feedback modules to read (must be a multiple of 8)
;   - FB_CHANGED0   The first register that holds module changed bits (each bit is a module)
;                     (FB_CHANGED0 .. FB_CHANGED0 + (FB_MOD_COUNT / 8) - 1)
;                     must be reserved as feedback changed registers
; ================================================
	
#define FBPS_Par	bcf	FBPS_PORT, FBPS_BIT	; Set to parallel, FBPS is inverted by 74HCT04!
#define FBPS_Ser	bsf	FBPS_PORT, FBPS_BIT	; Set to parallel, FBPS is inverted by 74HCT04!

; ================================================
; Initialize the Feedback registers
; ================================================
FB_Init:
	FBPS_Par
        bsf	FBCLK_PORT, FBCLK_BIT
        ; Initialize all FB_MODx registers
        movlw	FB_MOD0
        movwf	FSR
        movlw	FB_MOD_COUNT
        movwf	FB_TMP
        movlw	0xff
FB_InitLoop1:
	movwf	INDF
        incf	FSR, F
        decfsz	FB_TMP, F
        goto	FB_InitLoop1
        ; Initialize all FB_CHANGEDx registers
        movlw	FB_CHANGED0
        movwf	FSR
        movlw	FB_MOD_COUNT / 8
        movwf	FB_TMP
        clrw
FB_InitLoop2:
	movwf	INDF
        incf	FSR, F
        decfsz	FB_TMP, F
        goto	FB_InitLoop2
        return

; ================================================
; Read the feedback module chain
; ================================================

FB_ReadModule macro module
	call	FB_Module
        movfw	FB_MOD0+module
        xorwf	FB_TMP, W	; Has the module been changed?
        btfss	_zero		; If not zero, set changed bit, otherwise skip
        bsf	FB_CHANGED, module
        btfsc	FB_CHANGED, module
        LED2_On
        movfw	FB_TMP
        movwf	FB_MOD0+module
	endm

FB_Read:
	call	FB_Pulse
        FBPS_Ser
        ; Read all eight feedback modules

        movlw	FB_MOD_COUNT
        movwf	FB_CNT
FB_ReadMod:
	call	FB_Module
        movlw	FB_MOD0 + FB_MOD_COUNT
        movwf	FSR
        movfw	FB_CNT
        subwf	FSR, F			; FSR now contains FB_MOD0 + (MOD_COUNT - CNT)
        movfw	INDF			; Read the current FB_MODx value
        xorwf	FB_TMP, W		; Has the module been changed?
        btfsc	_zero			; If zero (no change) then goto end
	goto	FB_ReadModEnd
        movfw	FB_TMP			; Store new value
        movwf	INDF			;      in FB_MODx
        movfw	FB_CNT			; Load FB_CNT -> W
        sublw	FB_MOD_COUNT		; Correct W from high to low counter to low to high counter
        movwf	FB_TMP			; Store FB_CNT in FB_TMP
        bcf	_carry			; FB_TMP / 8 -> FB_TMP
        rrf	FB_TMP, F		;   ..
        bcf	_carry
        rrf	FB_TMP, F		;   ..
        bcf	_carry
        rrf	FB_TMP, W		;   .. and load the result in W
        addlw	FB_CHANGED0
        movwf	FSR			; FSR now contains FB_CHANGEDx where x = FB_CNT / 8
        movfw	FB_CNT			; Load FB_CNT -> W
        sublw	FB_MOD_COUNT		; Correct W from high to low counter to low to high counter
        call	getmod8bit		; Get the bit-pattern for FB_CNT % 8 -> W
        movwf	FB_TMP			; Store W in FB_TMP
        movfw	INDF			; Load FB_CHANGEDx -> W
        iorwf	FB_TMP, W		; OR bit-pattern (W or bit-pattern -> W)
        movwf	INDF			; Store W -> FB_CHANGEDx
        LED2_On				; Signal the change
FB_ReadModEnd:
	decfsz	FB_CNT, F
        goto	FB_ReadMod

        FBPS_Par
	return

; Read a single 8-bit module into W
FB_Module:
	movlw	.8
        movwf	FB_CNT2
        clrf	FB_TMP
FB_Module1:
        ; Read the data bit
	bcf	_carry
        btfss	FBDATA_PORT, FBDATA_BIT	; DATA line is inverted by 74HCT04, but the modules return active low.
        bsf	_carry
        rrf	FB_TMP, F
        ; Shift all inputs by one
        call	FB_Pulse
        ; Next one?
        decfsz	FB_CNT2, F
        goto	FB_Module1
        return


FB_Pulse:
	; Create a CLK pulse
        nop	; Wait a while
        nop
        nop
        nop
        bcf	FBCLK_PORT, FBCLK_BIT	; CLK line is inverted by 74HCT04!
        nop	; Wait a while
        nop
        nop
        nop
        bsf	FBCLK_PORT, FBCLK_BIT	; CLK line is inverted by 74HCT04!
        return;
