; ================================================
; $Id: m2.inc,v 1.1 2002/11/27 09:22:25 epr Exp $
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
; Motorola II protocol routines
;
; These routines assume the following:
;   - 4Mhz processor
;   - delay.inc is included before this file.
; The following constants must be set before including
; this source file.
;   - _carry        Defined as STATUS, 0
;   - RDATA_PORT    The output port where the booster is connected.
;   - RDATA_BIT     The output bit where the booster is connected.
;   - M2_ADDR	    Address register
;   - M2_DATA       Data register
;   - M2_TXCOUNT    Temporary register
;   - M2_TXBYTE     Temporary register
; ================================================
; The low-level protocol summerized for solenoids (double frequency)
;
; - Each packet consists of 4 address trits, a '0' trit and 4 data trits.
; - Each packet is send twice before recognized as a valid packet.
; - Between the two identical packets there is a delay (t1) in which there is no data send.
; - After the each packet, there is a delay (t2) before a new packet can be send.
; - t1 == 3 * trit-time (== 3 * 208 us == 624us)
; - t2 == 2.5 ms
; ================================================

#define M2_LOW	bcf	RDATA_PORT, RDATA_BIT
#define M2_HIGH	bsf	RDATA_PORT, RDATA_BIT

; ================================================
; Initialize the M2 registers
; ================================================
M2_Init:
	movlw	0xff	; address 0
	movwf	M2_ADDR
        clrf	M2_DATA
        return

; ================================================
; Send the M2 packet in M2_ADDR, M2_DATA
; The timing of this method is crucial!!!
; To save time in the rest of the main loop, this methods only sends
; a packet if the address != 80. If the data == 0, the address is made 80,
; so during the next loop, no data will be send.
; ================================================
M2_Packet:
	; If ADDR == 80 Then Return
        movlw	b'01010101'
        xorwf	M2_ADDR, W
        btfsc	_zero
        return
	; Now we can start sending the packet (twice)
	Delay	0xff
        M2_LOW
        Delay	0xff
	call	M2_PacketLL
        nop
        Delay	.624
	call	M2_PacketLL
        nop
        Delay	0xff*3
        Delay	0xff*3
        Delay	.450
	; If DATA == 0 Then ADDR := 80
	movf	M2_DATA, F	; Just test the Z flag
        movlw	b'01010101'	; This does not change  any flags!
        btfsc	_zero
        movwf	M2_ADDR
        return

; The timing of this method is crucial!!!
M2_PacketLL:
	; Send an address bit
	movfw	M2_ADDR
        movwf	M2_TXBYTE
        call	M2_Byte
        ; Send a '0' trit
        nop
        nop
        call	M2_Zero
        ; Prepare for the data byte (done here due to timing)
	movfw	M2_DATA
        movwf	M2_TXBYTE
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        call	M2_Zero
        nop
        ; Send the data byte
        call	M2_Byte
        return

; ================================================
; Send 1 byte from M2_TXBYTE to the booster
; The timing of this method is crucial!!!
; ================================================
M2_Byte:
	movlw	.8
        movwf	M2_TXCOUNT
        bcf	_carry
M2_Byte1:
	btfss	M2_TXBYTE, 0	; 1/2 cycle
        goto	M2_Byte2	; 2 cycles
        nop			; 1 cycles (ensure an equal timing for '1' and '0')
        call	M2_One		; 2 cycles
M2_Byte3:
	rrf	M2_TXBYTE, F	; 1 cycle
        decfsz	M2_TXCOUNT, F	; 1 cycle
        goto	M2_Byte1	; 2 cycles
        return
M2_Byte2:
	call	M2_Zero		; 2 cycles
        goto	M2_Byte3	; 2 cycles

; ================================================
; Send a '0' bit
; RDATA is assumed to be '0' on entry
; The timing of this method is crucial!!!
; ================================================
M2_Zero:
	M2_HIGH
        Delay	.10
        nop
        nop
        nop
        M2_LOW
	Delay	.77		; 91-1-14 cycles needed for an the next '0' or '1'
        nop
        return

; ================================================
; Send a '1' bit
; RDATA is assumed to be '0' on entry
; The timing of this method is crucial!!!
; ================================================
M2_One:
	M2_HIGH
        Delay	.91-1
        M2_LOW
        nop			; Delay for 13-1-9 (Delay cannot copy with this)
        return

