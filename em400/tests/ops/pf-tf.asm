; PRE r1 = 50
; PRE r2 = 60
; PRE r3 = 70

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61

	lw r4, stack
	rw r4, stackp
	lw r4, nomem_proc
	rw r4, int_nomem

	lw r4, 0b0000000000000001
	ou r4, 0b0000000000000011
	.word   err, err, ok, err
ok:
	mb blk
	im blk

	pf 20

	lwt r1, 0
	lwt r2, 0
	lwt r3, 0

	tf 20

	hlt 077

data:	.res 7
blk:	.word 0b0100000000000001

err:	hlt 040

nomem_proc:
	hlt 040
stack:

; XPCT int(rz[6]) : 0
; XPCT bin(sr) : 0b0100000000000001

; XPCT int(r1): 50
; XPCT int(r2): 60
; XPCT int(r3): 70
; XPCT oct(ir[10-15]) : 077
