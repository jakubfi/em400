; Test used for K-202 performance evaluation done
; on December 4th 1972 by the K-202 Evaluation Committee
;
; Results: http://mera400.pl/index.php/Wydajno%C5%9B%C4%87_EM400

	.cpu mera400

	lw	r4, 0
	lw	r3, 0
	lw	r7, adr_1
	lw	r6, adr_2
	lw	r2, adr_pocz

adr_pocz:
	aw	r4, [r7]
	ac	r3, [r6]
	uj	r2

	hlt	077

adr_1:	.word	1
adr_2:	.word	0
