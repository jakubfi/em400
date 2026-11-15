; DW by -32768 (C = -1.0) with a negative dividend -k * 32768: when the bit
; right above k's lowest set bit is 0, the quotient is k - 1 and the remainder
; -32768. Otherwise the division is exact. k = 32768 does not overflow.

	.include awp-dword.inc

operation:
	ld	r7+ARG1
	dw	r7+ARG2
	uj	r5

tests:
	; k = 1
	.word	0x56a5, 0x96a5, 0
	.word	0xffff, 0x8000
	.word	0x8000, 0
	.word	0x8000, 0x0000

	; k = 2
	.word	0, 0, 0
	.word	0xffff, 0x0000
	.word	0x8000, 0
	.word	0x8000, 0x0001

	; k = 3, exact
	.word	0, 0, 0
	.word	0xfffe, 0x8000
	.word	0x8000, 0
	.word	0x0000, 0x0003

	; k = 4
	.word	0xd269, 0x1269, 0
	.word	0xfffe, 0x0000
	.word	0x8000, 0
	.word	0x8000, 0x0003

	; k = 32
	.word	0x9a65, 0x1a65, 0
	.word	0xfff0, 0x0000
	.word	0x8000, 0
	.word	0x8000, 0x001f

	; k = 32767, exact
	.word	0x81a2, 0x01a2, 0
	.word	0xc000, 0x8000
	.word	0x8000, 0
	.word	0x0000, 0x7fff

	; k = 32768
	.word	0x5f9e, 0x1f9e, 0
	.word	0xc000, 0x0000
	.word	0x8000, 0
	.word	0x8000, 0x7fff
fin:
