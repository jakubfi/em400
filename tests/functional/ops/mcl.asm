
	lw	r0, 0xfafa
	lw	r1, 12
	lw	r2, 41
	lw	r3, 523
	lw	r4, 5347
	lw	r5, 1
	lw	r6, 236
	lw	r7, -24886

	im	the_sr
	mb	the_sr
	sil
	sit

	.word	1

	mcl
	hlt	077

the_sr:	.word	0b1000011110010000

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r0 : 0
; XPCT r1 : 12
; XPCT r2 : 41
; XPCT r3 : 523
; XPCT r4 : 5347
; XPCT r5 : 1
; XPCT r6 : 236
; XPCT r7 : -24886
