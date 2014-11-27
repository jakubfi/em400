; OPTS -c  configs/mod-noclock.cfg

; CRON is not only illegal, it's illegal in user mode too (and does nothing)

	.cpu	mx16

	.const	ALIGN_INTV	0x40
	.const	ALIGN_EXLP	0x60
	.const	ALIGN_STP	0x61
	.const	ALIGN_CODE	0x70

	uj	start

; ---- RUNTIME CONF AND STUFF --------------------------------------------

	.const	USER_BLOCK	1
	.const	INT_MASK	0b1111100000000000
	.const	Q		0b0000000000100000

sys_mb:
	.word	0
user_sr:
	.word	INT_MASK + USER_BLOCK

; ---- STACK -------------------------------------------------------------

stack:	.res	12*4
	.res	ALIGN_INTV - .

; ---- INT VECTORS -------------------------------------------------------

intv:	.res	6, e
	.word	int6
	.res	25, e
exlp:	.word	e
stp:	.word	stack
	.res	ALIGN_CODE - .

; ---- INT HANDLERS ------------------------------------------------------

e:	hlt	040

int6:	; illegal instruction
	sint				; try to generate int11
	ki	ints
	lw	r1, [ints]
	nr	r1, 0b0000000000010000	; should be no int11 reported
	bn	r0, ?Z
	hlt	077
	hlt	044
ints:	.res	1

; ---- CODE --------------------------------------------------------------

start:
	lw	r1, 0\3 + 1\15
	ou 	r1, 3\10 + 0\14 + 1
	.word	e, e, block1, e
block1:
	lw	r1, 1\3 + 1\15
	ou 	r1, 4\10 + 0\14 + 1
	.word 	e, e, go, e

go:	
	mb	user_sr
	im	user_sr

	; copy user prog

	lw	r1, [user_prog]
	pw	r1, 0

	; start user process

	mb	sys_mb
	sp	userv
	hlt	043

	; IC, R0, SR, expected_read
userv:	.word 0, 0, INT_MASK + Q + USER_BLOCK, 0x0044

user_prog:
	cron

user_prog_end:

; XPCT oct(ir[10-15]) : 077
