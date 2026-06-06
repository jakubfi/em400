; Exercise the evaluator's access to the components of the SR register:
; nb, q, bs and rm. SR layout is: rm<<6 | q<<5 | bs<<4 | nb (see SR_READ).
; Stays in system mode (q=0) so the final hlt actually halts and the
; instruction fetch keeps reading from block 0 (fetch uses block q*nb).

	im	rmw		; rm <- bits 6..15 of rmw
	mb	srw		; q,bs,nb <- bits 5,4,0..3 of srw
	hlt	077

rmw:	.word	0b1010101010_000000	; rm = 0b1010101010 = 682
srw:	.word	0b00_1_1011		; q=0, bs=1, nb=0b1011=11

; XPCT nb : 11
; XPCT bs : 1
; XPCT q : 0
; XPCT rm : 682

; SR recomposed from the four fields
; XPCT sr : 0xaa9b

; arithmetic over the new paths
; XPCT rm & 0b1111 : 0b1010
; XPCT nb + bs : 12
; XPCT bs && q : 0
; XPCT bs || q : 1
