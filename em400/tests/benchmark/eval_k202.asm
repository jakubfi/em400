.program "ocena K-202"

; Slightly modified (see notes below) version of a test used
; for K-202 evaluation done on December 4th 1972 by
; K-202 Evaluation Committee
;
; Results:
;
; original K-202                                0.424 MIPS
; em400 w/debugger       (Core2Duo @2.80GHz)    0.858 MIPS
; em400                  (Core2Duo @2.80GHz)    7.8   MIPS
; em400 +regs opt.       (Core2Duo @2.80GHz)   10.422 MIPS
; em400 +mutex opt.      (Core2Duo @2.80GHz)   25     MIPS
; em400 +mem_ptr() opt.  (Core2Duo @2.80GHz)   27.7   MIPS
; em400 +arg_norm() opt. (Core2Duo @2.80GHz)   30     MIPS

	lw r4, 0
	lw r3, -10		; -10 instead of 0
	lw r7, adr_1
	lw r6, adr_2
	lw r2, adr_pocz

adr_pocz:
	aw r4, [r7]
	ac r3, [r6]
	jm r2			; jm instead of uj allowing program to stop

	hlt 077

adr_1:	.data 1
adr_2:	.data 0

.endprog
