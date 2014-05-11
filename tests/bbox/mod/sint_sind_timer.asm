; CONFIG configs/mod.cfg

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
set1:	.word 0x100, 0x101, 0x102
set2:	.word 0x200, 0x201, 0x202

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

	lf set1
	im mask
	hlt 0
	sint
	sind
	im zmask

	cron

	lf set2
	im mask
	hlt 0
	hlt 0
	hlt 0
	sint
	sind
	im zmask

	hlt 077

; XPCT int([0x100]) : 1
; XPCT int([0x101]) : 2
; XPCT int([0x102]) : 0

; XPCT int([0x202]) : 3
; XPCT int([0x201]) : 0
; XPCT int([0x200]) : 2
