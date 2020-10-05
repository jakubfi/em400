; INPUT Seq(Int, range(-32768, 32767))
; OUTPUT Int

	lw	r0, [r7]
	slz	r0
	rw	r0, r7
	uj	r4
data:
