; OPTS -c configs/mod-noclock.cfg

; 17-bit byte addressing works in user mode only when BS=0

	.cpu	mx16

	.include hw.inc
	.include io.inc

	uj	start

; ---- RUNTIME CONF AND STUFF --------------------------------------------

	.const	USER_BLOCK	1
	.const	INT_MASK	IMASK_PARITY | IMASK_NOMEM | IMASK_2CPU_HIGH | IMASK_IFPOWER | IMASK_CPU
	.const	Q		0b0000000000100000
	.const	BBS		0b0000000000010000

	.const	ADDR		0x200
	.const	MAGIC1		0x4455
	.const	MAGIC2		0xfeba

sys_mb:	.word	0
user_sr:.word	INT_MASK + USER_BLOCK

; ---- INT VECTORS -------------------------------------------------------
	.res	INTV - .
intv:	.res	6, e
	.word	x
	.res	25, e
exlp:	.word	exl_handler
stp:	.word	stack
	.res	OS_MEM_BEG - .

; ---- INT HANDLERS ------------------------------------------------------

e:	hlt	040
x:	lip

exl_handler:
	; pop stack
	lw	r7, stp
	awt	r7, -4
	rw	r7, stp

	; check result
	md	3
	cw	r5, [r1]
	bb	r0, ?E
	hlt	042

	; start next process
	awt	r1, 4
	uj	load

; ---- CODE --------------------------------------------------------------

start:
	lw	r1, 0\3 + 1\15
	ou 	r1, 3\10 + 0\14 + MEM_CFG
	.word	e, e, block1, e
block1:
	lw	r1, 1\3 + 1\15
	ou 	r1, 4\10 + 0\14 + MEM_CFG
	.word	e, e, block9, e
block9:
	lw	r1, 9\3 + 1\15
	ou 	r1, 5\10 + 0\14 + MEM_CFG
	.word	e, e, go, e

go:	
	cron
	mb	user_sr
	im	user_sr

copy_user_prog:
	md	-user_prog_end
	lw	r1, user_prog
	lw	r2, user_prog_end
	sw	r2, user_prog
copy_loop:
	lw	r3, [r1+user_prog_end]
	pw	r3, r1+r2
	irb	r1, copy_loop

	lw	r1, MAGIC1
	pw	r1, 9\3 + ADDR
	lw	r1, MAGIC2
	pw	r1, (9\3 + ADDR) & 0x7fff

	; start user process

	lw	r1, [uservp]
load:
	cw	r1, uservp
	je	fin
	mb	sys_mb
	sp	r1
fin:
	hlt	077

	; IC, R0, SR, expected_read
userv1:	.word	0, 0, INT_MASK + Q + USER_BLOCK, 0x0044
userv2:	.word	0, 0, INT_MASK + Q + BBS + USER_BLOCK, 0x00fe
uservp:	.word	userv1

user_prog:
	lw	r7, 9\3 + ADDR
	lb	r5, r7+r7
	exl	0	; exit user process

stack:	.res	12*4

user_prog_end:

; XPCT ir : 0xec3f
