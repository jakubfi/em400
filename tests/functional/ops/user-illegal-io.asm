; OPTS -c configs/minimal-user-io-illegal.ini
; in user mode, I/O instructions are illegal if "user_io_illegal = true"

	.include cpu.inc
	.include io.inc

	.const USER_BLOCK 1
	.const USER_PROG_DEST 0
	.const USER_R0_INIT 0x2185

	uj	start

exl_handler:
	hlt	077

illegal_handler:
	awt	r7, 1
	lip

nomem_handler:
	hlt	040
ok:	hlt	041

sys_mb:
zerod:	.word	0
	.const	USER_IMASK IMASK_PARITY | IMASK_NOMEM | IMASK_CPU_H | IMASK_IFPOWER | IMASK_GROUP_H
user_sr:.word	USER_IMASK | USER_BLOCK\SR_NB

	.org OS_START
start:
	lwt	r7, 0
	lwt	r1, 0\MEM_PAGE | USER_BLOCK\MEM_SEGMENT
	ou	r1, 4\MEM_FRAME | 0\MEM_MODULE | MEM_CFG
	.word	err, err, setup, err
err:
	hlt	042
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
	hlt	043

user_vec:
	.word	USER_PROG_DEST, USER_R0_INIT, USER_IMASK | 1\SR_Q | 0\SR_BS | USER_BLOCK\SR_NB
user_prog:
	; one-word variants of I/O instructions illegal in userspace
	; for each one of these, additional 'illegal instruction'
	; interrupt is fired for each I/O return address
	lwt	r1, 0
	in	r6, r1		; 1
	.word	ok, ok, ok, ok	; 2-5
	lwt	r1, 0
	ou	r6, r1		; 6
	.word	ok, ok, ok, ok	; 7-10

	; two-word variants of I/O instructions illegal in userspace
	; for each one of these, additional 'illegal instruction'
	; interrupt is fired for the argument, and 4 additional
	; interrupts should be fired for the 'return addresses'
	in	r6, 0		; 11-12
	.word	ok, ok, ok, ok	; 13-16
	ou	r6, 0		; 17-18
	.word	ok, ok, ok, ok	; 19-22

	; leave to os
	exl	0
user_prog_end:
stack:	.res	32*4

; XPCT rz[6] : 0
; XPCT SR : 0b1111100000000001

; XPCT r7 : 22
; XPCT ir : 0xec3f
