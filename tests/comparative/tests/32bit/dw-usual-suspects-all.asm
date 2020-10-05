; INPUT Seq(Flags, [x<<12 for x in range(0,16)])
; INPUT Seq(Dint, [0, 1, -1, 2147483647, -2147483648])
; INPUT Seq(Int, [0, 1, -1, 32767, -32768])
; OUTPUT Flags
; OUTPUT Int
; OUTPUT Int
; OUTPUT Word

	lw	r0, [r7]
	ld	r7+1

	dw	r7+3

	rw	r0, r7
	rw	r1, r7+1
	rw	r2, r7+2
	ki	r7+3

	uj	r4
data:
