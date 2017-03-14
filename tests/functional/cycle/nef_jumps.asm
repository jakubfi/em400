
; Jumps that wouldn't be taken (relevant CPU flag is not set)
; are treated as ineffective instructions - instruction
; cycle is finished after the decode phase.
; That means effective argument is not calculated.

	.cpu	mera400

	MCL
	LWT	r0, 0
	JL	[0x8000]
	JE	[0x8000]
	JG	[0x8000]
	JZ	[0x8000]
	JM	[0x8000]
	LW	r0, ?E
	JN	[0x8000]

	HLT	077


; XPCT ir : 0xec3f
; XPCT rz[2] : 0
; XPCT rz[6] : 0
