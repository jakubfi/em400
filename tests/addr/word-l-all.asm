; CONFIG configs/mega_max.cfg

; test various word addressing methods across all address space

	.cpu	mera400

	.include addr-skeleton.asm
	.include addr-fill-lin.asm

; ------------------------------------------------------------------------
; test addressing modes
run_test:
	.res	1
	lw	r1, test_start
	lw	r2, r1
	lw	r3, 0
	lw	r4, 4096
next_t:

	; --- long loads ---

	; TEST: N = [rC]
	lw	r7, [r1]
	cl	r7, r1
	bb	r0, ?E
	hlt	042
	; TEST: N = [rC + rB]
	lw	r7, [r2+r3]
	cl	r7, r1
	bb	r0, ?E
	hlt	043
	; TEST: N = [M + rB]
	lw	r7, [test_start+r3]
	cl	r7, r1
	bb	r0, ?E
	hlt	044
	; TEST: N = [premod + rC]
	md	test_start
	lw	r7, [r3]
	cl	r7, r1
	bb	r0, ?E
	hlt	045
	; TEST: N = [premod + rC + rB]
	md	test_start-4096
	lw	r7, [r3+r4]
	cl	r7, r1
	bb	r0, ?E
	hlt	046
	; TEST: N = [premod + M + rB]
	md	test_start-4096
	lw	r7, [r3+4096]
	cl	r7, r1
	bb	r0, ?E
	hlt	047

	; --- short loads ---

	; TEST: T = [premod + IC]
	md	r1 - .
	lws	r7, -2
	cl	r7, r1
	bb	r0, ?E
	hlt	050
	; TEST: T = [premod + 0]
	md	r1
	lwt	r7, 0
	cl	r7, r1
	bb	r0, ?E
	hlt	051

	; next word
	awt	r3, 1
	irb	r1, next_t
	
	uj	[run_test]

; ------------------------------------------------------------------------
; ---- MAIN --------------------------------------------------------------
; ------------------------------------------------------------------------
start:
	lj	mem_cfg
	im	mask
	lj	fill_lin
	lj	run_test

	hlt	077

test_start:

; XPCT oct(ir[10-15]) : 077
