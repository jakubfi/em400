; DW: quotient -32768 with a nonzero remainder overflows (I_DIV_OF, registers
; and flags unchanged). With a zero remainder -32768 is a valid result.

	.include awp-dword.inc

operation:
	ld	r7+ARG1
	dw	r7+ARG2
	uj	r5

tests:
	; 65537 / -2 = -32768.5
	.word	0, 0, I_DIV_OF
	.word	0x0001, 0x0001
	.word	0xfffe, 0
	.word	0x0001, 0x0001

	; 1073741822 / -32767 = -32768, r 32766
	.word	?C, ?C, I_DIV_OF
	.word	0x3fff, 0xfffe
	.word	0x8001, 0
	.word	0x3fff, 0xfffe

	; -536884885 / 16384 = -32768, r -13973
	.word	?Z|?M|?V|?C, ?Z|?M|?V|?C, I_DIV_OF
	.word	0xdfff, 0xc96b
	.word	0x4000, 0
	.word	0xdfff, 0xc96b

	; 3276800 / -100 = -32768 exactly
	.word	0, ?M, 0
	.word	0x0032, 0x0000
	.word	0xff9c, 0
	.word	0x0000, 0x8000

	; -98304 / 3 = -32768 exactly
	.word	?C, ?M|?C, 0
	.word	0xfffe, 0x8000
	.word	0x0003, 0
	.word	0x0000, 0x8000
fin:
