; CONFIG configs/mod.cfg

	.cpu mx16

	.equ inttable 0x40
	.equ int_cpu2 inttable + 3
	.equ int_mx inttable + 12 + 1
	.equ stackp 0x61
	.equ start 0x70

	uj start

mask:	.word 0b1111110000000000
zmask:	.word 0
raise:	.word 1\3
	.org inttable
	.res 32, empty

.org start
	lw r1, stack
	rw r1, stackp
	lw r1, cpu2x
	rw r1, int_cpu2
	lw r1, mx
	rw r1, int_mx

	lw r7, 0x200
	im mask
	hlt 0
	nop
	im zmask
	mcl
	cron
	lw r7, 0x201
	im mask
	hlt 0
	nop

	hlt 077

empty:	lip

cpu2x:	md [stackp]
	lw r1, [-2]
	rw r1, r7
	lip

mx:	fi raise
	md [stackp]
	ib -4
	lip

stack:	.res 4*16

; XPCT bin([0x200]) : 0b1111100000000000
; XPCT bin([0x201]) : 0b1111000000000000
