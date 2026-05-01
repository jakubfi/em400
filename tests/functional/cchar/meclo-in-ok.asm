; OPTS -c configs/meclo.ini

; MECLO-400 accepts all IN commands

	.cpu	mera400

	.include cpu.inc
	.include io.inc

	mcl
	uj	start

	.const	CHAR_CHAN 7
	.const	MECLO_DEV 0
	.const	MECLO CHAR_CHAN\IO_CHAN | MECLO_DEV\IO_DEV
	.org	OS_START

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
; ------------------------------------------------------------------------

start:
	lwt	r1, 0b001000
.next:
	lw	r4, r1
	shc	r4, 6
	in	r3, r4 + MECLO
	.word	.no1, .en1, .ok1, .pe1
.no1:	hlt	041
.pe1:	hlt	042
.en1:	hlt	043
.ok1:
	awt	r1, 1
	cw	r1, 0b1000000
	jl	.next
	hlt	077

; XPCT ir : 0xec3f
; XPCT alarm : 0
