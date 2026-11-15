; BENCHMARK

; DW with a quotient in range. Each DW overwrites the dividend, so it is
; reloaded before every division.

	lw	r7, -10_000

loop:
	ld	a
	dw	n
	ld	a
	dw	n
	ld	a
	dw	n
	ld	a
	dw	n
	ld	a
	dw	n
	ld	a
	dw	n
	ld	a
	dw	n
	ld	a
	dw	n
	irb	r7, loop
	ujs	loop

	hlt	077

a:	.dword	1_000_000
n:	.word	1000
