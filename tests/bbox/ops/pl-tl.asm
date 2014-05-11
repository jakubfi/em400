; PRE r5 = 50
; PRE r6 = 60
; PRE r7 = 70

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61

	lw r3, stack
	rw r3, stackp
	lw r3, nomem_proc
	rw r3, int_nomem

	lw r1, 0b0000000000000001
	ou r1, 0b0000000000000011
	.word   err, err, ok, err
ok:
	mb blk
	im blk

	pa 20

	lwt r5, 0
	lwt r6, 0
	lwt r7, 0

	ta 20

	hlt 077

err:	hlt 040
data:	.res 7
blk:	.word 0b0100000000000001

nomem_proc:
	hlt 040
stack:

; XPCT int(rz[6]) : 0
; XPCT bin(sr) : 0b0100000000000001

; XPCT int(r1): 1
; XPCT int(r5): 50
; XPCT int(r6): 60
; XPCT int(r7): 70
; XPCT oct(ir[10-15]) : 077
