; ================================================
; $Id: dcc.inc,v 1.1 2002/11/27 09:22:25 epr Exp $
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
; DCC protocol routines
;
; These routines assume the following:
;   - 4Mhz processor
;   - delay.inc is included before this file.
; The following constants must be set before including
; this source file.
;   - _carry        Defined as STATUS, 0
;   - RDATA_PORT    The output port where the booster is connected.
;   - RDATA_BIT     The output bit where the booster is connected.
;   - DCC_TXBYTE    Temporary register
;   - DCC_TXCOUNT   Temporary register
;   - DCC_TXBYTE1   Temporary register
;   - DCC_TXBYTE2   Temporary register
;   - DCC_TXBYTE3   Temporary register
;   - DCC_TXBYTE4   Temporary register
;   - DCC_TMP1      Temporary register
; ================================================

; ================================================
; Send the DCC packet present in DCC_ADDR+DCC_DATA1 to the booster
; ================================================
DCC_Packet:
	; Is the address a service mode address?
        movfw	DCC_ADDR
        andlw	b'01110000'
        xorlw	b'01110000'
        btfss	_zero		; Is it a service mode address?
        goto	DCC_PacketSend	; No, it is a normal packet
        ; Yes it is, send it twice with resets
        call	DCC_Reset1	; DCC_Reset also clear DCC_ADDR, .. which we don't want here
        bsf	DCC_CTRL, DCC_LONGPR_BIT
	call	DCC_PacketSend
        call	DCC_PacketSend
        bcf	DCC_CTRL, DCC_LONGPR_BIT
        goto	DCC_Reset

DCC_PacketSend:
	clrf	DCC_TXBYTE3
	clrf	DCC_TXBYTE4
	call	DCC_Preamble
        movfw	DCC_ADDR
        movwf	DCC_TXBYTE
        movwf	DCC_TXBYTE1
        call	DCC_Byte
        call	DCC_Zero
        movfw	DCC_DATA1
        movwf	DCC_TXBYTE
        movwf	DCC_TXBYTE2
        call	DCC_Byte
        call	DCC_Zero
        ; Should we send data byte 2?
        btfss	DCC_CTRL, DCC_DATA2_BIT
        goto	DCC_ErrorByte	; DCC_ErrorByte will do the return
        ; Yes, send data byte 2
        movfw	DCC_DATA2
        movwf	DCC_TXBYTE
        movwf	DCC_TXBYTE3
        call	DCC_Byte
        call	DCC_Zero
        ; Should we send data byte 3?
        btfss	DCC_CTRL, DCC_DATA3_BIT
        goto	DCC_ErrorByte	; DCC_ErrorByte will do the return
        ; Yes, send data byte 3
        movfw	DCC_DATA3
        movwf	DCC_TXBYTE
        movwf	DCC_TXBYTE4
        call	DCC_Byte
        call	DCC_Zero
        ; We're done with the packet, only the errorbyte remains
        goto	DCC_ErrorByte	; DCC_ErrorByte will do the return

; ================================================
; Send a DCC idle packet to the booster
; ================================================
DCC_Idle:
	call	DCC_Preamble
	movlw	b'11111111'
        movwf	DCC_TXBYTE
        movwf	DCC_TXBYTE1
        call	DCC_Byte
        call	DCC_Zero
        clrf	DCC_TXBYTE
        clrf	DCC_TXBYTE2
        call	DCC_Byte
        call	DCC_Zero
        goto	DCC_ErrorByte	; DCC_ErrorByte will do the return

; ================================================
; Send a DCC reset packet to the booster (a couple of times)
; ================================================
DCC_Reset:
	clrf	DCC_TXBYTE3
	clrf	DCC_TXBYTE4
        clrf	DCC_DATA2
        clrf	DCC_DATA3
        clrf	DCC_CTRL
        movlw	.1
        movwf	DCC_ADDR
        movlw	b'01100000'
        movwf	DCC_DATA1

	movlw	.5
	movwf	DCC_TMP1
DCC_Reset1:
	call	DCC_Preamble
	clrf	DCC_TXBYTE
	clrf	DCC_TXBYTE1
	clrf	DCC_TXBYTE2
        call	DCC_Byte
        call	DCC_Zero
        clrf	DCC_TXBYTE
        call	DCC_Byte
        call	DCC_Zero
        call	DCC_ErrorByte
        decfsz	DCC_TMP1, F
        goto	DCC_Reset1
        return

; ================================================
; Send a preamble of 15 (10+) '1' bits + the '0' address start bit to the booster
; ================================================
DCC_Preamble:
	movlw	.15
        btfsc	DCC_CTRL, DCC_LONGPR_BIT	; Is the long preamble bit is set, do 25 '1' bits instead of 15.
        addlw	.10
        movwf	DCC_TXCOUNT
DCC_Preamble1:
	call	DCC_One
        decfsz	DCC_TXCOUNT, F
        goto	DCC_Preamble1
        goto	DCC_Zero	; DCC_Zero will do the return

; ================================================
; Send the error byte + packet end bit to the booster. The error byte
; it calculated from DCC_TXBYTE1-4
; ================================================
DCC_ErrorByte:
	movfw	DCC_TXBYTE1
        xorwf	DCC_TXBYTE2, W
        xorwf	DCC_TXBYTE3, W
        xorwf	DCC_TXBYTE4, W
        movwf	DCC_TXBYTE
        call 	DCC_Byte
        call	DCC_One			; Packet end bit
        clrf	DCC_TXBYTE3
        clrf	DCC_TXBYTE4

; ================================================
; Send 1 byte from DCC_TXBYTE to the booster
; ================================================
DCC_Byte:
	movlw	.8
        movwf	DCC_TXCOUNT
        bcf	_carry
DCC_Byte1:
	btfss	DCC_TXBYTE, 7
        goto	DCC_Byte2
        call	DCC_One
DCC_Byte3:
	rlf	DCC_TXBYTE, F
        decfsz	DCC_TXCOUNT, F
        goto	DCC_Byte1
        return
DCC_Byte2:
	call	DCC_Zero
        goto	DCC_Byte3

; ================================================
; Send a '0' bit
; ================================================
DCC_Zero:
	nop		; Useful for simulation-debugging
	Delay 0x64	; 100uS
        call DCC_toggle
        Delay 0x64	; 100uS
        goto DCC_toggle	; Toggle will do the return

; ================================================
; Send a '1' bit
; ================================================
DCC_One:
	nop		; Useful for simulation-debugging
	Delay 0x2e	; 58uS - 4 (call + toggle till xor)
        call DCC_toggle
        Delay 0x34	; 58uS - 4 (goto + toggle till xor)
        goto DCC_toggle	; Toggle will do the return

; ================================================
; Toggle the RDATA bit (this routines takes 4 cycles, without the call/goto to it)
; ================================================
DCC_toggle:
	movlw	(1<<RDATA_BIT)	; 1 cycle
        xorwf	RDATA_PORT, F	; 1 cycle
        return			; 2 cycles
