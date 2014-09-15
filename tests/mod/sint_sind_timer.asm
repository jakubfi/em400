; CONFIG configs/mod.cfg

; Modified (MX-16) CPU has two additional instructions,
; which generate new software interrupt 5: SINT and SIND.
; Also, it moves timer interrupt from position 5 to 11

	.cpu mx16

	.equ int 0x40
	.equ int_timer int+5
	.equ int_illegal int+6
	.equ int_extra int+11
	.equ stackp 0x61
	.equ start 0x70

	uj start

stack:	.res 4
mask:	.word 1\4
zmask:	.word 0
set1:	.word 0x100, 0x101, 0x102	; interrupt counters for vanilla CPU (timer, illegal, extra)
set2:	.word 0x200, 0x201, 0x202	; interrupt counters for MC-16 CPU (timer, illegal, extra)

timerx:	ib r1
	lip

illx:	ib r2
	lip

extrax:	ib r3
	lip

.org start
	lw r1, timerx
	rw r1, int_timer
	lw r1, extrax
	rw r1, int_extra
	lw r1, illx
	rw r1, int_illegal

; first, check for vanilla CPU
	lf set1
	im mask
	hlt 0		; wait for timer interrupt (int 5)
	sint		; timer interrupt (int 5)
	sind		; timer interrupt (int 5)

	im zmask

; then, repeat for CPU with enabled modifications
	cron		; illegal instruction (int 6)
	lf set2
	im mask
	hlt 0		; wait for timer interrupt (int 11)
	hlt 0		; wait for timer interrupt (int 11)
	hlt 0		; wait for timer interrupt (int 11)
	sint		; software interrupt (int 5)
	sind		; software interrupt (int 5)

	im zmask

	hlt 077

; XPCT int([0x100]) : 3
; XPCT int([0x101]) : 0
; XPCT int([0x102]) : 0

; XPCT int([0x202]) : 3
; XPCT int([0x201]) : 1
; XPCT int([0x200]) : 2
