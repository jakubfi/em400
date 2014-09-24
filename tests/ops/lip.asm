
start:
	lw r1, stack
	rw r1, 97

	lip
ok:	hlt 077
	hlt 040

data:	.org	200
	.word	ok, 0xfafa, 0b1100000000000001, 0
stack:

; XPCT int(rz[6]) : 0

; new process vector

; XPCT bin(sr) : 0b1100000000000001
; XPCT hex(r0) : 0xfafa
; XPCT oct(ir[10-15]) : 077

; XPCT int([97]) : 200
