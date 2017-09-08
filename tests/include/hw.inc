
	.ifndef __HW_INC__
	.const __HW_INC__ 1

; general system pointers

	.const	INTV		0x40	; interrupt vectors table pointer
	.const	EXLP		0x60	; EXL procedure pointer
	.const  STACKP		0x61	; stack pointer
	.const	SFPV		0x64	; uninstalled options table pointer (software FP procedure vectors, if AWP is missing)
	.const  OS_MEM_BEG	0x70	; first address of hardware-cruft-free memory in OS block

; Interrupt numbers

	.const	I_POWER		0
	.const	I_PARITY	1
	.const	I_NOMEM		2
	.const	I_2CPU_HIGH	3
	.const	I_IFPOWER	4
	.const	I_TIMER		5
	.const	I_ILLEGAL	6
	.const	I_DIV_OF	7
	.const	I_FP_UF		8
	.const	I_FP_OF		9
	.const	I_FP_ERR	10
	.const	I_DIV0		I_FP_ERR
	.const	I_EXTRA		11
	.const	I_CHAN0		12
	.const	I_CHAN1		13
	.const	I_CHAN2		14
	.const	I_CHAN3		15
	.const	I_CHAN4		16
	.const	I_CHAN5		17
	.const	I_CHAN6		18
	.const	I_CHAN7		19
	.const	I_CHAN8		20
	.const	I_CHAN9		21
	.const	I_CHAN10	22
	.const	I_CHAN11	23
	.const	I_CHAN12	24
	.const	I_CHAN13	25
	.const	I_CHAN14	26
	.const	I_CHAN15	27
	.const	I_OPRQ		28
	.const	I_2CPU_LOW	29
	.const	I_SW_HIGH	30
	.const	I_SW_LOW	31

; Interrupt Vectors

	.const	IV_POWER	INTV + I_POWER
	.const	IV_PARITY	INTV + I_PARITY
	.const	IV_NOMEM	INTV + I_NOMEM
	.const	IV_2CPU_HIGH	INTV + I_2CPU_HIGH
	.const	IV_IFPOWER	INTV + I_IFPOWER
	.const	IV_TIMER	INTV + I_TIMER
	.const	IV_ILLEGAL	INTV + I_ILLEGAL
	.const	IV_DIV_OF	INTV + I_DIV_OF
	.const	IV_FP_UF	INTV + I_FP_UF
	.const	IV_FP_OF	INTV + I_FP_OF
	.const	IV_FP_ERR	INTV + I_FP_ERR
	.const	IV_FP_DIV0	INTV + I_DIV0
	.const	IV_EXTRA	INTV + I_EXTRA
	.const	IV_CHAN0	INTV + I_CHAN0
	.const	IV_CHAN1	INTV + I_CHAN1
	.const	IV_CHAN2	INTV + I_CHAN2
	.const	IV_CHAN3	INTV + I_CHAN3
	.const	IV_CHAN4	INTV + I_CHAN4
	.const	IV_CHAN5	INTV + I_CHAN5
	.const	IV_CHAN6	INTV + I_CHAN6
	.const	IV_CHAN7	INTV + I_CHAN7
	.const	IV_CHAN8	INTV + I_CHAN8
	.const	IV_CHAN9	INTV + I_CHAN9
	.const	IV_CHAN10	INTV + I_CHAN10
	.const	IV_CHAN11	INTV + I_CHAN11
	.const	IV_CHAN12	INTV + I_CHAN12
	.const	IV_CHAN13	INTV + I_CHAN13
	.const	IV_CHAN14	INTV + I_CHAN14
	.const	IV_CHAN15	INTV + I_CHAN15
	.const	IV_OPRQ		INTV + I_OPRQ
	.const	IV_2CPU_LOW	INTV + I_2CPU_LOW
	.const	IV_SW_HIGH	INTV + I_SW_HIGH
	.const	IV_SW_LOW	INTV + I_SW_LOW

; Interrupt MASKs

	.const	IMASK_PARITY	0b1000000000000000	; I_PARITY
	.const	IMASK_NOMEM	0b0100000000000000	; I_NOMEM
	.const	IMASK_2CPU_HIGH	0b0010000000000000	; I_2CPU_HIGH
	.const	IMASK_IFPOWER	0b0001000000000000	; I_IFPOWER
	.const	IMASK_CPU	0b0000100000000000	; I_TIMER, I_ILLEGAL, I_DIV_OF, I_FP_UF, I_FP_OF, I_FP_ERR, I_FP_DIV0, I_EXTRA
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
