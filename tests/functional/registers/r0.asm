; in user mode, only LPC may change upper r0 byte

	.include hw.inc
	.include io.inc

	.const	user_block 1
	.const	user_prog_dest 0
	.const	user_r0_init 0x2185
	.const	user_r0_test 0x12b1

	uj	start

nomem_handler:
	hlt	040

exl_handler:
	hlt	077

sys_sr:	.word	IMASK_NOMEM | 0
user_sr:.word	IMASK_NOMEM | user_block

	.org	OS_MEM_BEG
start:
	lwt	r1, 0\3 | user_block\15
	ou	r1, 1\14 | MEM_CFG
	.word	err, err, setup, err
err:
	hlt	040
setup:
	lwt	r1, nomem_handler
	rw	r1, IV_NOMEM
	lwt	r1, exl_handler
	rw	r1, EXLP
	lw	r1, stack
	rw	r1, STACKP

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
	mb	sys_sr
	sp	user_vec
	hlt	040

user_vec:
	.word	user_prog_dest, user_r0_init, 0b0100000000100001

user_prog:
	lw	r0, user_r0_test
	rpc	r1
	lw	r3, user_r0_test
	lpc	r3
	rpc	r2
	exl	0
	hlt	040
	.const	user_prog_end .

stack:

; XPCT rz[6] : 0
; XPCT SR : 0b0100000000000001

; XPCT r1 : 0x21b1
; XPCT r2 : 0x12b1
; XPCT ir : 0xec3f

