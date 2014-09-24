; CONFIG configs/mega_max.cfg

; test various word addressing methods across all address space

	.cpu	mera400

	.include addr-skeleton.inc
	.include addr-fill-lin.inc

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
	tw	r7, [r1]
	cl	r7, r1
	bb	r0, ?E
	hlt	042
	; TEST: N = [rC + rB]
	tw	r7, [r2+r3]
	cl	r7, r1
	bb	r0, ?E
	hlt	043
	; TEST: N = [M + rB]
	tw	r7, [test_start+r3]
	cl	r7, r1
	bb	r0, ?E
	hlt	044
	; TEST: N = [premod + rC]
	md	test_start
	tw	r7, [r3]
	cl	r7, r1
	bb	r0, ?E
	hlt	045
	; TEST: N = [premod + rC + rB]
	md	test_start-4096
	tw	r7, [r3+r4]
	cl	r7, r1
	bb	r0, ?E
	hlt	046
	; TEST: N = [premod + M + rB]
	md	test_start-4096
	tw	r7, [r3+4096]
	cl	r7, r1
	bb	r0, ?E
	hlt	047

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
