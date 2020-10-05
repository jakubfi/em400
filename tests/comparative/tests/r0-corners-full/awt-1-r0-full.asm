; INPUT Seq(Int, range(-32768, 32768))
; OUTPUT Int

	lw	r0, [r7]
	awt	r0, 1
	rw	r0, r7
	uj	r4
data:
