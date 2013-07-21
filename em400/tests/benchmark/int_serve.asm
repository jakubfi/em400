.prog "benchmark/int_serve"

	.equ astack 0x61
	.equ asoft 94
	.equ astart 0x70


	uj start

softint_handler:
	lip
stack:	.res 4
mask:	.data 0b0000000001000000

	.ic astart
start:
	lwt r1, stack
	rw r1, astack
	lwt r1, softint_handler
	rw r1, asoft
	lwt r1, 1
	im mask

loop:
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	siu
	irb r1, loop
	hlt 077

.finprog
