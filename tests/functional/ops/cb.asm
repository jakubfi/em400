
	.equ int_nomem 0x40 + 2
	.equ stackp 0x61

	lw r3, stack
	rw r3, stackp
	lw r3, nomem_proc
	rw r3, int_nomem

	lw r1, 0b0000000000000001
	ou r1, 0b0000000000000011
	.word   err, err, ok, err
ok:	mb blk
	im blk

	lw r1, 0b1111100000000001
	pw r1, 40

	lw r1, 0b0000000000000010
	cb r1, 80
	rpc r5

	lw r2, 0b0000000000000010
	cb r2, 81
	rpc r6

	lw r3, 0b0000000011111000
	cb r3, 80
	rpc r7

	lw r4, 0b0000000000000001
	cb r4, 81
exitok:
	hlt 077

err:	hlt 040
blk:	.word 0b0100000000000001

nomem_proc:
	hlt 040
stack:

; XPCT rz[6] : 0
; XPCT sr : 0b0100000000000001

; XPCT r5 : 0b0000100000000000
; XPCT r6 : 0b0000001000000000
; XPCT r7 : 0b0000010000000000
; XPCT r0 : 0b0000010000000000
; XPCT ir : 0xec3f
