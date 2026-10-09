; PRECMD clock on

	.include cpu.inc

	lw	r1, stack
	rw	r1, STACKP
	lw	r1, proc
	rw	r1, INTV_TIMER

	; hlt should continue with the next instruction after the interrupt
	; hlt should not touch flags
	; hlt should not wake up on a masked interrupt (soft L is pending, GROUP_L masked)
	sil
	lwt	r0, -1
	im	mask
halt:	hlt
	; woke up with nothing to serve: masked soft L did it, as the timer is ms away
	; (a timer landing exactly at this wrong wake-up would hide it, but that's very unlikely)
	hlt	040

proc:	lw	r1, [stack]
	cw	r1, halt+1
	jes	fin
	; timer fired before hlt got executed, retry
	lip

fin:	hlt	077

mask:	.word	IMASK_GROUP_H

	.org	100
stack:

; XPCT [101] : 0xffff
; soft L stays masked, never served
; XPCT rz[31] : 1
