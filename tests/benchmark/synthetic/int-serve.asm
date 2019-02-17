
	.include cpu.inc

	uj	start

softint_handler:
	lip
stack:	.res	4
mask:	.word	IMASK_GROUP_H

	.org	OS_START

start:	lwt	r1, stack
	rw	r1, STACKP
	lwt	r1, softint_handler
	rw	r1, INTV_SW_H
	lwt	r1, 1
	im	mask

loop:
	siu
	irb	r1, loop
	ujs	loop
	hlt	077
