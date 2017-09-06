; ------------------------------------------------------------------------
; Extract random bit from interrupted program address
; in: r7 - bit counter
; out: [r1, r2] - generated seed
; temp: r3
prngtimer:
	md	[STACKP]	; load IC of the interrupted program
	lw	r3, [-4]

	srz	r3
	sly	r1
	srz	r3
	sly	r1

	cwt	r7, -16
	blc	?E
	lw	r2, r1
	awt	r7, 2

	lip

; ------------------------------------------------------------------------
; Seed PRNG with a reasonably good 32-bit random number
; It is obtained by interrupting a tight loop 32 times with timer interrupt,
; each time storing least significant bit of the return address
; temp: r1, r2, r4, r7
im_tm:	.word	IMASK_CPU
im_0:	.word	0
otimv:	.res	1
ostck:	.res	1
cseed:	.dword	1
prngseed:
	.res	1
	lwt	r1, 0
	lwt	r2, 0
	lwt	r7, -32
	lw	r1, [IV_TIMER]
	rw	r1, otimv
	lw	r1, [STACKP]
	rw	r1, ostck
	lw	r1, prngtimer
	rw	r1, IV_TIMER
	lw	r1, prng_stack
	rw	r1, STACKP
	im	im_tm
rngloop:
	nop
	nop
	nop
	nop
	nop
	cwt	r7, 0
	jes	prngseed_fin
	ujs	rngloop
prngseed_fin:
	rd	cseed
	im	im_0
	lw	r4, [otimv]
	rw	r4, IV_TIMER
	lw	r4, ostck
	rw	r4, STACKP
	uj	[prngseed]

; ------------------------------------------------------------------------
; Park-Miller-Carta Pseudo-Random Number Generator implementation for MERA-400
; out: [r1, r2] - random 32-bit number
; temp: r3
	.const	constapmc 16807
lo:	.res	2
hi:	.res	2
tmp_r3:	.res	1

rand:	.res	1
	rw	r3, tmp_r3
	; lo = self.constapmc * (self.seed & 0xFFFF)
	lw	r2, constapmc
	mw	cseed+1
	rd	lo
	; hi = self.constapmc * (self.seed >> 16)
	lw	r2, constapmc
	mw	cseed
	rd	hi
	; lo += (hi & 0x7FFF) << 16
	lw	r1, [hi+1]
	er	r1, 1\0
	lwt	r2, 0
	ad	lo
	rd	lo
	; lo += hi >> 15
	lwt	r1, 0
	lw	r2, [hi]
	lw	r3, [hi+1]
	slz	r3
	sly	r2
	sly	r1
	ad	lo
	; lo = (lo & 0x7FFFFFFF) + (lo >> 31);
	sxu	r1
	er	r1, 1\0
	rd	lo
	lwt	r1, 0
	lwt	r2, 0
	slx	r2
	ad	lo
	; self.seed = lo
	rd	cseed
	; return lo
	lw	r3, [tmp_r3]
	uj	[rand]

; ------------------------------------------------------------------------
; Seed the PRNG
; in: [r1, r2] - seed
seed:	.res	1
	rd	cseed
	uj	[seed]

prng_stack:
	.res	7*4

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
