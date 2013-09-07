.prog "op/IS"

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61

	lw r3, stack
	rw r3, stackp
	lw r3, nomem_proc
	rw r3, int_nomem

	lw r1, 0b0000000000000001
	ou r1, 0b0000000000000011
	.data   err, err, ok, err
ok:
	mb blk
	im blk

	lw r1, 0b1010101011111111
	pw r1, 30

	lw r1, 0b1010101010101010
	is r1, 30
	hlt 040
	lw r1, 0b1111101010101010
	is r1, 30
	hlt 077
	hlt 040

err:	hlt 040
blk:	.data 0b0100000000000001

nomem_proc:
	hlt 040
stack:

.finprog

; XPCT int(rz[6]) : 0
; XPCT bin(sr) : 0b0100000000000001

; XPCT oct(ir[10-15]) : 077
; XPCT bin([1:30]) : 0b1111101011111111
