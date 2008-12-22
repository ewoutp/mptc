; ================================================
; $Id: delay.inc,v 1.1 2002/11/27 09:22:25 epr Exp $
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
; Simple busy wait routines.
;
; These routines assume the following:
;   - 4Mhz processor
; The following constants must be set before including
; this source file.
;   -DELAY_TMP1, a temporary register, only used within these functions, can
;        be destroyed in between.
; ================================================

; ================================================
; A macro for calling the read handler is a data
; is about to be received.
; ================================================

Delay macro cycles
	movlw	(cycles - 6) / 3	; 1 cycle
        call Delay_Wait			; already counted in Delay_Wait
        if ((((cycles - 6) / 3) % 3) == 1)
        	nop
        else
	        if ((((cycles - 6) / 3) % 3) == 2)
                	nop
                        nop
         	endif
        endif
	endm

; Loop (W) times.
; The total number of cycles is: 5 + (DELAY_TMP1)*3
;   call Delay_Wait : 2 cycles
;   loop            : (DELAY_TMP1) * 3
;   return          : 2 cycles
Delay_Wait:
        movwf	DELAY_TMP1		; 1 cycle
	decfsz DELAY_TMP1, 1 ; 1 cycle
        goto $-1	; 2 cycles
	return		; 2 cycles

; ================================================
; A utility method that gets a number in W (mod 8) and
; returns the indicated bit in W
; E.g. 3 -> b'00001000'
;      1 -> b'00000010'
; ================================================

getmod8bit:
	andlw	b'00000111'	; mod 8
	addwf	PCL, F
        retlw	b'00000001'
        retlw	b'00000010'
        retlw	b'00000100'
        retlw	b'00001000'
        retlw	b'00010000'
        retlw	b'00100000'
        retlw	b'01000000'
        retlw	b'10000000'
