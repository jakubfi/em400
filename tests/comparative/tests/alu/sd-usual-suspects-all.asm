; INPUT Seq(Flags, [x<<12 for x in range(0,16)])
; INPUT Seq(Dint, [0, 1, -1, 2147483647, -2147483648])
; INPUT Seq(Dint, [0, 1, -1, 2147483647, -2147483648])
; OUTPUT Flags
; OUTPUT Dint

	ric	r7
	awt	r7, data-.

	lw	r0, [r7]
	ld	r7+1

	sd	r7+3

	rw	r0, r7
	rd	r7+1

	uj	r4
data:
