	.cpu mera400

	.include cpu.inc

	uj	start

	.org	OS_START

	.include prng.inc

start:
	lw	r1, 666
	lw	r2, 99
	lj	seed

	lw	r7, -10_000
loop:
	lj	urand
	irb	r7, loop
	ujs	loop
	hlt	077
