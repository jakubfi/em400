; OPTS -c configs/mod_mega_max.cfg

; test full 17-bit byte addressing across all address space
; on modified (MX-16) cpu with enabled modifications

	.cpu	mx16

	.include addr-skeleton.inc
	.include addr-fill-7.inc

; ------------------------------------------------------------------------
; ---- MAIN --------------------------------------------------------------
; ------------------------------------------------------------------------
start:
	lj	mem_cfg
	im	mask
	lj	fill_7
	lj	run_test

	hlt	077

; ------------------------------------------------------------------------
; test simple byte addressing
run_test:
	.res	1
	lw	r6, test_start

next_t:
	lw	r5, r6
	nr	r5, 0b0111111111111111	; we really check only 32kwords due to lost 16th bit in byte addressing
	cl	r5, test_start
	jls	skip_t		; skip checking program body
	
	lw	r2, r5		; load original value
	mw	mul

	lw	r1, r2		; prepare original left byte
	shc	r1, 8
	nr	r1, 0x00ff

	lwt	r7, 0		; load left byte
	lb	r7, r6+r6

	cw	r1, r7		; check left byte
	bb	r0, ?E
	hlt	042

	lw	r1, r2		; prepare original right byte
	nr	r1, 0x00ff

	lwt	r7, 0		; load right byte
	md	1
	lb	r7, r6+r6

	cw	r1, r7		; check right byte
	bb	r0, ?E
	hlt	042

skip_t:
	; next word
	irb	r6, next_t

	uj	[run_test]

test_start:

; XPCT oct(ir[10-15]) : 077
