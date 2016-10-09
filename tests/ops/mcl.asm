
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

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(r0) : 0
; XPCT int(r1) : 12
; XPCT int(r2) : 41
; XPCT int(r3) : 523
; XPCT int(r4) : 5347
; XPCT int(r5) : 1
; XPCT int(r6) : 236
; XPCT int(r7) : -24886
