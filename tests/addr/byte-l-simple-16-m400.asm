; CONFIG configs/mega_max.cfg

; test simple byte-size memory addressing across all possible
; (32k) address space on vanilla cpu

	.cpu	mera400

	.include addr-skeleton.inc
	.include addr-fill-7.inc

; ------------------------------------------------------------------------
; test simple byte addressing
run_test:
	.res	1
	lw	r6, test_start
	slz	r6

next_t:
	lwt	r7, 0
	lb	r7, r6

	lw	r2, r6
	srz	r2	; Y = 0/1 = l/r byte
	mw	mul
	bb	r0, ?Y
	shc	r2, 8
	nr	r2, 0x00ff

	cw	r2, r7
	bb	r0, ?E
	hlt	042

	; next word
	irb	r6, next_t

	uj	[run_test]

; ------------------------------------------------------------------------
; ---- MAIN --------------------------------------------------------------
; ------------------------------------------------------------------------
start:
	lj	mem_cfg
	im	mask
	lj	fill_7
	lj	run_test

	hlt	077

test_start:

; XPCT oct(ir[10-15]) : 077
