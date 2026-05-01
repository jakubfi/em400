; OPTS -c configs/meclo.ini
; PRECMD CLOCK ON

; MECLO-400 reports interrupts every 500ms after enabling

	.cpu	mera400

	.include cpu.inc
	.include io.inc

	mcl
	uj	start

	.const	CHAR_CHAN 7
	.const	MECLO_DEV 0
	.const	MECLO CHAR_CHAN\IO_CHAN | MECLO_DEV\IO_DEV
	.const	MECLO_PROM_READ 1\0
	.const	MECLO_HS_SET 0\1
	.const	MECLO_HS_CLEAR 1\1
	.const	MECLO_MS_SET 0\2
	.const	MECLO_MS_CLEAR 1\2
	.const	MECLO_DR_SET 1\3
	.const	MECLO_DR_CLEAR 0\3
	.const	MECLO_TR_SET 1\4
	.const	MECLO_TR_CLEAR 0\4
	.const	MECLO_INT_ENABLE 1\5
	.const	MECLO_INT_DISABLE 0\5
	.org	OS_START

imask:	.word	IMASK_GROUP_H | IMASK_CH4_9

; ------------------------------------------------------------------------
int_meclo:
	awt	r7, 1
	lip
; ------------------------------------------------------------------------
int_timer:
	awt	r6, 1
	cw	r6, 103	; 100 * 10ms (+3 ticks just in case)
	jes	.done
	lip
.done:
	hlt	077
	ujs	.done

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
; ------------------------------------------------------------------------

start:
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, int_meclo
	rw	r1, INTV_CH7
	lw	r1, int_timer
	rw	r1, INTV_TIMER

	lw	r6, 0
	lw	r7, 0
	im	imask

	; enable clock interrupts

	in	r3, MECLO_HS_CLEAR | MECLO_MS_CLEAR | MECLO_INT_ENABLE | MECLO
	.word	.no1, .en1, .ok1, .pe1
.no1:	hlt	041
.pe1:	hlt	042
.en1:	hlt	043
.ok1:
	hlt
	ujs	.ok1

stack:

; XPCT ir : 0xec3f
; XPCT alarm : 0
; XPCT r6 : 103
; XPCT r7 : 2
