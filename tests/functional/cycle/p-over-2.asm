; P should skip over next instruction, whether it's 1-word, or 2-words

	lwt r1, 15
	lwt r2, 10
	bn r1, 0
	lw r1, 0b1111\3 + 2 ; N argument in 2nd word is: 0b1111\3 = 'UJ' opcode, 2 = r2 as rC
exitok:
	hlt 077

	.org 10
err:
	hlt 040

; XPCT sr : 0
; XPCT rz[6] : 0

; XPCT r1 : 15
; XPCT ir&0x3f : 0o77
