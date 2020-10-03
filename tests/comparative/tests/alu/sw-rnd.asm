; LIMIT 200
; INPUT Rnd(Flags, 0, 65536)
; INPUT Rnd(Int, -32768, 32768)
; INPUT Rnd(Int, -32768, 32768)
; OUTPUT Flags
; OUTPUT Int

	ric	r7
	awt	r7, data-.

	lw	r0, [r7]
	lw	r1, [r7+1]
	lw	r2, [r7+2]

	sw	r1, r2

	rw	r0, r7
	rw	r1, r7+1

	uj	r4
data:
