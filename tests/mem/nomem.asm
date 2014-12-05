; OPTS -c configs/no_user_mem.cfg

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61

	uj start

mask:	.word 0b0100000000000001
nomem_proc:
	hlt 077

start:
	lw r3, stack
	rw r3, stackp
	lw r3, nomem_proc
	rw r3, int_nomem
	im mask
	mb mask

	pw r1, 10

	hlt 040

stack:
	.res 4

; XPCT int(rz[6]) : 0
; XPCT oct(ir[10-15]) : 077
