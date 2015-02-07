; OPTS -c configs/mod.cfg

; Vanilla CPU masks all interrupts below and including current one.
; Modified (MX-16) CPU handles interrupt masking differently:
; any channel interrupt causes masking interrupts 5-31.

	.cpu	mx16

	.equ	inttable 0x40
	.equ	int_cpu2 inttable + 3
	.equ	int_mx inttable + 12 + 1
	.equ	stackp 0x61
	.equ	start 0x70

	UJ	start

mask:	.word	0b1111110000000000
zmask:	.word	0
raise:	.word	1\3

	.org	inttable
	.res	32, empty

	.org	start

	LW	r1, stack
	RW	r1, stackp
	LW	r1, cpu2x
	RW	r1, int_cpu2
	LW	r1, mx
	RW	r1, int_mx
	LWT	r6, 0

; first, check int masking for vanilla CPU (store mask at [0x200])

	LW	r7, 0x200
	IM	mask
w1:	HLT
	BB	r6, ?X	; did we wake up because of MULTIX interrupt?
	UJS	w1

	ER	r6, ?X
	IM	zmask
	MCL

; then, repeat for MX-16 CPU (store mask at [0x201])

	CRON
	AWT	r7, 1
	IM	mask
w2:	HLT
	BB	r6, ?X	; did we wake up because of MULTIX interrupt?
	UJS	w2
	HLT	077

; we use simulated 2nd CPU interrupt to get interrupt mask
; that was set by previous interrupt
cpu2x:	MD	[stackp]
	LW	r1, [-2]
	RW	r1, r7	; store previous int mask
	LIP

; we use multix as a source of channel interrupt
; (as it always sends interrupt after initialization)
mx:	FI	raise	; raise 2nd CPU interrupt
	OR	r6, ?X	; indicate that we've received MULTIX interrupt
; all other interrupts do nothing
empty:	LIP
stack:

; XPCT bin([0x200]) : 0b1111100000000000
; XPCT bin([0x201]) : 0b1111000000000000
