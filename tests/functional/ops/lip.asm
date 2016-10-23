
start:
	lw r1, stack
	rw r1, 97

	lip
ok:	hlt 077
	hlt 040

data:	.org	200
	.word	ok, 0xfafa, 0b1100000000000001, 0
stack:

; XPCT rz[6] : 0

; new process vector

; XPCT sr : 0b1100000000000001
; XPCT r0 : 0xfafa
; XPCT ir&0x3f : 0o77

; XPCT [97] : 200
