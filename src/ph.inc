; ================================================
; $Id: ph.inc,v 1.1 2002/11/27 09:22:25 epr Exp $
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
; PC to controller protocol handler
;
; These routines assume the following:
;   - 4Mhz processor
; The following constants must be set before including
; this source file.
;   - _carry        Defined as STATUS, 0
; ================================================

; ================================================
; Protocol handler macro's
; ================================================

StateSwitch	macro state, handler
	movlw	state
        xorwf	MAIN_STATE, W
        btfsc	_zero
        goto	handler
        endm

OnCommand	macro cmdbyte, handler
        movlw	cmdbyte
        xorwf	RS232_DATA, W
        btfsc	_zero
        goto	handler
        endm

; ================================================
; The entrypoint of the protocol handler
; ================================================

PH_Handler:
        ; Switch upon the current state
        StateSwitch ST_BEGIN, PH_ST_BEGIN
        StateSwitch ST_SPEED1, PH_ST_SPEED1
        StateSwitch ST_SPEED2, PH_ST_SPEED2
        StateSwitch ST_SWITCHDATA1, PH_ST_SWITCHDATA1
        StateSwitch ST_SWITCHDATA2, PH_ST_SWITCHDATA2
        StateSwitch ST_DCCCMD1, PH_ST_DCCCMD1
        StateSwitch ST_DCCCMD2, PH_ST_DCCCMD2
        StateSwitch ST_DCCCMD3, PH_ST_DCCCMD3
        StateSwitch ST_DCCCMD4, PH_ST_DCCCMD4
        ; Unknown state
        LED3_Off
        goto	PH_Goto_ST_BEGIN

; Handle a received byte in state ST_BEGIN
PH_ST_BEGIN:
	; Save the first byte
        movfw	RS232_DATA
        movwf	MAIN_BYTE1
	clrf	MAIN_CTRL

        ; Is this a 0..15 speed byte?
        movlw	b'11110000'
        andwf	RS232_DATA, W	; RS232_DATA & 11110000 -> W
        btfsc	_zero
        goto	PH_SpeedByte1

        ; Is this a switch reset command .32?
        OnCommand .32, PH_SwitchReset

        ; Is this a 2-byte DCC command .128?
        OnCommand .128, PH_Dcc2ByteCommand

        ; Is this a 3-byte DCC command .129?
        OnCommand .129, PH_Dcc3ByteCommand

        ; Is this a 4-byte DCC command .130?
        OnCommand .130, PH_Dcc4ByteCommand

        ; Is this a Reset .136?
        OnCommand .136, PH_Reset

        ; Is this a Nop .137?
        OnCommand .137, PH_End

        ; Is this a switch data command .144?
        OnCommand .144, PH_SwitchData1

        ; Is this a Last Changed .190?
        OnCommand .190, PH_LastChanged

        ; Is this a Return Module command 192..255
        movfw	RS232_DATA
        andlw	b'11000000'
        xorlw	b'11000000'
        btfsc	_zero
        goto	PH_ReturnModule

        ; This is an unknown byte, goto state ST_BEGIN
        LED3_Off
PH_Goto_ST_BEGIN:
        movlw	ST_BEGIN
PH_SaveState:
        movwf	MAIN_STATE
        goto	PH_End

PH_SpeedByte1:
	; The first byte received is a speed byte (0..15)
        movlw	ST_SPEED1
        goto	PH_SaveState

; Handle a received byte in state ST_SPEED1
PH_ST_SPEED1:
	; The second byte is always the address
        movfw	RS232_DATA
        movwf	MAIN_BYTE2
        ; Goto the next state
        movlw	ST_SPEED2
        goto	PH_SaveState

; Handle a received byte in state ST_SPEED2
PH_ST_SPEED2:
	; Save, but forget about the third byte for now (it deals with functions)
        movfw	RS232_DATA
        movwf	MAIN_BYTE3
        ; Clear the DCC_CTRL, meaning 1 address byte and 1 data byte
        clrf	DCC_CTRL
        ; Convert first byte to data byte (see NMRA spec)
        movfw	MAIN_BYTE1
        iorlw	b'01000000'	; Add '01DU' pattern
        btfsc	MAIN_BYTE3, 0
        iorlw	b'00100000'	; Set direction bit
        btfsc	MAIN_BYTE3, 1
        iorlw	b'00010000'	; Set FL bit
        movwf	DCC_DATA1	; Store data byte
        ; Convert second byte to address byte
        movfw	MAIN_BYTE2
        movwf	DCC_ADDR
        ; Goto the begin state
        goto	PH_Goto_ST_BEGIN

PH_Reset:
	call	RS232_Write
        goto	Start

PH_SwitchData1:
	movlw	ST_SWITCHDATA1
        goto	PH_SaveState

PH_ST_SWITCHDATA1:
	movfw	RS232_DATA
        movwf	MAIN_BYTE1
        movlw	ST_SWITCHDATA2
        goto	PH_SaveState

PH_ST_SWITCHDATA2:
	; Set the M2_ADDR byte
        movfw	MAIN_BYTE1
        movwf	M2_ADDR
	; Set the M2_DATA byte
        movfw	RS232_DATA
        movwf	M2_DATA
	; Set the begin state
        goto	PH_Goto_ST_BEGIN

; Switch reset command (32)
PH_SwitchReset:
	clrf	M2_DATA
        goto	PH_End

; 2-byte DCC command
PH_Dcc2ByteCommand:
        movlw	ST_DCCCMD1
        goto	PH_SaveState

; 3-byte DCC command
PH_Dcc3ByteCommand:
	bsf	MAIN_CTRL, DCC_DATA2_BIT
        goto    PH_Dcc2ByteCommand

; 4-byte DCC command
PH_Dcc4ByteCommand:
	bsf	MAIN_CTRL, DCC_DATA3_BIT
        goto    PH_Dcc3ByteCommand

; Process the 1 byte (always the address byte)
PH_ST_DCCCMD1:
	movfw	RS232_DATA
        movwf	MAIN_BYTE1
        movlw	ST_DCCCMD2
        goto	PH_SaveState

; Process the second byte (always a data byte)
PH_ST_DCCCMD2:
	movfw	RS232_DATA
        btfss	MAIN_CTRL, DCC_DATA2_BIT
        goto	PH_DCCCMD2_END
        movwf	MAIN_BYTE2
        movlw	ST_DCCCMD3
        goto	PH_SaveState

; It was a 2-byte command, stop here
PH_DCCCMD2_END:
	movwf	DCC_DATA1	; RS232_DATA(in W) -> DCC_DATA1
        movfw	MAIN_BYTE1	; MAIN_BYTE1 -> DCC_ADDR
        movwf	DCC_ADDR
        movfw	MAIN_CTRL	; MAIN_CTRL -> DCC_CTRL
        movwf	DCC_CTRL
        goto	PH_Goto_ST_BEGIN

PH_ST_DCCCMD3:
	movfw	RS232_DATA
        btfss	MAIN_CTRL, DCC_DATA3_BIT
        goto	PH_DCCCMD3_END
        movwf	MAIN_BYTE3
        movlw	ST_DCCCMD4
        goto	PH_SaveState

; It was a 3-byte command, stop here
PH_DCCCMD3_END:
	movwf	DCC_DATA2	; RS232_DATA(in W) -> DCC_DATA2
        movfw	MAIN_BYTE2	; MAIN_BYTE2 -> DCC_DATA1(for now in W)
        goto	PH_DCCCMD2_END

; It was a 4-byte command, stop here
PH_ST_DCCCMD4:
	movfw	RS232_DATA
        movwf	DCC_DATA3	; RS232_DATA(in W) -> DCC_DATA3
        movfw	MAIN_BYTE3	; MAIN_BYTE3 -> DCC_DATA2(for now in W)
        goto	PH_DCCCMD3_END

PH_LastChanged:
	clrf	RS232_DATA
        LED2_Off
        movlw	FB_CHANGED0
        movwf	FSR
        clrf	PH_TMP
        bsf	_carry
        rlf	PH_TMP, F
PH_LastChangedLoop:
	incf	RS232_DATA, F
        movfw	PH_TMP
        andwf	INDF, W		; Is changed bit x set? (W == 0 means bit is not set)
        btfss	_zero
        goto	PH_FoundChanged	; We found a changed bit, goto the end
        ; Prepare to continue the loop
        rlf	PH_TMP, F
        btfss	_carry		; If bit comes "out of" the 8-bit PH_TMP, incr FSR and reset PH_TMP
        goto	PH_LastChangedLoop2
        incf	FSR, F		; Increment FSR
        rlf	PH_TMP,	F	; Move C bit back into first position of PH_TMP
PH_LastChangedLoop2:
	movlw	FB_MOD_COUNT	; Compare RS232_DATA with MOD_COUNT, if equal there was
	xorwf	RS232_DATA, W	;    no last changed bit
        btfss	_zero
	goto	PH_LastChangedLoop
        clrf	RS232_DATA	; Return '0' means no last changed modules
PH_LastChangedEnd:
	goto	PH_End

PH_FoundChanged:
	movfw	PH_TMP
        xorwf	INDF, F		; Clear the changed bit
        goto	PH_End

PH_ReturnModule:
	movlw	.192
        subwf	RS232_DATA, F	; RS232_DATA now contains the module number to return (0..)
        movfw	RS232_DATA
        movwf	FSR		; Store module number in FSR
        rrf	FSR, F		; FSR / 8 -> FSR
        rrf	FSR, F		;   ..
        rrf	FSR, W		;   .. and store it in W
        addlw	FB_CHANGED0
        movwf	FSR		; FSR now contains FB_CHANGEDx where x = module / 8
        movfw	RS232_DATA
        call	getmod8bit	; Get the correct bit
        xorlw	b'11111111'	; Negate the bit pattern (a sort of bcf)
        andwf	INDF, F		; Clear the changed bit
        movfw	RS232_DATA
        addlw	FB_MOD0
        movwf	FSR		; FSR now contains FB_MODx where x = module
        movfw	INDF		; Load the module value
        movwf	RS232_DATA	; Store it in RS232_DATA
        goto	PH_End

PH_End:
	return


