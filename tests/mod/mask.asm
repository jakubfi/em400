; CONFIG configs/mod.cfg

; Vanilla CPU masks all interrupts below and including current one.
; Modified (MX-16) CPU handles interrupt masking differently:
; any channel interrupt causes masging interrupts  5-31.

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

; first, check int masking for vanilla CPU (store mask at [0x200])
	lw r7, 0x200
	im mask
	hlt 0
	nop

	im zmask
	mcl

; then, repeat for MX-16 CPU (store mask at [0x201])
	cron
	lw r7, 0x201
	im mask
	hlt 0
	nop

	hlt 077

; we use simulated 2nd CPU interrupt to get interrupt mask
; that was set by previous interrupt
cpu2x:	md [stackp]
	lw r1, [-2]
	rw r1, r7	; store previous int mask
	lip

; we use multix as a source of channel interrupt
; (as it always sends interrupt after initialization)
mx:	fi raise	; raise 2nd CPU interrupt
	md [stackp]
	ib -4		; in case interrupt is ready early at 'im' (i.e. before 'hlt')
	lip

; all other interrupts do nothing
empty:	lip

stack:	.res 4*16

; XPCT bin([0x200]) : 0b1111100000000000
; XPCT bin([0x201]) : 0b1111000000000000
