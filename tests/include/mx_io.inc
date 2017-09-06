; default MULTIX I/O channel

	.const	MX_CHAN	1\14
	.const	MX_IV	IV_CHAN1

; MULTIX I/O commands

	.const	MX_IO_RESET	0b000\2 | 0b00\4	; IN
	.const	MX_IO_EXISTS	0b000\2 | 0b10\4	; IN
	.const	MX_IO_INTSPEC	0b000\2 | 0b01\4	; IN
	.const	MX_IO_REQUEUE	0b001\2	; IN
	.const	MX_IO_TEST	0b001\2	; OU
	.const	MX_IO_SETCFG	0b101\2	; OU
	.const	MX_IO_ATTACH	0b010\2	; OU
	.const	MX_IO_DETACH	0b010\2	; IN
	.const	MX_IO_STATUS	0b011\2	; OU
	.const	MX_IO_TRANSMIT	0b100\2	; OU
	.const	MX_IO_ABORT	0b011\2	; IN

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
