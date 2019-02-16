
	.ifndef __HW_INC__
	.const __HW_INC__ 1

; general system pointers

	.const	INTV		0x40	; interrupt vectors table pointer
	.const	EXLP		0x60	; EXL procedure pointer
	.const  STACKP		0x61	; stack pointer
	.const	SFPV		0x64	; uninstalled options table pointer (software FP procedure vectors, if AWP is missing)
	.const  OS_MEM_BEG	0x70	; first address of hardware-cruft-free memory in OS block

; SR field positions

	.const SR_NB		15
	.const SR_BS		11
	.const SR_Q		10
	.const SR_RM		9

; Interrupt Vectors

	.const	IV_POWER	INTV + 0
	.const	IV_PARITY	INTV + 1
	.const	IV_NOMEM	INTV + 2
	.const	IV_2CPU_HIGH	INTV + 3
	.const	IV_IFPOWER	INTV + 4
	.const	IV_TIMER	INTV + 5
	.const	IV_ILLEGAL	INTV + 6
	.const	IV_DIV_OF	INTV + 7
	.const	IV_FP_UF	INTV + 8
	.const	IV_FP_OF	INTV + 9
	.const	IV_FP_ERR	INTV + 10
	.const	IV_FP_DIV0	IV_FP_ERR
	.const	IV_EXTRA	INTV + 11
	.const	IV_CHAN0	INTV + 12
	.const	IV_CHAN1	INTV + 13
	.const	IV_CHAN2	INTV + 14
	.const	IV_CHAN3	INTV + 15
	.const	IV_CHAN4	INTV + 16
	.const	IV_CHAN5	INTV + 17
	.const	IV_CHAN6	INTV + 18
	.const	IV_CHAN7	INTV + 19
	.const	IV_CHAN8	INTV + 20
	.const	IV_CHAN9	INTV + 21
	.const	IV_CHAN10	INTV + 22
	.const	IV_CHAN11	INTV + 23
	.const	IV_CHAN12	INTV + 24
	.const	IV_CHAN13	INTV + 25
	.const	IV_CHAN14	INTV + 26
	.const	IV_CHAN15	INTV + 27
	.const	IV_OPRQ		INTV + 28
	.const	IV_2CPU_LOW	INTV + 29
	.const	IV_SW_HIGH	INTV + 30
	.const	IV_SW_LOW	INTV + 31

; Interrupts as seen in the 16-bit subset of RZ

	.const	I_POWER		0b1000000000000000
	.const	I_PARITY	0b0100000000000000
	.const	I_NOMEM		0b0010000000000000
	.const	I_2CPU_HIGH	0b0001000000000000
	.const	I_IFPOWER	0b0000100000000000
	.const	I_TIMER		0b0000010000000000
	.const	I_ILLEGAL	0b0000001000000000
	.const	I_DIV_OF	0b0000000100000000
	.const	I_FP_UF		0b0000000010000000
	.const	I_FP_OF		0b0000000001000000
	.const	I_FP_ERR	0b0000000000100000
	.const	I_DIV0		I_FP_ERR
	.const	I_EXTRA		0b0000000000010000
	.const	I_OPRQ		0b0000000000001000
	.const	I_2CPU_LOW	0b0000000000000100
	.const	I_SW_HIGH	0b0000000000000010
	.const	I_SW_LOW	0b0000000000000001

	.const	I_ALL		0b1111111111111111
	.const	I_ALL_CPU	0b0001000000000100
	.const	I_ALL_ARITH	0b0000000111100000
	.const	I_ALL_FP	0b0000000011100000
	.const	I_ALL_SW	0b0000000000000011
	.const	I_MASKABLE	0b0111111111111111
	.const	I_NONE		0b0000000000000000

; Interrupt MASKs

	.const	IMASK_PARITY	0b1000000000000000	; I_PARITY
	.const	IMASK_NOMEM	0b0100000000000000	; I_NOMEM
	.const	IMASK_2CPU_HIGH	0b0010000000000000	; I_2CPU_HIGH
	.const	IMASK_IFPOWER	0b0001000000000000	; I_IFPOWER
	.const	IMASK_CPU	0b0000100000000000	; I_TIMER, I_ILLEGAL, I_DIV_OF, I_FP_UF, I_FP_OF, I_FP_ERR, I_EXTRA
	.const	IMASK_CH0_1	0b0000010000000000	; I_CHAN0, I_CHAN1
	.const	IMASK_CH2_3	0b0000001000000000	; I_CHAN2, I_CHAN3
	.const	IMASK_CH4_9	0b0000000100000000	; I_CHAN4...I_CHAN9
	.const	IMASK_CH10_15	0b0000000010000000	; I_CHAN10...I_CHAN15
	.const	IMASK_SOFT	0b0000000001000000	; I_OPRQ, I_2CPU_LOW, I_SW_HIGH, I_SW_LOW
	.const	IMASK_CH_ALL	0b0000011110000000	; all channell interrupts
	.const	IMASK_ALL	0b1111111111000000	; all interrupts
	.const	IMASK_NONE	0b0000000000000000	; no interrupts

; Software FP procedure Vectors

	.const	SFPV_NRF0	SFPV + 0
	.const	SFPV_NRF1	SFPV + 1
	.const	SFPV_NRF2	SFPV + 2
	.const	SFPV_NRF3	SFPV + 3
	.const	SFPV_AD		SFPV + 4
	.const	SFPV_SD		SFPV + 5
	.const	SFPV_MW		SFPV + 6
	.const	SFPV_DW		SFPV + 7
	.const	SFPV_AF		SFPV + 8
	.const	SFPV_SF		SFPV + 9
	.const	SFPV_MF		SFPV + 10
	.const	SFPV_DF		SFPV + 11

	.endif

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
