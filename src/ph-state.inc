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
; States
; ================================================

#define ST_BEGIN	1	; A normal loop, no bytes have been received
#define ST_SPEED1	2	; The first byte of a speed command has been received
#define ST_SPEED2	3	; The first & second bytes of a speed command has been received

#define ST_SWITCHDATA1	4	; The first byte of a switch data command has been received
#define ST_SWITCHDATA2	5	; The first & second byte of a switch data command has been received

#define ST_DCCCMD1	6	; The first byte of a dcc command (2 or 3 byte) has been received
#define ST_DCCCMD2	7	; The first & second byte of a dcc command (2 or 3 byte) has been received
#define ST_DCCCMD3	8	; The first, second & third byte of a dcc command (2 or 3 byte) has been received
#define ST_DCCCMD4	9	; The first, second, third & fourth byte of a dcc command (2 or 3 byte) has been received

