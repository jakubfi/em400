; INPUT Seq(Flags, [x<<12 for x in range(0, 16)])
; INPUT Seq(Int, [0, 1, -1, 32767, -32768])
; OUTPUT Flags
; OUTPUT Int

	lw	r0, [r7]
	lw	r1, [r7+1]

	ngc	r1

	rw	r0, r7
	rw	r1, r7+1

	uj	r4
data:
