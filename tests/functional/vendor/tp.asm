; OPTS -c configs/minimal.ini
;
; MERA-400 CPU test
;
; The original "TP" CPU test provided by the manufacturer.
; Disassembled and rewritten for beter readability.
;
; If 'EM400' constant is set, assembly is targeted for em400 emulation tests.
; Otherwise output is identical to the TP binary and targeted for h/w.

	.cpu	mera400

	.include cpu.inc
	.include io.inc

	; use EM400 compatibile hlt codes if running in an emulator
	.ifdef	EM400
	.const	ERR_CODE 040
	.else
	.const	ERR_CODE 0
	.endif

	uj	start

mask:	.word	0

	.org	INTV

	.word	int00, int01, int02, int03, int04, int05, int06, int07
	.word	int08, int09, int10, int11, int12, int13, int14, int15
	.word	int16, int17, int18, int19, int20, int21, int22, int23
	.word	int24, int25, int26, int27, int28, int29, int30, int31

exlp:	.word	0
stackp:	.word	0
stack:

	.org	0xc1

int01:	hlt	ERR_CODE	lip
int02:	hlt	ERR_CODE	lip
int03:	hlt	ERR_CODE	lip
int04:	hlt	ERR_CODE	lip
int05:	hlt	ERR_CODE	lip
int06:	hlt	ERR_CODE	lip
int07:	hlt	ERR_CODE	lip
int08:	hlt	ERR_CODE	lip
int09:	hlt	ERR_CODE	lip
int10:	hlt	ERR_CODE	lip
int11:	hlt	ERR_CODE	lip
int12:	hlt	ERR_CODE	lip
int13:	hlt	ERR_CODE	lip
int14:	hlt	ERR_CODE	lip
int15:	hlt	ERR_CODE	lip
int16:	hlt	ERR_CODE	lip
int17:	hlt	ERR_CODE	lip
int18:	hlt	ERR_CODE	lip
int19:	hlt	ERR_CODE	lip
int20:	hlt	ERR_CODE	lip
int21:	hlt	ERR_CODE	lip
int22:	hlt	ERR_CODE	lip
int23:	hlt	ERR_CODE	lip
int24:	hlt	ERR_CODE	lip
int25:	hlt	ERR_CODE	lip
int26:	hlt	ERR_CODE	lip
int27:	hlt	ERR_CODE	lip
int28:	hlt	ERR_CODE	lip
int29:	hlt	ERR_CODE	lip
int30:	hlt	ERR_CODE	lip
int31:	hlt	ERR_CODE	lip

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
; ------------------------------------------------------------------------

	.org	0x100

start:

; test rozkazów w bloku podstawowym (Q=0, NB=0)

; wstępne przetestowanie czy użyte w testach testowanie działa
; kombinacje lw, lwt, cw, cwt w różnych trybach adresowania

x0100:	lw	r1, -1
x0102:	cw	r1, -1
x0104:	jes	1
x0105:	hlt	ERR_CODE

x0106:	lw	r1, -1
x0108:	cwt	r1, -1
x0109:	jes	1
x010a:	hlt	ERR_CODE

x010b:	lwt	r1, -1
x010c:	cw	r1, -1
x010e:	jes	1
x010f:	hlt	ERR_CODE

x0110:	lwt	r1, -1
x0111:	cwt	r1, -1
x0112:	jes	1
x0113:	hlt	ERR_CODE

x0114:	lwt	r1, 0
x0115:	lw	r1, [pat_aaaa]
x0117:	cw	r1, 0xaaaa
x0119:	jes	1
x011a:	hlt	ERR_CODE

x011b:	lwt	r2, 0
x011c:	lw	r2, 0xaaaa
x011e:	cw	r2, 0xaaaa
x0120:	jes	1
x0121:	hlt	ERR_CODE

x0122:	lwt	r1, 0
x0123:	lw	r1, r2
x0124:	cw	r1, 0xaaaa
x0126:	jes	1
x0127:	hlt	ERR_CODE

x0128:	lw	r1, 0xaaaa
x012a:	lw	r2, [pat_aaaa]
x012c:	cw	r2, r1
x012d:	jes	1
x012e:	hlt	ERR_CODE

; rw, rz

x012f:	lw	r1, 0xaaaa
x0131:	rw	r1, temp3
x0133:	lw	r2, [temp3]
x0135:	cw	r2, 0xaaaa
x0137:	jes	1
x0138:	hlt	ERR_CODE

x0139:	lw	r1, 0xaaaa
x013b:	cw	r1, [pat_aaaa]
x013d:	jes	1
x013e:	hlt	ERR_CODE

x013f:	rz	pat_aaaa
x0141:	lwt	r1, 0
x0142:	cw	r1, [pat_aaaa]
x0144:	jes	1
x0145:	hlt	ERR_CODE

x0146:	lw	r1, 0xaaaa
x0148:	rw	r1, pat_aaaa
x014a:	rz	mask

; czy mcl kasuje r0

x014c:	mcl
x014d:	cwt	r0, 0
x014e:	jes	1
x014f:	hlt	ERR_CODE

; Test is restarted from this location, but with the interrupt mask
; (stored also at mask) set to IMASK_ALL (@ 0x1ade).
; It will be restarted once again (but from location 0x100) @ 0x0c36.

restart:

; tw, pw

x0150:	lw	r2, [pat_aaaa]
x0152:	rw	r2, temp3
x0154:	lwt	r1, 0
x0155:	tw	r1, temp3
x0157:	cw	r1, [pat_aaaa]
x0159:	jes	1
x015a:	hlt	ERR_CODE

x015b:	lw	r2, [pat10]
x015d:	rz	temp3
x015f:	pw	r2, temp3
x0161:	lw	r1, [temp3]
x0163:	cw	r1, [pat10]
x0165:	jes	1
x0166:	hlt	ERR_CODE

; ls

x0167:	lwt	r1, 0
x0168:	lwt	r7, 0
x0169:	ls	r1, -1
x016b:	cwt	r1, 0
x016c:	jes	1
x016d:	hlt	ERR_CODE

x016e:	lw	r1, [pat10]
x0170:	lwt	r7, -1
x0171:	ls	r1, [pat01]
x0173:	cw	r1, [pat01]
x0175:	jes	1
x0176:	hlt	ERR_CODE

x0177:	lwt	r1, -1
x0178:	lw	r7, [pat01]
x017a:	ls	r1, -1
x017c:	cwt	r1, -1
x017d:	jes	1
x017e:	hlt	ERR_CODE

x017f:	lwt	r1, -1
x0180:	lw	r7, [pat10]
x0182:	ls	r1, -1
x0184:	cwt	r1, -1
x0185:	jes	1
x0186:	hlt	ERR_CODE

; ri

x0187:	lw	r1, temp3
x0189:	rz	temp3
x018b:	rz	temp4
x018d:	ri	r1, [pat10]
x018f:	lw	r2, [pat10]
x0191:	cw	r2, [temp3]
x0193:	jes	1
x0194:	hlt	ERR_CODE
x0195:	cw	r1, temp4
x0197:	jes	1
x0198:	hlt	ERR_CODE

x0199:	ri	r1, [pat10]
x019b:	lw	r2, [pat10]
x019d:	cw	r2, [temp4]
x019f:	jes	1
x01a0:	hlt	ERR_CODE
x01a1:	cw	r1, temp5
x01a3:	jes	1
x01a4:	hlt	ERR_CODE

; rj

x01a5:	rj	r7, rj_jump
rj_jump:
x01a7:	cw	r7, rj_jump
x01a9:	jes	1
x01aa:	hlt	ERR_CODE

; is

x01ab:	rz	temp3
x01ad:	lwt	r1, 0
x01ae:	is	r1, temp3
x01b0:	hlt	ERR_CODE
x01b1:	lw	r1, [temp3]
x01b3:	cwt	r1, 0
x01b4:	jes	1
x01b5:	hlt	ERR_CODE

x01b6:	rz	temp3
x01b8:	lwt	r1, -1
x01b9:	is	r1, temp3
x01bb:	ujs	1
x01bc:	hlt	ERR_CODE
x01bd:	lw	r1, [temp3]
x01bf:	cwt	r1, -1
x01c0:	jes	1
x01c1:	hlt	ERR_CODE

x01c2:	lwt	r1, -1
x01c3:	rw	r1, temp3
x01c5:	lwt	r1, 0
x01c6:	is	r1, temp3
x01c8:	hlt	ERR_CODE
x01c9:	lw	r1, [temp3]
x01cb:	cwt	r1, -1
x01cc:	jes	1
x01cd:	hlt	ERR_CODE

x01ce:	lwt	r1, -1
x01cf:	rw	r1, temp3
x01d1:	is	r1, temp3
x01d3:	hlt	ERR_CODE
x01d4:	lw	r1, [temp3]
x01d6:	cwt	r1, -1
x01d7:	jes	1
x01d8:	hlt	ERR_CODE

x01d9:	lw	r1, [pat01]
x01db:	rw	r1, temp3
x01dd:	is	r1, temp3
x01df:	hlt	ERR_CODE
x01e0:	lw	r1, [temp3]
x01e2:	cw	r1, [pat01]
x01e4:	jes	1
x01e5:	hlt	ERR_CODE

x01e6:	lw	r1, [pat10]
x01e8:	rw	r1, temp3
x01ea:	is	r1, temp3
x01ec:	hlt	ERR_CODE
x01ed:	lw	r1, [temp3]
x01ef:	cw	r1, [pat10]
x01f1:	jes	1
x01f2:	hlt	ERR_CODE

x01f3:	lw	r1, [pat01]
x01f5:	lw	r2, [pat10]
x01f7:	rw	r2, temp3
x01f9:	is	r1, temp3
x01fb:	ujs	1
x01fc:	hlt	ERR_CODE
x01fd:	lw	r1, [temp3]
x01ff:	cwt	r1, -1
x0200:	jes	1
x0201:	hlt	ERR_CODE

x0202:	lw	r1, [pat01]
x0204:	lw	r2, [pat10]
x0206:	rw	r1, temp3
x0208:	is	r2, temp3
x020a:	ujs	1
x020b:	hlt	ERR_CODE
x020c:	lw	r1, [temp3]
x020e:	cwt	r1, -1
x020f:	jes	1
x0210:	hlt	ERR_CODE

; bc

x0211:	lwt	r1, 0
x0212:	lwt	r2, 0
x0213:	bc	r1, r2
x0214:	ujs	1
x0215:	hlt	ERR_CODE

x0216:	lwt	r1, -1
x0217:	lwt	r2, -1
x0218:	bc	r1, r2
x0219:	ujs	1
x021a:	hlt	ERR_CODE

x021b:	lwt	r1, -1
x021c:	lwt	r2, 0
x021d:	bc	r1, r2
x021e:	ujs	1
x021f:	hlt	ERR_CODE

x0220:	lwt	r1, 0
x0221:	lwt	r2, -1
x0222:	bc	r1, r2
x0223:	hlt	ERR_CODE

x0224:	lw	r1, [pat01]
x0226:	lw	r2, r1
x0227:	bc	r1, r2
x0228:	ujs	1
x0229:	hlt	ERR_CODE

x022a:	lw	r1, [pat10]
x022c:	lw	r2, r1
x022d:	bc	r1, r2
x022e:	ujs	1
x022f:	hlt	ERR_CODE

x0230:	lw	r1, [pat01]
x0232:	lw	r2, [pat10]
x0234:	bc	r1, r2
x0235:	hlt	ERR_CODE

x0236:	lw	r1, [pat10]
x0238:	lw	r2, [pat01]
x023a:	bc	r1, r2
x023b:	hlt	ERR_CODE

; bm

x023c:	lwt	r1, 0
x023d:	rw	r1, temp3
x023f:	bm	r1, temp3
x0241:	hlt	ERR_CODE

x0242:	lwt	r1, -1
x0243:	rw	r1, temp3
x0245:	bm	r1, temp3
x0247:	hlt	ERR_CODE

x0248:	lwt	r1, -1
x0249:	rz	temp3
x024b:	bm	r1, temp3
x024d:	ujs	1
x024e:	hlt	ERR_CODE

x024f:	lwt	r1, 0
x0250:	lwt	r2, -1
x0251:	rw	r2, temp3
x0253:	bm	r1, temp3
x0255:	hlt	ERR_CODE

x0256:	lw	r1, [pat01]
x0258:	rw	r1, temp3
x025a:	bm	r1, temp3
x025c:	hlt	ERR_CODE

x025d:	lw	r1, [pat10]
x025f:	rw	r1, temp3
x0261:	bm	r1, temp3
x0263:	hlt	ERR_CODE

x0264:	lw	r1, [pat01]
x0266:	lw	r2, [pat10]
x0268:	rw	r2, temp3
x026a:	bm	r1, temp3

x026c:	ujs	1
x026d:	hlt	ERR_CODE
x026e:	lw	r1, [pat10]
x0270:	lw	r2, [pat01]
x0272:	rw	r2, temp3
x0274:	bm	r1, temp3

; bn

x0276:	ujs	1
x0277:	hlt	ERR_CODE
x0278:	lwt	r1, 0
x0279:	lw	r2, r1
x027a:	bn	r1, r2
x027b:	hlt	ERR_CODE

x027c:	lwt	r1, -1
x027d:	lw	r2, r1
x027e:	bn	r1, r2
x027f:	ujs	1
x0280:	hlt	ERR_CODE

x0281:	lwt	r1, -1
x0282:	lwt	r2, 0
x0283:	bn	r1, r2
x0284:	hlt	ERR_CODE

x0285:	lwt	r1, 0
x0286:	lwt	r2, -1
x0287:	bn	r1, r2
x0288:	hlt	ERR_CODE

x0289:	lw	r1, [pat01]
x028b:	lw	r2, r1
x028c:	bn	r1, r2
x028d:	ujs	1
x028e:	hlt	ERR_CODE

x028f:	lw	r1, [pat10]
x0291:	lw	r2, r1
x0292:	bn	r1, r2
x0293:	ujs	1
x0294:	hlt	ERR_CODE

x0295:	lw	r1, [pat01]
x0297:	lw	r2, [pat10]
x0299:	bn	r1, r2
x029a:	hlt	ERR_CODE

x029b:	lw	r1, [pat10]
x029d:	lw	r2, [pat01]
x029f:	bn	r1, r2
x02a0:	hlt	ERR_CODE

; bb

x02a1:	lwt	r1, 0
x02a2:	lwt	r2, 0
x02a3:	bb	r1, r2
x02a4:	hlt	ERR_CODE

x02a5:	lwt	r1, -1
x02a6:	lw	r2, r1
x02a7:	bb	r1, r2
x02a8:	hlt	ERR_CODE

x02a9:	lwt	r1, -1
x02aa:	lwt	r2, 0
x02ab:	bb	r1, r2
x02ac:	hlt	ERR_CODE

x02ad:	lwt	r1, 0
x02ae:	lwt	r2, -1
x02af:	bb	r1, r2
x02b0:	ujs	1
x02b1:	hlt	ERR_CODE

x02b2:	lw	r1, [pat01]
x02b4:	lw	r2, r1
x02b5:	bb	r1, r2
x02b6:	hlt	ERR_CODE

x02b7:	lw	r1, [pat10]
x02b9:	lw	r2, r1
x02ba:	bb	r1, r2
x02bb:	hlt	ERR_CODE

x02bc:	lw	r1, [pat01]
x02be:	lw	r2, [pat10]
x02c0:	bb	r1, r2
x02c1:	ujs	1
x02c2:	hlt	ERR_CODE

x02c3:	lw	r1, [pat10]
x02c5:	lw	r2, [pat01]
x02c7:	bb	r1, r2
x02c8:	ujs	1
x02c9:	hlt	ERR_CODE

; bs

x02ca:	lwt	r1, 0
x02cb:	lwt	r7, 0
x02cc:	lwt	r2, -1
x02cd:	bs	r1, r2
x02ce:	hlt	ERR_CODE

x02cf:	lw	r1, [pat10]
x02d1:	lwt	r7, -1
x02d2:	lw	r2, [pat01]
x02d4:	bs	r1, r2
x02d5:	ujs	1
x02d6:	hlt	ERR_CODE

x02d7:	lwt	r1, -1
x02d8:	lwt	r2, -1
x02d9:	lw	r7, [pat01]
x02db:	bs	r1, r2
x02dc:	hlt	ERR_CODE

x02dd:	lwt	r1, -1
x02de:	lwt	r2, -1
x02df:	lw	r7, [pat10]
x02e1:	bs	r1, r2
x02e2:	hlt	ERR_CODE

; or

x02e3:	lwt	r0, 0
x02e4:	lwt	r1, 0
x02e5:	lwt	r2, -1
x02e6:	or	r1, r2
x02e7:	cwt	r0, 0
x02e8:	jes	1
x02e9:	hlt	ERR_CODE
x02ea:	cwt	r1, -1
x02eb:	jes	1
x02ec:	hlt	ERR_CODE

x02ed:	lwt	r0, 0
x02ee:	lwt	r1, -1
x02ef:	lwt	r2, 0
x02f0:	or	r1, r2
x02f1:	cwt	r0, 0
x02f2:	jes	1
x02f3:	hlt	ERR_CODE
x02f4:	cwt	r1, -1
x02f5:	jes	1
x02f6:	hlt	ERR_CODE

x02f7:	lwt	r0, 0
x02f8:	lwt	r1, 0
x02f9:	lwt	r2, 0
x02fa:	or	r1, r2
x02fb:	cw	r0, ?Z
x02fd:	jes	1
x02fe:	hlt	ERR_CODE
x02ff:	cwt	r1, 0
x0300:	jes	1
x0301:	hlt	ERR_CODE

x0302:	lwt	r0, 0
x0303:	lwt	r1, -1
x0304:	lwt	r2, -1
x0305:	or	r1, r2
x0306:	cwt	r0, 0
x0307:	jes	1
x0308:	hlt	ERR_CODE
x0309:	cwt	r1, -1
x030a:	jes	1
x030b:	hlt	ERR_CODE

x030c:	lwt	r0, 0
x030d:	lw	r1, [pat01]
x030f:	lw	r2, r1
x0310:	or	r1, r2
x0311:	cwt	r0, 0
x0312:	jes	1
x0313:	hlt	ERR_CODE
x0314:	cw	r1, [pat01]
x0316:	jes	1
x0317:	hlt	ERR_CODE

x0318:	lwt	r0, 0
x0319:	lw	r1, [pat10]
x031b:	lw	r2, r1
x031c:	or	r1, r2
x031d:	cwt	r0, 0
x031e:	jes	1
x031f:	hlt	ERR_CODE
x0320:	cw	r1, [pat10]
x0322:	jes	1
x0323:	hlt	ERR_CODE

x0324:	lwt	r0, 0
x0325:	lw	r1, [pat10]
x0327:	or	r1, [pat01]
x0329:	cwt	r0, 0
x032a:	jes	1
x032b:	hlt	ERR_CODE
x032c:	cwt	r1, -1
x032d:	jes	1
x032e:	hlt	ERR_CODE

x032f:	lwt	r0, 0
x0330:	lw	r1, [pat01]
x0332:	or	r1, [pat10]
x0334:	cwt	r0, 0
x0335:	jes	1
x0336:	hlt	ERR_CODE
x0337:	cwt	r1, -1
x0338:	jes	1
x0339:	hlt	ERR_CODE

; om

x033a:	lwt	r0, 0
x033b:	lwt	r1, -1
x033c:	rw	r1, temp3
x033e:	lwt	r1, 0
x033f:	om	r1, temp3
x0341:	cwt	r0, 0
x0342:	jes	1
x0343:	hlt	ERR_CODE
x0344:	lw	r1, [temp3]
x0346:	cwt	r1, -1
x0347:	jes	1
x0348:	hlt	ERR_CODE

x0349:	lwt	r0, 0
x034a:	rz	temp3
x034c:	lwt	r1, -1
x034d:	om	r1, temp3
x034f:	cwt	r0, 0
x0350:	jes	1
x0351:	hlt	ERR_CODE
x0352:	lw	r1, [temp3]
x0354:	cwt	r1, -1
x0355:	jes	1
x0356:	hlt	ERR_CODE

x0357:	lwt	r0, 0
x0358:	rz	temp3
x035a:	lwt	r1, 0
x035b:	om	r1, temp3
x035d:	cw	r0, ?Z
x035f:	jes	1
x0360:	hlt	ERR_CODE
x0361:	lw	r1, [temp3]
x0363:	cwt	r1, 0
x0364:	jes	1
x0365:	hlt	ERR_CODE

x0366:	lwt	r0, 0
x0367:	lwt	r1, -1
x0368:	rw	r1, temp3
x036a:	om	r1, temp3
x036c:	cwt	r0, 0
x036d:	jes	1
x036e:	hlt	ERR_CODE
x036f:	lw	r1, [temp3]
x0371:	cwt	r1, -1
x0372:	jes	1
x0373:	hlt	ERR_CODE

x0374:	lwt	r0, 0
x0375:	lw	r1, [pat01]
x0377:	rw	r1, temp3
x0379:	om	r1, temp3
x037b:	cwt	r0, 0
x037c:	jes	1
x037d:	hlt	ERR_CODE
x037e:	lw	r1, [temp3]
x0380:	cw	r1, [pat01]
x0382:	jes	1
x0383:	hlt	ERR_CODE

x0384:	lwt	r0, 0
x0385:	lw	r1, [pat10]
x0387:	rw	r1, temp3
x0389:	om	r1, temp3
x038b:	cwt	r0, 0
x038c:	jes	1
x038d:	hlt	ERR_CODE
x038e:	lw	r1, [temp3]
x0390:	cw	r1, [pat10]
x0392:	jes	1
x0393:	hlt	ERR_CODE

x0394:	lwt	r0, 0
x0395:	lw	r1, [pat01]
x0397:	rw	r1, temp3
x0399:	lw	r1, [pat10]
x039b:	om	r1, temp3
x039d:	cwt	r0, 0
x039e:	jes	1
x039f:	hlt	ERR_CODE
x03a0:	lw	r1, [temp3]
x03a2:	cwt	r1, -1
x03a3:	jes	1
x03a4:	hlt	ERR_CODE

x03a5:	lwt	r0, 0
x03a6:	lw	r1, [pat10]
x03a8:	rw	r1, temp3
x03aa:	lw	r1, [pat01]
x03ac:	om	r1, temp3
x03ae:	cwt	r0, 0
x03af:	jes	1
x03b0:	hlt	ERR_CODE
x03b1:	lw	r1, [temp3]
x03b3:	cwt	r1, -1
x03b4:	jes	1
x03b5:	hlt	ERR_CODE

; nr

x03b6:	lwt	r0, 0
x03b7:	lwt	r1, 0
x03b8:	nr	r1, -1
x03ba:	cw	r0, ?Z
x03bc:	jes	1
x03bd:	hlt	ERR_CODE
x03be:	cwt	r1, 0
x03bf:	jes	1
x03c0:	hlt	ERR_CODE

x03c1:	lwt	r0, 0
x03c2:	lwt	r1, -1
x03c3:	nr	r1, 0
x03c5:	cw	r0, ?Z
x03c7:	jes	1
x03c8:	hlt	ERR_CODE
x03c9:	cwt	r1, 0
x03ca:	jes	1
x03cb:	hlt	ERR_CODE

x03cc:	lwt	r0, 0
x03cd:	lwt	r1, 0
x03ce:	lw	r2, r1
x03cf:	nr	r1, r2
x03d0:	cw	r0, ?Z
x03d2:	jes	1
x03d3:	hlt	ERR_CODE
x03d4:	cwt	r1, 0
x03d5:	jes	1
x03d6:	hlt	ERR_CODE

x03d7:	lwt	r0, 0
x03d8:	lwt	r1, -1
x03d9:	lw	r2, r1
x03da:	nr	r1, r2
x03db:	cwt	r0, 0
x03dc:	jes	1
x03dd:	hlt	ERR_CODE
x03de:	cwt	r1, -1
x03df:	jes	1
x03e0:	hlt	ERR_CODE

x03e1:	lwt	r0, 0
x03e2:	lw	r1, [pat01]
x03e4:	lw	r2, r1
x03e5:	nr	r1, r2
x03e6:	cwt	r0, 0
x03e7:	jes	1
x03e8:	hlt	ERR_CODE
x03e9:	cw	r1, [pat01]
x03eb:	jes	1
x03ec:	hlt	ERR_CODE

x03ed:	lwt	r0, 0
x03ee:	lw	r1, [pat10]
x03f0:	lw	r2, r1
x03f1:	nr	r1, r2
x03f2:	cwt	r0, 0
x03f3:	jes	1
x03f4:	hlt	ERR_CODE
x03f5:	cw	r1, [pat10]
x03f7:	jes	1
x03f8:	hlt	ERR_CODE

x03f9:	lwt	r0, 0
x03fa:	lw	r1, [pat10]
x03fc:	nr	r1, [pat01]
x03fe:	cw	r0, ?Z
x0400:	jes	1
x0401:	hlt	ERR_CODE
x0402:	cwt	r1, 0
x0403:	jes	1
x0404:	hlt	ERR_CODE

x0405:	lwt	r0, 0
x0406:	lw	r1, [pat01]
x0408:	nr	r1, [pat10]
x040a:	cw	r0, ?Z
x040c:	jes	1
x040d:	hlt	ERR_CODE
x040e:	cwt	r1, 0
x040f:	jes	1
x0410:	hlt	ERR_CODE

; nm

x0411:	lwt	r0, 0
x0412:	lwt	r1, -1
x0413:	rw	r1, temp3
x0415:	lwt	r1, 0
x0416:	nm	r1, temp3
x0418:	cw	r0, ?Z
x041a:	jes	1
x041b:	hlt	ERR_CODE
x041c:	lw	r1, [temp3]
x041e:	cwt	r1, 0
x041f:	jes	1
x0420:	hlt	ERR_CODE

x0421:	lwt	r0, 0
x0422:	lwt	r1, -1
x0423:	rz	temp3
x0425:	nm	r1, temp3
x0427:	cw	r0, ?Z
x0429:	jes	1
x042a:	hlt	ERR_CODE
x042b:	lw	r1, [temp3]
x042d:	cwt	r1, 0
x042e:	jes	1
x042f:	hlt	ERR_CODE

x0430:	lwt	r0, 0
x0431:	rz	temp3
x0433:	lwt	r1, 0
x0434:	nm	r1, temp3
x0436:	cw	r0, ?Z
x0438:	jes	1
x0439:	hlt	ERR_CODE
x043a:	lw	r1, [temp3]
x043c:	cwt	r1, 0
x043d:	jes	1
x043e:	hlt	ERR_CODE

x043f:	lwt	r0, 0
x0440:	lwt	r1, -1
x0441:	rw	r1, temp3
x0443:	nm	r1, temp3
x0445:	cwt	r0, 0
x0446:	jes	1
x0447:	hlt	ERR_CODE
x0448:	lw	r1, [temp3]
x044a:	cwt	r1, -1
x044b:	jes	1
x044c:	hlt	ERR_CODE

x044d:	lwt	r0, 0
x044e:	lw	r1, [pat01]
x0450:	rw	r1, temp3
x0452:	nm	r1, temp3
x0454:	cwt	r0, 0
x0455:	jes	1
x0456:	hlt	ERR_CODE
x0457:	lw	r1, [temp3]
x0459:	cw	r1, [pat01]
x045b:	jes	1
x045c:	hlt	ERR_CODE

x045d:	lwt	r0, 0
x045e:	lw	r1, [pat10]
x0460:	rw	r1, temp3
x0462:	nm	r1, temp3
x0464:	cwt	r0, 0
x0465:	jes	1
x0466:	hlt	ERR_CODE
x0467:	lw	r1, [temp3]
x0469:	cw	r1, [pat10]
x046b:	jes	1
x046c:	hlt	ERR_CODE

x046d:	lwt	r0, 0
x046e:	lw	r1, [pat10]
x0470:	rw	r1, temp3
x0472:	lw	r1, [pat01]
x0474:	nm	r1, temp3
x0476:	cw	r0, ?Z
x0478:	jes	1
x0479:	hlt	ERR_CODE
x047a:	lw	r1, [temp3]
x047c:	cwt	r1, 0
x047d:	jes	1
x047e:	hlt	ERR_CODE

x047f:	lwt	r0, 0
x0480:	lw	r1, [pat01]
x0482:	rw	r1, temp3
x0484:	lw	r1, [pat10]
x0486:	nm	r1, temp3
x0488:	cw	r0, ?Z
x048a:	jes	1
x048b:	hlt	ERR_CODE
x048c:	lw	r1, [temp3]
x048e:	cwt	r1, 0
x048f:	jes	1
x0490:	hlt	ERR_CODE

; er

x0491:	lwt	r0, 0
x0492:	lwt	r1, 0
x0493:	lwt	r2, -1
x0494:	er	r1, r2
x0495:	cw	r0, ?Z
x0497:	jes	1
x0498:	hlt	ERR_CODE
x0499:	cwt	r1, 0
x049a:	jes	1
x049b:	hlt	ERR_CODE

x049c:	lwt	r0, 0
x049d:	lwt	r1, -1
x049e:	lwt	r2, 0
x049f:	er	r1, r2
x04a0:	cwt	r0, 0
x04a1:	jes	1
x04a2:	hlt	ERR_CODE
x04a3:	cwt	r1, -1
x04a4:	jes	1
x04a5:	hlt	ERR_CODE

x04a6:	lwt	r0, 0
x04a7:	lwt	r1, 0
x04a8:	lwt	r2, 0
x04a9:	er	r1, r2
x04aa:	cw	r0, ?Z
x04ac:	jes	1
x04ad:	hlt	ERR_CODE
x04ae:	cwt	r1, 0
x04af:	jes	1
x04b0:	hlt	ERR_CODE

x04b1:	lwt	r0, 0
x04b2:	lwt	r1, -1
x04b3:	lwt	r2, -1
x04b4:	er	r1, r2
x04b5:	cw	r0, ?Z
x04b7:	jes	1
x04b8:	hlt	ERR_CODE
x04b9:	cwt	r1, 0
x04ba:	jes	1
x04bb:	hlt	ERR_CODE

x04bc:	lwt	r0, 0
x04bd:	lw	r1, [pat01]
x04bf:	lw	r2, r1
x04c0:	er	r1, r2
x04c1:	cw	r0, ?Z
x04c3:	jes	1
x04c4:	hlt	ERR_CODE
x04c5:	cwt	r1, 0
x04c6:	jes	1
x04c7:	hlt	ERR_CODE

x04c8:	lwt	r0, 0
x04c9:	lw	r1, [pat10]
x04cb:	lw	r2, r1
x04cc:	er	r1, r2
x04cd:	cw	r0, ?Z
x04cf:	jes	1
x04d0:	hlt	ERR_CODE
x04d1:	cwt	r1, 0
x04d2:	jes	1
x04d3:	hlt	ERR_CODE

x04d4:	lwt	r0, 0
x04d5:	lw	r1, [pat01]
x04d7:	lw	r2, [pat10]
x04d9:	er	r1, r2
x04da:	cwt	r0, 0
x04db:	jes	1
x04dc:	hlt	ERR_CODE
x04dd:	cw	r1, [pat01]
x04df:	jes	1
x04e0:	hlt	ERR_CODE

x04e1:	lwt	r0, 0
x04e2:	lw	r1, [pat10]
x04e4:	lw	r2, [pat01]
x04e6:	er	r1, r2
x04e7:	cwt	r0, 0
x04e8:	jes	1
x04e9:	hlt	ERR_CODE
x04ea:	cw	r1, [pat10]
x04ec:	jes	1
x04ed:	hlt	ERR_CODE

; em

x04ee:	lwt	r0, 0
x04ef:	lwt	r1, -1
x04f0:	rw	r1, temp3
x04f2:	lwt	r1, 0
x04f3:	em	r1, temp3
x04f5:	cwt	r0, 0
x04f6:	jes	1
x04f7:	hlt	ERR_CODE
x04f8:	lw	r1, [temp3]
x04fa:	cwt	r1, -1
x04fb:	jes	1
x04fc:	hlt	ERR_CODE

x04fd:	lwt	r0, 0
x04fe:	lwt	r1, 0
x04ff:	rw	r1, temp3
x0501:	lwt	r1, -1
x0502:	em	r1, temp3
x0504:	cw	r0, ?Z
x0506:	jes	1
x0507:	hlt	ERR_CODE
x0508:	lw	r1, [temp3]
x050a:	cwt	r1, 0
x050b:	jes	1
x050c:	hlt	ERR_CODE

x050d:	lwt	r0, 0
x050e:	lwt	r1, 0
x050f:	rw	r1, temp3
x0511:	em	r1, temp3
x0513:	cw	r0, ?Z
x0515:	jes	1
x0516:	hlt	ERR_CODE
x0517:	lw	r1, [temp3]
x0519:	cwt	r1, 0
x051a:	jes	1
x051b:	hlt	ERR_CODE

x051c:	lwt	r0, 0
x051d:	lwt	r1, -1
x051e:	rw	r1, temp3
x0520:	em	r1, temp3
x0522:	cw	r0, ?Z
x0524:	jes	1
x0525:	hlt	ERR_CODE
x0526:	lw	r1, [temp3]
x0528:	cwt	r1, 0
x0529:	jes	1
x052a:	hlt	ERR_CODE

x052b:	lwt	r0, 0
x052c:	lw	r1, [pat01]
x052e:	rw	r1, temp3
x0530:	em	r1, temp3
x0532:	cw	r0, ?Z
x0534:	jes	1
x0535:	hlt	ERR_CODE
x0536:	lw	r1, [temp3]
x0538:	cwt	r1, 0
x0539:	jes	1
x053a:	hlt	ERR_CODE

x053b:	lwt	r0, 0
x053c:	lw	r1, [pat10]
x053e:	rw	r1, temp3
x0540:	em	r1, temp3
x0542:	cw	r0, ?Z
x0544:	jes	1
x0545:	hlt	ERR_CODE
x0546:	lw	r1, [temp3]
x0548:	cwt	r1, 0
x0549:	jes	1
x054a:	hlt	ERR_CODE

x054b:	lwt	r0, 0
x054c:	lw	r1, [pat10]
x054e:	rw	r1, temp3
x0550:	lw	r1, [pat01]
x0552:	em	r1, temp3
x0554:	cwt	r0, 0
x0555:	jes	1
x0556:	hlt	ERR_CODE
x0557:	lw	r1, [temp3]
x0559:	cw	r1, [pat10]
x055b:	jes	1
x055c:	hlt	ERR_CODE

x055d:	lwt	r0, 0
x055e:	lw	r1, [pat01]
x0560:	rw	r1, temp3
x0562:	lw	r1, [pat10]
x0564:	em	r1, temp3
x0566:	cwt	r0, 0
x0567:	jes	1
x0568:	hlt	ERR_CODE
x0569:	lw	r1, [temp3]
x056b:	cw	r1, [pat01]
x056d:	jes	1
x056e:	hlt	ERR_CODE

; xr

x056f:	lwt	r0, 0
x0570:	lwt	r1, 0
x0571:	lwt	r2, -1
x0572:	xr	r1, r2
x0573:	cwt	r0, 0
x0574:	jes	1
x0575:	hlt	ERR_CODE
x0576:	cwt	r1, -1
x0577:	jes	1
x0578:	hlt	ERR_CODE

x0579:	lwt	r0, 0
x057a:	lwt	r1, -1
x057b:	lwt	r2, 0
x057c:	xr	r1, r2
x057d:	cwt	r0, 0
x057e:	jes	1
x057f:	hlt	ERR_CODE
x0580:	cwt	r1, -1
x0581:	jes	1
x0582:	hlt	ERR_CODE

x0583:	lwt	r0, 0
x0584:	lwt	r1, 0
x0585:	lwt	r2, 0
x0586:	xr	r1, r2
x0587:	cw	r0, ?Z
x0589:	jes	1
x058a:	hlt	ERR_CODE
x058b:	cwt	r1, 0
x058c:	jes	1
x058d:	hlt	ERR_CODE

x058e:	lwt	r0, 0
x058f:	lwt	r1, -1
x0590:	lwt	r2, -1
x0591:	xr	r1, r2
x0592:	cw	r0, ?Z
x0594:	jes	1
x0595:	hlt	ERR_CODE
x0596:	cwt	r1, 0
x0597:	jes	1
x0598:	hlt	ERR_CODE

x0599:	lwt	r0, 0
x059a:	lw	r1, [pat10]
x059c:	xr	r1, [pat10]
x059e:	cw	r0, ?Z
x05a0:	jes	1
x05a1:	hlt	ERR_CODE
x05a2:	cwt	r1, 0
x05a3:	jes	1
x05a4:	hlt	ERR_CODE

x05a5:	lwt	r0, 0
x05a6:	lw	r1, [pat01]
x05a8:	xr	r1, [pat01]
x05aa:	cw	r0, ?Z
x05ac:	jes	1
x05ad:	hlt	ERR_CODE
x05ae:	cwt	r1, 0
x05af:	jes	1
x05b0:	hlt	ERR_CODE

x05b1:	lwt	r0, 0
x05b2:	lw	r1, [pat01]
x05b4:	lw	r2, [pat10]
x05b6:	xr	r1, r2
x05b7:	cwt	r0, 0
x05b8:	jes	1
x05b9:	hlt	ERR_CODE
x05ba:	cwt	r1, -1
x05bb:	jes	1
x05bc:	hlt	ERR_CODE

x05bd:	lwt	r0, 0
x05be:	lw	r1, [pat10]
x05c0:	lw	r2, [pat01]
x05c2:	xr	r1, r2
x05c3:	cwt	r0, 0
x05c4:	jes	1
x05c5:	hlt	ERR_CODE
x05c6:	cwt	r1, -1
x05c7:	jes	1
x05c8:	hlt	ERR_CODE

; xm

x05c9:	lwt	r0, 0
x05ca:	lwt	r1, -1
x05cb:	rw	r1, temp3
x05cd:	lwt	r1, 0
x05ce:	xm	r1, temp3
x05d0:	cwt	r0, 0
x05d1:	jes	1
x05d2:	hlt	ERR_CODE
x05d3:	lw	r1, [temp3]
x05d5:	cwt	r1, -1
x05d6:	jes	1
x05d7:	hlt	ERR_CODE

x05d8:	lwt	r0, 0
x05d9:	rz	temp3
x05db:	lwt	r1, -1
x05dc:	xm	r1, temp3
x05de:	cwt	r0, 0
x05df:	jes	1
x05e0:	hlt	ERR_CODE
x05e1:	lw	r1, [temp3]
x05e3:	cwt	r1, -1
x05e4:	jes	1
x05e5:	hlt	ERR_CODE

x05e6:	lwt	r0, 0
x05e7:	rz	temp3
x05e9:	lwt	r1, 0
x05ea:	xm	r1, temp3
x05ec:	cw	r0, ?Z
x05ee:	jes	1
x05ef:	hlt	ERR_CODE
x05f0:	lw	r1, [temp3]
x05f2:	cwt	r1, 0
x05f3:	jes	1
x05f4:	hlt	ERR_CODE

x05f5:	lwt	r0, 0
x05f6:	lwt	r1, -1
x05f7:	rw	r1, temp3
x05f9:	xm	r1, temp3
x05fb:	cw	r0, ?Z
x05fd:	jes	1
x05fe:	hlt	ERR_CODE
x05ff:	lw	r1, [temp3]
x0601:	cwt	r1, 0
x0602:	jes	1
x0603:	hlt	ERR_CODE

x0604:	lwt	r0, 0
x0605:	lw	r1, [pat01]
x0607:	rw	r1, temp3
x0609:	xm	r1, temp3
x060b:	cw	r0, ?Z
x060d:	jes	1
x060e:	hlt	ERR_CODE
x060f:	lw	r1, [temp3]
x0611:	cwt	r1, 0
x0612:	jes	1
x0613:	hlt	ERR_CODE

x0614:	lwt	r0, 0
x0615:	lw	r1, [pat10]
x0617:	rw	r1, temp3
x0619:	xm	r1, temp3
x061b:	cw	r0, ?Z
x061d:	jes	1
x061e:	hlt	ERR_CODE
x061f:	lw	r1, [temp3]
x0621:	cwt	r1, 0
x0622:	jes	1
x0623:	hlt	ERR_CODE

x0624:	lwt	r0, 0
x0625:	lw	r1, [pat10]
x0627:	rw	r1, temp3
x0629:	lw	r1, [pat01]
x062b:	xm	r1, temp3
x062d:	cwt	r0, 0
x062e:	jes	1
x062f:	hlt	ERR_CODE
x0630:	lw	r1, [temp3]
x0632:	cwt	r1, -1
x0633:	jes	1
x0634:	hlt	ERR_CODE

x0635:	lwt	r0, 0
x0636:	lw	r1, [pat01]
x0638:	rw	r1, temp3
x063a:	lw	r1, [pat10]
x063c:	xm	r1, temp3
x063e:	cwt	r0, 0
x063f:	jes	1
x0640:	hlt	ERR_CODE
x0641:	lw	r1, [temp3]
x0643:	cwt	r1, -1
x0644:	jes	1
x0645:	hlt	ERR_CODE

; aw

x0646:	lwt	r0, 0
x0647:	lwt	r1, -1
x0648:	lwt	r2, 1
x0649:	aw	r1, r2
x064a:	cw	r0, ?ZC
x064c:	jes	1
x064d:	hlt	ERR_CODE
x064e:	cwt	r1, 0
x064f:	jes	1
x0650:	hlt	ERR_CODE

x0651:	lwt	r0, 0
x0652:	lw	r1, [pat10]
x0654:	aw	r1, r1
x0655:	cw	r0, ?MVC
x0657:	jes	1
x0658:	hlt	ERR_CODE
x0659:	cw	r1, 0x5554
x065b:	jes	1
x065c:	hlt	ERR_CODE

x065d:	lwt	r0, 0
x065e:	lw	r1, 0x5555
x0660:	aw	r1, r1
x0661:	cw	r0, ?V
x0663:	jes	1
x0664:	hlt	ERR_CODE
x0665:	cw	r1, 0xaaaa
x0667:	jes	1
x0668:	hlt	ERR_CODE

x0669:	lwt	r0, 0
x066a:	lwt	r1, -1
x066b:	lw	r2, 0x7fff
x066d:	aw	r1, r2
x066e:	cw	r0, ?C
x0670:	jes	1
x0671:	hlt	ERR_CODE
x0672:	cw	r1, 0x7ffe
x0674:	jes	1
x0675:	hlt	ERR_CODE

x0676:	lwt	r1, -1
x0677:	lwt	r0, 0
x0678:	lw	r2, r1
x0679:	aw	r1, r2
x067a:	cw	r0, ?MC
x067c:	jes	1
x067d:	hlt	ERR_CODE
x067e:	cwt	r1, -2
x067f:	jes	1
x0680:	hlt	ERR_CODE

x0681:	lwt	r0, 0
x0682:	lw	r1, 0x7fff
x0684:	lw	r2, 0x7fff
x0686:	aw	r1, r2
x0687:	cw	r0, ?V
x0689:	jes	1
x068a:	hlt	ERR_CODE
x068b:	cwt	r1, -2
x068c:	jes	1
x068d:	hlt	ERR_CODE

x068e:	lwt	r0, 0
x068f:	lwt	r1, -1
x0690:	lw	r2, 0x8000
x0692:	aw	r1, r2
x0693:	cw	r0, ?MVC
x0695:	jes	1
x0696:	hlt	ERR_CODE
x0697:	cw	r1, 0x7fff
x0699:	jes	1
x069a:	hlt	ERR_CODE

x069b:	lwt	r0, 0
x069c:	lw	r1, 0x8000
x069e:	lw	r2, r1
x069f:	aw	r1, r2
x06a0:	cw	r0, ?MVC
x06a2:	jes	1
x06a3:	hlt	ERR_CODE
x06a4:	cwt	r1, 0
x06a5:	jes	1
x06a6:	hlt	ERR_CODE

; ac

x06a7:	lw	r0, ?C
x06a9:	lw	r1, 0xfffd
x06ab:	lwt	r2, 1
x06ac:	ac	r1, r2
x06ad:	cw	r0, ?M
x06af:	jes	1
x06b0:	hlt	ERR_CODE
x06b1:	cwt	r1, -1
x06b2:	jes	1
x06b3:	hlt	ERR_CODE

x06b4:	lw	r0, ?C
x06b6:	lw	r1, 0xfffe
x06b8:	lwt	r2, 1
x06b9:	ac	r1, r2
x06ba:	cw	r0, ?ZC
x06bc:	jes	1
x06bd:	hlt	ERR_CODE
x06be:	cwt	r1, 0
x06bf:	jes	1
x06c0:	hlt	ERR_CODE

x06c1:	lw	r0, ?C
x06c3:	lw	r1, 0x7fff
x06c5:	ac	r1, 0
x06c7:	cw	r0, ?V
x06c9:	jes	1
x06ca:	hlt	ERR_CODE
x06cb:	cw	r1, 0x8000
x06cd:	jes	1
x06ce:	hlt	ERR_CODE

; sw

x06cf:	lwt	r0, 0
x06d0:	lwt	r1, 0
x06d1:	lw	r2, r1
x06d2:	sw	r1, r2
x06d3:	cw	r0, ?ZC
x06d5:	jes	1
x06d6:	hlt	ERR_CODE
x06d7:	cwt	r1, 0
x06d8:	jes	1
x06d9:	hlt	ERR_CODE

x06da:	lwt	r0, 0
x06db:	lw	r1, 0x8000
x06dd:	lwt	r2, 1
x06de:	sw	r1, r2
x06df:	cw	r0, ?MVC
x06e1:	jes	1
x06e2:	hlt	ERR_CODE
x06e3:	cw	r1, 0x7fff
x06e5:	jes	1
x06e6:	hlt	ERR_CODE

; cl

x06e7:	lwt	r0, 0
x06e8:	lwt	r1, 0
x06e9:	lwt	r2, -1
x06ea:	cl	r1, r2
x06eb:	cw	r0, ?L
x06ed:	jes	1
x06ee:	hlt	ERR_CODE

x06ef:	cl	r2, r1
x06f0:	cw	r0, ?G
x06f2:	jes	1
x06f3:	hlt	ERR_CODE

x06f4:	lw	r1, 0x7fff
x06f6:	lw	r2, 0x8000
x06f8:	cl	r1, r2
x06f9:	cw	r0, ?L
x06fb:	jes	1
x06fc:	hlt	ERR_CODE

x06fd:	lw	r1, [pat10]
x06ff:	lw	r2, r1
x0700:	cl	r1, r2
x0701:	cw	r0, ?E
x0703:	jes	1
x0704:	hlt	ERR_CODE

; lj

x0705:	lwt	r1, 0
x0706:	lj	lj_test

lj_return:
x0708:	cw	r1, lj_return
x070a:	jes	1
x070b:	hlt	ERR_CODE
x070c:	ujs	uj1_test

lj_test:

x070d:	.res	1
x070e:	lw	r1, [lj_test]
x0710:	cw	r1, lj_return
x0712:	jes	1
x0713:	hlt	ERR_CODE

x0714:	lw	r1, lj_return
x0716:	ujs	lj_return

; uj

uj1_test:

x0717:	uj	uj2_test
x0719:	hlt	ERR_CODE

uj2_test:

x071a:	lw	r1, uj3_test
x071c:	uj	r1
x071d:	hlt	ERR_CODE

uj3_test:

x071e:	lw	r1, uj4_test
x0720:	rw	r1, temp3
x0722:	uj	[temp3]
x0724:	hlt	ERR_CODE

uj4_test:

x0725:	lw	r1, ld_test
x0727:	rw	r1, temp3
x0729:	lw	r1, temp3
x072b:	uj	[r1]
x072c:	hlt	ERR_CODE

; ld

ld_test:

x072d:	lj	all_regs_load_minus1
x072f:	ld	pat01
x0731:	cw	r1, [pat01]
x0733:	jes	1
x0734:	hlt	ERR_CODE
x0735:	cw	r2, [pat01+1]
x0737:	jes	1
x0738:	hlt	ERR_CODE
x0739:	cwt	r3, -1
x073a:	jes	1
x073b:	ujs	ld_err
x073c:	cwt	r4, -1
x073d:	jes	1
x073e:	ujs	ld_err
x073f:	cwt	r5, -1
x0740:	jes	1
x0741:	ujs	ld_err
x0742:	cwt	r6, -1
x0743:	jes	1
x0744:	ujs	ld_err
x0745:	cwt	r7, -1
x0746:	jes	1
ld_err:
x0747:	hlt	ERR_CODE

; lf

x0748:	lj	all_regs_load_minus1
x074a:	lf	pat01
x074c:	cw	r1, [pat01]
x074e:	jes	1
x074f:	hlt	ERR_CODE
x0750:	cw	r2, [pat01+1]
x0752:	jes	1
x0753:	hlt	ERR_CODE
x0754:	cw	r3, [pat01+2]
x0756:	jes	1
x0757:	hlt	ERR_CODE
x0758:	cwt	r4, -1
x0759:	jes	1
x075a:	ujs	lf_err
x075b:	cwt	r5, -1
x075c:	jes	1
x075d:	ujs	lf_err
x075e:	cwt	r6, -1
x075f:	jes	1
x0760:	ujs	lf_err
x0761:	cwt	r7, -1
x0762:	jes	1
lf_err:
x0763:	hlt	ERR_CODE

x0764:	lj	all_regs_load_minus1
x0766:	la	pat01
x0768:	cw	r1, [pat01]
x076a:	jes	1
x076b:	hlt	ERR_CODE
x076c:	cw	r2, [pat01+1]
x076e:	jes	1
x076f:	hlt	ERR_CODE
x0770:	cw	r3, [pat01+2]
x0772:	jes	1
x0773:	hlt	ERR_CODE
x0774:	cw	r4, [pat01+3]
x0776:	jes	1
x0777:	hlt	ERR_CODE
x0778:	cw	r5, [pat01+4]
x077a:	jes	1
x077b:	hlt	ERR_CODE
x077c:	cw	r6, [pat01+5]
x077e:	jes	1
x077f:	hlt	ERR_CODE
x0780:	cw	r7, [pat01+6]
x0782:	jes	1
x0783:	hlt	ERR_CODE

; ll

x0784:	lj	all_regs_load_minus1
x0786:	ll	pat01
x0788:	cw	r5, [pat01]
x078a:	jes	1
x078b:	hlt	ERR_CODE
x078c:	cw	r6, [pat01+1]
x078e:	jes	1
x078f:	hlt	ERR_CODE
x0790:	cw	r7, [pat01+2]
x0792:	jes	1
x0793:	hlt	ERR_CODE
x0794:	cwt	r1, -1
x0795:	jes	1
x0796:	ujs	ll_err
x0797:	cwt	r2, -1
x0798:	jes	1
x0799:	ujs	ll_err
x079a:	cwt	r3, -1
x079b:	jes	1
x079c:	ujs	ll_err
x079d:	cwt	r4, -1
x079e:	jes	1
ll_err:
x079f:	hlt	ERR_CODE

; td

x07a0:	lj	all_regs_load_minus1
x07a2:	td	pat01
x07a4:	cw	r1, [pat01]
x07a6:	jes	1
x07a7:	hlt	ERR_CODE
x07a8:	cw	r2, [pat01+1]
x07aa:	jes	1
x07ab:	hlt	ERR_CODE
x07ac:	cwt	r3, -1
x07ad:	jes	1
x07ae:	ujs	td_err
x07af:	cwt	r4, -1
x07b0:	jes	1
x07b1:	ujs	td_err
x07b2:	cwt	r5, -1
x07b3:	jes	1
x07b4:	ujs	td_err
x07b5:	cwt	r6, -1
x07b6:	jes	1
x07b7:	ujs	td_err
x07b8:	cwt	r7, -1
x07b9:	jes	1
td_err:
x07ba:	hlt	ERR_CODE

; tf

x07bb:	lj	all_regs_load_minus1
x07bd:	tf	pat01
x07bf:	cw	r1, [pat01]
x07c1:	jes	1
x07c2:	hlt	ERR_CODE
x07c3:	cw	r2, [pat01+1]
x07c5:	jes	1
x07c6:	hlt	ERR_CODE
x07c7:	cw	r3, [pat01+2]
x07c9:	jes	1
x07ca:	hlt	ERR_CODE
x07cb:	cwt	r4, -1
x07cc:	jes	1
x07cd:	ujs	tf_err
x07ce:	cwt	r5, -1
x07cf:	jes	1
x07d0:	ujs	tf_err
x07d1:	cwt	r6, -1
x07d2:	jes	1
x07d3:	ujs	tf_err
x07d4:	cwt	r7, -1
x07d5:	jes	1
tf_err:
x07d6:	hlt	ERR_CODE

; ta

x07d7:	lj	all_regs_load_minus1
x07d9:	ta	pat01
x07db:	cw	r1, [pat01]
x07dd:	jes	1
x07de:	hlt	ERR_CODE
x07df:	cw	r2, [pat01+1]
x07e1:	jes	1
x07e2:	hlt	ERR_CODE
x07e3:	cw	r3, [pat01+2]
x07e5:	jes	1
x07e6:	hlt	ERR_CODE
x07e7:	cw	r4, [pat01+3]
x07e9:	jes	1
x07ea:	hlt	ERR_CODE
x07eb:	cw	r5, [pat01+4]
x07ed:	jes	1
x07ee:	hlt	ERR_CODE
x07ef:	cw	r6, [pat01+5]
x07f1:	jes	1
x07f2:	hlt	ERR_CODE
x07f3:	cw	r7, [pat01+6]
x07f5:	jes	1
x07f6:	hlt	ERR_CODE

; tl

x07f7:	lj	all_regs_load_minus1
x07f9:	tl	pat01
x07fb:	cw	r5, [pat01]
x07fd:	jes	1
x07fe:	hlt	ERR_CODE
x07ff:	cw	r6, [pat01+1]
x0801:	jes	1
x0802:	hlt	ERR_CODE
x0803:	cw	r7, [pat01+2]
x0805:	jes	1
x0806:	hlt	ERR_CODE
x0807:	cwt	r1, -1
x0808:	jes	1
x0809:	ujs	tl_err
x080a:	cwt	r2, -1
x080b:	jes	1
x080c:	ujs	tl_err
x080d:	cwt	r3, -1
x080e:	jes	1
x080f:	ujs	tl_err
x0810:	cwt	r4, -1
x0811:	jes	1
tl_err:
x0812:	hlt	ERR_CODE

; rd, rf, ra, rl
; pd, pf, pa, pl

x0813:	ta	seven_regs
x0815:	rd	temp3
x0817:	ld	temp3
x0819:	lj	compare_all_regs
x081b:	ta	seven_regs
x081d:	rf	temp3
x081f:	lf	temp3
x0821:	lj	compare_all_regs
x0823:	ta	seven_regs
x0825:	ra	temp3
x0827:	la	temp3
x0829:	lj	compare_all_regs
x082b:	ta	seven_regs
x082d:	rl	temp3
x082f:	ll	temp3
x0831:	lj	compare_all_regs
x0833:	ta	seven_regs
x0835:	pd	temp3
x0837:	ld	temp3
x0839:	lj	compare_all_regs
x083b:	ta	seven_regs
x083d:	pf	temp3
x083f:	lf	temp3
x0841:	lj	compare_all_regs
x0843:	ta	seven_regs
x0845:	pa	temp3
x0847:	la	temp3
x0849:	lj	compare_all_regs
x084b:	ta	seven_regs
x084d:	pl	temp3
x084f:	ll	temp3
x0851:	lj	compare_all_regs
x0853:	ujs	awt_test

; ------------------------------------------------------------------------
all_regs_load_minus1:
x0854:	.res	1
x0855:	lwt	r1, -1
x0856:	lwt	r2, -1
x0857:	lwt	r3, -1
x0858:	lwt	r4, -1
x0859:	lwt	r5, -1
x085a:	lwt	r6, -1
x085b:	lwt	r7, -1
x085c:	uj	[all_regs_load_minus1]

; ------------------------------------------------------------------------
compare_all_regs:
x085e:	.res	1
x085f:	cw	r1, [seven_regs]
x0861:	jes	1
x0862:	hlt	ERR_CODE
x0863:	cw	r2, [seven_regs+1]
x0865:	jes	1
x0866:	hlt	ERR_CODE
x0867:	cw	r3, [seven_regs+2]
x0869:	jes	1
x086a:	hlt	ERR_CODE
x086b:	cw	r4, [seven_regs+3]
x086d:	jes	1
x086e:	hlt	ERR_CODE
x086f:	cw	r5, [seven_regs+4]
x0871:	jes	1
x0872:	hlt	ERR_CODE
x0873:	cw	r6, [seven_regs+5]
x0875:	jes	1
x0876:	hlt	ERR_CODE
x0877:	cw	r7, [seven_regs+6]
x0879:	jes	1
x087a:	hlt	ERR_CODE
x087b:	uj	[compare_all_regs]

; awt

awt_test:

x087d:	lwt	r0, 0
x087e:	lwt	r1, -3
x087f:	awt	r1, 1
x0880:	cw	r0, ?M
x0882:	jes	1
x0883:	hlt	ERR_CODE
x0884:	cwt	r1, -2
x0885:	jes	1
x0886:	hlt	ERR_CODE

x0887:	lwt	r0, 0
x0888:	lwt	r1, -1
x0889:	awt	r1, -1
x088a:	cw	r0, ?MC
x088c:	jes	1
x088d:	hlt	ERR_CODE
x088e:	cwt	r1, -2
x088f:	jes	1
x0890:	hlt	ERR_CODE

x0891:	lwt	r0, 0
x0892:	lwt	r1, -1
x0893:	awt	r1, 1
x0894:	cw	r0, ?ZC
x0896:	jes	1
x0897:	hlt	ERR_CODE
x0898:	cwt	r1, 0
x0899:	jes	1
x089a:	hlt	ERR_CODE

x089b:	lwt	r0, 0
x089c:	lw	r1, 0x7fff
x089e:	awt	r1, 1
x089f:	cw	r0, ?V
x08a1:	jes	1
x08a2:	hlt	ERR_CODE
x08a3:	cw	r1, 0x8000
x08a5:	jes	1
x08a6:	hlt	ERR_CODE

; irb

x08a7:	lwt	r1, -2
x08a8:	irb	r1, 1
x08a9:	hlt	ERR_CODE
x08aa:	cwt	r1, -1
x08ab:	jes	1
x08ac:	hlt	ERR_CODE

x08ad:	lwt	r1, -1
x08ae:	irb	r1, 1
x08af:	ujs	1
x08b0:	hlt	ERR_CODE
x08b1:	cwt	r1, 0
x08b2:	jes	1
x08b3:	hlt	ERR_CODE

; drb

x08b4:	lwt	r1, 2
x08b5:	drb	r1, 1
x08b6:	hlt	ERR_CODE
x08b7:	cwt	r1, 1
x08b8:	jes	1
x08b9:	hlt	ERR_CODE

x08ba:	lwt	r1, 1
x08bb:	drb	r1, 1
x08bc:	ujs	1
x08bd:	hlt	ERR_CODE
x08be:	cwt	r1, 0
x08bf:	jes	1
x08c0:	hlt	ERR_CODE

; trb

x08c1:	lwt	r1, -2
x08c2:	trb	r1, 1
x08c3:	ujs	1
x08c4:	hlt	ERR_CODE
x08c5:	cwt	r1, -1
x08c6:	jes	1
x08c7:	hlt	ERR_CODE

x08c8:	lwt	r1, -1
x08c9:	trb	r1, 1
x08ca:	hlt	ERR_CODE
x08cb:	cwt	r1, 0
x08cc:	jes	1
x08cd:	hlt	ERR_CODE

; lws

x08ce:	ujs	1
lws1:
x08cf:	.res	1
x08d0:	lw	r1, [pat10]
x08d2:	rw	r1, lws1
x08d4:	lwt	r1, -1
x08d5:	lws	r1, lws1
x08d6:	cw	r1, [pat10]
x08d8:	jes	1
x08d9:	hlt	ERR_CODE

x08da:	lw	r1, [pat01]
x08dc:	rw	r1, lws2
x08de:	lwt	r1, -1
x08df:	lws	r1, lws2
x08e0:	cw	r1, [pat01]
x08e2:	jes	rws_test
x08e3:	hlt	ERR_CODE
lws2:
x08e4:	.res	1

; rws

rws_test:

x08e5:	ujs	1
rws1:
x08e6:	.res	1
x08e7:	lw	r1, [pat01]
x08e9:	rws	r1, rws1
x08ea:	cw	r1, [rws1]
x08ec:	jes	1
x08ed:	hlt	ERR_CODE

x08ee:	lw	r1, [pat10]
x08f0:	rws	r1, rws2
x08f1:	cw	r1, [rws2]
x08f3:	jes	jl_test
x08f4:	hlt	ERR_CODE
rws2:
x08f5:	.res	1

; jl

jl_test:

x08f6:	lwt	r0, 0
x08f7:	jl	jl1_err
x08f9:	lw	r0, ?L
x08fb:	jl	jl2_test
x08fd:	hlt	ERR_CODE
jl1_err:
x08fe:	hlt	ERR_CODE

jl2_test:
x08ff:	lw	r0, ?ZMVCEGYX1234567
x0901:	jl	jl2_err
x0903:	ujs	1
jl2_err:
x0904:	hlt	ERR_CODE

; je

x0905:	lwt	r0, 0
x0906:	je	je1_err
x0908:	lw	r0, ?E
x090a:	je	je2_test
x090c:	hlt	ERR_CODE
je1_err:
x090d:	hlt	ERR_CODE

je2_test:

x090e:	lw	r0, ?ZMVCLGYX1234567
x0910:	je	je2_err
x0912:	ujs	1
je2_err:
x0913:	hlt	ERR_CODE

; jg

x0914:	lwt	r0, 0
x0915:	jg	jg1_err
x0917:	lw	r0, ?G
x0919:	jg	jg2_test
x091b:	hlt	ERR_CODE
jg1_err:
x091c:	hlt	ERR_CODE

jg2_test:

x091d:	lw	r0, ?ZMVCLEYX1234567
x091f:	jg	jg2_err
x0921:	ujs	1
jg2_err:
x0922:	hlt	ERR_CODE

; jz

x0923:	lwt	r0, 0
x0924:	jz	jz1_err
x0926:	lw	r0, ?Z
x0928:	jz	jz2_test
x092a:	hlt	ERR_CODE
jz1_err:
x092b:	hlt	ERR_CODE

jz2_test:

x092c:	lw	r0, ?MVCLEGYX1234567
x092e:	jz	jz2_err
x0930:	ujs	1
jz2_err:
x0931:	hlt	ERR_CODE

; jm

x0932:	lwt	r0, 0
x0933:	jm	jm1_err
x0935:	lw	r0, ?M
x0937:	jm	jm2_test
x0939:	hlt	ERR_CODE
jm1_err:
x093a:	hlt	ERR_CODE

jm2_test:

x093b:	lw	r0, ?ZVCLEGYX1234567
x093d:	jm	jm2_err
x093f:	ujs	1
jm2_err:
x0940:	hlt	ERR_CODE

; jn

x0941:	lw	r0, ?ZMVCLGYX1234567
x0943:	jn	jn2_test
x0945:	hlt	ERR_CODE

jn2_test:

x0946:	lw	r0, ?E
x0948:	jn	jn2_err
x094a:	ujs	1
jn2_err:
x094b:	hlt	ERR_CODE

; jls

x094c:	lwt	r0, 0
x094d:	jls	jls1_err
x094e:	lw	r0, ?L
x0950:	jls	jls2_test
x0951:	hlt	ERR_CODE
jls1_err:
x0952:	hlt	ERR_CODE

jls2_test:

x0953:	lw	r0, ?ZMVCEGYX1234567
x0955:	jls	jls2_err
x0956:	ujs	1
jls2_err:
x0957:	hlt	ERR_CODE

; jgs

x0958:	lwt	r0, 0
x0959:	jgs	jgs1_err
x095a:	lw	r0, ?G
x095c:	jgs	jgs2_test
x095d:	hlt	ERR_CODE
jgs1_err:
x095e:	hlt	ERR_CODE

jgs2_test:

x095f:	lw	r0, ?ZMVCLEYX1234567
x0961:	jgs	jgs2_err
x0962:	ujs	1
jgs2_err:
x0963:	hlt	ERR_CODE

; jxs

x0964:	lwt	r0, 0
x0965:	jxs	jxs1_err
x0966:	lw	r0, ?X
x0968:	jxs	jxs2_test
x0969:	hlt	ERR_CODE
jxs1_err:
x096a:	hlt	ERR_CODE

jxs2_test:

x096b:	lw	r0, ?ZMVCLEGY1234567
x096d:	jxs	jxs2_err
x096e:	ujs	1
jxs2_err:
x096f:	hlt	ERR_CODE

; jys

x0970:	lwt	r0, 0
x0971:	jys	jys1_err
x0972:	lw	r0, ?Y
x0974:	jys	jys2_test
x0975:	hlt	ERR_CODE
jys1_err:
x0976:	hlt	ERR_CODE

jys2_test:

x0977:	lw	r0, ?ZMVCLEGX1234567
x0979:	jys	jys2_err
x097a:	ujs	1
jys2_err:
x097b:	hlt	ERR_CODE

; jcs

x097c:	lwt	r0, 0
x097d:	jcs	jcs1_err
x097e:	lw	r0, ?C
x0980:	jcs	jcs2_test
x0981:	hlt	ERR_CODE
jcs1_err:
x0982:	hlt	ERR_CODE

jcs2_test:

x0983:	lw	r0, ?ZMVLEGYX1234567
x0985:	jcs	jcs2_err
x0986:	ujs	1
jcs2_err:
x0987:	hlt	ERR_CODE

; jvs

x0988:	lwt	r0, 0
x0989:	jvs	jvs1_err
x098a:	lw	r0, ?V
x098c:	jvs	jvs2_test
x098d:	hlt	ERR_CODE
jvs1_err:
x098e:	hlt	ERR_CODE

jvs2_test:

x098f:	lw	r0, ?ZMCLEGYX1234567
x0991:	jvs	jvs2_err
x0992:	ujs	1
jvs2_err:
x0993:	hlt	ERR_CODE

; rky

x0994:	rky	r1
x0995:	rky	r2
x0996:	rky	r3
x0997:	rky	r4
x0998:	cw	r1, r2
x0999:	jes	1
x099a:	hlt	ERR_CODE
x099b:	cw	r3, r4
x099c:	jes	1
x099d:	hlt	ERR_CODE
x099e:	cw	r3, r1
x099f:	jes	1
x09a0:	hlt	ERR_CODE

; sxu

x09a1:	lwt	r0, 0
x09a2:	lw	r1, 0x8000
x09a4:	sxu	r1
x09a5:	cw	r0, ?X
x09a7:	jes	1
x09a8:	hlt	ERR_CODE

x09a9:	lwt	r0, -1
x09aa:	lw	r1, 0x7fff
x09ac:	sxu	r1
x09ad:	cw	r0, ?ZMVCLEGY1234567
x09af:	jes	1
x09b0:	hlt	ERR_CODE

; sxl

x09b1:	lwt	r0, 0
x09b2:	lwt	r1, 1
x09b3:	sxl	r1
x09b4:	cw	r0, ?X
x09b6:	jes	1
x09b7:	hlt	ERR_CODE

x09b8:	lwt	r0, -1
x09b9:	lw	r1, 0xfffe
x09bb:	sxl	r1
x09bc:	cw	r0, ?ZMVCLEGY1234567
x09be:	jes	1
x09bf:	hlt	ERR_CODE

; srz

x09c0:	lw	r1, [pat10]
x09c2:	lw	r2, [pat01]
x09c4:	lw	r0, ?Y
x09c6:	srz	r1
x09c7:	cwt	r0, 0
x09c8:	jes	1
x09c9:	hlt	ERR_CODE
x09ca:	cw	r1, r2
x09cb:	jes	1
x09cc:	hlt	ERR_CODE

x09cd:	lwt	r0, 0
x09ce:	srz	r2
x09cf:	cw	r0, ?Y
x09d1:	jes	1
x09d2:	hlt	ERR_CODE
x09d3:	cw	r2, 0x2aaa
x09d5:	jes	1
x09d6:	hlt	ERR_CODE

; sry

x09d7:	lwt	r0, 0
x09d8:	lw	r1, [pat10]
x09da:	lw	r2, [pat01]
x09dc:	sry	r1
x09dd:	cwt	r0, 0
x09de:	jes	1
x09df:	hlt	ERR_CODE
x09e0:	cw	r1, r2
x09e1:	jes	1
x09e2:	hlt	ERR_CODE

x09e3:	lw	r0, ?Y
x09e5:	lw	r1, [pat10]
x09e7:	sry	r1
x09e8:	cwt	r0, 0
x09e9:	jes	1
x09ea:	hlt	ERR_CODE
x09eb:	cw	r1, 0xd555
x09ed:	jes	1
x09ee:	hlt	ERR_CODE

x09ef:	lwt	r0, 0
x09f0:	sry	r2
x09f1:	cw	r0, ?Y
x09f3:	jes	1
x09f4:	hlt	ERR_CODE
x09f5:	cw	r2, 0x2aaa
x09f7:	jes	1
x09f8:	hlt	ERR_CODE

x09f9:	lw	r1, [pat01]
x09fb:	lw	r0, ?Y
x09fd:	sry	r1
x09fe:	cw	r0, ?Y
x0a00:	jes	1
x0a01:	hlt	ERR_CODE
x0a02:	cw	r1, [pat10]
x0a04:	jes	1
x0a05:	hlt	ERR_CODE

; srx

x0a06:	lwt	r0, 0
x0a07:	lw	r1, [pat10]
x0a09:	lw	r2, [pat01]
x0a0b:	srx	r1
x0a0c:	cwt	r0, 0
x0a0d:	jes	1
x0a0e:	hlt	ERR_CODE
x0a0f:	cw	r1, r2
x0a10:	jes	1
x0a11:	hlt	ERR_CODE

x0a12:	lw	r0, ?X
x0a14:	lw	r1, [pat10]
x0a16:	srx	r1
x0a17:	cw	r0, ?X
x0a19:	jes	1
x0a1a:	hlt	ERR_CODE
x0a1b:	cw	r1, 0xd555
x0a1d:	jes	1
x0a1e:	hlt	ERR_CODE

x0a1f:	lwt	r0, 0
x0a20:	srx	r2
x0a21:	cw	r0, ?Y
x0a23:	jes	1
x0a24:	hlt	ERR_CODE
x0a25:	cw	r2, 0x2aaa
x0a27:	jes	1
x0a28:	hlt	ERR_CODE

x0a29:	lw	r1, [pat01]
x0a2b:	lw	r0, ?X
x0a2d:	srx	r1
x0a2e:	cw	r0, ?YX
x0a30:	jes	1
x0a31:	hlt	ERR_CODE
x0a32:	cw	r1, [pat10]
x0a34:	jes	1
x0a35:	hlt	ERR_CODE

; slz

x0a36:	lw	r1, [pat10]
x0a38:	lw	r2, [pat01]
x0a3a:	lw	r0, ?Y
x0a3c:	slz	r2
x0a3d:	cwt	r0, 0
x0a3e:	jes	1
x0a3f:	hlt	ERR_CODE
x0a40:	cw	r1, r2
x0a41:	jes	1
x0a42:	hlt	ERR_CODE

x0a43:	lwt	r0, 0
x0a44:	slz	r1
x0a45:	cw	r0, ?Y
x0a47:	jes	1
x0a48:	hlt	ERR_CODE
x0a49:	cw	r1, 0x5554
x0a4b:	jes	1
x0a4c:	hlt	ERR_CODE

; slx

x0a4d:	lw	r0, ?X
x0a4f:	lw	r1, [pat10]
x0a51:	slx	r1
x0a52:	cw	r0, ?YX
x0a54:	jes	1
x0a55:	hlt	ERR_CODE
x0a56:	cw	r1, [pat01]
x0a58:	jes	1
x0a59:	hlt	ERR_CODE

x0a5a:	lw	r1, [pat01]
x0a5c:	lwt	r0, 0
x0a5d:	slx	r1
x0a5e:	cwt	r0, 0
x0a5f:	jes	1
x0a60:	hlt	ERR_CODE
x0a61:	cw	r1, [pat10]
x0a63:	jes	1
x0a64:	hlt	ERR_CODE

x0a65:	lw	r0, ?YX
x0a67:	lw	r1, [pat01]
x0a69:	slx	r1
x0a6a:	cw	r0, ?X
x0a6c:	jes	1
x0a6d:	hlt	ERR_CODE
x0a6e:	cw	r1, 0xaaab
x0a70:	jes	1
x0a71:	hlt	ERR_CODE

; svx

x0a72:	lw	r0, ?Y
x0a74:	lw	r1, [pat01]
x0a76:	svx	r1
x0a77:	cw	r0, ?V
x0a79:	jes	1
x0a7a:	hlt	ERR_CODE
x0a7b:	cw	r1, [pat10]
x0a7d:	jes	1
x0a7e:	hlt	ERR_CODE

x0a7f:	lw	r0, ?X
x0a81:	lw	r1, [pat10]
x0a83:	svx	r1
x0a84:	cw	r0, ?VYX
x0a86:	jes	1
x0a87:	hlt	ERR_CODE
x0a88:	cw	r1, [pat01]
x0a8a:	jes	1
x0a8b:	hlt	ERR_CODE

; svz

x0a8c:	lw	r0, ?VY
x0a8e:	lw	r1, [pat01]
x0a90:	svz	r1
x0a91:	cw	r0, ?V
x0a93:	jes	1
x0a94:	hlt	ERR_CODE
x0a95:	cw	r1, [pat10]
x0a97:	jes	1
x0a98:	hlt	ERR_CODE

x0a99:	lw	r0, ?V
x0a9b:	lwt	r1, -1
x0a9c:	svz	r1
x0a9d:	cw	r0, ?VY
x0a9f:	jes	1
x0aa0:	hlt	ERR_CODE
x0aa1:	cw	r1, 0xfffe
x0aa3:	jes	1
x0aa4:	hlt	ERR_CODE

x0aa5:	lwt	r0, 0
x0aa6:	lw	r1, [pat10]
x0aa8:	svz	r1
x0aa9:	cw	r0, ?VY
x0aab:	jes	1
x0aac:	hlt	ERR_CODE
x0aad:	cw	r1, 0x5554
x0aaf:	jes	1
x0ab0:	hlt	ERR_CODE

; sly

x0ab1:	lw	r0, ?Y
x0ab3:	lw	r1, [pat01]
x0ab5:	sly	r1
x0ab6:	cwt	r0, 0
x0ab7:	jes	1
x0ab8:	hlt	ERR_CODE
x0ab9:	cw	r1, 0xaaab
x0abb:	jes	1
x0abc:	hlt	ERR_CODE

x0abd:	lwt	r0, 0
x0abe:	lw	r1, [pat01]
x0ac0:	sly	r1
x0ac1:	cwt	r0, 0
x0ac2:	jes	1
x0ac3:	hlt	ERR_CODE
x0ac4:	cw	r1, [pat10]
x0ac6:	jes	1
x0ac7:	hlt	ERR_CODE

x0ac8:	lw	r0, ?Y
x0aca:	lw	r1, [pat10]
x0acc:	sly	r1
x0acd:	cw	r0, ?Y
x0acf:	jes	1
x0ad0:	hlt	ERR_CODE
x0ad1:	cw	r1, [pat01]
x0ad3:	jes	1
x0ad4:	hlt	ERR_CODE

; svy

x0ad5:	lwt	r0, 0
x0ad6:	lw	r1, [pat01]
x0ad8:	svy	r1
x0ad9:	cw	r0, ?V
x0adb:	jes	1
x0adc:	hlt	ERR_CODE
x0add:	cw	r1, [pat10]
x0adf:	jes	1
x0ae0:	hlt	ERR_CODE

x0ae1:	lwt	r0, 0
x0ae2:	lw	r1, [pat10]
x0ae4:	svy	r1
x0ae5:	cw	r0, ?VY
x0ae7:	jes	1
x0ae8:	hlt	ERR_CODE
x0ae9:	cw	r1, 0x5554
x0aeb:	jes	1
x0aec:	hlt	ERR_CODE

x0aed:	lw	r0, ?VY
x0aef:	lw	r1, [pat01]
x0af1:	svy	r1
x0af2:	cw	r0, ?V
x0af4:	jes	1
x0af5:	hlt	ERR_CODE
x0af6:	cw	r1, 0xaaab
x0af8:	jes	1
x0af9:	hlt	ERR_CODE

x0afa:	lw	r0, ?Y
x0afc:	lw	r1, [pat10]
x0afe:	svy	r1
x0aff:	cw	r0, ?VY
x0b01:	jes	1
x0b02:	hlt	ERR_CODE
x0b03:	cw	r1, [pat01]
x0b05:	jes	1
x0b06:	hlt	ERR_CODE

; shc

x0b07:	lwt	r0, 0
x0b08:	lw	r1, [pat01]
x0b0a:	shc	r1, 1
x0b0b:	cw	r1, [pat10]
x0b0d:	jes	1
x0b0e:	hlt	ERR_CODE

x0b0f:	lw	r1, [pat10]
x0b11:	shc	r1, 1
x0b12:	cw	r1, [pat01]
x0b14:	jes	1
x0b15:	hlt	ERR_CODE

x0b16:	lw	r1, [pat01]
x0b18:	shc	r1, 1
x0b19:	shc	r1, 2
x0b1a:	shc	r1, 3
x0b1b:	shc	r1, 4
x0b1c:	shc	r1, 5
x0b1d:	shc	r1, 6
x0b1e:	shc	r1, 7
x0b1f:	shc	r1, 8
x0b20:	shc	r1, 9
x0b21:	shc	r1, 10
x0b22:	shc	r1, 11
x0b23:	shc	r1, 12
x0b24:	shc	r1, 13
x0b25:	shc	r1, 14
x0b26:	shc	r1, 15
x0b27:	cw	r1, [pat01]
x0b29:	jes	1
x0b2a:	hlt	ERR_CODE

x0b2b:	lwt	r1, -1
x0b2c:	shc	r1, 15
x0b2d:	cwt	r1, -1
x0b2e:	jes	1
x0b2f:	hlt	ERR_CODE

; ngc

x0b30:	lwt	r0, 0
x0b31:	lwt	r1, -1
x0b32:	ngc	r1
x0b33:	cw	r0, ?Z
x0b35:	jes	1
x0b36:	hlt	ERR_CODE
x0b37:	cwt	r1, 0
x0b38:	jes	1
x0b39:	hlt	ERR_CODE

x0b3a:	lw	r0, ?C
x0b3c:	lw	r1, 0x7fff
x0b3e:	ngc	r1
x0b3f:	cw	r0, ?M
x0b41:	jes	1
x0b42:	hlt	ERR_CODE
x0b43:	cw	r1, 0x8001
x0b45:	jes	1
x0b46:	hlt	ERR_CODE

x0b47:	lw	r1, 0x8001
x0b49:	lw	r0, ?C
x0b4b:	ngc	r1
x0b4c:	cwt	r0, 0
x0b4d:	jes	1
x0b4e:	hlt	ERR_CODE
x0b4f:	cw	r1, 0x7fff
x0b51:	jes	1
x0b52:	hlt	ERR_CODE

; nga

x0b53:	lwt	r0, 0
x0b54:	lwt	r1, -1
x0b55:	nga	r1
x0b56:	cwt	r0, 0
x0b57:	jes	1
x0b58:	hlt	ERR_CODE
x0b59:	cwt	r1, 1
x0b5a:	jes	1
x0b5b:	hlt	ERR_CODE

x0b5c:	lwt	r0, 0
x0b5d:	lw	r1, 0x8000
x0b5f:	nga	r1
x0b60:	cw	r0, ?V
x0b62:	jes	1
x0b63:	hlt	ERR_CODE
x0b64:	cw	r1, 0x8000
x0b66:	jes	1
x0b67:	hlt	ERR_CODE

x0b68:	lwt	r0, 0
x0b69:	lwt	r1, 0
x0b6a:	nga	r1
x0b6b:	cw	r0, ?ZC
x0b6d:	jes	1
x0b6e:	hlt	ERR_CODE
x0b6f:	cwt	r1, 0
x0b70:	jes	1
x0b71:	hlt	ERR_CODE

x0b72:	lwt	r0, 0
x0b73:	lw	r1, 0x7fff
x0b75:	nga	r1
x0b76:	cw	r0, ?M
x0b78:	jes	1
x0b79:	hlt	ERR_CODE
x0b7a:	cw	r1, 0x8001
x0b7c:	jes	1
x0b7d:	hlt	ERR_CODE

; ngl

x0b7e:	lwt	r0, 0
x0b7f:	lwt	r1, -1
x0b80:	ngl	r1
x0b81:	cw	r0, ?Z
x0b83:	jes	1
x0b84:	hlt	ERR_CODE
x0b85:	cwt	r1, 0
x0b86:	jes	1
x0b87:	hlt	ERR_CODE

x0b88:	lwt	r1, 0
x0b89:	lwt	r0, 0
x0b8a:	ngl	r1
x0b8b:	cwt	r0, 0
x0b8c:	jes	1
x0b8d:	hlt	ERR_CODE
x0b8e:	cwt	r1, -1
x0b8f:	jes	1
x0b90:	hlt	ERR_CODE

x0b91:	lwt	r0, 0
x0b92:	lw	r1, [pat01]
x0b94:	ngl	r1
x0b95:	cwt	r0, 0
x0b96:	jes	1
x0b97:	hlt	ERR_CODE
x0b98:	cw	r1, [pat10]
x0b9a:	jes	1
x0b9b:	hlt	ERR_CODE

; zrb

x0b9c:	lwt	r1, -1
x0b9d:	zrb	r1
x0b9e:	cw	r1, 0xff00
x0ba0:	jes	1
x0ba1:	hlt	ERR_CODE

x0ba2:	lwt	r1, -1
x0ba3:	zlb	r1
x0ba4:	cw	r1, 0xff
x0ba6:	jes	1
x0ba7:	hlt	ERR_CODE

; lb

x0ba8:	lwt	r1, 0
x0ba9:	lb	r1, pat01 << 1
x0bab:	cw	r1, 0x55
x0bad:	jes	1
x0bae:	hlt	ERR_CODE

x0baf:	lwt	r1, -1
x0bb0:	rz	temp3
x0bb2:	lb	r1, (temp3 << 1) + 1
x0bb4:	cw	r1, 0xff00
x0bb6:	jes	1
x0bb7:	hlt	ERR_CODE

; rb

x0bb8:	lwt	r1, -1
x0bb9:	rz	temp3
x0bbb:	rb	r1, (temp3 << 1) + 1
x0bbd:	lw	r2, [temp3]
x0bbf:	cw	r2, 0xff
x0bc1:	jes	1
x0bc2:	hlt	ERR_CODE

x0bc3:	lwt	r1, -1
x0bc4:	rw	r1, temp3
x0bc6:	lwt	r1, 0
x0bc7:	rb	r1, temp3 << 1
x0bc9:	lw	r1, [temp3]
x0bcb:	cw	r1, 0xff
x0bcd:	jes	1
x0bce:	hlt	ERR_CODE

; cb

x0bcf:	lwt	r0, 0
x0bd0:	rz	temp3
x0bd2:	lwt	r1, 0
x0bd3:	cb	r1, temp3 << 1
x0bd5:	cw	r0, ?E
x0bd7:	jes	1
x0bd8:	hlt	ERR_CODE

x0bd9:	lwt	r1, -1
x0bda:	cb	r1, (temp3 << 1) + 1
x0bdc:	cw	r0, ?G
x0bde:	jes	1
x0bdf:	hlt	ERR_CODE

x0be0:	lwt	r1, -1
x0be1:	rw	r1, temp3
x0be3:	lwt	r1, 0
x0be4:	cb	r1, temp3 << 1
x0be6:	cw	r0, ?L
x0be8:	jes	1
x0be9:	hlt	ERR_CODE

; blc

x0bea:	lwt	r0, -1
x0beb:	blc	?ZMVCLEGY
x0bec:	ujs	1
x0bed:	hlt	ERR_CODE

x0bee:	lwt	r0, 0
x0bef:	blc	?ZVCEY
x0bf0:	hlt	ERR_CODE

; brc

x0bf1:	lwt	r0, -1
x0bf2:	brc	?X1234567
x0bf3:	ujs	1
x0bf4:	hlt	ERR_CODE

x0bf5:	lwt	r0, 0
x0bf6:	brc	?X2357
x0bf7:	hlt	ERR_CODE

; ib

x0bf8:	lwt	r1, -3
x0bf9:	rw	r1, temp3
x0bfb:	ib	temp3
x0bfd:	lw	r1, [temp3]
x0bff:	cwt	r1, -2
x0c00:	jes	1
x0c01:	ujs	ib_err
x0c02:	ib	temp3
x0c04:	lw	r1, [temp3]
x0c06:	cwt	r1, -1
x0c07:	jes	1
x0c08:	ujs	ib_err
x0c09:	ib	temp3
x0c0b:	hlt	ERR_CODE
x0c0c:	lw	r1, [temp3]
x0c0e:	cwt	r1, 0
x0c0f:	jes	1
ib_err:
x0c10:	hlt	ERR_CODE

; ric

x0c11:	lwt	r1, 0
x0c12:	ric	r1
ric_pos:
x0c13:	cw	r1, ric_pos
x0c15:	jes	1
x0c16:	hlt	ERR_CODE

; rpc

x0c17:	lwt	r0, 0
x0c18:	lwt	r1, -1
x0c19:	rpc	r1
x0c1a:	cwt	r1, 0
x0c1b:	jes	1
x0c1c:	hlt	ERR_CODE

x0c1d:	lwt	r0, -1
x0c1e:	lwt	r1, 0
x0c1f:	rpc	r1
x0c20:	cwt	r1, -1
x0c21:	jes	1
x0c22:	hlt	ERR_CODE

; lpc

x0c23:	lwt	r0, 0
x0c24:	lwt	r1, -1
x0c25:	lpc	r1
x0c26:	cwt	r0, -1
x0c27:	jes	1
x0c28:	hlt	ERR_CODE

x0c29:	lwt	r0, -1
x0c2a:	lwt	r1, 0
x0c2b:	lpc	r1
x0c2c:	cwt	r0, 0
x0c2d:	jes	1
x0c2e:	hlt	ERR_CODE

; If test was restarted with interrupt mask set to IMASK_ALL,
; restart it from the very beginning.
; Initial routine will set the mask to IMASK_NONE again @ 0x014a.

x0c2f:	lw	r1, [mask]
x0c31:	cwt	r1, 0
x0c32:	jes	kifi_test
x0c33:	lj	reset_stack
x0c35:	mcl

	.ifdef	EM400 ; finish and indicate no error if running in EM400 emulator
x0c36:	hlt	077
	.res	1 ; padding for consistent addressing
	.else ; loop over if running on a real hardware
x0c36:	uj	start
	.endif

; fi, ki

kifi_test:

x0c38:	rz	intrs
x0c3a:	fi	intrs
x0c3c:	lwt	r1, -1
x0c3d:	rw	r1, intrs
x0c3f:	ki	intrs
x0c41:	lw	r1, [intrs]
x0c43:	cwt	r1, 0
x0c44:	jes	1
x0c45:	hlt	ERR_CODE

x0c46:	lw	r1, 0x7fff
x0c48:	rw	r1, intrs
x0c4a:	fi	intrs
x0c4c:	rz	intrs
x0c4e:	ki	intrs
x0c50:	lw	r1, [intrs]
x0c52:	cw	r1, 0x7fff
x0c54:	jes	1
x0c55:	hlt	ERR_CODE

x0c56:	lw	r1, 0x5555
x0c58:	rw	r1, intrs
x0c5a:	fi	intrs
x0c5c:	rz	intrs
x0c5e:	ki	intrs
x0c60:	lw	r1, [intrs]
x0c62:	cw	r1, 0x5555
x0c64:	jes	1
x0c65:	hlt	ERR_CODE

; interrupt serve (check if SR is stored on the stack)

x0c66:	mcl
x0c67:	lw	r1, int_sw_low_proc1
x0c69:	rw	r1, INTV_SW_L
x0c6b:	lw	r1, stack
x0c6d:	rw	r1, stackp
x0c6f:	lw	r1, IMASK_GROUP_L
x0c71:	rw	r1, tmp_mask
x0c73:	im	tmp_mask
x0c75:	lwt	r1, 1 ; I_SW_L
x0c76:	rw	r1, intrs
x0c78:	fi	intrs

int_sw_low_proc1:
x0c7a:	lw	r1, int31
x0c7c:	rw	r1, INTV_SW_L
x0c7e:	lw	r1, [stack+2] ; load SR from the stack
x0c80:	cw	r1, IMASK_GROUP_L
x0c82:	jes	1
x0c83:	hlt	ERR_CODE

x0c84:	mcl
x0c85:	ujs	exl_test

intrs:
tmp_mask_2:
x0c86:	.word	0

; exl serve

exl_test_stack_contents:
x0c87:	.word	exl_proc
x0c88:	.word	-1
x0c89:	.word	IMASK_ALL
x0c8a:	.word	255
x0c8b:	.word	stack+4 ; stack pointer after the exl call

exl_test:

x0c8c:	mcl
x0c8d:	lw	r1, stack
x0c8f:	rw	r1, stackp
x0c91:	lwt	r0, -1
x0c92:	lw	r1, IMASK_ALL
x0c94:	rw	r1, tmp_mask_2
x0c96:	im	tmp_mask_2
x0c98:	lw	r1, exl_proc
x0c9a:	rw	r1, EXLV
x0c9c:	exl	255

exl_proc:
x0c9d:	la	exl_test_stack_contents
x0c9f:	cw	r1, [stack]
x0ca1:	jes	1
x0ca2:	hlt	ERR_CODE
x0ca3:	cw	r2, [stack+1]
x0ca5:	jes	1
x0ca6:	hlt	ERR_CODE
x0ca7:	cw	r3, [stack+2]
x0ca9:	jes	1
x0caa:	hlt	ERR_CODE
x0cab:	cw	r4, [stack+3]
x0cad:	jes	1
x0cae:	hlt	ERR_CODE
x0caf:	cw	r0, ?E
x0cb1:	jes	1
x0cb2:	hlt	ERR_CODE
x0cb3:	cw	r5, [STACKP]
x0cb5:	jes	1
x0cb6:	hlt	ERR_CODE

; test interrupt reported "inside" an exl routine
; (interrupts 28-31 are masked when serving exl)

x0cb7:	lw	r1, exl_int_proc
x0cb9:	rw	r1, INTV_SW_L
x0cbb:	lwt	r1, I_SW_L
x0cbc:	rw	r1, intrs
x0cbe:	fi	intrs ; this shouldn't report any interrupt
x0cc0:	mcl
x0cc1:	ujs	exl_int_continue

exl_int_proc: ; this shouldn't be called
x0cc2:	lw	r1, int31
x0cc4:	rw	r1, INTV_SW_L
x0cc6:	hlt	ERR_CODE

exl_int_continue:
x0cc7:	lw	r1, int31
x0cc9:	rw	r1, INTV_SW_L
x0ccb:	mcl
x0ccc:	ujs	sp_test

; sp

sp_test_data:
x0ccd:	.word	sp_continue	; IC
x0cce:	.word	-1		; r0
x0ccf:	.word	IMASK_ALL	; SR

sp_test:

x0cd0:	mcl
x0cd1:	sp	sp_test_data
sp_continue:
x0cd3:	cwt	r0, -1
x0cd4:	jes	1
x0cd5:	hlt	ERR_CODE

; at this point SR should already be set to IMASK_ALL by the sp call

x0cd6:	lw	r1, int_sw_low_proc2
x0cd8:	rw	r1, INTV_SW_L
x0cda:	lw	r1, stack
x0cdc:	rw	r1, stackp
x0cde:	lwt	r1, 1
x0cdf:	rw	r1, intrs
x0ce1:	fi	intrs

int_sw_low_proc2:
x0ce3:	lw	r1, int31
x0ce5:	rw	r1, INTV_SW_L
x0ce7:	lw	r1, IMASK_ALL
x0ce9:	cw	r1, [stack+2] ; SR
x0ceb:	jes	1
x0cec:	hlt	ERR_CODE

; mb

x0ced:	lw	r1, IMASK_GROUP_L | 0\SR_Q | 1\SR_BS | 15\SR_NB ; try to sneak setting mask for soft interrupts
x0cef:	rw	r1, temp3
x0cf1:	mcl
x0cf2:	mb	temp3
x0cf4:	lw	r1, int_sw_low_proc3
x0cf6:	rw	r1, INTV_SW_L
x0cf8:	lw	r1, stack
x0cfa:	rw	r1, stackp
x0cfc:	lw	r1, IMASK_GROUP_L
x0cfe:	rw	r1, tmp_mask
x0d00:	im	tmp_mask
x0d02:	lwt	r1, 1
x0d03:	rw	r1, intr2
x0d05:	fi	intr2

int_sw_low_proc3:
x0d07:	lw	r1, int31
x0d09:	rw	r1, INTV_SW_L
x0d0b:	lw	r1, [stack+2] ; SR
x0d0d:	cw	r1, IMASK_GROUP_L | 0\SR_Q | 1\SR_BS | 15\SR_NB ; SR should not change (=no interrupt served)
x0d0f:	jes	1
x0d10:	hlt	ERR_CODE

; lip

; no interrupt should be served with interrupt mask = 0 (set with mcl)

x0d11:	mcl
x0d12:	lw	r1, lip_cont
x0d14:	rw	r1, stack ; IC
x0d16:	lwt	r1, -1
x0d17:	rw	r1, stack+1 ; R0
x0d19:	rz	stack+2 ; SR
x0d1b:	mcl
x0d1c:	lw	r1, I_MASKABLE
x0d1e:	rw	r1, intr2
x0d20:	fi	intr2 ; should do nothing

; no other interrupt should be served after lip

x0d22:	lw	r1, lip_err
x0d24:	rw	r1, INTV_PARITY
x0d26:	lw	r1, stack+4
x0d28:	rw	r1, STACKP
x0d2a:	lip

lip_cont:
x0d2b:	cwt	r0, -1
x0d2c:	jes	1
x0d2d:	hlt	ERR_CODE

; stack should be restored

x0d2e:	lw	r1, stack
x0d30:	cw	r1, [stackp]
x0d32:	jes	1
x0d33:	hlt	ERR_CODE

x0d34:	mcl
x0d35:	ujs	1
lip_err:
x0d36:	hlt	ERR_CODE

x0d37:	lw	r1, int01
x0d39:	rw	r1, INTV_PARITY
x0d3b:	ujs	nb_gt0

tmp_mask:
x0d3c:	.word	0
intr2:
x0d3d:	.word	0

; test rozkazów współpracujących z NB niezerowym

nb_gt0:

; configure memory

x0d3e:	lj	reset_stack
x0d40:	lw	r1, 1\MEM_PAGE | 15\MEM_SEGMENT
x0d42:	lw	r2, 3\MEM_FRAME | 0\MEM_MODULE | MEM_CFG
x0d44:	ou	r1, r2
x0d45:	.word	memcfg_io_no, memcfg_io_en, memcfg_io_ok, memcfg_io_pe
memcfg_io_no:
x0d49:	hlt	ERR_CODE
memcfg_io_en:
x0d4a:	hlt	ERR_CODE
memcfg_io_pe:
x0d4b:	hlt	ERR_CODE
memcfg_io_ok:

; memory operations on NB=15

; prepare memory contents

x0d4c:	mb	nb15
x0d4e:	lwt	r3, 0
x0d4f:	lw	r4, 0x1001
x0d51:	lw	r5, -4000
nb15_clear_loop:
x0d53:	pw	r3, r4
x0d54:	awt	r4, 1
x0d55:	irb	r5, nb15_clear_loop

x0d56:	lj	nb15_copy

; pw, tw (NB=15)

x0d58:	tw	r2, pat_aaaa
x0d5a:	pw	r2, temp3
x0d5c:	lwt	r1, 0
x0d5d:	tw	r1, temp3
x0d5f:	cw	r1, r2
x0d60:	jes	1
x0d61:	hlt	ERR_CODE

x0d62:	tw	r2, pat10
x0d64:	pw	r2, temp3
x0d66:	tw	r1, temp3
x0d68:	cw	r1, r2
x0d69:	jes	1
x0d6a:	hlt	ERR_CODE

; is (NB=15)

x0d6b:	lwt	r1, 0
x0d6c:	pw	r1, temp3
x0d6e:	is	r1, temp3
x0d70:	hlt	ERR_CODE
x0d71:	tw	r1, temp3
x0d73:	cwt	r1, 0
x0d74:	jes	1
x0d75:	hlt	ERR_CODE

x0d76:	lwt	r1, 0
x0d77:	pw	r1, temp3
x0d79:	lwt	r1, -1
x0d7a:	is	r1, temp3
x0d7c:	ujs	1
x0d7d:	hlt	ERR_CODE
x0d7e:	tw	r1, temp3
x0d80:	cwt	r1, -1
x0d81:	jes	1
x0d82:	hlt	ERR_CODE

x0d83:	lwt	r1, -1
x0d84:	pw	r1, temp3
x0d86:	lwt	r1, 0
x0d87:	is	r1, temp3
x0d89:	hlt	ERR_CODE
x0d8a:	tw	r1, temp3
x0d8c:	cwt	r1, -1
x0d8d:	jes	1
x0d8e:	hlt	ERR_CODE

x0d8f:	lwt	r1, -1
x0d90:	pw	r1, temp3
x0d92:	is	r1, temp3
x0d94:	hlt	ERR_CODE
x0d95:	tw	r1, temp3
x0d97:	cwt	r1, -1
x0d98:	jes	1
x0d99:	hlt	ERR_CODE

x0d9a:	tw	r1, pat01
x0d9c:	pw	r1, temp3
x0d9e:	is	r1, temp3
x0da0:	hlt	ERR_CODE
x0da1:	tw	r1, temp3
x0da3:	tw	r2, pat01
x0da5:	cw	r1, r2
x0da6:	jes	1
x0da7:	hlt	ERR_CODE

x0da8:	tw	r1, pat10
x0daa:	pw	r1, temp3
x0dac:	is	r1, temp3
x0dae:	hlt	ERR_CODE
x0daf:	tw	r1, temp3
x0db1:	tw	r2, pat10
x0db3:	cw	r1, r2
x0db4:	jes	1
x0db5:	hlt	ERR_CODE

x0db6:	tw	r1, pat01
x0db8:	tw	r2, pat10
x0dba:	pw	r2, temp3
x0dbc:	is	r1, temp3
x0dbe:	ujs	1
x0dbf:	hlt	ERR_CODE
x0dc0:	tw	r1, temp3
x0dc2:	cwt	r1, -1
x0dc3:	jes	1
x0dc4:	hlt	ERR_CODE

x0dc5:	tw	r1, pat01
x0dc7:	tw	r2, pat10
x0dc9:	pw	r1, temp3
x0dcb:	is	r2, temp3
x0dcd:	ujs	1
x0dce:	hlt	ERR_CODE
x0dcf:	tw	r1, temp3
x0dd1:	cwt	r1, -1
x0dd2:	jes	1
x0dd3:	hlt	ERR_CODE

; bm (NB=15)

x0dd4:	lwt	r1, 0
x0dd5:	pw	r1, temp3
x0dd7:	bm	r1, temp3
x0dd9:	hlt	ERR_CODE

x0dda:	lwt	r1, -1
x0ddb:	pw	r1, temp3
x0ddd:	bm	r1, temp3
x0ddf:	hlt	ERR_CODE

x0de0:	lwt	r1, -1
x0de1:	lwt	r2, 0
x0de2:	pw	r2, temp3
x0de4:	bm	r1, temp3
x0de6:	ujs	1
x0de7:	hlt	ERR_CODE

x0de8:	lwt	r1, 0
x0de9:	lwt	r2, -1
x0dea:	pw	r2, temp3
x0dec:	bm	r1, temp3
x0dee:	hlt	ERR_CODE

x0def:	tw	r1, pat01
x0df1:	pw	r1, temp3
x0df3:	bm	r1, temp3
x0df5:	hlt	ERR_CODE

x0df6:	tw	r1, pat10
x0df8:	pw	r1, temp3
x0dfa:	bm	r1, temp3
x0dfc:	hlt	ERR_CODE

x0dfd:	tw	r1, pat01
x0dff:	tw	r2, pat10
x0e01:	pw	r2, temp3
x0e03:	bm	r1, temp3
x0e05:	ujs	1
x0e06:	hlt	ERR_CODE

x0e07:	tw	r1, pat10
x0e09:	tw	r2, pat01
x0e0b:	pw	r2, temp3
x0e0d:	bm	r1, temp3
x0e0f:	ujs	1
x0e10:	hlt	ERR_CODE

; om (NB=15)

x0e11:	lwt	r3, 0
x0e12:	mb	nb0
x0e14:	lpc	r3
x0e15:	mb	nb15
x0e17:	lwt	r1, -1
x0e18:	pw	r1, temp3
x0e1a:	lwt	r1, 0
x0e1b:	om	r1, temp3
x0e1d:	cwt	r0, 0
x0e1e:	jes	1
x0e1f:	hlt	ERR_CODE
x0e20:	tw	r1, temp3
x0e22:	cwt	r1, -1
x0e23:	jes	1
x0e24:	hlt	ERR_CODE

x0e25:	lwt	r3, 0
x0e26:	mb	nb0
x0e28:	lpc	r3
x0e29:	mb	nb15
x0e2b:	pw	r0, temp3
x0e2d:	lwt	r1, -1
x0e2e:	om	r1, temp3
x0e30:	cwt	r0, 0
x0e31:	jes	1
x0e32:	hlt	ERR_CODE
x0e33:	tw	r1, temp3
x0e35:	cwt	r1, -1
x0e36:	jes	1
x0e37:	hlt	ERR_CODE

x0e38:	lwt	r3, 0
x0e39:	mb	nb0
x0e3b:	lpc	r3
x0e3c:	mb	nb15
x0e3e:	pw	r0, temp3
x0e40:	lwt	r1, 0
x0e41:	om	r1, temp3
x0e43:	cw	r0, ?Z
x0e45:	jes	1
x0e46:	hlt	ERR_CODE
x0e47:	tw	r1, temp3
x0e49:	cwt	r1, 0
x0e4a:	jes	1
x0e4b:	hlt	ERR_CODE

x0e4c:	lwt	r3, 0
x0e4d:	mb	nb0
x0e4f:	lpc	r3
x0e50:	mb	nb15
x0e52:	lwt	r1, -1
x0e53:	pw	r1, temp3
x0e55:	om	r1, temp3
x0e57:	cwt	r0, 0
x0e58:	jes	1
x0e59:	hlt	ERR_CODE
x0e5a:	tw	r1, temp3
x0e5c:	cwt	r1, -1
x0e5d:	jes	1
x0e5e:	hlt	ERR_CODE

x0e5f:	lwt	r3, 0
x0e60:	mb	nb0
x0e62:	lpc	r3
x0e63:	mb	nb15
x0e65:	tw	r1, pat01
x0e67:	pw	r1, temp3
x0e69:	om	r1, temp3
x0e6b:	cwt	r0, 0
x0e6c:	jes	1
x0e6d:	hlt	ERR_CODE
x0e6e:	tw	r1, temp3
x0e70:	tw	r2, pat01
x0e72:	cw	r1, r2
x0e73:	jes	1
x0e74:	hlt	ERR_CODE

x0e75:	lwt	r3, 0
x0e76:	mb	nb0
x0e78:	lpc	r3
x0e79:	mb	nb15
x0e7b:	tw	r1, pat10
x0e7d:	pw	r1, temp3
x0e7f:	om	r1, temp3
x0e81:	cwt	r0, 0
x0e82:	jes	1
x0e83:	hlt	ERR_CODE
x0e84:	tw	r1, temp3
x0e86:	tw	r2, pat10
x0e88:	cw	r1, r2
x0e89:	jes	1
x0e8a:	hlt	ERR_CODE

x0e8b:	lwt	r3, 0
x0e8c:	mb	nb0
x0e8e:	lpc	r3
x0e8f:	mb	nb15
x0e91:	tw	r1, pat01
x0e93:	pw	r1, temp3
x0e95:	tw	r1, pat10
x0e97:	om	r1, temp3
x0e99:	cwt	r0, 0
x0e9a:	jes	1
x0e9b:	hlt	ERR_CODE
x0e9c:	tw	r1, temp3
x0e9e:	cwt	r1, -1
x0e9f:	jes	1
x0ea0:	hlt	ERR_CODE

x0ea1:	lwt	r3, 0
x0ea2:	mb	nb0
x0ea4:	lpc	r3
x0ea5:	mb	nb15
x0ea7:	tw	r1, pat10
x0ea9:	pw	r1, temp3
x0eab:	tw	r1, pat01
x0ead:	om	r1, temp3
x0eaf:	cwt	r0, 0
x0eb0:	jes	1
x0eb1:	hlt	ERR_CODE
x0eb2:	tw	r1, temp3
x0eb4:	cwt	r1, -1
x0eb5:	jes	1
x0eb6:	hlt	ERR_CODE

; nm (NB=15)

x0eb7:	lwt	r3, 0
x0eb8:	mb	nb0
x0eba:	lpc	r3
x0ebb:	mb	nb15
x0ebd:	lwt	r1, -1
x0ebe:	pw	r1, temp3
x0ec0:	lwt	r1, 0
x0ec1:	nm	r1, temp3
x0ec3:	cw	r0, ?Z
x0ec5:	jes	1
x0ec6:	hlt	ERR_CODE
x0ec7:	tw	r1, temp3
x0ec9:	cwt	r1, 0
x0eca:	jes	1
x0ecb:	hlt	ERR_CODE

x0ecc:	lwt	r3, 0
x0ecd:	mb	nb0
x0ecf:	lpc	r3
x0ed0:	mb	nb15
x0ed2:	lwt	r1, -1
x0ed3:	pw	r0, temp3
x0ed5:	nm	r1, temp3
x0ed7:	cw	r0, ?Z
x0ed9:	jes	1
x0eda:	hlt	ERR_CODE
x0edb:	tw	r1, temp3
x0edd:	cwt	r1, 0
x0ede:	jes	1
x0edf:	hlt	ERR_CODE

x0ee0:	lwt	r3, 0
x0ee1:	mb	nb0
x0ee3:	lpc	r3
x0ee4:	mb	nb15
x0ee6:	pw	r0, temp3
x0ee8:	lwt	r1, 0
x0ee9:	nm	r1, temp3
x0eeb:	cw	r0, ?Z
x0eed:	jes	1
x0eee:	hlt	ERR_CODE
x0eef:	tw	r1, temp3
x0ef1:	cwt	r1, 0
x0ef2:	jes	1
x0ef3:	hlt	ERR_CODE

x0ef4:	lwt	r3, 0
x0ef5:	mb	nb0
x0ef7:	lpc	r3
x0ef8:	mb	nb15
x0efa:	lwt	r1, -1
x0efb:	pw	r1, temp3
x0efd:	nm	r1, temp3
x0eff:	cwt	r0, 0
x0f00:	jes	1
x0f01:	hlt	ERR_CODE
x0f02:	tw	r1, temp3
x0f04:	cwt	r1, -1
x0f05:	jes	1
x0f06:	hlt	ERR_CODE

x0f07:	lwt	r3, 0
x0f08:	mb	nb0
x0f0a:	lpc	r3
x0f0b:	mb	nb15
x0f0d:	tw	r1, pat01
x0f0f:	pw	r1, temp3
x0f11:	nm	r1, temp3
x0f13:	cwt	r0, 0
x0f14:	jes	1
x0f15:	hlt	ERR_CODE
x0f16:	tw	r1, temp3
x0f18:	tw	r2, pat01
x0f1a:	cw	r1, r2
x0f1b:	jes	1
x0f1c:	hlt	ERR_CODE

x0f1d:	lwt	r3, 0
x0f1e:	mb	nb0
x0f20:	lpc	r3
x0f21:	mb	nb15
x0f23:	tw	r1, pat10
x0f25:	pw	r1, temp3
x0f27:	nm	r1, temp3
x0f29:	cwt	r0, 0
x0f2a:	jes	1
x0f2b:	hlt	ERR_CODE
x0f2c:	tw	r1, temp3
x0f2e:	tw	r2, pat10
x0f30:	cw	r1, r2
x0f31:	jes	1
x0f32:	hlt	ERR_CODE

x0f33:	lwt	r3, 0
x0f34:	mb	nb0
x0f36:	lpc	r3
x0f37:	mb	nb15
x0f39:	tw	r1, pat10
x0f3b:	pw	r1, temp3
x0f3d:	tw	r1, pat01
x0f3f:	nm	r1, temp3
x0f41:	cw	r0, ?Z
x0f43:	jes	1
x0f44:	hlt	ERR_CODE
x0f45:	tw	r1, temp3
x0f47:	cwt	r1, 0
x0f48:	jes	1
x0f49:	hlt	ERR_CODE

x0f4a:	lwt	r3, 0
x0f4b:	mb	nb0
x0f4d:	lpc	r3
x0f4e:	mb	nb15
x0f50:	tw	r1, pat01
x0f52:	pw	r1, temp3
x0f54:	tw	r1, pat10
x0f56:	nm	r1, temp3
x0f58:	cw	r0, ?Z
x0f5a:	jes	1
x0f5b:	hlt	ERR_CODE
x0f5c:	tw	r1, temp3
x0f5e:	cwt	r1, 0
x0f5f:	jes	1
x0f60:	hlt	ERR_CODE

; em (NB=15)

x0f61:	lwt	r3, 0
x0f62:	mb	nb0
x0f64:	lpc	r3
x0f65:	mb	nb15
x0f67:	lwt	r1, -1
x0f68:	pw	r1, temp3
x0f6a:	lwt	r1, 0
x0f6b:	em	r1, temp3
x0f6d:	cwt	r0, 0
x0f6e:	jes	1
x0f6f:	hlt	ERR_CODE
x0f70:	tw	r1, temp3
x0f72:	cwt	r1, -1
x0f73:	jes	1
x0f74:	hlt	ERR_CODE

x0f75:	lwt	r3, 0
x0f76:	mb	nb0
x0f78:	lpc	r3
x0f79:	mb	nb15
x0f7b:	lwt	r1, 0
x0f7c:	pw	r1, temp3
x0f7e:	lwt	r1, -1
x0f7f:	em	r1, temp3
x0f81:	cw	r0, ?Z
x0f83:	jes	1
x0f84:	hlt	ERR_CODE
x0f85:	tw	r1, temp3
x0f87:	cwt	r1, 0
x0f88:	jes	1
x0f89:	hlt	ERR_CODE

x0f8a:	lwt	r3, 0
x0f8b:	mb	nb0
x0f8d:	lpc	r3
x0f8e:	mb	nb15
x0f90:	lwt	r1, 0
x0f91:	pw	r1, temp3
x0f93:	em	r1, temp3
x0f95:	cw	r0, ?Z
x0f97:	jes	1
x0f98:	hlt	ERR_CODE
x0f99:	tw	r1, temp3
x0f9b:	cwt	r1, 0
x0f9c:	jes	1
x0f9d:	hlt	ERR_CODE

x0f9e:	lwt	r3, 0
x0f9f:	mb	nb0
x0fa1:	lpc	r3
x0fa2:	mb	nb15
x0fa4:	lwt	r1, -1
x0fa5:	pw	r1, temp3
x0fa7:	em	r1, temp3
x0fa9:	cw	r0, ?Z
x0fab:	jes	1
x0fac:	hlt	ERR_CODE
x0fad:	tw	r1, temp3
x0faf:	cwt	r1, 0
x0fb0:	jes	1
x0fb1:	hlt	ERR_CODE

x0fb2:	lwt	r3, 0
x0fb3:	mb	nb0
x0fb5:	lpc	r3
x0fb6:	mb	nb15
x0fb8:	tw	r1, pat01
x0fba:	pw	r1, temp3
x0fbc:	em	r1, temp3
x0fbe:	cw	r0, ?Z
x0fc0:	jes	1
x0fc1:	hlt	ERR_CODE
x0fc2:	tw	r1, temp3
x0fc4:	cwt	r1, 0
x0fc5:	jes	1
x0fc6:	hlt	ERR_CODE

x0fc7:	lwt	r3, 0
x0fc8:	mb	nb0
x0fca:	lpc	r3
x0fcb:	mb	nb15
x0fcd:	tw	r1, pat10
x0fcf:	pw	r1, temp3
x0fd1:	em	r1, temp3
x0fd3:	cw	r0, ?Z
x0fd5:	jes	1
x0fd6:	hlt	ERR_CODE
x0fd7:	tw	r1, temp3
x0fd9:	cwt	r1, 0
x0fda:	jes	1
x0fdb:	hlt	ERR_CODE

x0fdc:	lwt	r3, 0
x0fdd:	mb	nb0
x0fdf:	lpc	r3
x0fe0:	mb	nb15
x0fe2:	tw	r1, pat10
x0fe4:	pw	r1, temp3
x0fe6:	tw	r1, pat01
x0fe8:	em	r1, temp3
x0fea:	cwt	r0, 0
x0feb:	jes	1
x0fec:	hlt	ERR_CODE
x0fed:	tw	r1, temp3
x0fef:	tw	r2, pat10
x0ff1:	cw	r1, r2
x0ff2:	jes	1
x0ff3:	hlt	ERR_CODE

x0ff4:	lwt	r3, 0
x0ff5:	mb	nb0
x0ff7:	lpc	r3
x0ff8:	mb	nb15
x0ffa:	tw	r1, pat01
x0ffc:	pw	r1, temp3
x0ffe:	tw	r1, pat10
x1000:	em	r1, temp3
x1002:	cwt	r0, 0
x1003:	jes	1
x1004:	hlt	ERR_CODE
x1005:	tw	r1, temp3
x1007:	tw	r2, pat01
x1009:	cw	r1, r2
x100a:	jes	1
x100b:	hlt	ERR_CODE

; xm (NB=15)

x100c:	lwt	r3, 0
x100d:	mb	nb0
x100f:	lpc	r3
x1010:	mb	nb15
x1012:	lwt	r1, -1
x1013:	pw	r1, temp3
x1015:	lwt	r1, 0
x1016:	xm	r1, temp3
x1018:	cwt	r0, 0
x1019:	jes	1
x101a:	hlt	ERR_CODE
x101b:	tw	r1, temp3
x101d:	cwt	r1, -1
x101e:	jes	1
x101f:	hlt	ERR_CODE

x1020:	lwt	r3, 0
x1021:	mb	nb0
x1023:	lpc	r3
x1024:	mb	nb15
x1026:	pw	r0, temp3
x1028:	lwt	r1, -1
x1029:	xm	r1, temp3
x102b:	cwt	r0, 0
x102c:	jes	1
x102d:	hlt	ERR_CODE
x102e:	tw	r1, temp3
x1030:	cwt	r1, -1
x1031:	jes	1
x1032:	hlt	ERR_CODE

x1033:	lwt	r3, 0
x1034:	mb	nb0
x1036:	lpc	r3
x1037:	mb	nb15
x1039:	pw	r0, temp3
x103b:	lwt	r1, 0
x103c:	xm	r1, temp3
x103e:	cw	r0, ?Z
x1040:	jes	1
x1041:	hlt	ERR_CODE
x1042:	tw	r1, temp3
x1044:	cwt	r1, 0
x1045:	jes	1
x1046:	hlt	ERR_CODE

x1047:	lwt	r3, 0
x1048:	mb	nb0
x104a:	lpc	r3
x104b:	mb	nb15
x104d:	lwt	r1, -1
x104e:	pw	r1, temp3
x1050:	xm	r1, temp3
x1052:	cw	r0, ?Z
x1054:	jes	1
x1055:	hlt	ERR_CODE
x1056:	tw	r1, temp3
x1058:	cwt	r1, 0
x1059:	jes	1
x105a:	hlt	ERR_CODE

x105b:	lwt	r3, 0
x105c:	mb	nb0
x105e:	lpc	r3
x105f:	mb	nb15
x1061:	tw	r1, pat01
x1063:	pw	r1, temp3
x1065:	xm	r1, temp3
x1067:	cw	r0, ?Z
x1069:	jes	1
x106a:	hlt	ERR_CODE
x106b:	tw	r1, temp3
x106d:	cwt	r1, 0
x106e:	jes	1
x106f:	hlt	ERR_CODE

x1070:	lwt	r3, 0
x1071:	mb	nb0
x1073:	lpc	r3
x1074:	mb	nb15
x1076:	tw	r1, pat10
x1078:	pw	r1, temp3
x107a:	xm	r1, temp3
x107c:	cw	r0, ?Z
x107e:	jes	1
x107f:	hlt	ERR_CODE
x1080:	tw	r1, temp3
x1082:	cwt	r1, 0
x1083:	jes	1
x1084:	hlt	ERR_CODE

x1085:	lwt	r3, 0
x1086:	mb	nb0
x1088:	lpc	r3
x1089:	mb	nb15
x108b:	tw	r1, pat10
x108d:	pw	r1, temp3
x108f:	tw	r1, pat01
x1091:	xm	r1, temp3
x1093:	cwt	r0, 0
x1094:	jes	1
x1095:	hlt	ERR_CODE
x1096:	tw	r1, temp3
x1098:	cwt	r1, -1
x1099:	jes	1
x109a:	hlt	ERR_CODE

x109b:	lwt	r3, 0
x109c:	mb	nb0
x109e:	lpc	r3
x109f:	mb	nb15
x10a1:	tw	r1, pat01
x10a3:	pw	r1, temp3
x10a5:	tw	r1, pat10
x10a7:	xm	r1, temp3
x10a9:	cwt	r0, 0
x10aa:	jes	1
x10ab:	hlt	ERR_CODE
x10ac:	tw	r1, temp3
x10ae:	cwt	r1, -1
x10af:	jes	1
x10b0:	hlt	ERR_CODE

; td (NB=15)

x10b1:	lwt	r1, -1
x10b2:	lwt	r2, -1
x10b3:	lwt	r3, -1
x10b4:	lwt	r4, -1
x10b5:	lwt	r5, -1
x10b6:	lwt	r6, -1
x10b7:	lwt	r7, -1
x10b8:	td	pat01
x10ba:	cw	r1, 0x5555
x10bc:	jes	1
x10bd:	hlt	ERR_CODE
x10be:	cw	r2, 0xaaaa
x10c0:	jes	1
x10c1:	hlt	ERR_CODE
x10c2:	cwt	r3, -1
x10c3:	jes	1
x10c4:	ujs	nb_td_err
x10c5:	cwt	r4, -1
x10c6:	jes	1
x10c7:	ujs	nb_td_err
x10c8:	cwt	r5, -1
x10c9:	jes	1
x10ca:	ujs	nb_td_err
x10cb:	cwt	r6, -1
x10cc:	jes	1
x10cd:	ujs	nb_td_err
x10ce:	cwt	r7, -1
x10cf:	jes	1
nb_td_err:
x10d0:	hlt	ERR_CODE

; lb (NB=15)

x10d1:	lwt	r1, 0
x10d2:	lb	r1, pat01 << 1
x10d4:	cw	r1, 0x55
x10d6:	jes	1
x10d7:	hlt	ERR_CODE

x10d8:	lwt	r1, -1
x10d9:	lwt	r2, 0
x10da:	pw	r2, temp3
x10dc:	lb	r1, (temp3 << 1) + 1
x10de:	cw	r1, 0xff00
x10e0:	jes	1
x10e1:	hlt	ERR_CODE

; rb (NB=15)

x10e2:	lwt	r1, -1
x10e3:	lwt	r2, 0
x10e4:	pw	r2, temp3
x10e6:	rb	r1, (temp3 << 1) + 1
x10e8:	tw	r2, temp3
x10ea:	cw	r2, 0xff
x10ec:	jes	1
x10ed:	hlt	ERR_CODE

x10ee:	lwt	r1, -1
x10ef:	pw	r1, temp3
x10f1:	lwt	r1, 0
x10f2:	rb	r1, temp3 << 1
x10f4:	tw	r1, temp3
x10f6:	cw	r1, 0xff
x10f8:	jes	1
x10f9:	hlt	ERR_CODE

; cb (NB=15)

x10fa:	lwt	r3, 0
x10fb:	mb	nb0
x10fd:	lpc	r3
x10fe:	mb	nb15
x1100:	pw	r0, temp3
x1102:	lwt	r1, 0
x1103:	cb	r1, temp3 << 1
x1105:	cw	r0, ?E
x1107:	jes	1
x1108:	hlt	ERR_CODE

x1109:	lwt	r1, -1
x110a:	cb	r1, (temp3 << 1) + 1
x110c:	cw	r0, ?G
x110e:	jes	1
x110f:	hlt	ERR_CODE

x1110:	lwt	r1, -1
x1111:	pw	r1, temp3
x1113:	lwt	r1, 0
x1114:	cb	r1, temp3 << 1
x1116:	cw	r0, ?L
x1118:	jes	1
x1119:	hlt	ERR_CODE

; initial tests run in NB=15

x111a:	lw	r1, nb15_initial_exl_proc
x111c:	rw	r1, EXLV
x111e:	lj	nb15_initial_copy
x1120:	mb	nb0
x1122:	sp	nb15_initial_sp ; jumps to nb15_initial_copy_start in NB=15

nb15_initial_copy_start:
x1124:	lwt	r1, 0
x1125:	lw	r1, pat10
x1127:	cw	r1, pat10
x1129:	jes	1
x112a:	hlt	ERR_CODE

x112b:	lwt	r1, 0
x112c:	lw	r1, [pat10]
x112e:	cw	r1, 0xaaaa
x1130:	jes	1
x1131:	hlt	ERR_CODE

x1132:	lwt	r1, 0
x1133:	lw	r1, [pat01]
x1135:	cw	r1, 0x5555
x1137:	jes	1
x1138:	hlt	ERR_CODE

x1139:	lwt	r1, 0
x113a:	lw	r1, [seven_regs]
x113c:	cwt	r1, 1
x113d:	jes	1
x113e:	hlt	ERR_CODE

x113f:	lwt	r1, 0
x1140:	rw	r1, temp3
x1142:	cw	r1, [temp3]
x1144:	jes	1
x1145:	hlt	ERR_CODE

x1146:	lwt	r1, -1
x1147:	rw	r1, temp3
x1149:	cw	r1, [temp3]
x114b:	jes	1
x114c:	hlt	ERR_CODE

x114d:	lw	r1, [pat10]
x114f:	rw	r1, temp3
x1151:	cw	r1, [temp3]
x1153:	jes	1
x1154:	hlt	ERR_CODE

x1155:	lw	r1, [pat01]
x1157:	rw	r1, temp3
x1159:	cw	r1, [temp3]
x115b:	jes	1
x115c:	hlt	ERR_CODE

x115d:	lw	r1, temp3
x115f:	rw	r1, temp4
x1161:	lwt	r2, 0
x1162:	rw	r2, [temp4]
x1164:	cw	r2, [temp3]
x1166:	jes	1
x1167:	hlt	ERR_CODE

x1168:	lwt	r2, -1
x1169:	rw	r2, [temp4]
x116b:	cw	r2, [temp3]
x116d:	jes	1
x116e:	hlt	ERR_CODE

x116f:	lw	r2, [pat01]
x1171:	rw	r2, [temp4]
x1173:	cw	r2, [temp3]
x1175:	jes	1
x1176:	hlt	ERR_CODE

x1177:	lw	r2, [pat10]
x1179:	rw	r2, [temp4]
x117b:	cw	r2, [temp3]
x117d:	jes	1
x117e:	hlt	ERR_CODE
x117f:	exl	0 ; effectively exits to NB=0 @nb15_initial_exl_proc
nb15_initial_copy_end:

nb15_initial_exl_proc:

; test kodów nielegalnych (NB=15)

x1180:	ujs	1
illegal_op_intrs:
x1181:	.word	0
x1182:	lw	r1, stack
x1184:	rw	r1, stackp
x1186:	mcl

; single illegal ops

x1187:	lwt	r1, 0
illegal_op_loop1:
x1188:	lw	r2, [illegal_opcodes1+r1]
x118a:	rj	r7, exec_illegal
x118c:	awt	r1, 1
x118d:	cwt	r1, 19
x118e:	jes	1
x118f:	ujs	illegal_op_loop1

; ranges of illegal ops

x1190:	lwt	r1, 0
illegal_op_loop2:
x1191:	lw	r3, [illegal_opcodes2_cnt+r1]
x1193:	lw	r2, [illegal_opcodes2+r1]
illegal_op_next:
x1195:	rj	r7, exec_illegal
x1197:	drb	r3, illegal_op_add
x1198:	awt	r1, 1
x1199:	cwt	r1, 12
x119a:	jes	test_user_illegal
x119b:	ujs	illegal_op_loop2
illegal_op_add:
x119c:	awt	r2, 1
x119d:	ujs	illegal_op_next

exec_illegal:
x119e:	rws	r2, the_illegal_op
the_illegal_op:
x119f:	.res	1
x11a0:	ki	illegal_op_intrs
x11a2:	lws	r4, illegal_op_intrs
x11a3:	cw	r4, I_ILLEGAL ; is illegal instruction interrupt set?
x11a5:	jes	1
x11a6:	hlt	ERR_CODE
x11a7:	mcl
x11a8:	uj	r7

; ???

x11a9:	.word	0x03ff
x11aa:	.word	0

illegal_opcodes1:
x11ab:	.word	0
x11ac:	.word	0x0400
x11ad:	.word	0x0800
x11ae:	.word	0x0c00
x11af:	.word	0x1000
x11b0:	.word	0x1400
x11b1:	.word	0x1800
x11b2:	.word	0x1c00
x11b3:	.word	0x2000
x11b4:	.word	0x2400
x11b5:	.word	0x2800
x11b6:	.word	0x2c00
x11b7:	.word	0x3000
x11b8:	.word	0x3400
x11b9:	.word	0x3800
x11ba:	.word	0x3c00
x11bb:	.word	0xed40
x11bc:	.word	0xed80
x11bd:	.word	0xedc0

illegal_opcodes2:
x11be:	.word	0xe80a
x11bf:	.word	0xea0a
x11c0:	.word	0xe818
x11c1:	.word	0xea18
x11c2:	.word	0xe820
x11c3:	.word	0xea20
x11c4:	.word	0xe828
x11c5:	.word	0xea28
x11c6:	.word	0xe830
x11c7:	.word	0xea30
x11c8:	.word	0xe838
x11c9:	.word	0xea38

illegal_opcodes2_cnt: ; n consecutive ops after the base one
x11ca:	.word	6
x11cb:	.word	6
x11cc:	.word	8
x11cd:	.word	8
x11ce:	.word	8
x11cf:	.word	8
x11d0:	.word	8
x11d1:	.word	8
x11d2:	.word	8
x11d3:	.word	8
x11d4:	.word	8
x11d5:	.word	8

test_user_illegal:

; test nielegalnych rozkazów w bloku użytkowym (Q=1, NB=1)

x11d6:	lw	r1, int_user_illegal_proc
x11d8:	rw	r1, INTV_ILLEGAL
x11da:	lw	r1, int_user_illegal_exl_proc
x11dc:	rw	r1, EXLV
x11de:	lf	user_illegal_stack ; trick for jumping to user code with 'lip' later on
x11e0:	rf	stack
x11e2:	lw	r1, stack+4
x11e4:	rw	r1, stackp

; configure memory in segment 1

x11e6:	lwt	r1, 0\MEM_PAGE | 1\MEM_SEGMENT
x11e7:	lw	r2, 2\MEM_FRAME | 0\MEM_MODULE | MEM_CFG
x11e9:	lwt	r3, 0
io_user_repeat:
x11ea:	ou	r1, r2
x11eb:	.word	io_user_no, io_user_en, io_user_ok, io_user_pe

	.const	nb1_code_start 0x100

io_user_okreg_test_dest2:
x11ef:	mb	nb1
x11f1:	ld	leave_to_nb0_snippet
x11f3:	pd	nb1_code_start+1
x11f5:	lwt	r2, 0
int_user_illegal_next:
x11f6:	lw	r1, [user_illegal_instructions+r2]
x11f8:	pw	r1, nb1_code_start ; writes the illegal instruction @ 0x100 for later execution
x11fa:	lip ; jump to user code @ 0x100 (as described by user_illegal_stack)

int_user_illegal_proc:
x11fb:	awt	r2, 1 ; select next illegal instruction
x11fc:	cwt	r2, 16
x11fd:	jes	int_user_illegal_done
x11fe:	lw	r1, nb1_code_start
x1200:	rw	r1, stack
x1202:	ujs	int_user_illegal_next
int_user_illegal_done:
x1203:	lw	r1, int06
x1205:	rw	r1, INTV_ILLEGAL
x1207:	lw	r1, stack
x1209:	rw	r1, stackp
x120b:	uj	int_user_illegal_finished

int_user_illegal_exl_proc:
x120d:	ki	illegal_op_intrs
x120f:	lw	r4, [illegal_op_intrs]
x1211:	hlt	ERR_CODE
x1212:	ujs	int_user_illegal_proc

user_illegal_stack:
x1213:	.word	nb1_code_start	; IC (NB=1)
x1214:	.word	0		; R0
x1215:	.word	IMASK_GROUP_H | 1\SR_Q | 1\SR_NB	; SR

leave_to_nb0_snippet:
x1216:	exl	0 ; this should never be reached, each illegal istruction should set the interrupt
x1217:	ujs	-3 ; jump to 0x100 (NB=1)
nb1:
x1218:	.word	1

io_user_no:
x1219:	awt	r3, 1
io_user_en:
x121a:	awt	r3, 1
io_user_pe:
x121b:	awt	r3, 1
x121c:	hlt	ERR_CODE
x121d:	ujs	io_user_repeat
io_user_ok:
x121e:	ujs	io_user_okreg_test_dest2

user_illegal_instructions:
x121f:	hlt	ERR_CODE
x1220:	mcl
x1221:	cit
x1222:	sil
x1223:	siu
x1224:	sit
x1225:	giu
x1226:	gil
x1227:	lip
x1228:	mb	r0
x1229:	im	r0
x122a:	ki	r0
x122b:	fi	r0
x122c:	sp	r0
x122d:	ou	r0, r0
x122e:	in	r0, r0

int_user_illegal_finished:

; software interrupts

x122f:	ujs	soft_int_test

soft_int_test_vectors:
x1230:	.word	I_MASKABLE - I_SW_H - I_SW_L
x1231:	.word	I_MASKABLE - I_SW_H
x1232:	.word	I_SW_H
x1233:	.word	I_SW_H | I_SW_L

soft_int_siu_vectors:
x1234:	.word	I_MASKABLE - I_SW_L
x1235:	.word	I_MASKABLE
x1236:	.word	I_SW_H
x1237:	.word	I_SW_H | I_SW_L

soft_int_sil_vectors:
x1238:	.word	I_MASKABLE - I_SW_H
x1239:	.word	I_MASKABLE - I_SW_H
x123a:	.word	I_SW_H | I_SW_L
x123b:	.word	I_SW_H | I_SW_L

soft_int_cit_vectors:
x123c:	.word	I_MASKABLE - I_SW_H - I_SW_L
x123d:	.word	I_MASKABLE - I_SW_H - I_SW_L
x123e:	.word	I_NONE
x123f:	.word	I_NONE

soft_int_sit_vectors:
x1240:	.word	I_MASKABLE
x1241:	.word	I_MASKABLE
x1242:	.word	I_SW_H | I_SW_L
x1243:	.word	I_SW_H | I_SW_L

soft_int_test:

x1244:	mcl

x1245:	lwt	r4, -4
siu_loop:
x1246:	lw	r1, [soft_int_test_vectors+4+r4]
x1248:	lw	r3, [soft_int_siu_vectors+4+r4]
x124a:	rws	r1, intr_soft
x124b:	fi	intr_soft
x124d:	siu
x124e:	ki	intr_soft
x1250:	lws	r2, intr_soft
x1251:	cw	r2, r3
x1252:	jes	1
x1253:	hlt	ERR_CODE
x1254:	irb	r4, siu_loop

x1255:	lwt	r4, -4
sil_loop:
x1256:	lw	r1, [soft_int_test_vectors+4+r4]
x1258:	lw	r3, [soft_int_sil_vectors+4+r4]
x125a:	rws	r1, intr_soft
x125b:	fi	intr_soft
x125d:	sil
x125e:	ki	intr_soft
x1260:	lws	r2, intr_soft
x1261:	cw	r2, r3
x1262:	jes	1
x1263:	hlt	ERR_CODE
x1264:	irb	r4, sil_loop

x1265:	lwt	r4, -4
cit_loop:
x1266:	lw	r1, [soft_int_test_vectors+4+r4]
x1268:	lw	r3, [soft_int_cit_vectors+4+r4]
x126a:	rws	r1, intr_soft
x126b:	fi	intr_soft
x126d:	cit
x126e:	ki	intr_soft
x1270:	lws	r2, intr_soft
x1271:	cw	r2, r3
x1272:	jes	2
x1273:	hlt	ERR_CODE
intr_soft:
x1274:	.word	0
x1275:	irb	r4, cit_loop

x1276:	lwt	r4, -4
sit_loop:
x1277:	lw	r1, [soft_int_test_vectors+4+r4]
x1279:	lw	r3, [soft_int_sit_vectors+4+r4]
x127b:	rws	r1, intr_soft
x127c:	fi	intr_soft
x127e:	sit
x127f:	ki	intr_soft
x1281:	lws	r2, intr_soft
x1282:	cw	r2, r3
x1283:	jes	1
x1284:	hlt	ERR_CODE
x1285:	irb	r4, sit_loop

; ?

x1286:	nop
x1287:	rky	r1
x1288:	bn	r1, 1\7 ; key 7 mask
x128a:	ujs	test_soft_awp ; key 7 up
x128b:	uj	test_registers ; key 7 down

soft_awp_intr:
x128d:	.word	0

soft_awp_current_op_idx:
x128e:	.word	0
soft_awp_mask:
x128f:	.word	IMASK_ALL

	.const	AWP_OP_N_ARG 0xff

soft_awp_stack_template:
x1290:	.word	awp_op_insert+1	; IC
x1291:	.word	-1		; R0
x1292:	.word	IMASK_ALL	; SR
x1293:	.word	AWP_OP_N_ARG	; N

x1294:	.word	0x0066
soft_awp_ops:
x1295:	nrf
x1296:	ad	r4
x1297:	sd	r4
x1298:	mw	r4
x1299:	dw	r4
x129a:	af	r4
x129b:	sf	r4
x129c:	mf	r4
x129d:	df	r4

soft_awp_init_vectors:
x129e:	.word	soft_awp_proc+8
x129f:	.word	soft_awp_proc+7
x12a0:	.word	soft_awp_proc+6
x12a1:	.word	soft_awp_proc+5
x12a2:	.word	soft_awp_proc+4
x12a3:	.word	soft_awp_proc+3
x12a4:	.word	soft_awp_proc+2
x12a5:	.word	soft_awp_proc+1
x12a6:	.word	soft_awp_proc

test_soft_awp:
x12a7:	mcl
x12a8:	la	soft_awp_init_vectors
x12aa:	ra	SFPV+3 ; first 3 'nrf' variants are skipped
x12ac:	ld	soft_awp_init_vectors+7
x12ae:	rd	SFPV+3+7
x12b0:	lwt	r6, 9
soft_awp_next:
x12b1:	rws	r6, soft_awp_current_op_idx
x12b2:	lj	soft_awp_routine
x12b4:	lws	r6, soft_awp_current_op_idx
x12b5:	drb	r6, soft_awp_next
x12b6:	uj	test_registers ; all done, start the next test

; ------------------------------------------------------------------------
soft_awp_routine:
x12b8:	.res	1
x12b9:	lw	r2, stack
x12bb:	rw	r2, stackp
x12bd:	lwt	r3, -4
soft_awp_clear_stack_loop:
x12be:	ri	r2, 0
x12c0:	irb	r3, soft_awp_clear_stack_loop
x12c1:	im	soft_awp_mask
x12c3:	lwt	r0, -1
x12c4:	lw	r1, [soft_awp_ops-1+r6]
x12c6:	rw	r1, awp_op_insert
x12c8:	lw	r4, AWP_OP_N_ARG
x12ca:	lwt	r7, 0
awp_op_insert:
x12cb:	.res	1
x12cc:	hlt	ERR_CODE
x12cd:	uj	[soft_awp_routine]

soft_awp_proc:
x12cf:	awt	r7, 1
x12d0:	awt	r7, 1
x12d1:	awt	r7, 1
x12d2:	awt	r7, 1
x12d3:	awt	r7, 1
x12d4:	awt	r7, 1
x12d5:	awt	r7, 1
x12d6:	awt	r7, 1
x12d7:	awt	r7, 1
x12d8:	lws	r4, awp_op_insert
x12d9:	rpc	r2
x12da:	cwt	r2, 0
x12db:	jes	2	; -> 0x12de
x12dc:	hlt	ERR_CODE
x12dd:	lwt	r0, 0
x12de:	cw	r7, r6 ; r6 == instruction index?
x12df:	or	r0, r6 ; ???
x12e0:	jes	1
x12e1:	hlt	ERR_CODE

x12e2:	lf	stack
x12e4:	ll	soft_awp_stack_template
x12e6:	cw	r1, r5 ; IC ok?
x12e7:	jes	1
x12e8:	hlt	ERR_CODE

x12e9:	cw	r2, r6 ; R0 ok?
x12ea:	jes	1
x12eb:	hlt	ERR_CODE

x12ec:	cw	r3, r7 ; SR ok?
x12ed:	jes	1
x12ee:	hlt	ERR_CODE

x12ef:	lw	r3, [stack+3]
x12f1:	ld	soft_awp_stack_template+3
x12f3:	cw	r3, r1 ; N ok?
x12f4:	jes	1
x12f5:	hlt	ERR_CODE

x12f6:	lw	r1, [STACKP]
x12f8:	cw	r1, r2 ; stack pointer ok?
x12f9:	jes	1
x12fa:	hlt	ERR_CODE

; software AWP should mask interrupts 28-31 (SR9)

x12fb:	lw	r1, soft_awp_int_sw_low_proc
x12fd:	rw	r1, INTV_SW_L
x12ff:	lwt	r1, 1 ; software low interrupt
x1300:	rw	r1, soft_awp_intr
x1302:	fi	soft_awp_intr ; this shouldn't report any interrupt
x1304:	mcl
x1305:	ujs	3	; -> 0x1309
soft_awp_int_sw_low_proc:
x1306:	lw	r0, [soft_awp_current_op_idx]
x1308:	hlt	ERR_CODE
x1309:	lw	r1, int31
x130b:	rw	r1, INTV_SW_L
x130d:	uj	[soft_awp_routine]

; ------------------------------------------------------------------------
nb15_copy:
x130f:	.res	1
x1310:	lw	r1, nb15_copy_start
x1312:	lwt	r2, -24
x1313:	lw	r3, nb15_copy_dest ; destination start (in NB=15)
nb15_copy_loop:
x1315:	lw	r4, [r1]
x1316:	pw	r4, r3
x1317:	awt	r3, 1
x1318:	awt	r1, 1
x1319:	irb	r2, nb15_copy_loop
x131a:	uj	[nb15_copy]

; ------------------------------------------------------------------------
nb15_initial_copy:
x131c:	.res	1
x131d:	lw	r1, nb15_initial_copy_start
nb15_initial_copy_loop:
x131f:	lw	r2, [r1]
x1320:	pw	r2, r1
x1321:	awt	r1, 1
x1322:	cw	r1, nb15_initial_copy_end
x1324:	jes	1
x1325:	ujs	nb15_initial_copy_loop
x1326:	uj	[nb15_initial_copy]

nb0:
x1328:	.word	0
nb15_copy_dest:
x1329:	.word	0
x132a:	.word	1

pat_aaaa:
x132b:	.word	0b1010101010101010
x132c:	.word	0
x132d:	.word	0
temp3:
x132e:	.word	0
temp4:
x132f:	.word	0
temp5:
x1330:	.word	0
x1331:	.word	0
x1332:	.word	0
x1333:	.word	0
x1334:	.word	0
x1335:	.word	0
x1336:	.word	0
x1337:	.word	0

pat01:
x1338:	.word	0b0101010101010101
pat10:
x1339:	.word	0b1010101010101010

seven_regs:
x133a:	.word	1
x133b:	.word	2
x133c:	.word	3
x133d:	.word	4
x133e:	.word	5
x133f:	.word	6
x1340:	.word	7

nb15:
x1341:	.word	IMASK_IFPOWER | 15\SR_NB ; SR
x1342:	.word	0x0041

nb15_initial_sp:
x1343:	.word	nb15_initial_copy_start ; IC
x1344:	.word	0 ; r0
x1345:	.word	IMASK_NONE | 1\SR_Q | 15\SR_NB ; SR

nb15_copy_start:
x1346:	.word	0
x1347:	.word	1
x1348:	.word	0b1010101010101010
x1349:	.word	0
x134a:	.word	0
x134b:	.word	0
x134c:	.word	0
x134d:	.word	0
x134e:	.word	0
x134f:	.word	0
x1350:	.word	0
x1351:	.word	0
x1352:	.word	0
x1353:	.word	0
x1354:	.word	0
x1355:	.word	0b0101010101010101
x1356:	.word	0b1010101010101010
x1357:	.word	1
x1358:	.word	2
x1359:	.word	3
x135a:	.word	4
x135b:	.word	5
x135c:	.word	6
x135d:	.word	7
; end of nb15 data

; register pack tests
test_registers:
x135e:	ujs	1
x135f:	.word	0
x1360:	lw	r2, stack
x1362:	lw	r1, -94
_loop:
x1364:	ri	r2, 0
x1366:	irb	r1, _loop
x1367:	lw	r1, stack
x1369:	rw	r1, STACKP
x136b:	mcl
x136c:	ujs	6	; -> 0x1373

reg_test_tmp:
x136d:	.word	0
x136e:	.word	0

reg_testv:
x136f:	.word	0
x1370:	.word	0xffff
x1371:	.word	0x5555
x1372:	.word	0xaaaa

x1373:	lw	r1, [reg_testv+0]
x1375:	rw	r1, reg_test_tmp+1
x1377:	lj	regs_load_all
x1379:	lj	regs_copy_vals
x137b:	lw	r1, [reg_testv+1]
x137d:	rw	r1, reg_test_tmp+1
x137f:	lj	regs_load_all
x1381:	lj	regs_copy_vals
x1383:	lw	r1, [reg_testv+2]
x1385:	rw	r1, reg_test_tmp+1
x1387:	lj	regs_load_all
x1389:	lj	regs_copy_vals
x138b:	lw	r1, [reg_testv+3]
x138d:	rw	r1, reg_test_tmp+1
x138f:	lj	regs_load_all
x1391:	lj	regs_copy_vals
x1393:	uj	reg_test_phase2

; ------------------------------------------------------------------------
regs_copy_vals:
x1395:	.res	1
x1396:	lw	r1, [reg_testv+0]
x1398:	rw	r1, reg_test_tmp
x139a:	lj	regs_do_test1
x139c:	lw	r1, [reg_testv+1]
x139e:	rw	r1, reg_test_tmp
x13a0:	lj	regs_do_test1
x13a2:	lw	r1, [reg_testv+2]
x13a4:	rw	r1, reg_test_tmp
x13a6:	lj	regs_do_test1
x13a8:	lw	r1, [reg_testv+3]
x13aa:	rw	r1, reg_test_tmp
x13ac:	lj	regs_do_test1
x13ae:	uj	[regs_copy_vals]

; ------------------------------------------------------------------------
regs_load_all:
x13b0:	.res	1
x13b1:	lw	r1, [reg_test_tmp+1]
x13b3:	lw	r2, [reg_test_tmp+1]
x13b5:	lw	r3, [reg_test_tmp+1]
x13b7:	lw	r4, [reg_test_tmp+1]
x13b9:	lw	r5, [reg_test_tmp+1]
x13bb:	lw	r6, [reg_test_tmp+1]
x13bd:	lw	r7, [reg_test_tmp+1]
x13bf:	lw	r0, [reg_test_tmp+1]
x13c1:	uj	[regs_load_all]

; ------------------------------------------------------------------------
regs_do_test1:
x13c3:	.res	1
x13c4:	lw	r1, [reg_test_tmp+1]
x13c6:	lw	r1, [reg_test_tmp]
x13c8:	cw	r1, [reg_test_tmp]
x13ca:	jes	1
x13cb:	hlt	ERR_CODE
x13cc:	lw	r1, [reg_test_tmp+1]
x13ce:	lj	regs_do_test2
x13d0:	lw	r2, [reg_test_tmp]
x13d2:	cw	r2, [reg_test_tmp]
x13d4:	jes	1
x13d5:	hlt	ERR_CODE
x13d6:	lw	r2, [reg_test_tmp+1]
x13d8:	lj	regs_do_test2
x13da:	lw	r3, [reg_test_tmp]
x13dc:	cw	r3, [reg_test_tmp]
x13de:	jes	1
x13df:	hlt	ERR_CODE
x13e0:	lw	r3, [reg_test_tmp+1]
x13e2:	lj	regs_do_test2
x13e4:	lw	r4, [reg_test_tmp]
x13e6:	cw	r4, [reg_test_tmp]
x13e8:	jes	1
x13e9:	hlt	ERR_CODE
x13ea:	lw	r4, [reg_test_tmp+1]
x13ec:	lj	regs_do_test2
x13ee:	lw	r5, [reg_test_tmp]
x13f0:	cw	r5, [reg_test_tmp]
x13f2:	jes	1
x13f3:	hlt	ERR_CODE
x13f4:	lw	r5, [reg_test_tmp+1]
x13f6:	lj	regs_do_test2
x13f8:	lw	r6, [reg_test_tmp]
x13fa:	cw	r6, [reg_test_tmp]
x13fc:	jes	1
x13fd:	hlt	ERR_CODE
x13fe:	lw	r6, [reg_test_tmp+1]
x1400:	lj	regs_do_test2
x1402:	lw	r7, [reg_test_tmp]
x1404:	cw	r7, [reg_test_tmp]
x1406:	jes	1
x1407:	hlt	ERR_CODE
x1408:	lw	r7, [reg_test_tmp+1]
x140a:	lj	regs_do_test2
x140c:	lw	r0, [reg_test_tmp]
x140e:	cw	r0, [reg_test_tmp]
x1410:	jes	1
x1411:	hlt	ERR_CODE
x1412:	lw	r0, [reg_test_tmp+1]
x1414:	lj	regs_do_test2
x1416:	uj	[regs_do_test1]

; ------------------------------------------------------------------------
regs_do_test2:
x1418:	.word	0
x1419:	cw	r1, [reg_test_tmp+1]
x141b:	jes	1
x141c:	hlt	ERR_CODE
x141d:	cw	r2, [reg_test_tmp+1]
x141f:	jes	1
x1420:	hlt	ERR_CODE
x1421:	cw	r3, [reg_test_tmp+1]
x1423:	jes	1
x1424:	hlt	ERR_CODE
x1425:	cw	r4, [reg_test_tmp+1]
x1427:	jes	1
x1428:	hlt	ERR_CODE
x1429:	cw	r5, [reg_test_tmp+1]
x142b:	jes	1
x142c:	hlt	ERR_CODE
x142d:	cw	r6, [reg_test_tmp+1]
x142f:	jes	1
x1430:	hlt	ERR_CODE
x1431:	cw	r7, [reg_test_tmp+1]
x1433:	jes	1
x1434:	hlt	ERR_CODE
x1435:	rw	r0, reg_test_tmp2
x1437:	lw	r7, [reg_test_tmp2]
x1439:	cw	r1, [reg_testv]
x143b:	jes	x1446
x143c:	cw	r1, [reg_testv+1]
x143e:	jes	x144a
x143f:	cw	r1, [reg_testv+2]
x1441:	jes	x144e
x1442:	cw	r7, [reg_testv2+3]
x1444:	jes	x1452
x1445:	ujs	x1451
x1446:	cw	r7, [reg_testv2]
x1448:	jes	x1452
x1449:	ujs	x1451
x144a:	cw	r7, [reg_testv2+1]
x144c:	jes	x1452
x144d:	ujs	x1451
x144e:	cw	r1, [reg_testv2+2]
x1450:	jes	1
x1451:	hlt	ERR_CODE
x1452:	lw	r7, [reg_test_tmp+1]
x1454:	uj	[regs_do_test2]

reg_testv2:
x1456:	.word	0x0400
x1457:	.word	0xf5ff
x1458:	.word	0x5555
x1459:	.word	0xa4aa
reg_test_tmp2:
x145a:	.word	0

reg_test_phase2:
x145b:	lw	r1, [reg_testv]
x145d:	rw	r1, reg_test_tmp+1
x145f:	lj	regs_load_all
x1461:	lj	regs_copy_vals2
x1463:	lw	r1, [reg_testv+1]
x1465:	rw	r1, reg_test_tmp+1
x1467:	lj	regs_load_all
x1469:	lj	regs_copy_vals2
x146b:	lw	r1, [reg_testv+2]
x146d:	rw	r1, reg_test_tmp+1
x146f:	lj	regs_load_all
x1471:	lj	regs_copy_vals2
x1473:	lw	r1, [reg_testv+3]
x1475:	rw	r1, reg_test_tmp+1
x1477:	lj	regs_load_all
x1479:	lj	regs_copy_vals2
x147b:	uj	_next_test

; ------------------------------------------------------------------------
regs_copy_vals2:
x147d:	.res	1
x147e:	lw	r1, [reg_testv]
x1480:	rw	r1, reg_test_tmp
x1482:	lj	regs_do_test3
x1484:	lw	r1, [reg_testv+1]
x1486:	rw	r1, reg_test_tmp
x1488:	lj	regs_do_test3
x148a:	lw	r1, [reg_testv+2]
x148c:	rw	r1, reg_test_tmp
x148e:	lj	regs_do_test3
x1490:	lw	r1, [reg_testv+3]
x1492:	rw	r1, reg_test_tmp
x1494:	lj	regs_do_test3
x1496:	uj	[regs_copy_vals2]

; ------------------------------------------------------------------------
regs_do_test3:
x1498:	.res	1
x1499:	lw	r1, [reg_test_tmp]
x149b:	lw	r2, r1
x149c:	cw	r1, r2
x149d:	jes	1
x149e:	hlt	ERR_CODE
x149f:	lw	r1, [reg_test_tmp+1]
x14a1:	lw	r2, [reg_test_tmp+1]
x14a3:	lj	regs_do_test2
x14a5:	lw	r2, [reg_test_tmp]
x14a7:	lw	r3, r2
x14a8:	cw	r2, r3
x14a9:	jes	1
x14aa:	hlt	ERR_CODE
x14ab:	lw	r2, [reg_test_tmp+1]
x14ad:	lw	r3, [reg_test_tmp+1]
x14af:	lj	regs_do_test2
x14b1:	lw	r3, [reg_test_tmp]
x14b3:	lw	r4, r3
x14b4:	cw	r3, r4
x14b5:	jes	1
x14b6:	hlt	ERR_CODE
x14b7:	lw	r3, [reg_test_tmp+1]
x14b9:	lw	r4, [reg_test_tmp+1]
x14bb:	lj	regs_do_test2
x14bd:	lw	r4, [reg_test_tmp]
x14bf:	lw	r5, r4
x14c0:	cw	r4, r5
x14c1:	jes	1
x14c2:	hlt	ERR_CODE
x14c3:	lw	r4, [reg_test_tmp+1]
x14c5:	lw	r5, [reg_test_tmp+1]
x14c7:	lj	regs_do_test2
x14c9:	lw	r5, [reg_test_tmp]
x14cb:	lw	r6, r5
x14cc:	cw	r5, r6
x14cd:	jes	1
x14ce:	hlt	ERR_CODE
x14cf:	lw	r5, [reg_test_tmp+1]
x14d1:	lw	r6, [reg_test_tmp+1]
x14d3:	lj	regs_do_test2
x14d5:	lw	r6, [reg_test_tmp]
x14d7:	lw	r7, r6
x14d8:	cw	r6, r7
x14d9:	jes	1
x14da:	hlt	ERR_CODE
x14db:	lw	r6, [reg_test_tmp+1]
x14dd:	lw	r7, [reg_test_tmp+1]
x14df:	lj	regs_do_test2
x14e1:	lw	r7, [reg_test_tmp]
x14e3:	lw	r1, r7
x14e4:	cw	r7, r1
x14e5:	jes	1
x14e6:	hlt	ERR_CODE
x14e7:	lw	r7, [reg_test_tmp+1]
x14e9:	lw	r1, [reg_test_tmp+1]
x14eb:	lj	regs_do_test2
x14ed:	uj	[regs_do_test3]

; testy pętli podstawowej

_next_test:

x14ef:	ujs	1
x14f0:	.word	0
x14f1:	lj	reset_stack
x14f3:	ujs	x14fa

md_testv:
x14f4:	.word	13
x14f5:	.word	15
x14f6:	.word	13+15
x14f7:	.word	2*(13+15)
x14f8:	.word	md_testv+5
x14f9:	.word	-1

; ric

x14fa:	ric	r1
ric_here:
x14fb:	cw	r1, ric_here
x14fd:	jes	1
x14fe:	hlt	ERR_CODE

; pre-, b-, d- mod

x14ff:	lw	r2, [md_testv+1]
x1501:	lw	r3, [md_testv]
x1503:	md	r2
x1504:	lw	r1, r3
x1505:	cw	r1, [md_testv+2]
x1507:	jes	1
x1508:	hlt	ERR_CODE

x1509:	lw	r2, [md_testv]
x150b:	lw	r3, [md_testv+1]
x150d:	lw	r1, r2+r3
x150e:	cw	r1, [md_testv+2]
x1510:	jes	1
x1511:	hlt	ERR_CODE

x1512:	lw	r4, [md_testv+2]
x1514:	lw	r2, [md_testv]
x1516:	lw	r3, [md_testv+1]
x1518:	md	r1
x1519:	lw	r1, r2+r3
x151a:	cw	r1, [md_testv+3]
x151c:	jes	1
x151d:	hlt	ERR_CODE

x151e:	lw	r2, md_testv
x1520:	lw	r1, [r2]
x1521:	cw	r1, [md_testv]
x1523:	jes	1
x1524:	hlt	ERR_CODE

x1525:	lwt	r1, 1
x1526:	lw	r2, md_testv+1
x1528:	md	r1
x1529:	lw	r1, [r2]
x152a:	cw	r1, [md_testv+2]
x152c:	jes	1
x152d:	hlt	ERR_CODE

x152e:	lwt	r3, 1
x152f:	lw	r2, md_testv+2
x1531:	lw	r1, [r2+r3]
x1532:	cw	r1, [md_testv+3]
x1534:	jes	1
x1535:	hlt	ERR_CODE

x1536:	lwt	r1, 1
x1537:	lwt	r3, 1
x1538:	lw	r2, md_testv+1
x153a:	md	r1
x153b:	lw	r1, [r3+r2]
x153c:	cw	r1, [md_testv+3]
x153e:	jes	1
x153f:	hlt	ERR_CODE

x1540:	lw	r1, -1
x1542:	rw	r1, md_testv+5
x1544:	lw	r2, md_testv+5
x1546:	rz	r2
x1547:	lw	r1, [md_testv+5]
x1549:	cwt	r1, 0
x154a:	jes	1
x154b:	hlt	ERR_CODE

x154c:	lw	r1, -1
x154e:	rw	r1, md_testv+5
x1550:	lw	r2, md_testv+4
x1552:	lwt	r3, 1
x1553:	md	r3
x1554:	rz	r2
x1555:	lw	r1, [md_testv+5]
x1557:	cwt	r1, 0
x1558:	jes	1
x1559:	hlt	ERR_CODE

x155a:	lw	r1, -1
x155c:	rw	r1, md_testv+5
x155e:	lwt	r3, 1
x155f:	lw	r2, md_testv+4
x1561:	rz	r2+r3
x1562:	lw	r1, [md_testv+5]
x1564:	cwt	r1, 0
x1565:	jes	1
x1566:	hlt	ERR_CODE

x1567:	lw	r1, -1
x1569:	rw	r1, md_testv+5
x156b:	lwt	r3, 1
x156c:	lwt	r1, 1
x156d:	lw	r2, md_testv+3
x156f:	md	r1
x1570:	rz	r2+r3
x1571:	lw	r1, [md_testv+5]
x1573:	cwt	r1, 0
x1574:	jes	1
x1575:	hlt	ERR_CODE

x1576:	lw	r1, -1
x1578:	rw	r1, md_testv+5
x157a:	lw	r2, md_testv+4
x157c:	rz	[r2]
x157d:	lw	r1, [md_testv+5]
x157f:	cwt	r1, 0
x1580:	jes	1
x1581:	hlt	ERR_CODE

x1582:	lw	r1, -1
x1584:	rw	r1, md_testv+5
x1586:	lw	r2, md_testv+3
x1588:	lwt	r3, 1
x1589:	md	r3
x158a:	rz	[r2]
x158b:	lw	r1, [md_testv+5]
x158d:	cwt	r1, 0
x158e:	jes	1
x158f:	hlt	ERR_CODE

x1590:	lw	r1, -1
x1592:	rw	r1, md_testv+5
x1594:	lw	r2, md_testv+3
x1596:	lwt	r3, 1
x1597:	rz	[r2+r3]
x1598:	lw	r1, [md_testv+5]
x159a:	cwt	r1, 0
x159b:	jes	1
x159c:	hlt	ERR_CODE

x159d:	lw	r1, -1
x159f:	rw	r1, md_testv+5
x15a1:	lw	r2, md_testv+2
x15a3:	lwt	r3, 1
x15a4:	lwt	r1, 1
x15a5:	md	r1
x15a6:	rz	[r2+r3]
x15a7:	lw	r1, [md_testv+5]
x15a9:	cwt	r1, 0
x15aa:	jes	1
x15ab:	hlt	ERR_CODE

x15ac:	lw	r3, md_testv+3
x15ae:	md	r3
x15af:	lwt	r1, 2
x15b0:	cw	r1, md_testv+5
x15b2:	jes	1
x15b3:	hlt	ERR_CODE

x15b4:	lwt	r0, 0
x15b5:	brc	64
x15b6:	hlt	ERR_CODE

x15b7:	lw	r1, 0xff
x15b9:	lw	r0, 0xff
x15bb:	md	r1
x15bc:	brc	64
x15bd:	hlt	ERR_CODE

x15be:	lwt	r1, 1
x15bf:	md	r1
x15c0:	lw	r2, md_testv
x15c2:	cw	r2, md_testv+1
x15c4:	jes	1
x15c5:	hlt	ERR_CODE

x15c6:	lwt	r1, 1
x15c7:	lw	r2, md_testv+r1
x15c9:	cw	r2, md_testv+1
x15cb:	jes	1
x15cc:	hlt	ERR_CODE

x15cd:	lwt	r1, 1
x15ce:	lwt	r3, 1
x15cf:	md	r1
x15d0:	lw	r2, md_testv+r3
x15d2:	cw	r2, md_testv+2
x15d4:	jes	1
x15d5:	hlt	ERR_CODE

x15d6:	lwt	r1, 1
x15d7:	md	r1
x15d8:	lw	r2, [md_testv+3]
x15da:	cw	r2, md_testv+5
x15dc:	jes	1
x15dd:	hlt	ERR_CODE

x15de:	lwt	r1, 1
x15df:	lw	r2, [md_testv+3+r1]
x15e1:	cw	r2, md_testv+5
x15e3:	jes	1
x15e4:	hlt	ERR_CODE

x15e5:	lwt	r1, 1
x15e6:	lwt	r3, 1
x15e7:	md	r1
x15e8:	lw	r2, [md_testv+2+r3]
x15ea:	cw	r2, md_testv+5
x15ec:	jes	1
x15ed:	hlt	ERR_CODE

x15ee:	lw	r1, -1
x15f0:	rw	r1, md_testv+5
x15f2:	lwt	r1, 1
x15f3:	md	r1
x15f4:	rz	md_testv+4
x15f6:	lw	r2, [md_testv+5]
x15f8:	cwt	r2, 0
x15f9:	jes	1
x15fa:	hlt	ERR_CODE

x15fb:	lw	r1, -1
x15fd:	rw	r1, md_testv+5
x15ff:	lwt	r1, 1
x1600:	rz	md_testv+4+r1
x1602:	lw	r2, [md_testv+5]
x1604:	cwt	r2, 0
x1605:	jes	1
x1606:	hlt	ERR_CODE

x1607:	lw	r1, -1
x1609:	rw	r1, md_testv+5
x160b:	lwt	r1, 1
x160c:	lwt	r3, 1
x160d:	md	r1
x160e:	rz	md_testv+3+r3
x1610:	lw	r2, [md_testv+5]
x1612:	cwt	r2, 0
x1613:	jes	1
x1614:	hlt	ERR_CODE

x1615:	lw	r1, -1
x1617:	rw	r1, md_testv+5
x1619:	lwt	r1, 1
x161a:	md	r1
x161b:	rz	[md_testv+3]
x161d:	lw	r2, [md_testv+5]
x161f:	cwt	r2, 0
x1620:	jes	1
x1621:	hlt	ERR_CODE

x1622:	lw	r1, -1
x1624:	rw	r1, md_testv+5
x1626:	lwt	r1, 1
x1627:	rz	[md_testv+3+r1]
x1629:	lw	r2, [md_testv+5]
x162b:	cwt	r2, 0
x162c:	jes	1
x162d:	hlt	ERR_CODE

x162e:	lw	r1, -1
x1630:	rw	r1, md_testv+5
x1632:	lwt	r1, 1
x1633:	lwt	r3, 1
x1634:	md	r1
x1635:	rz	[md_testv+2+r3]
x1637:	lw	r2, [md_testv+5]
x1639:	cwt	r2, 0
x163a:	jes	1
x163b:	hlt	ERR_CODE

; interrupt system test

x163c:	mcl
x163d:	ujs	itest_memcfg

int00:
x163e:	hlt	ERR_CODE
x163f:	ujs	int00

itest_no:
x1640:	hlt	ERR_CODE

itest_memcfg:
x1641:	lwt	r3, 0\MEM_PAGE | 1\MEM_SEGMENT
x1642:	lw	r4, 2\MEM_FRAME | 0\MEM_MODULE | MEM_CFG
itest_memcfg_restart:
x1644:	ou	r3, r4
x1645:	.word	itest_no, itest_en, itest_ok, itest_pe
itest_en:
x1649:	hlt	ERR_CODE
x164a:	ujs	itest_memcfg_restart
itest_pe:
x164b:	hlt	ERR_CODE
x164c:	ujs	itest_memcfg_restart

itest_mask:
x164d:	.res	1
itest_intr1:
x164e:	.res	1
itest_intr2:
x164f:	.res	1

itest_ok:
x1650:	lj	reset_stack
x1652:	rz	itest_mask
x1654:	im	itest_mask
x1656:	lw	r7, I_MASKABLE
x1658:	rw	r7, itest_intr1
x165a:	fi	itest_intr1

; interrupt I_PARITY

x165c:	lw	r1, itest_int_parity_proc
x165e:	rw	r1, INTV_PARITY
x1660:	lwt	r0, -1
x1661:	lw	r1, IMASK_PARITY
x1663:	rw	r1, itest_mask
x1665:	im	itest_mask
itest_ic_before_int_parity:
x1667:	ujs	itest_int_nomem
itest_int_parity_proc:
x1668:	la	stack
x166a:	cw	r1, itest_ic_before_int_parity
x166c:	jes	1
x166d:	hlt	ERR_CODE
x166e:	cwt	r2, -1
x166f:	jes	1
x1670:	hlt	ERR_CODE
x1671:	cw	r3, IMASK_PARITY
x1673:	jes	1
x1674:	hlt	ERR_CODE
x1675:	cwt	r4, 0
x1676:	jes	1
x1677:	hlt	ERR_CODE
x1678:	lw	r1, [STACKP]
x167a:	cw	r1, stack+4
x167c:	jes	1
x167d:	hlt	ERR_CODE
x167e:	cw	r0, ?E
x1680:	jes	1
x1681:	hlt	ERR_CODE
x1682:	ki	itest_intr2
x1684:	lw	r1, [itest_intr2]
x1686:	cw	r1, I_MASKABLE-I_PARITY ; I_PARITY should be gone now
x1688:	jes	1
x1689:	hlt	ERR_CODE
x168a:	lw	r1, int01
x168c:	rw	r1, INTV_PARITY
x168e:	lip

; interrupt I_NOMEM

itest_int_nomem:
x168f:	lw	r1, itest_int_nomem_proc
x1691:	rw	r1, INTV_NOMEM
x1693:	lw	r1, IMASK_NOMEM
x1695:	rw	r1, itest_mask
x1697:	lwt	r0, -1
x1698:	im	itest_mask
itest_ic_before_int_nomem:
x169a:	ujs	itest_int_cpu_h
itest_int_nomem_proc:
x169b:	la	stack
x169d:	cw	r1, itest_ic_before_int_nomem
x169f:	jes	1
x16a0:	hlt	ERR_CODE
x16a1:	cwt	r2, -1
x16a2:	jes	1
x16a3:	hlt	ERR_CODE
x16a4:	cw	r3, IMASK_NOMEM
x16a6:	jes	1
x16a7:	hlt	ERR_CODE
x16a8:	cwt	r4, 0
x16a9:	jes	1
x16aa:	hlt	ERR_CODE
x16ab:	lw	r1, [STACKP]
x16ad:	cw	r1, stack+4
x16af:	jes	1
x16b0:	hlt	ERR_CODE
x16b1:	cw	r0, ?E
x16b3:	jes	1
x16b4:	hlt	ERR_CODE
x16b5:	ki	itest_intr2
x16b7:	lw	r1, [itest_intr2]
x16b9:	cw	r1, I_MASKABLE-I_PARITY-I_NOMEM ; I_NOMEM should be gone now
x16bb:	jes	1
x16bc:	hlt	ERR_CODE
x16bd:	lw	r1, int02
x16bf:	rw	r1, INTV_NOMEM
x16c1:	lip

; interrupt I_CPU_H

itest_int_cpu_h:
x16c2:	lw	r1, itest_int_cpu_h_proc
x16c4:	rw	r1, INTV_CPU_H
x16c6:	lw	r1, IMASK_CPU_H
x16c8:	rw	r1, itest_mask
x16ca:	lwt	r0, -1
x16cb:	im	itest_mask
itest_ic_before_int_cpu_h:
x16cd:	ujs	itest_int_ifpower
itest_int_cpu_h_proc:
x16ce:	la	stack
x16d0:	cw	r1, itest_ic_before_int_cpu_h
x16d2:	jes	1
x16d3:	hlt	ERR_CODE
x16d4:	cwt	r2, -1
x16d5:	jes	1
x16d6:	hlt	ERR_CODE
x16d7:	cw	r3, IMASK_CPU_H
x16d9:	jes	1
x16da:	hlt	ERR_CODE
x16db:	cwt	r4, 0
x16dc:	jes	1
x16dd:	hlt	ERR_CODE
x16de:	lw	r1, [STACKP]
x16e0:	cw	r1, stack+4
x16e2:	jes	1
x16e3:	hlt	ERR_CODE
x16e4:	cw	r0, ?E
x16e6:	jes	1
x16e7:	hlt	ERR_CODE
x16e8:	ki	itest_intr2
x16ea:	lw	r1, [itest_intr2]
x16ec:	cw	r1, I_MASKABLE-I_PARITY-I_NOMEM-I_CPU_H ; I_CPU_H should be gone now
x16ee:	jes	1
x16ef:	hlt	ERR_CODE
x16f0:	lw	r1, int03
x16f2:	rw	r1, INTV_CPU_H
x16f4:	lip

; interrupt I_IFPOWER

itest_int_ifpower:
x16f5:	lw	r1, itest_int_ifpower_proc
x16f7:	rw	r1, INTV_IFPOWER
x16f9:	lw	r1, IMASK_IFPOWER
x16fb:	rw	r1, itest_mask
x16fd:	lwt	r0, -1
x16fe:	im	itest_mask
itest_ic_before_int_ifpower:
x1700:	ujs	itest_int_cpu
itest_int_ifpower_proc:
x1701:	la	stack
x1703:	cw	r1, itest_ic_before_int_ifpower
x1705:	jes	1
x1706:	hlt	ERR_CODE
x1707:	cwt	r2, -1
x1708:	jes	1
x1709:	hlt	ERR_CODE
x170a:	cw	r3, IMASK_IFPOWER
x170c:	jes	1
x170d:	hlt	ERR_CODE
x170e:	cwt	r4, 0
x170f:	jes	1
x1710:	hlt	ERR_CODE
x1711:	lw	r1, [STACKP]
x1713:	cw	r1, stack+4
x1715:	jes	1
x1716:	hlt	ERR_CODE
x1717:	cw	r0, ?E
x1719:	jes	1
x171a:	hlt	ERR_CODE
x171b:	ki	itest_intr2
x171d:	lw	r1, [itest_intr2]
x171f:	cw	r1, I_MASKABLE-I_PARITY-I_NOMEM-I_CPU_H-I_IFPOWER ; I_IFPOWER should be gone now
x1721:	jes	1
x1722:	hlt	ERR_CODE
x1723:	lw	r1, int04
x1725:	rw	r1, INTV_IFPOWER
x1727:	lip

; interrupts I_TIMER, I_ILLEGAL, I_DIV_OF, I_FP_UF, I_FP_OF, I_FP_ERR, I_UNUSED

itest_int_cpu:
x1728:	lw	r1, itest_int_timer_proc
x172a:	lw	r2, itest_int_illegal_proc
x172c:	lw	r3, itest_int_div_of_proc
x172e:	lw	r4, itest_int_fp_uf_proc
x1730:	lw	r5, itest_int_fp_of_proc
x1732:	lw	r6, itest_int_fp_err_proc
x1734:	lw	r7, itest_int_extra_proc
x1736:	ra	INTV_TIMER ; set vectors for INTV_TIMER - INTV_UNUSED
x1738:	lw	r1, IMASK_GROUP_H
x173a:	rw	r1, itest_mask
x173c:	lwt	r0, -1
x173d:	im	itest_mask
itest_ic_before_int_cpu:
x173f:	uj	itest_oprq

itest_int_timer_proc:
x1741:	la	stack
x1743:	cw	r1, itest_ic_before_int_cpu
x1745:	jes	1
x1746:	hlt	ERR_CODE
x1747:	cwt	r2, -1
x1748:	jes	1
x1749:	hlt	ERR_CODE
x174a:	cw	r3, IMASK_GROUP_H
x174c:	jes	1
x174d:	hlt	ERR_CODE
x174e:	cwt	r4, 0
x174f:	jes	1
x1750:	hlt	ERR_CODE
x1751:	lw	r1, [STACKP]
x1753:	cw	r1, stack+4
x1755:	jes	1
x1756:	hlt	ERR_CODE
x1757:	cw	r0, ?E
x1759:	jes	1
x175a:	hlt	ERR_CODE
x175b:	ki	itest_intr2
x175d:	lw	r1, [itest_intr2]
x175f:	cw	r1, I_MASKABLE-I_PARITY-I_NOMEM-I_CPU_H-I_IFPOWER-I_TIMER
x1761:	jes	1
x1762:	hlt	ERR_CODE
x1763:	lw	r1, int05
x1765:	rw	r1, INTV_TIMER
x1767:	lip

itest_int_illegal_proc:
x1768:	la	stack
x176a:	cw	r1, itest_ic_before_int_cpu
x176c:	jes	1
x176d:	hlt	ERR_CODE
x176e:	cwt	r2, -1
x176f:	jes	1
x1770:	hlt	ERR_CODE
x1771:	cw	r3, IMASK_GROUP_H
x1773:	jes	1
x1774:	hlt	ERR_CODE
x1775:	cwt	r4, 0
x1776:	jes	1
x1777:	hlt	ERR_CODE
x1778:	lw	r1, [STACKP]
x177a:	cw	r1, stack+4
x177c:	jes	1
x177d:	hlt	ERR_CODE
x177e:	cw	r0, ?E
x1780:	jes	1
x1781:	hlt	ERR_CODE
x1782:	ki	itest_intr2
x1784:	lw	r1, [itest_intr2]
x1786:	cw	r1, I_MASKABLE-I_PARITY-I_NOMEM-I_CPU_H-I_IFPOWER-I_TIMER-I_ILLEGAL
x1788:	jes	1
x1789:	hlt	ERR_CODE
x178a:	lw	r1, int06
x178c:	rw	r1, INTV_ILLEGAL
x178e:	lip

itest_int_div_of_proc:
x178f:	la	stack
x1791:	cw	r1, itest_ic_before_int_cpu
x1793:	jes	1
x1794:	hlt	ERR_CODE
x1795:	cwt	r2, -1
x1796:	jes	1
x1797:	hlt	ERR_CODE
x1798:	cw	r3, IMASK_GROUP_H
x179a:	jes	1
x179b:	hlt	ERR_CODE
x179c:	cwt	r4, 0
x179d:	jes	1
x179e:	hlt	ERR_CODE
x179f:	lw	r1, [STACKP]
x17a1:	cw	r1, stack+4
x17a3:	jes	1
x17a4:	hlt	ERR_CODE
x17a5:	cw	r0, ?E
x17a7:	jes	1
x17a8:	hlt	ERR_CODE
x17a9:	ki	itest_intr2
x17ab:	lw	r1, [itest_intr2]
x17ad:	cw	r1, I_FP_UF|I_FP_OF|I_FP_ERR|I_UNUSED|I_OPRQ|I_CPU_L|I_SW_H|I_SW_L
x17af:	jes	1
x17b0:	hlt	ERR_CODE
x17b1:	lw	r1, int07
x17b3:	rw	r1, INTV_DIV_OF
x17b5:	lip

itest_int_fp_uf_proc:
x17b6:	la	stack
x17b8:	cw	r1, itest_ic_before_int_cpu
x17ba:	jes	1
x17bb:	hlt	ERR_CODE
x17bc:	cwt	r2, -1
x17bd:	jes	1
x17be:	hlt	ERR_CODE
x17bf:	cw	r3, IMASK_GROUP_H
x17c1:	jes	1
x17c2:	hlt	ERR_CODE
x17c3:	cwt	r4, 0
x17c4:	jes	1
x17c5:	hlt	ERR_CODE
x17c6:	lw	r1, [STACKP]
x17c8:	cw	r1, stack+4
x17ca:	jes	1
x17cb:	hlt	ERR_CODE
x17cc:	cw	r0, ?E
x17ce:	jes	1
x17cf:	hlt	ERR_CODE
x17d0:	ki	itest_intr2
x17d2:	lw	r1, [itest_intr2]
x17d4:	cw	r1, I_FP_OF|I_FP_ERR|I_UNUSED|I_OPRQ|I_CPU_L|I_SW_H|I_SW_L
x17d6:	jes	1
x17d7:	hlt	ERR_CODE
x17d8:	lw	r1, int08
x17da:	rw	r1, INTV_FP_UF
x17dc:	lip

itest_int_fp_of_proc:
x17dd:	la	stack
x17df:	cw	r1, itest_ic_before_int_cpu
x17e1:	jes	1
x17e2:	hlt	ERR_CODE
x17e3:	cwt	r2, -1
x17e4:	jes	1
x17e5:	hlt	ERR_CODE
x17e6:	cw	r3, IMASK_GROUP_H
x17e8:	jes	1
x17e9:	hlt	ERR_CODE
x17ea:	cwt	r4, 0
x17eb:	jes	1
x17ec:	hlt	ERR_CODE
x17ed:	lw	r1, [STACKP]
x17ef:	cw	r1, stack+4
x17f1:	jes	1
x17f2:	hlt	ERR_CODE
x17f3:	cw	r0, ?E
x17f5:	jes	1
x17f6:	hlt	ERR_CODE
x17f7:	ki	itest_intr2
x17f9:	lw	r1, [itest_intr2]
x17fb:	cw	r1, I_FP_ERR|I_UNUSED|I_OPRQ|I_CPU_L|I_SW_H|I_SW_L
x17fd:	jes	1
x17fe:	hlt	ERR_CODE
x17ff:	lw	r1, int09
x1801:	rw	r1, INTV_FP_OF
x1803:	lip

itest_int_fp_err_proc:
x1804:	la	stack
x1806:	cw	r1, itest_ic_before_int_cpu
x1808:	jes	1
x1809:	hlt	ERR_CODE
x180a:	cwt	r2, -1
x180b:	jes	1
x180c:	hlt	ERR_CODE
x180d:	cw	r3, IMASK_GROUP_H
x180f:	jes	1
x1810:	hlt	ERR_CODE
x1811:	cwt	r4, 0
x1812:	jes	1
x1813:	hlt	ERR_CODE
x1814:	lw	r1, [STACKP]
x1816:	cw	r1, stack+4
x1818:	jes	1
x1819:	hlt	ERR_CODE
x181a:	cw	r0, ?E
x181c:	jes	1
x181d:	hlt	ERR_CODE
x181e:	ki	itest_intr2
x1820:	lw	r1, [itest_intr2]
x1822:	cw	r1, I_UNUSED|I_OPRQ|I_CPU_L|I_SW_H|I_SW_L
x1824:	jes	1
x1825:	hlt	ERR_CODE
x1826:	lw	r1, int10
x1828:	rw	r1, INTV_FP_ERR
x182a:	lip

itest_int_extra_proc:
x182b:	la	stack
x182d:	cw	r1, itest_ic_before_int_cpu
x182f:	jes	1
x1830:	hlt	ERR_CODE
x1831:	cwt	r2, -1
x1832:	jes	1
x1833:	hlt	ERR_CODE
x1834:	cw	r3, IMASK_GROUP_H
x1836:	jes	1
x1837:	hlt	ERR_CODE
x1838:	cwt	r4, 0
x1839:	jes	1
x183a:	hlt	ERR_CODE
x183b:	lw	r1, [STACKP]
x183d:	cw	r1, stack+4
x183f:	jes	1
x1840:	hlt	ERR_CODE
x1841:	cw	r0, ?E
x1843:	jes	1
x1844:	hlt	ERR_CODE
x1845:	ki	itest_intr2
x1847:	lw	r1, [itest_intr2]
x1849:	cw	r1, I_OPRQ|I_CPU_L|I_SW_H|I_SW_L
x184b:	jes	1
x184c:	hlt	ERR_CODE
x184d:	lw	r1, int11
x184f:	rw	r1, INTV_UNUSED
x1851:	lip

; interrupts I_OPRQ, I_CPU_L, I_SW_H, I_SW_L

itest_oprq:
x1852:	lw	r1, itest_int_oprq_proc
x1854:	lw	r2, itest_int_cpu_l_proc
x1856:	lw	r3, itest_int_sw_h_proc
x1858:	lw	r4, itest_int_sw_l_proc
x185a:	rf	INTV_OPRQ
x185c:	rw	r4, INTV_SW_L
x185e:	lw	r1, IMASK_GROUP_L
x1860:	rw	r1, itest_mask
x1862:	lwt	r0, -1
x1863:	im	itest_mask
itest_ic_before_int_soft:
x1865:	uj	test_next

itest_int_oprq_proc:
x1867:	la	stack
x1869:	cw	r1, itest_ic_before_int_soft
x186b:	jes	1
x186c:	hlt	ERR_CODE
x186d:	cwt	r2, -1
x186e:	jes	1
x186f:	hlt	ERR_CODE
x1870:	cw	r3, IMASK_GROUP_L
x1872:	jes	1
x1873:	hlt	ERR_CODE
x1874:	cwt	r4, 0
x1875:	jes	1
x1876:	hlt	ERR_CODE
x1877:	lw	r1, [STACKP]
x1879:	cw	r1, stack+4
x187b:	jes	1
x187c:	hlt	ERR_CODE
x187d:	cw	r0, ?E
x187f:	jes	1
x1880:	hlt	ERR_CODE
x1881:	ki	itest_intr2
x1883:	lw	r1, [itest_intr2]
x1885:	cwt	r1, 7
x1886:	jes	1
x1887:	hlt	ERR_CODE
x1888:	lw	r1, int28
x188a:	rw	r1, INTV_OPRQ
x188c:	lip

itest_int_cpu_l_proc:
x188d:	la	stack
x188f:	cw	r1, itest_ic_before_int_soft
x1891:	jes	1
x1892:	hlt	ERR_CODE
x1893:	cwt	r2, -1
x1894:	jes	1
x1895:	hlt	ERR_CODE
x1896:	cw	r3, IMASK_GROUP_L
x1898:	jes	1
x1899:	hlt	ERR_CODE
x189a:	cwt	r4, 0
x189b:	jes	1
x189c:	hlt	ERR_CODE
x189d:	lw	r1, [STACKP]
x189f:	cw	r1, stack+4
x18a1:	jes	1
x18a2:	hlt	ERR_CODE
x18a3:	cw	r0, ?E
x18a5:	jes	1
x18a6:	hlt	ERR_CODE
x18a7:	ki	itest_intr2
x18a9:	lw	r1, [itest_intr2]
x18ab:	cwt	r1, 3
x18ac:	jes	1
x18ad:	hlt	ERR_CODE
x18ae:	lw	r1, int29
x18b0:	rw	r1, INTV_CPU_L
x18b2:	lip

itest_int_sw_h_proc:
x18b3:	la	stack
x18b5:	cw	r1, itest_ic_before_int_soft
x18b7:	jes	1
x18b8:	hlt	ERR_CODE
x18b9:	cwt	r2, -1
x18ba:	jes	1
x18bb:	hlt	ERR_CODE
x18bc:	cw	r3, IMASK_GROUP_L
x18be:	jes	1
x18bf:	hlt	ERR_CODE
x18c0:	cwt	r4, 0
x18c1:	jes	1
x18c2:	hlt	ERR_CODE
x18c3:	lw	r1, [STACKP]
x18c5:	cw	r1, stack+4
x18c7:	jes	1
x18c8:	hlt	ERR_CODE
x18c9:	cw	r0, ?E
x18cb:	jes	1
x18cc:	hlt	ERR_CODE
x18cd:	ki	itest_intr2
x18cf:	lw	r1, [itest_intr2]
x18d1:	cwt	r1, 1
x18d2:	jes	1
x18d3:	hlt	ERR_CODE
x18d4:	lw	r1, int30
x18d6:	rw	r1, INTV_SW_H
x18d8:	lip

itest_int_sw_l_proc:
x18d9:	la	stack
x18db:	cw	r1, itest_ic_before_int_soft
x18dd:	jes	1
x18de:	hlt	ERR_CODE
x18df:	cwt	r2, -1
x18e0:	jes	1
x18e1:	hlt	ERR_CODE
x18e2:	cw	r3, IMASK_GROUP_L
x18e4:	jes	1
x18e5:	hlt	ERR_CODE
x18e6:	cwt	r4, 0
x18e7:	jes	1
x18e8:	hlt	ERR_CODE
x18e9:	lw	r1, [STACKP]
x18eb:	cw	r1, stack+4
x18ed:	jes	1
x18ee:	hlt	ERR_CODE
x18ef:	cw	r0, ?E
x18f1:	jes	1
x18f2:	hlt	ERR_CODE
x18f3:	ki	itest_intr2
x18f5:	lw	r1, [itest_intr2]
x18f7:	cwt	r1, 0
x18f8:	jes	1
x18f9:	hlt	ERR_CODE
x18fa:	lw	r1, int31
x18fc:	rw	r1, INTV_SW_L
x18fe:	lip

; oprq interrupt should be served

test_next:
x18ff:	lw	r1, i_oprq_proc
x1901:	rw	r1, INTV_OPRQ
x1903:	lwt	r0, -1
x1904:	fi	itest_intr1 ; set all interrupts while mask is set to IMASK_GROUP_H
i_oprq_proc:
x1906:	lw	r1, [stack]
x1908:	cw	r1, i_oprq_proc
x190a:	jes	1
x190b:	hlt	ERR_CODE
x190c:	ki	itest_intr2
x190e:	lw	r1, [itest_intr2]
x1910:	cw	r1, I_MASKABLE-I_OPRQ ; I_OPRQ should be gone now
x1912:	jes	1
x1913:	hlt	ERR_CODE
x1914:	lw	r1, int28
x1916:	rw	r1, INTV_OPRQ

; cpu low interrupt should be served

x1918:	lw	r1, i_cpu_l_proc
x191a:	rw	r1, INTV_CPU_L
x191c:	lwt	r0, -1
x191d:	lw	r1, IMASK_GROUP_L
x191f:	rw	r1, itest_mask
x1921:	im	itest_mask
i_cpu_l_proc:
x1923:	lw	r1, [stack + 4]
x1925:	cw	r1, i_cpu_l_proc
x1927:	jes	1
x1928:	hlt	ERR_CODE
x1929:	ki	itest_intr2
x192b:	lw	r1, [itest_intr2]
x192d:	cw	r1, I_MASKABLE-I_OPRQ-I_CPU_L; I_CPU_L should be gone now
x192f:	jes	1
x1930:	hlt	ERR_CODE
x1931:	lw	r1, int29
x1933:	rw	r1, INTV_CPU_L

; sw high should be served

x1935:	lw	r1, i_sw_h_proc
x1937:	rw	r1, INTV_SW_H
x1939:	lwt	r0, -1
x193a:	im	itest_mask
i_sw_h_proc:
x193c:	lw	r1, [stack + 2*4]
x193e:	cw	r1, i_sw_h_proc
x1940:	jes	1
x1941:	hlt	ERR_CODE
x1942:	ki	itest_intr2
x1944:	lw	r1, [itest_intr2]
x1946:	cw	r1, I_MASKABLE-I_OPRQ-I_CPU_L-I_SW_H ; I_SW_H should be gone now
x1948:	jes	1
x1949:	hlt	ERR_CODE
x194a:	lw	r1, int30
x194c:	rw	r1, INTV_SW_H

; sw low should be served

x194e:	lw	r1, i_sw_l_proc
x1950:	rw	r1, INTV_SW_L
x1952:	lwt	r0, -1
x1953:	im	itest_mask
i_sw_l_proc:
x1955:	lw	r1, [stack + 3*4]
x1957:	cw	r1, i_sw_l_proc
x1959:	jes	1
x195a:	hlt	ERR_CODE
x195b:	ki	itest_intr2
x195d:	lw	r1, [itest_intr2]
x195f:	cw	r1, I_MASKABLE-I_OPRQ-I_CPU_L-I_SW_H-I_SW_L ; I_SW_L should be gone now
x1961:	jes	1
x1962:	hlt	ERR_CODE

; cleanup and 'return'

x1963:	lw	r1, i_return
x1965:	rw	r1, stack + 3*4 ; IC = i_return
x1967:	lw	r1, int31
x1969:	rw	r1, INTV_SW_L
x196b:	rz	stack + 3*4 + 2 ; SR = 0
x196d:	lip
i_return:
x196e:	lw	r1, [STACKP]
x1970:	cw	r1, stack + 3*4
x1972:	jes	1
x1973:	hlt	ERR_CODE

; timer should be served

x1974:	lw	r1, i_timer_proc
x1976:	rw	r1, INTV_TIMER
x1978:	lwt	r0, -1
x1979:	lw	r1, IMASK_GROUP_H
x197b:	rw	r1, itest_mask
x197d:	fi	itest_intr1 ; set all interrupts while mask is set to IMASK_GROUP_H
x197f:	im	itest_mask
i_timer_proc:
x1981:	lw	r1, [stack + 3*4]
x1983:	cw	r1, i_timer_proc
x1985:	jes	1
x1986:	hlt	ERR_CODE
x1987:	ki	itest_intr2
x1989:	lw	r1, [itest_intr2]
x198b:	cw	r1, I_MASKABLE-I_TIMER ; I_TIMER should be gone now
x198d:	jes	1
x198e:	hlt	ERR_CODE
x198f:	lw	r1, int05
x1991:	rw	r1, INTV_TIMER

; illegal instruction should be served

x1993:	lw	r1, i_illegal_proc
x1995:	rw	r1, INTV_ILLEGAL
x1997:	lwt	r0, -1
x1998:	im	itest_mask
i_illegal_proc:
x199a:	lw	r1, [stack + 4*4]
x199c:	cw	r1, i_illegal_proc
x199e:	jes	1
x199f:	hlt	ERR_CODE
x19a0:	ki	itest_intr2
x19a2:	lw	r1, [itest_intr2]
x19a4:	cw	r1, I_MASKABLE-I_TIMER-I_ILLEGAL ; I_ILLEGAL should be gone now
x19a6:	jes	1
x19a7:	hlt	ERR_CODE
x19a8:	lw	r1, int06
x19aa:	rw	r1, INTV_ILLEGAL

; division overflow should be served

x19ac:	lw	r1, i_div_of_proc
x19ae:	rw	r1, INTV_DIV_OF
x19b0:	lwt	r0, -1
x19b1:	im	itest_mask
i_div_of_proc:
x19b3:	lw	r1, [stack + 5*4]
x19b5:	cw	r1, i_div_of_proc
x19b7:	jes	1
x19b8:	hlt	ERR_CODE
x19b9:	ki	itest_intr2
x19bb:	lw	r1, [itest_intr2]
x19bd:	cw	r1, I_MASKABLE-I_TIMER-I_ILLEGAL-I_DIV_OF ; I_DIV_OF should be gone now
x19bf:	jes	1
x19c0:	hlt	ERR_CODE
x19c1:	lw	r1, int07
x19c3:	rw	r1, INTV_DIV_OF

; floating point underflow should be served

x19c5:	lw	r1, i_fp_uf_proc
x19c7:	rw	r1, INTV_FP_UF
x19c9:	lwt	r0, -1
x19ca:	im	itest_mask
i_fp_uf_proc:
x19cc:	lw	r1, [stack + 6*4]
x19ce:	cw	r1, i_fp_uf_proc
x19d0:	jes	1
x19d1:	hlt	ERR_CODE
x19d2:	ki	itest_intr2
x19d4:	lw	r1, [itest_intr2]
x19d6:	cw	r1, I_MASKABLE-I_TIMER-I_ILLEGAL-I_DIV_OF-I_FP_UF ; I_FP_UF should be gone now
x19d8:	jes	1
x19d9:	hlt	ERR_CODE
x19da:	lw	r1, int08
x19dc:	rw	r1, INTV_FP_UF

; floating point overflow should be served

x19de:	lw	r1, i_fp_of_proc
x19e0:	rw	r1, INTV_FP_OF
x19e2:	lwt	r0, -1
x19e3:	im	itest_mask
i_fp_of_proc:
x19e5:	lw	r1, [stack + 7*4]
x19e7:	cw	r1, i_fp_of_proc
x19e9:	jes	1
x19ea:	hlt	ERR_CODE
x19eb:	ki	itest_intr2
x19ed:	lw	r1, [itest_intr2]
x19ef:	cw	r1, I_MASKABLE-I_TIMER-I_ILLEGAL-I_DIV_OF-I_FP_UF-I_FP_OF ; I_FP_OF should be gone now
x19f1:	jes	1
x19f2:	hlt	ERR_CODE
x19f3:	lw	r1, int09
x19f5:	rw	r1, INTV_FP_OF

; floating point error should be served

x19f7:	lw	r1, i_fp_err_proc
x19f9:	rw	r1, INTV_FP_ERR
x19fb:	lwt	r0, -1
x19fc:	im	itest_mask
i_fp_err_proc:
x19fe:	lw	r1, [stack + 8*4]
x1a00:	cw	r1, i_fp_err_proc
x1a02:	jes	1
x1a03:	hlt	ERR_CODE
x1a04:	ki	itest_intr2
x1a06:	lw	r1, [itest_intr2]
x1a08:	cw	r1, I_MASKABLE-I_TIMER-I_ILLEGAL-I_DIV_OF-I_FP_UF-I_FP_OF-I_FP_ERR ; I_FP_ERR should be gone now
x1a0a:	jes	1
x1a0b:	hlt	ERR_CODE
x1a0c:	lw	r1, int10
x1a0e:	rw	r1, INTV_FP_ERR

; extra (unused interrupt) should be served

x1a10:	lw	r1, i_extra_proc
x1a12:	rw	r1, INTV_UNUSED
x1a14:	lwt	r0, -1
x1a15:	im	itest_mask
i_extra_proc:
x1a17:	lw	r1, [stack + 9*4]
x1a19:	cw	r1, i_extra_proc
x1a1b:	jes	1
x1a1c:	hlt	ERR_CODE
x1a1d:	ki	itest_intr2
x1a1f:	lw	r1, [itest_intr2]
x1a21:	cw	r1, I_MASKABLE-I_TIMER-I_ILLEGAL-I_DIV_OF-I_FP_UF-I_FP_OF-I_FP_ERR-I_UNUSED ; I_UNUSED should be gone now
x1a23:	jes	1
x1a24:	hlt	ERR_CODE
x1a25:	lw	r1, i_return2
x1a27:	rw	r1, stack + 9*4 ; IC
x1a29:	lw	r1, int11
x1a2b:	rw	r1, INTV_UNUSED
x1a2d:	rz	stack + 9*4 + 2 ; SR
x1a2f:	lip

i_return2:
x1a30:	lw	r1, [STACKP]
x1a32:	cw	r1, stack + 9*4
x1a34:	jes	1
x1a35:	hlt	ERR_CODE

; ifpower should be served

x1a36:	lw	r1, i_ifpower_proc
x1a38:	rw	r1, INTV_IFPOWER
x1a3a:	lwt	r0, -1
x1a3b:	lw	r1, IMASK_IFPOWER
x1a3d:	rw	r1, itest_mask
x1a3f:	fi	itest_intr1
x1a41:	im	itest_mask
i_ifpower_proc:
x1a43:	lw	r1, [stack + 9*4]
x1a45:	cw	r1, i_ifpower_proc
x1a47:	jes	1
x1a48:	hlt	ERR_CODE
x1a49:	ki	itest_intr2
x1a4b:	lw	r1, [itest_intr2]
x1a4d:	cw	r1, I_MASKABLE-I_IFPOWER ; I_IFPOWER should be gone now
x1a4f:	jes	1
x1a50:	hlt	ERR_CODE
x1a51:	lw	r1, int04
x1a53:	rw	r1, INTV_IFPOWER
x1a55:	lw	r1, i_cpu_h
x1a57:	rw	r1, stack + 9*4 ; IC
x1a59:	lip

i_cpu_h:
x1a5a:	lw	r1, i_cpu_h_proc
x1a5c:	rw	r1, INTV_CPU_H
x1a5e:	lwt	r0, -1
x1a5f:	lw	r1, IMASK_CPU_H
x1a61:	rw	r1, itest_mask
x1a63:	im	itest_mask
i_cpu_h_proc:
x1a65:	lw	r1, [stack + 9*4]
x1a67:	cw	r1, i_cpu_h_proc
x1a69:	jes	1
x1a6a:	hlt	ERR_CODE
x1a6b:	ki	itest_intr2
x1a6d:	lw	r1, [itest_intr2]
x1a6f:	cw	r1, I_MASKABLE-I_IFPOWER-I_CPU_H ; I_CPU_H should be gone now
x1a71:	jes	1
x1a72:	hlt	ERR_CODE
x1a73:	lw	r1, int03
x1a75:	rw	r1, INTV_CPU_H
x1a77:	lw	r1, i_nomem
x1a79:	rw	r1, stack + 9*4 ; IC
x1a7b:	lip

i_nomem:
x1a7c:	lw	r1, i_nomem_proc
x1a7e:	rw	r1, INTV_NOMEM
x1a80:	lwt	r0, -1
x1a81:	lw	r1, IMASK_NOMEM
x1a83:	rw	r1, itest_mask
x1a85:	im	itest_mask
i_nomem_proc:
x1a87:	lw	r1, [stack + 9*4]
x1a89:	cw	r1, i_nomem_proc
x1a8b:	jes	1
x1a8c:	hlt	ERR_CODE
x1a8d:	ki	itest_intr2
x1a8f:	lw	r1, [itest_intr2]
x1a91:	cw	r1, I_MASKABLE-I_IFPOWER-I_CPU_H-I_NOMEM ; I_NOMEM should be gone now
x1a93:	jes	1
x1a94:	hlt	ERR_CODE
x1a95:	lw	r1, int02
x1a97:	rw	r1, INTV_NOMEM
x1a99:	lw	r1, i_power
x1a9b:	rw	r1, stack + 9*4 ; IC
x1a9d:	rz	stack + 9*4 + 2 ; SR
x1a9f:	lip

i_power:
x1aa0:	lw	r1, i_power_proc
x1aa2:	rw	r1, INTV_POWER
x1aa4:	rz	itest_intr1
x1aa6:	fi	itest_intr1
x1aa8:	lw	r1, I_POWER
x1aaa:	rw	r1, itest_intr1
x1aac:	lwt	r0, -1
x1aad:	lw	r1, IMASK_ALL
x1aaf:	rw	r1, itest_mask
x1ab1:	im	itest_mask
x1ab3:	fi	itest_intr1
i_power_proc:
x1ab5:	la	stack + 9*4
x1ab7:	cw	r1, i_power_proc
x1ab9:	jes	1
x1aba:	hlt	ERR_CODE
x1abb:	cwt	r2, -1
x1abc:	jes	1
x1abd:	hlt	ERR_CODE
x1abe:	cw	r3, IMASK_ALL
x1ac0:	jes	1
x1ac1:	hlt	ERR_CODE
x1ac2:	cwt	r4, 0
x1ac3:	jes	1
x1ac4:	hlt	ERR_CODE
x1ac5:	lw	r1, [STACKP]
x1ac7:	cw	r1, stack + 10*4
x1ac9:	jes	1
x1aca:	hlt	ERR_CODE
x1acb:	cw	r0, ?E
x1acd:	jes	1
x1ace:	hlt	ERR_CODE
x1acf:	ki	itest_intr2
x1ad1:	lw	r1, [itest_intr2]
x1ad3:	cwt	r1, 0
x1ad4:	jes	1
x1ad5:	hlt	ERR_CODE
x1ad6:	lw	r1, int00
x1ad8:	rw	r1, INTV_POWER
x1ada:	lj	reset_stack
x1adc:	lw	r1, IMASK_ALL
x1ade:	rw	r1, mask
x1ae0:	im	mask

; all tests finished

	uj	restart

; ------------------------------------------------------------------------
reset_stack:
x1ae4:	.res	1
x1ae5:	lw	r2, stack
x1ae7:	lw	r1, stack-int01+1
resetloop:
x1ae9:	ri	r2, 0
x1aeb:	irb	r1, resetloop
x1aec:	lw	r1, stack
x1aee:	rw	r1, stackp
x1af0:	mcl
x1af1:	uj	[reset_stack]

; XPCT ir : 0xec3f
