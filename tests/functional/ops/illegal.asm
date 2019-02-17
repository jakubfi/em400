; in user mode, some instructions are illegal

	.include cpu.inc

	.equ user_block 1
	.equ user_prog_dest 0
	.equ user_r0_init 0x2185
	.equ user_r0_test 0x12b1

	uj	start

exl_handler:
	hlt	077

illegal_handler:
	awt	r7, 1
	lip

nomem_handler:
	hlt	040

sys_mb:
zerod:	.word	0
user_sr:.word	IMASK_PARITY | IMASK_NOMEM | IMASK_CPU_H | IMASK_IFPOWER | IMASK_GROUP_H | user_block

	.org OS_START
start:
	lwt	r7, 0
	lwt	r1, user_block
	ou	r1, 0b0000000000000011
	.word	err, err, setup, err
err:
	hlt	040
setup:
	lwt	r1, exl_handler
	rw	r1, EXLV
	lw	r1, stack
	rw	r1, STACKP
	lwt	r1, illegal_handler
	rw	r1, INTV_ILLEGAL
	lwt	r1, nomem_handler
	rw	r1, INTV_NOMEM

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

run_user_prog:
	mb	sys_mb
	sp	user_vec
	hlt	040

user_vec:
	.word	user_prog_dest, user_r0_init, 0b1111100000100001
user_prog:
	hlt	040
	cit
	sit
	siu
	sil
	gil
	giu
	mcl
	lw	r1, user_sr
	mb	r1
	lw	r1, user_sr
	im	r1
	lw	r1, zerod
	ki	r1
	lw	r1, zerod
	fi	r1
	lip
	lw	r1, zerod
	sp	r1
	in	r6, 0
	.word	ok, ok, ok, ok
	ou	r6, 0
	.word	ok, ok, ok, ok
	exl	0
ok:	hlt	040

stack:	.res	32*4
user_prog_end:

; XPCT rz[6] : 0
; XPCT SR : 0b1111100000000001

; XPCT r7 : 26
; XPCT ir : 0xec3f
