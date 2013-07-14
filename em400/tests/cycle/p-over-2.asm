.prog "cycle/P-over-2"

; P should skip over next instruction, whether it's 1-word, or 2-words

	lwt r1, 15
	lwt r2, 10
	bn r1, 0
	lw r1, 0b1111\3 + 2 ; N argument in 2nd word is: 0b1111\3 = 'UJ' opcode, 2 = r2 as rC

	hlt 077

	.ic 10
	hlt 077

.finprog

; XPCT int(sr) : 0
; XPCT int(rz[6]) : 0

; XPCT int(r1) : 15
; XPCT int(ic) : 7
