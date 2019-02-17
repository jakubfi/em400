; OPTS -c configs/soft_awp.cfg

	.include cpu.inc

	uj	start

int:	hlt	040
stack:	.res	16
mask:	.word	IMASK_ALL

.org INTV
	.res	32, int

.org SFPV
	.word	pNRF0
	.word	pNRF1
	.word	pNRF2
	.word	pNRF3
	.word	pAD
	.word	pSD
	.word	pMW
	.word	pDW
	.word	pAF
	.word	pSF
	.word	pMF
	.word	pDF

mark:	lw	r1, [STACKP]
	lw	r1, [r1-1]
	aw	r1, r6
	rw	r1, r1
	lip

pNRF0:	lw	r6, 901	ujs mark
pNRF1:	lw	r6, 902	ujs mark
pNRF2:	lw	r6, 903	ujs mark
pNRF3:	lw	r6, 904	ujs mark
pAD:	lwt	r6, 5	ujs mark
pSD:	lwt	r6, 6	ujs mark
pMW:	lwt	r6, 7	ujs mark
pDW:	lwt	r6, 8	ujs mark
pAF:	lwt	r6, 9	ujs mark
pSF:	lwt	r6, 10	ujs mark
pMF:	lwt	r6, 11	ujs mark
pDF:	lwt	r6, 12	ujs mark

start:	lw	r1, stack
	rw	r1, STACKP
	im	mask

	ad	1000
	sd	1100
	mw	1200
	dw	1300
	af	1400
	sf	1500
	mf	1600
	df	1700
	nrf	0b00000000
	nrf	0b01000000
	nrf	0b10000000
	nrf	0b11000000

	hlt 077

; XPCT [1005] : 1005
; XPCT [1106] : 1106
; XPCT [1207] : 1207
; XPCT [1308] : 1308
; XPCT [1409] : 1409
; XPCT [1510] : 1510
; XPCT [1611] : 1611
; XPCT [1712] : 1712
; XPCT [901] : 901
; XPCT [966] : 966
; XPCT [1031] : 1031
; XPCT [1096] : 1096

; XPCT ir : 0xec3f
