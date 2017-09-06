
; Jumps that wouldn't be taken (relevant CPU flag is not set)
; are treated as ineffective instructions - instruction
; cycle is finished after the decode phase.
; That means effective argument is not calculated.

	.cpu	mera400

	mcl
	lwt	r0, 0
	jl	[0x8000]
	je	[0x8000]
	jg	[0x8000]
	jz	[0x8000]
	jm	[0x8000]
	lw	r0, ?E
	jn	[0x8000]

	hlt	077


; XPCT ir : 0xec3f
; XPCT rz[2] : 0
; XPCT rz[6] : 0
