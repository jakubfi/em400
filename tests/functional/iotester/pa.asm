; OPTS -c configs/iotester.cfg

; Test if I/O channel properly sends Power Alarm interrupt

	.cpu	mera400

	.include cpu.inc

	uj	start

	.org	OS_START

	.include iotester.inc

mask_0:	.word	IMASK_NONE
mask_if:.word	IMASK_IFPOWER

; ------------------------------------------------
if_power:
	hlt	077

; ------------------------------------------------
start:
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, if_power
	rw	r1, INTV_IFPOWER
	im	mask_if

	lj	iotester_pa

	im	mask_0
	hlt	044
stack:

; XPCT ir : 0xec3f
