; ================================================
; $Id: rs232.inc,v 1.1 2002/11/27 09:22:25 epr Exp $
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
; Simple RS232 routines without using interrupts.
;
; These routines assume the following:
;   - Controlflow using RTS/CTS
;   - 1 Startbit, 8 databits, No parity, 1 Stopbit
;   - 9600 baud
;   - 4Mhz processor
;   - delay.inc is included before this file.
; The following constants must be set before including
; this source file.
;   - CTS_PORT, CTS_BIT
;   - RTS_PORT, RTS_BIT
;   - RXD_PORT, RXD_BIT
;   - TXD_PORT, TXD_BIT
;   - RS232_DATA, the register number where to store/retrieve the data byte
;   - RS232_TMP1, a temporary register, only used within these functions, can
;        be destroyed in between.
;   - RS232_CNT, a temporary bit-counter, only used within these functions, can
;        be destroyed in between.
; ================================================

	IFDEF __16F628
        	#define HW_SPI	1
 	ENDIF

#define CTS_On	bcf	CTS_PORT, CTS_BIT
#define CTS_Off	bsf	CTS_PORT, CTS_BIT

; ================================================
; A macro for jumping to a label when there is no data to read.
; ================================================

RS232_JumpOnNoData macro noDataHandler
	IFDEF HW_SPI
        	btfss	PIR1, RCIF	; Test for valid data
                goto	noDataHandler
 	ELSE
		; If RTS is set to "mark", there is no more data
		btfsc RTS_PORT, RTS_BIT
		goto noDataHandler
  	ENDIF
	endm

; ================================================
; RS232_Init
; ================================================
RS232_Init:
	LED1_On
        IFDEF HW_SPI
        	; 1) Set baud-rate
                Page1
                bsf	TXSTA, BRGH	; Set HIGH baudrates
                movlw	.103		; 38.4 Kb
                movwf	SPBRG
                ; 2a) Clear SYNC bit
                bcf	TXSTA, SYNC
                ; 5) Enable transmission
                bsf	TXSTA, TXEN
                ; Enable reception
                bsf	RCSTA, CREN
                Page0
                ; 2b) Enable serial port
                bsf	RCSTA, SPEN
        	; Activate CTS
                CTS_On
        ELSE
		; De-activate CTS
        	CTS_Off
		; Wait for an in-active RTS
		btfsc	RTS_PORT, RTS_BIT
        	goto $-1
  	ENDIF
        LED1_Off
        return

; ================================================
; Read a single byte of data
; ================================================

RS232_Read:
	LED1_On
        IFDEF HW_SPI
        	movfw	RCREG
                movwf	RS232_DATA
                Page1
                btfss	RCSTA, OERR	; Is overrun error set?
                goto	rs232_read_end
                bcf	RCSTA, CREN	; Reset the receive logic to clear the error
                nop
                bsf	RCSTA, CREN
rs232_read_end:
		Page0
        ELSE
		; Initialize the bit-counter
        	movlw	0x09
        	movwf	RS232_CNT
        	clrf	RS232_DATA
		; Set CTS to "space"
        	CTS_On
		; Loop until the start-bit (0) is detected.
		btfsc	RXD_PORT, RXD_BIT
        	goto	$-1
		; No wait half a bit, so we can read the data-bits in the middle of
        	; each bit.
        	Delay	0x68 		; 104 cycles
rs232_rbitcnt:
		decfsz	RS232_CNT, 1	; 1 cycle
		goto rs232_read_bit	; 2 cycles
        	; Now we have read 8 bits, wait for the stop-bit and then return
        	Delay 0x68 		; 104 (not acurate, doesn't mind anyway)
        	; Now set CTS back to "mark"
        	CTS_Off
        	; And wait for RTS to reset again
		btfss RTS_PORT, RTS_BIT
		goto $-1
  	ENDIF
	LED1_Off
        return

        IFNDEF HW_SPI
rs232_read_bit: ; read a single bit
        	bcf	_carry			; 1 cycle
        	btfsc	RXD_PORT, RXD_BIT	; 1 cycle, or 2 but the the next is skipped
        	bsf	_carry			; 1 cycle
        	rrf	RS232_DATA, 1		; 1 cycle
        	Delay 0x60 			; 104 - 6 - 3 + 1 (i'm not suyre why, but gpsim says so)
		goto rs232_rbitcnt		; 2 cycles
  	ENDIF

; ================================================
; Write a single byte of data
; ================================================

RS232_Write:
	IFDEF HW_SPI
        	; Wait for the TSR register to be empty
                Page1
                btfss	TXSTA, TRMT
                goto	$-1
                Page0
        	movfw	RS232_DATA
                movwf	TXREG
                return
 	ELSE
		; Initialize the bit-counter
        	movlw	0x09
        	movwf	RS232_CNT
        	; Send the start-bit
        	bcf	TXD_PORT, TXD_BIT
		; No wait one bit, so we can write the data-bits
        	Delay	0x61		; 104 - 7
rs232_wbitcnt:
		decfsz	RS232_CNT, 1	; 1 cycle
		goto rs232_write_bit	; 2 cycles
        	; Now we have written 8 bits, write the stop-bit and then return
        	bsf	TXD_PORT, TXD_BIT
        	Delay 	0x58		; 104 (not acurate, doesn't mind anyway)
        	return

rs232_write_bit: ; write a single bit
        	rrf	RS232_DATA, 1		; 1 cycle
        	btfss	_carry			; 1-2 cycle
          	goto	rs232_wlow		; 2 cycles
          	nop ; correct the timing	; 1 cycle
          	bsf	TXD_PORT, TXD_BIT	; 1 cycle
	  	Delay	0x5e			; 104 - 3 - 7
          	goto rs232_wbitcnt
rs232_wlow:
	          bcf	TXD_PORT, TXD_BIT	; 1 cycle
	  	Delay	0x5e			; 104 - 3 - 7
          	goto rs232_wbitcnt
	ENDIF
