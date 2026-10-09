; PRECMD REG IC 1

	; no answer from memory reads as 0: sp should load IC=0, R0=0, SR=0
	hlt	077
	lwt	r0, -1
	im	blk
	mb	blk
	sp	0x8000
	hlt	040

	; RM: group L, BS=1, NB=0
blk:	.word	0b0000000001010000

; XPCT rz[2] : 1
; XPCT r0 : 0
; XPCT sr : 0
