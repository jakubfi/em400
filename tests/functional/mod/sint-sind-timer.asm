; OPTS -c configs/mod.cfg
; PRECMD CLOCK ON

; Modified (MX-16) CPU has two additional instructions,
; which generate new software interrupt 5: SINT and SIND.
; Also, it moves timer interrupt from position 5 to 11

	.cpu	mx16

	.include cpu.inc

	uj	start

mask:	.word	IMASK_GROUP_H
zmask:	.word	0, 0, 0
set1:	.word	0x100, 0x101, 0x102	; interrupt counters for vanilla CPU (timer, illegal, extra)
set2:	.word	0x200, 0x201, 0x202	; interrupt counters for MC-16 CPU (timer, illegal, extra)

timerx:	ib	r1
	lip

illx:	ib	r2
	lip

extrax:	ib	r3
	lip

	.org	OS_START
start:
	lf	zmask
	rf	0x100
	rf	0x200
	lw	r1, timerx
	rw	r1, INTV_TIMER
	lw	r1, extrax
	rw	r1, INTV_UNUSED
	lw	r1, illx
	rw	r1, INTV_ILLEGAL

; first, check for vanilla CPU
	lf	set1
	im	mask
	hlt		; wait for timer interrupt (int 5)
	sint		; timer interrupt (int 5)
	sind		; timer interrupt (int 5)

	im	zmask

; then, repeat for CPU with enabled modifications
	cron		; illegal instruction (int 6)
	lf	set2
	im	mask
	hlt		; wait for timer interrupt (int 11)
	hlt		; wait for timer interrupt (int 11)
	hlt		; wait for timer interrupt (int 11)
	sint		; software interrupt (int 5)
	sind		; software interrupt (int 5)

	im	zmask

	hlt	077

stack:

; XPCT [0x100] >= 3 : 1
; XPCT [0x101] : 0
; XPCT [0x102] : 0

; XPCT [0x202] >= 3 : 1
; XPCT [0x201] : 1
; XPCT [0x200] : 2
