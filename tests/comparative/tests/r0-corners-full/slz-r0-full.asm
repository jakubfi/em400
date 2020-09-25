; INPUT Seq(1, range(0, 65536))
; OUTPUT 1

	ric	r7
	awt	r7, data-.

	lw	r0, [r7]
	slz	r0
	rw	r0, r7
	uj	r4
data:
