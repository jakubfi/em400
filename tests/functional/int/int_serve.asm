; OPTS -c configs/minimal.cfg

; When installing a mask that allows an interrupt to be served
; it should be served right after the instruction cycle ends.

	.equ ivt 0x40
	.equ stack 0x70
	.equ stackp 0x61

	LW	r1, proc
	RW	r1, ivt+3
	LW	r1, stack
	RW	r1, stackp
	FI	ints
	IM	mask
end:	UJS	done

proc:	LW	r1, [stack]
	CW	r1, end
	JES	1
	HLT	040
	LIP

done:	HLT	077

mask:	.word	0x2000
ints:	.word	0x7fff


; XPCT ir : 0xec3f
