
	.ifndef __MEGA_INC__
	.const __MEGA_INC__ 1

; MEGA memory configuration options

	.const	MEGA_ALLOC_FINISH	1\0
	.const	MEGA_PAS_HIDE		1\1 | 0\2
	.const	MEGA_PAS_SHOW		0\1 | 1\2
	.const	MEGA_DEALLOC		1\5 | 1\6
	.const	MEGA_ALLOC		0\5 | 1\6

	.endif

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
