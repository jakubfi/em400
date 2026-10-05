; CONFIG configs/soft_awp.ini

; pre-modification affects soft-AWP NRF byte argument
; full 16-bit premodified argument is stored on the stack,
; but the vector is selected by the unmodified instruction bits

	.include cpu.inc

	uj	start

.org SFPV
	.word	pNRF0
	.word	pNRF1
	.word	pNRF2
	.word	pNRF3
	.res	8, fail

fail:	hlt	050

pNRF0:	lwt	r6, 0
	ujs	store
pNRF1:	lwt	r6, 1
	ujs	store
pNRF2:	lwt	r6, 2
	ujs	store
pNRF3:	lwt	r6, 3
	ujs	store

store:	lw	r1, [STACKP]
	lw	r2, [r1-1]
	lip

start:	lw	r1, stack
	rw	r1, STACKP

	md	0x40
	nrf	0	; arg = 0x40
	cwt	r6, 0
	jes	1
	hlt	040
	cw	r2, 0x40
	jes	1
	hlt	041

	md	0x100
	nrf	0b11000000	; arg = 0x1c0
	cwt	r6, 3
	jes	1
	hlt	042
	cw	r2, 0x1c0
	jes	1
	hlt	043

	md	0xffff
	nrf	0b01000001	; arg = 0x40
	cwt	r6, 1
	jes	1
	hlt	044
	cw	r2, 0x40
	jes	1
	hlt	045

	hlt	077

stack:
