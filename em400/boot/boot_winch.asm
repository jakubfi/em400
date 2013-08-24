; bootstrap system from winchester

.prog

; ------------------------------------------------------------------------
	.equ chan 1
	.equ chan_mask 0b0000010000000000
	.equ mx 1
	.equ winch 0
	.equ mx_int 0x40+12+mx
	.equ stackp 0x61
	.equ prog_beg 0x70

	uj start

; ------------------------------------------------------------------------
stack:	.res 2*4
mask:	.data chan_mask

mx_cmd:	.data 0b101\2+chan\14, 0b010\2+chan\14, 0b100\2+chan\14
mx_ps:	.data psuk, 0, pstrx
intseq:	.data 2, 5, 10, 13

psuk:	.data 2\7 + 1
	.data 0
	.data 27
	.data 1\3 + 3\7
	.data 6\7 + 28
	.data 1\7 + 1
pstrx:	.data 2\7
	.data image
	.data imagesize-1
	.data 0
	.data 1

	.res 0x40-S	; padding up to 0x40

; ------------------------------------------------------------------------
	.ic prog_beg
pmx:
	irb r7, 0	; int_sequence++

	md [stackp]
	lw r1, [-1]
	lw r2, r1
	zlb r1		; r1 = device
	zrb r2
	shc r2, 8	; r2 = intspec

	cw r2, [intseq+r7]
	bb r0, ?E	; proper int sequence?
	hlt 2

	cw r2, 2	; is this the interrupt after multix reset (=2)?
	jes cont	; don't check device number

	cw r1, winch
	bb r0, ?E	; is this the right device?
	hlt 3
cont:
	cw r7, 3	; done loading?
	je boot
	lw r1, [mx_ps+r7]
	ou r1, [mx_cmd+r7]
	.data 0, 0, next, 0
next:
	lip

; ------------------------------------------------------------------------
boot:
	la lcp
	ra copier	; copy 7 words from @lcp to @copier
	lw r1, -imagesize
	lw r3, image
	uj copier	; run copier

; ------------------------------------------------------------------------
lcp:	
	lw r2, [image+imagesize+r1]
	rw r2, imagesize+r1
	irb r1, lcp
	uj 0		; start system
elcp:

; ------------------------------------------------------------------------
; ---- MAIN --------------------------------------------------------------
; ------------------------------------------------------------------------
start:
	mcl
	lw r7, -1
	lwt r1, stack
	rw r1, stackp
	lw r1, pmx
	rw r1, mx_int

	im mask
lhlt:
	hlt 1
	ujs lhlt	; forever waiting on interrupts

; ------------------------------------------------------------------------
image:
	.equ memsize 2*4096
	.equ imagesize (memsize-image)-(elcp-lcp)
	.equ copier image+imagesize

.finprog
