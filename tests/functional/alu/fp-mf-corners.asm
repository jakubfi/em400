
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	mf	r7+ARG2
	uj	r5

tests:
	; corner case examples found with comparative tests
	.word	0x71c9, 0x31c9, 0
	.word	0x4cd7, 0x05a9, 0x14dc
	.word	0x4999, 0xcb82, 0x0efd
	.word	0x585d, 0xe8d4, 0x8fd8

	.word	0xc111, 0x5111, 0
	.word	0x6f00, 0x3b71, 0x8090
	.word	0x8937, 0xcc5c, 0xe8fc
	.word	0x98fe, 0x2c0e, 0xf68c

	.word	0xe4b1, 0x74b1, 0
	.word	0xb3cd, 0x8886, 0xe0c9
	.word	0x5900, 0x1797, 0x92d2
	.word	0x9609, 0xb5c4, 0xf89a
fin:

; XPCT sr : 0
; XPCT ir : 0xec3f
