; OPTS -c configs/minimal.cfg

; When installing a mask that allows an interrupt to be served
; it should be served right after the instruction cycle ends.

	.include hw.inc

	uj	start

	.org	OS_MEM_BEG
start:
	lw	r1, proc
	rw	r1, IV_2CPU_HIGH
	lw	r1, stack
	rw	r1, STACKP
	fi	ints
	im	mask
end:	ujs	done

proc:	lw	r1, [stack]
	cw	r1, end
	jes	1
	hlt	040
	lip

done:	hlt	077

mask:	.word	IMASK_2CPU_HIGH
ints:	.word	0x7fff
stack:

; XPCT ir : 0xec3f
