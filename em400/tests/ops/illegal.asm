.prog "ops/illegal"

; in user mode, some instructions are illegal

	.equ aexl 0x60
	.equ astack 0x61
	.equ int_nomem 0x40 + 2
	.equ aillegal 0x40 + 6
	.equ astart 0x70

	.equ user_block 1
	.equ user_prog_dest 0
	.equ user_r0_init 0x2185
	.equ user_r0_test 0x12b1

	uj start

exl_handler:
	hlt 077

illegal_handler:
	awt r7, 1
	lip

nomem_handler:
	hlt 040

sys_mb:
zerod:
	.data 0
user_sr:
	.data 0b1111100000000000 + user_block
stack:
	.res astart-stack

	.ic astart
start:
	lwt r1, user_block
	ou r1, 0b0000000000000011
	.data   err, err, setup, err
err:
	hlt 040
setup:
	lwt r1, exl_handler
	rw r1, aexl
	lwt r1, stack
	rw r1, astack
	lwt r1, illegal_handler
	rw r1, aillegal
	lwt r1, nomem_handler
	rw r1, int_nomem

	mb user_sr
	im user_sr

copy_user_prog:
	md -user_prog_end
	lw r1, user_prog
	lw r2, user_prog_end
	sw r2, user_prog
copy_loop:
	lw r3, [r1+user_prog_end]
	pw r3, r1+r2
	irb r1, copy_loop

run_user_prog:
	mb sys_mb
	sp user_vec
	hlt 040

user_vec:
	.data user_prog_dest, user_r0_init, 0b1111100000100001
user_prog:
	hlt 040
	cit
	sit
	siu
	sil
	gil
	giu
	mcl
	lw r1, user_sr
	mb r1
	lw r1, user_sr
	im r1
	lw r1, zerod
	ki r1
	lw r1, zerod
	fi r1
	lip
	lw r1, zerod
	sp r1
	exl 0
	hlt 040
	.equ user_prog_end S

.finprog

; XPCT int(rz[6]) : 0
; XPCT bin(SR) : 0b1111100000000001

; XPCT int(r7) : 14
; XPCT oct(ir[10-15]) : 077
