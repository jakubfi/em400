; CONFIG configs/mod.ini

; CRON still raises illegal instruction on modified (MX-16) CPU

	.cpu	mx16

	cron
	hlt	077

; XPCT rz[6] : 1
