.prog "op/PD+TD"

; PRE r1 = 50
; PRE r2 = 60

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61

	lw r3, stack
	rw r3, stackp
	lw r3, nomem_proc
	rw r3, int_nomem

	lw r3, 0b0000000000000001
	ou r3, 0b0000000000000011
	.data   err, err, ok, err
ok:
	mb blk
	im blk

	pf 20

	lwt r1, 0
	lwt r2, 0

	tf 20

	hlt 077

err:	hlt 040
data:	.res 7
blk:	.data 0b0100000000000001

nomem_proc:
	hlt 040
stack:

.finprog

; XPCT int(rz[6]) : 0
; XPCT bin(sr) : 0b0100000000000001

; XPCT int(r1): 50
; XPCT int(r2): 60
; XPCT oct(ir[10-15]) : 077
