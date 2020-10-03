; INPUT Seq(Int, range(-32768, 32768))
; OUTPUT Int

	ric	r7
	awt	r7, data-.

	lw	r0, [r7]
	awt	r0, 1
	rw	r0, r7
	uj	r4
data:
