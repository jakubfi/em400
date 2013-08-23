.prog "mem/nomem-os"

; CONFIG configs/no_user_mem.cfg

; PRE sr = 0b0100000000000001

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61

	uj start

nomem_proc:
	hlt 077

start:
	lw r3, stack
	rw r3, stackp
	lw r3, nomem_proc
	rw r3, int_nomem

	pw r1, 10

	hlt 077

stack:
	.res 4

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(alarm) : 1
; XPCT int(IC) : 3
