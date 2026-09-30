; CONFIG configs/mega_max.ini

; OS accessing segments 1-15 through NB: every page, remapped at runtime,
; read with various addressing modes, checked against sample offsets.
; A single data frame is moved through all pages, the remaining pages
; of the segment point to a frame filled with a known mask value.

	.cpu	mera400

	.include cpu.inc
	.include io.inc
	.include mega.inc

	.const	MCFG MEM_CFG | MEGA_ALLOC_DONE | MEGA_EPROM_HIDE | MEGA_ALLOC
	.const	MFRAME 0\MEM_FRAME | 2\MEM_MODULE | MCFG
	.const	DFRAME 1\MEM_FRAME | 2\MEM_MODULE | MCFG
	.const	FILLPAGE 15\MEM_PAGE | 0\MEM_SEGMENT
	.const	MASK 0xaa77
	; offset ^ PATTERN never equals MASK for offsets < 0x1000
	.const	PATTERN 0x5a00
	.const	OFS_STEP 0x111

	uj	start

	.org	OS_START

nbw:	.res	1
taddr:	.res	1

mcerr:	hlt	040
cmper1:	hlt	041
cmper2:	hlt	042
cmper3:	hlt	043
cmper4:	hlt	044
cmper5:	hlt	045
cmper6:	hlt	046
mskerr:	hlt	047

start:
	lw	r1, FILLPAGE
	ou	r1, MFRAME
	.word	mcerr, mcerr, mfill, mcerr
mfill:	lw	r2, MASK
	lw	r1, 0xf000
mloop:	rw	r2, r1
	irb	r1, mloop

	lw	r1, FILLPAGE
	ou	r1, DFRAME
	.word	mcerr, mcerr, dfill, mcerr
dfill:	lw	r1, 0xf000
dloop:	lw	r2, r1
	nr	r2, 0x0fff
	xr	r2, PATTERN
	rw	r2, r1
	irb	r1, dloop

; r6 - segment, r7 - page
	lwt	r6, 1
next_segment:
	rw	r6, nbw
	mb	nbw
	lwt	r7, 0

next_page:
	lwt	r2, -1
mnext:	awt	r2, 1
	cwt	r2, 15
	jgs	mdone
	lw	r1, r2
	shc	r1, 4
	aw	r1, r6
	ou	r1, MFRAME
	.word	mcerr, mcerr, mnext, mcerr
mdone:
	lw	r1, r7
	shc	r1, 4
	aw	r1, r6
	ou	r1, DFRAME
	.word	mcerr, mcerr, dcheck, mcerr

; r5 - page base address, r3 - offset, r1 - expected value
dcheck:
	lw	r5, r7
	shc	r5, 4
	lwt	r3, 0
oloop:
	lw	r1, r3
	xr	r1, PATTERN

	lw	r2, r5
	aw	r2, r3
	rw	r2, taddr
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

	; pointer comes from block 0, operand from NB
	tw	r4, [taddr]
	cw	r1, r4
	jn	cmper5

	lw	r2, -27155
	tw	r4, [r2+taddr+27155]
	cw	r1, r4
	jn	cmper6

	aw	r3, OFS_STEP
	cw	r3, 0x1000
	jls	oloop

; all other pages in the segment should still show the mask frame
	lw	r3, 0x0123
pmloop:	lw	r2, r3
	nr	r2, 0xf000
	cw	r2, r5
	jes	pmnext
	tw	r4, r3
	cw	r4, MASK
	jn	mskerr
pmnext:	aw	r3, 0x1000
	cw	r3, 0x0123
	jn	pmloop

	awt	r7, 1
	cwt	r7, 16
	jl	next_page

	awt	r6, 1
	cwt	r6, 16
	jl	next_segment

	hlt	077
