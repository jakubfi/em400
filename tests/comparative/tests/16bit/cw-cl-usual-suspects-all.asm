; INPUT Seq(Flags, [x<<9 for x in range(0,8)])
; INPUT Seq(Int, [0, 1, -1, 32767, -32768])
; INPUT Seq(Int, [0, 1, -1, 32767, -32768])
; OUTPUT Flags
; OUTPUT Flags

	lw	r1, [r7+1]
	lw	r2, [r7+2]

	lw	r0, [r7]
	cw	r1, r2
	rw	r0, r7

	lw	r0, [r7]
	cl	r1, r2
	rw	r0, r7+1

	uj	r4
data:
