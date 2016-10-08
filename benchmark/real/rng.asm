	.include hw.inc
	UJ	start

	.org	OS_MEM_BEG

	.include prng.inc


start:
	LW	r1, 666
	LW	r2, 99
	RD	seed

	LW	r7, -10000
loop:
	LJ	rand
	IRB	r7, loop
	HLT	077
