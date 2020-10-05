; INPUT Seq(Flags, [x<<12 for x in range(0,16)])
; INPUT Seq(Int, [0, 1, -1, -32768, 32767])
; INPUT Seq(Int, [0, 1, -1, -32768, 32767])
; OUTPUT Flags
; OUTPUT Dint

	ric	r7
	awt	r7, data-.

	lw	r0, [r7]
	lw	r2, [r7+1]
	mw	r7+2

	rw	r0, r7
	rw	r1, r7+1
	rw	r2, r7+2
	uj	r4
data:
