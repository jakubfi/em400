
	.include hw.inc

	uj	start

softint_handler:
	lip
stack:	.res	4
mask:	.word	IMASK_SOFT

	.org	OS_MEM_BEG

start:	lwt	r1, stack
	rw	r1, STACKP
	lwt	r1, softint_handler
	rw	r1, IV_SW_HIGH
	lwt	r1, 1
	im	mask

loop:
	siu
	irb	r1, loop
	ujs	loop
	hlt	077
