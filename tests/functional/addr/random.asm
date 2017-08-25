; OPTS -c configs/mega_max.cfg
; PRECMD CLOCK ON

; Test addresing modes across available address space using random data.
; Assumes the memory is MEGA-like configurable (one frame can be mapped to many pages).
; Fills one frame with random data, then configures it for
; subsequent pages in all segments, reads data and checks for correctness.

; r6 - currently tested segment
; r7 - currently tested page

	.cpu	mera400

	.include hw.inc
	.include io.inc
	.include mega.inc

	.const	mega_cfg MEM_CFG | MEGA_ALLOC_FINISH | MEGA_HIDE_PAS | MEGA_ALLOC

	.const	mframe	0\10 + 2\14 + mega_cfg
	.const	tframe	1\10 + 2\14 + mega_cfg
	.const	dframe	2\10 + 2\14 + mega_cfg
	.const	cfgpage	15\3 + 0\15

	uj	start

	.org	OS_MEM_BEG

	.include prng.inc
mask_none:
	.word	IMASK_NONE
mask_mem:
	.word	I_PARITY | I_NOMEM
known_random:
	.res	2
mb_dframe:
	.res	1
mb_0:	.word	0
taddr:	.res	1

mcerr:	hlt	040
cmper1:	hlt	041
cmper2:	hlt	042
cmper3:	hlt	043
cmper4:	hlt	044
cmper5:	hlt	045
cmper6:	hlt	046
parity:	hlt	060
nomem:	hlt	061

; ------------------------------------------------------------------------
; ---- MAIN --------------------------------------------------------------
; ------------------------------------------------------------------------

start:
	lj	prngseed
	rd	known_random

	im	mask_none
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, parity
	rw	r1, IV_PARITY
	lw	r1, nomem
	rw	r1, IV_NOMEM

; fill the masking frame with a known value
	lw	r1, cfgpage
	ou	r1, mframe
	.word	mcerr, mcerr, onef, mcerr
onef:	lw	r2, 0b1010101001110111
	lw	r1, 0xf000
onefl:	rw	r2, r1
	irb	r1, onefl

; fill the test data frame with random numbers (but the seed is known)
; r4 - temp
; r5 - page (shifted)
; r3 - offset

	lw	r1, cfgpage
	ou	r1, dframe
	.word	mcerr, mcerr, dfill, mcerr
dfill:
	; preload seed
	ld	known_random
	lj	seed

	; loop over the whole page filling it with random numbers
	lwt	r3, 0
	lw	r5, 0xf000
floop:
	rd	r5+r3
	lj	rand
	awt	r3, 2
	cw	r3, 0x1000
	jl	floop

; unconfigure the page that we used for all the filling
	lw	r1, cfgpage
	ou	r1, tframe
	.word	mcerr, mcerr, go, mcerr

; read and compare segment by segment, page by page
go:

; for segment in 1..15
	lwt	r6, 1

next_segment:

; for page in 0..15
	lwt	r7, 0

next_page:

; configure all pages in the segment to the masking frame
; r2 - page
	lwt	r2, -1
nxtm0:	awt	r2, 1
	cwt	r2, 15
	jgs	m0done
	lw	r1, r2
	shc	r1, 4
	aw	r1, r6
	ou	r1, mframe
	.word	mcerr, mcerr, nxtm0, mcerr

m0done:

; configure the test page within the segment
	lw	r1, r7
	shc	r1, 4
	aw	r1, r6
	ou	r1, dframe
	.word	mcerr, mcerr, dcheck, mcerr

; for all addresing modes: read and compare
dcheck:
	; prepare and set the segment number
	lw	r4, r6
	rw	r4, mb_dframe
	mb	mb_dframe
	; prepare the page address
	lw	r5, r7
	shc	r5, 4
	; preload the known seed
	ld	known_random
	lj	seed

; loop over the whole page checking the contents
; r1 - expected value
; r4 - read value
; r5+r3 - current address:
;  r5 - current page
;  r3 - current offset within the page
	lwt	r3, 0

loop:	
	lw	r2, r5
	aw	r2, r3
	rw	r2, taddr ; for later use
	tw	r4, r2
	cw	r1, r4
	jn	cmper1

	tw	r4, r5+r3
	cw	r1, r4
	jn	cmper2

	md	r3
	tw	r4, r5
	cw	r1, r4
	jn	cmper3

	md	-17423
	md	r3
	tw	r4, r5+17423
	cw	r1, r4
	jn	cmper4

	tw	r4, [taddr]
	cw	r1, r4
	jn	cmper5

	lw	r2, -27155
	tw	r4, [r2 + taddr+27155]
	cw	r1, r4
	jn	cmper6

; next address
	awt	r3, 1
	lj	rand
	awt	r3, 1
	cw	r3, 0x1000
	jls	loop

; next page/segment
	awt	r7, 1
	cwt	r7, 16
	jl	next_page

	awt	r6, 1
	cwt	r6, 16
	jl	next_segment

	hlt	077

stack:

; XPCT ir : 0xec3f
