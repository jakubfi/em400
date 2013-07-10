.prog "registers/r0"

; in user mode, only LPC may change upper r0 byte

	.equ aexl 0x60
	.equ astack 0x61
	.equ astart 0x70

	.equ user_block 1
	.equ user_prog_dest 0
	.equ user_r0_init 0x2185
	.equ user_r0_test 0x12b1

	uj start

exl_handler:
	hlt 077

sys_mb:
	.data 0
user_mb:
	.data user_block
stack:
	.res astart-stack

	.ic astart
start:
	lwt r1, user_block
	ou r1, 0b0000000000000011
	.data   err, err, setup, err
err:
	hlt 077
setup:
	lwt r1, exl_handler
	rw r1, aexl
	lwt r1, stack
	rw r1, astack

copy_user_prog:
	md -user_prog_end
	lw r1, user_prog
	lw r2, user_prog_end
	sw r2, user_prog
	mb user_mb
copy_loop:
	lw r3, [r1+user_prog_end]
	pw r3, r1+r2
	irb r1, copy_loop

run_user_prog:
	mb sys_mb
	sp user_vec
	hlt 077

user_vec:
	.data user_prog_dest, user_r0_init, 0b0110000000100001
user_prog:
	lw r0, user_r0_test
	rpc r1
	lw r3, user_r0_test
	lpc r3
	rpc r2
	exl 0
	hlt 077
	.equ user_prog_end S

.finprog

; XPCT int(rz[6]) : 0
; XPCT bin(SR) : 0b0110000000000001

; XPCT hex(r1) : 0x21b1
; XPCT hex(r2) : 0x12b1
; XPCT hex(IC) : 0x0003
