; OPTS -c configs/minimal.cfg
;
; MERA-400 CPU test
;
; Based on the original "TP" test provided by the manufacturer.
; The only difference is that this version uses changed HLT codes.

	.include hw.inc

	.equ intproc 0xc1
	.equ start 0x100

	uj	start

mask:	.word	0

	.org INTV
	.word	int00, int01, int02, int03, int04, int05, int06, int07
	.word	int08, int09, int10, int11, int12, int13, int14, int15
	.word	int16, int17, int18, int19, int20, int21, int22, int23
	.word	int24, int25, int26, int27, int28, int29, int30, int31

exlp:	.word	0
stackp:	.word	0
stack:

	.org intproc

int01:	hlt	040	LIP
int02:	hlt	040	LIP
int03:	hlt	040	LIP
int04:	hlt	040	LIP
int05:	hlt	040	LIP
int06:	hlt	040	LIP
int07:	hlt	040	LIP
int08:	hlt	040	LIP
int09:	hlt	040	LIP
int10:	hlt	040	LIP
int11:	hlt	040	LIP
int12:	hlt	040	LIP
int13:	hlt	040	LIP
int14:	hlt	040	LIP
int15:	hlt	040	LIP
int16:	hlt	040	LIP
int17:	hlt	040	LIP
int18:	hlt	040	LIP
int19:	hlt	040	LIP
int20:	hlt	040	LIP
int21:	hlt	040	LIP
int22:	hlt	040	LIP
int23:	hlt	040	LIP
int24:	hlt	040	LIP
int25:	hlt	040	LIP
int26:	hlt	040	LIP
int27:	hlt	040	LIP
int28:	hlt	040	LIP
int29:	hlt	040	LIP
int30:	hlt	040	LIP
int31:	hlt	040	LIP

; test rozkazów w bloku podstawowym (Q=0, NB=0)

	.org start

x0100:	lw	r1, -1
x0102:	cw	r1, -1
x0104:	jes	1
x0105:	hlt	040
x0106:	lw	r1, -1
x0108:	cwt	r1, -1
x0109:	jes	1
x010a:	hlt	040
x010b:	lwt	r1, -1
x010c:	cw	r1, -1
x010e:	jes	1
x010f:	hlt	040
x0110:	lwt	r1, -1
x0111:	cwt	r1, -1
x0112:	jes	1
x0113:	hlt	040
x0114:	lwt	r1, 0
x0115:	lw	r1, [0x132b]
x0117:	cw	r1, 0xaaaa
x0119:	jes	1
x011a:	hlt	040
x011b:	lwt	r2, 0
x011c:	lw	r2, 0xaaaa
x011e:	cw	r2, 0xaaaa
x0120:	jes	1
x0121:	hlt	040
x0122:	lwt	r1, 0
x0123:	lw	r1, r2
x0124:	cw	r1, 0xaaaa
x0126:	jes	1
x0127:	hlt	040
x0128:	lw	r1, 0xaaaa
x012a:	lw	r2, [0x132b]
x012c:	cw	r2, r1
x012d:	jes	1
x012e:	hlt	040
x012f:	lw	r1, 0xaaaa
x0131:	rw	r1, 0x132e
x0133:	lw	r2, [0x132e]
x0135:	cw	r2, 0xaaaa
x0137:	jes	1
x0138:	hlt	040
x0139:	lw	r1, 0xaaaa
x013b:	cw	r1, [0x132b]
x013d:	jes	1
x013e:	hlt	040
x013f:	rz	0x132b
x0141:	lwt	r1, 0
x0142:	cw	r1, [0x132b]
x0144:	jes	1
x0145:	hlt	040
x0146:	lw	r1, 0xaaaa
x0148:	rw	r1, 0x132b
x014a:	rz	mask
x014c:	mcl
x014d:	cwt	r0, 0
x014e:	jes	1
x014f:	hlt	040

restart:
x0150:	lw	r2, [0x132b]
x0152:	rw	r2, 0x132e
x0154:	lwt	r1, 0
x0155:	tw	r1, 0x132e
x0157:	cw	r1, [0x132b]
x0159:	jes	1
x015a:	hlt	040
x015b:	lw	r2, [pat10]
x015d:	rz	0x132e
x015f:	pw	r2, 0x132e
x0161:	lw	r1, [0x132e]
x0163:	cw	r1, [pat10]
x0165:	jes	1
x0166:	hlt	040
x0167:	lwt	r1, 0
x0168:	lwt	r7, 0
x0169:	ls	r1, -1
x016b:	cwt	r1, 0
x016c:	jes	1
x016d:	hlt	040
x016e:	lw	r1, [pat10]
x0170:	lwt	r7, -1
x0171:	ls	r1, [pat01]
x0173:	cw	r1, [pat01]
x0175:	jes	1
x0176:	hlt	040
x0177:	lwt	r1, -1
x0178:	lw	r7, [pat01]
x017a:	ls	r1, -1
x017c:	cwt	r1, -1
x017d:	jes	1
x017e:	hlt	040
x017f:	lwt	r1, -1
x0180:	lw	r7, [pat10]
x0182:	ls	r1, -1
x0184:	cwt	r1, -1
x0185:	jes	1
x0186:	hlt	040
x0187:	lw	r1, 0x132e
x0189:	rz	0x132e
x018b:	rz	0x132f
x018d:	ri	r1, [pat10]
x018f:	lw	r2, [pat10]
x0191:	cw	r2, [0x132e]
x0193:	jes	1
x0194:	hlt	040
x0195:	cw	r1, 0x132f
x0197:	jes	1
x0198:	hlt	040
x0199:	ri	r1, [pat10]
x019b:	lw	r2, [pat10]
x019d:	cw	r2, [0x132f]
x019f:	jes	1
x01a0:	hlt	040
x01a1:	cw	r1, 0x1330
x01a3:	jes	1
x01a4:	hlt	040
x01a5:	rj	r7, 0x1a7
x01a7:	cw	r7, 0x1a7
x01a9:	jes	1
x01aa:	hlt	040
x01ab:	rz	0x132e
x01ad:	lwt	r1, 0
x01ae:	is	r1, 0x132e
x01b0:	hlt	040
x01b1:	lw	r1, [0x132e]
x01b3:	cwt	r1, 0
x01b4:	jes	1
x01b5:	hlt	040
x01b6:	rz	0x132e
x01b8:	lwt	r1, -1
x01b9:	is	r1, 0x132e
x01bb:	ujs	1
x01bc:	hlt	040
x01bd:	lw	r1, [0x132e]
x01bf:	cwt	r1, -1
x01c0:	jes	1
x01c1:	hlt	040
x01c2:	lwt	r1, -1
x01c3:	rw	r1, 0x132e
x01c5:	lwt	r1, 0
x01c6:	is	r1, 0x132e
x01c8:	hlt	040
x01c9:	lw	r1, [0x132e]
x01cb:	cwt	r1, -1
x01cc:	jes	1
x01cd:	hlt	040
x01ce:	lwt	r1, -1
x01cf:	rw	r1, 0x132e
x01d1:	is	r1, 0x132e
x01d3:	hlt	040
x01d4:	lw	r1, [0x132e]
x01d6:	cwt	r1, -1
x01d7:	jes	1
x01d8:	hlt	040
x01d9:	lw	r1, [pat01]
x01db:	rw	r1, 0x132e
x01dd:	is	r1, 0x132e
x01df:	hlt	040
x01e0:	lw	r1, [0x132e]
x01e2:	cw	r1, [pat01]
x01e4:	jes	1
x01e5:	hlt	040
x01e6:	lw	r1, [pat10]
x01e8:	rw	r1, 0x132e
x01ea:	is	r1, 0x132e
x01ec:	hlt	040
x01ed:	lw	r1, [0x132e]
x01ef:	cw	r1, [pat10]
x01f1:	jes	1
x01f2:	hlt	040
x01f3:	lw	r1, [pat01]
x01f5:	lw	r2, [pat10]
x01f7:	rw	r2, 0x132e
x01f9:	is	r1, 0x132e
x01fb:	ujs	1
x01fc:	hlt	040
x01fd:	lw	r1, [0x132e]
x01ff:	cwt	r1, -1
x0200:	jes	1
x0201:	hlt	040
x0202:	lw	r1, [pat01]
x0204:	lw	r2, [pat10]
x0206:	rw	r1, 0x132e
x0208:	is	r2, 0x132e
x020a:	ujs	1
x020b:	hlt	040
x020c:	lw	r1, [0x132e]
x020e:	cwt	r1, -1
x020f:	jes	1
x0210:	hlt	040
x0211:	lwt	r1, 0
x0212:	lwt	r2, 0
x0213:	bc	r1, r2
x0214:	ujs	1
x0215:	hlt	040
x0216:	lwt	r1, -1
x0217:	lwt	r2, -1
x0218:	bc	r1, r2
x0219:	ujs	1
x021a:	hlt	040
x021b:	lwt	r1, -1
x021c:	lwt	r2, 0
x021d:	bc	r1, r2
x021e:	ujs	1
x021f:	hlt	040
x0220:	lwt	r1, 0
x0221:	lwt	r2, -1
x0222:	bc	r1, r2
x0223:	hlt	040
x0224:	lw	r1, [pat01]
x0226:	lw	r2, r1
x0227:	bc	r1, r2
x0228:	ujs	1
x0229:	hlt	040
x022a:	lw	r1, [pat10]
x022c:	lw	r2, r1
x022d:	bc	r1, r2
x022e:	ujs	1
x022f:	hlt	040
x0230:	lw	r1, [pat01]
x0232:	lw	r2, [pat10]
x0234:	bc	r1, r2
x0235:	hlt	040
x0236:	lw	r1, [pat10]
x0238:	lw	r2, [pat01]
x023a:	bc	r1, r2
x023b:	hlt	040
x023c:	lwt	r1, 0
x023d:	rw	r1, 0x132e
x023f:	bm	r1, 0x132e
x0241:	hlt	040
x0242:	lwt	r1, -1
x0243:	rw	r1, 0x132e
x0245:	bm	r1, 0x132e
x0247:	hlt	040
x0248:	lwt	r1, -1
x0249:	rz	0x132e
x024b:	bm	r1, 0x132e
x024d:	ujs	1
x024e:	hlt	040
x024f:	lwt	r1, 0
x0250:	lwt	r2, -1
x0251:	rw	r2, 0x132e
x0253:	bm	r1, 0x132e
x0255:	hlt	040
x0256:	lw	r1, [pat01]
x0258:	rw	r1, 0x132e
x025a:	bm	r1, 0x132e
x025c:	hlt	040
x025d:	lw	r1, [pat10]
x025f:	rw	r1, 0x132e
x0261:	bm	r1, 0x132e
x0263:	hlt	040
x0264:	lw	r1, [pat01]
x0266:	lw	r2, [pat10]
x0268:	rw	r2, 0x132e
x026a:	bm	r1, 0x132e
x026c:	ujs	1
x026d:	hlt	040
x026e:	lw	r1, [pat10]
x0270:	lw	r2, [pat01]
x0272:	rw	r2, 0x132e
x0274:	bm	r1, 0x132e
x0276:	ujs	1
x0277:	hlt	040
x0278:	lwt	r1, 0
x0279:	lw	r2, r1
x027a:	bn	r1, r2
x027b:	hlt	040
x027c:	lwt	r1, -1
x027d:	lw	r2, r1
x027e:	bn	r1, r2
x027f:	ujs	1
x0280:	hlt	040
x0281:	lwt	r1, -1
x0282:	lwt	r2, 0
x0283:	bn	r1, r2
x0284:	hlt	040
x0285:	lwt	r1, 0
x0286:	lwt	r2, -1
x0287:	bn	r1, r2
x0288:	hlt	040
x0289:	lw	r1, [pat01]
x028b:	lw	r2, r1
x028c:	bn	r1, r2
x028d:	ujs	1
x028e:	hlt	040
x028f:	lw	r1, [pat10]
x0291:	lw	r2, r1
x0292:	bn	r1, r2
x0293:	ujs	1
x0294:	hlt	040
x0295:	lw	r1, [pat01]
x0297:	lw	r2, [pat10]
x0299:	bn	r1, r2
x029a:	hlt	040
x029b:	lw	r1, [pat10]
x029d:	lw	r2, [pat01]
x029f:	bn	r1, r2
x02a0:	hlt	040
x02a1:	lwt	r1, 0
x02a2:	lwt	r2, 0
x02a3:	bb	r1, r2
x02a4:	hlt	040
x02a5:	lwt	r1, -1
x02a6:	lw	r2, r1
x02a7:	bb	r1, r2
x02a8:	hlt	040
x02a9:	lwt	r1, -1
x02aa:	lwt	r2, 0
x02ab:	bb	r1, r2
x02ac:	hlt	040
x02ad:	lwt	r1, 0
x02ae:	lwt	r2, -1
x02af:	bb	r1, r2
x02b0:	ujs	1
x02b1:	hlt	040
x02b2:	lw	r1, [pat01]
x02b4:	lw	r2, r1
x02b5:	bb	r1, r2
x02b6:	hlt	040
x02b7:	lw	r1, [pat10]
x02b9:	lw	r2, r1
x02ba:	bb	r1, r2
x02bb:	hlt	040
x02bc:	lw	r1, [pat01]
x02be:	lw	r2, [pat10]
x02c0:	bb	r1, r2
x02c1:	ujs	1
x02c2:	hlt	040
x02c3:	lw	r1, [pat10]
x02c5:	lw	r2, [pat01]
x02c7:	bb	r1, r2
x02c8:	ujs	1
x02c9:	hlt	040
x02ca:	lwt	r1, 0
x02cb:	lwt	r7, 0
x02cc:	lwt	r2, -1
x02cd:	bs	r1, r2
x02ce:	hlt	040
x02cf:	lw	r1, [pat10]
x02d1:	lwt	r7, -1
x02d2:	lw	r2, [pat01]
x02d4:	bs	r1, r2
x02d5:	ujs	1
x02d6:	hlt	040
x02d7:	lwt	r1, -1
x02d8:	lwt	r2, -1
x02d9:	lw	r7, [pat01]
x02db:	bs	r1, r2
x02dc:	hlt	040
x02dd:	lwt	r1, -1
x02de:	lwt	r2, -1
x02df:	lw	r7, [pat10]
x02e1:	bs	r1, r2
x02e2:	hlt	040
x02e3:	lwt	r0, 0
x02e4:	lwt	r1, 0
x02e5:	lwt	r2, -1
x02e6:	or	r1, r2
x02e7:	cwt	r0, 0
x02e8:	jes	1
x02e9:	hlt	040
x02ea:	cwt	r1, -1
x02eb:	jes	1
x02ec:	hlt	040
x02ed:	lwt	r0, 0
x02ee:	lwt	r1, -1
x02ef:	lwt	r2, 0
x02f0:	or	r1, r2
x02f1:	cwt	r0, 0
x02f2:	jes	1
x02f3:	hlt	040
x02f4:	cwt	r1, -1
x02f5:	jes	1
x02f6:	hlt	040
x02f7:	lwt	r0, 0
x02f8:	lwt	r1, 0
x02f9:	lwt	r2, 0
x02fa:	or	r1, r2
x02fb:	cw	r0, 0x8000
x02fd:	jes	1
x02fe:	hlt	040
x02ff:	cwt	r1, 0
x0300:	jes	1
x0301:	hlt	040
x0302:	lwt	r0, 0
x0303:	lwt	r1, -1
x0304:	lwt	r2, -1
x0305:	or	r1, r2
x0306:	cwt	r0, 0
x0307:	jes	1
x0308:	hlt	040
x0309:	cwt	r1, -1
x030a:	jes	1
x030b:	hlt	040
x030c:	lwt	r0, 0
x030d:	lw	r1, [pat01]
x030f:	lw	r2, r1
x0310:	or	r1, r2
x0311:	cwt	r0, 0
x0312:	jes	1
x0313:	hlt	040
x0314:	cw	r1, [pat01]
x0316:	jes	1
x0317:	hlt	040
x0318:	lwt	r0, 0
x0319:	lw	r1, [pat10]
x031b:	lw	r2, r1
x031c:	or	r1, r2
x031d:	cwt	r0, 0
x031e:	jes	1
x031f:	hlt	040
x0320:	cw	r1, [pat10]
x0322:	jes	1
x0323:	hlt	040
x0324:	lwt	r0, 0
x0325:	lw	r1, [pat10]
x0327:	or	r1, [pat01]
x0329:	cwt	r0, 0
x032a:	jes	1
x032b:	hlt	040
x032c:	cwt	r1, -1
x032d:	jes	1
x032e:	hlt	040
x032f:	lwt	r0, 0
x0330:	lw	r1, [pat01]
x0332:	or	r1, [pat10]
x0334:	cwt	r0, 0
x0335:	jes	1
x0336:	hlt	040
x0337:	cwt	r1, -1
x0338:	jes	1
x0339:	hlt	040
x033a:	lwt	r0, 0
x033b:	lwt	r1, -1
x033c:	rw	r1, 0x132e
x033e:	lwt	r1, 0
x033f:	om	r1, 0x132e
x0341:	cwt	r0, 0
x0342:	jes	1
x0343:	hlt	040
x0344:	lw	r1, [0x132e]
x0346:	cwt	r1, -1
x0347:	jes	1
x0348:	hlt	040
x0349:	lwt	r0, 0
x034a:	rz	0x132e
x034c:	lwt	r1, -1
x034d:	om	r1, 0x132e
x034f:	cwt	r0, 0
x0350:	jes	1
x0351:	hlt	040
x0352:	lw	r1, [0x132e]
x0354:	cwt	r1, -1
x0355:	jes	1
x0356:	hlt	040
x0357:	lwt	r0, 0
x0358:	rz	0x132e
x035a:	lwt	r1, 0
x035b:	om	r1, 0x132e
x035d:	cw	r0, 0x8000
x035f:	jes	1
x0360:	hlt	040
x0361:	lw	r1, [0x132e]
x0363:	cwt	r1, 0
x0364:	jes	1
x0365:	hlt	040
x0366:	lwt	r0, 0
x0367:	lwt	r1, -1
x0368:	rw	r1, 0x132e
x036a:	om	r1, 0x132e
x036c:	cwt	r0, 0
x036d:	jes	1
x036e:	hlt	040
x036f:	lw	r1, [0x132e]
x0371:	cwt	r1, -1
x0372:	jes	1
x0373:	hlt	040
x0374:	lwt	r0, 0
x0375:	lw	r1, [pat01]
x0377:	rw	r1, 0x132e
x0379:	om	r1, 0x132e
x037b:	cwt	r0, 0
x037c:	jes	1
x037d:	hlt	040
x037e:	lw	r1, [0x132e]
x0380:	cw	r1, [pat01]
x0382:	jes	1
x0383:	hlt	040
x0384:	lwt	r0, 0
x0385:	lw	r1, [pat10]
x0387:	rw	r1, 0x132e
x0389:	om	r1, 0x132e
x038b:	cwt	r0, 0
x038c:	jes	1
x038d:	hlt	040
x038e:	lw	r1, [0x132e]
x0390:	cw	r1, [pat10]
x0392:	jes	1
x0393:	hlt	040
x0394:	lwt	r0, 0
x0395:	lw	r1, [pat01]
x0397:	rw	r1, 0x132e
x0399:	lw	r1, [pat10]
x039b:	om	r1, 0x132e
x039d:	cwt	r0, 0
x039e:	jes	1
x039f:	hlt	040
x03a0:	lw	r1, [0x132e]
x03a2:	cwt	r1, -1
x03a3:	jes	1
x03a4:	hlt	040
x03a5:	lwt	r0, 0
x03a6:	lw	r1, [pat10]
x03a8:	rw	r1, 0x132e
x03aa:	lw	r1, [pat01]
x03ac:	om	r1, 0x132e
x03ae:	cwt	r0, 0
x03af:	jes	1
x03b0:	hlt	040
x03b1:	lw	r1, [0x132e]
x03b3:	cwt	r1, -1
x03b4:	jes	1
x03b5:	hlt	040
x03b6:	lwt	r0, 0
x03b7:	lwt	r1, 0
x03b8:	nr	r1, -1
x03ba:	cw	r0, 0x8000
x03bc:	jes	1
x03bd:	hlt	040
x03be:	cwt	r1, 0
x03bf:	jes	1
x03c0:	hlt	040
x03c1:	lwt	r0, 0
x03c2:	lwt	r1, -1
x03c3:	nr	r1, 0x0
x03c5:	cw	r0, 0x8000
x03c7:	jes	1
x03c8:	hlt	040
x03c9:	cwt	r1, 0
x03ca:	jes	1
x03cb:	hlt	040
x03cc:	lwt	r0, 0
x03cd:	lwt	r1, 0
x03ce:	lw	r2, r1
x03cf:	nr	r1, r2
x03d0:	cw	r0, 0x8000
x03d2:	jes	1
x03d3:	hlt	040
x03d4:	cwt	r1, 0
x03d5:	jes	1
x03d6:	hlt	040
x03d7:	lwt	r0, 0
x03d8:	lwt	r1, -1
x03d9:	lw	r2, r1
x03da:	nr	r1, r2
x03db:	cwt	r0, 0
x03dc:	jes	1
x03dd:	hlt	040
x03de:	cwt	r1, -1
x03df:	jes	1
x03e0:	hlt	040
x03e1:	lwt	r0, 0
x03e2:	lw	r1, [pat01]
x03e4:	lw	r2, r1
x03e5:	nr	r1, r2
x03e6:	cwt	r0, 0
x03e7:	jes	1
x03e8:	hlt	040
x03e9:	cw	r1, [pat01]
x03eb:	jes	1
x03ec:	hlt	040
x03ed:	lwt	r0, 0
x03ee:	lw	r1, [pat10]
x03f0:	lw	r2, r1
x03f1:	nr	r1, r2
x03f2:	cwt	r0, 0
x03f3:	jes	1
x03f4:	hlt	040
x03f5:	cw	r1, [pat10]
x03f7:	jes	1
x03f8:	hlt	040
x03f9:	lwt	r0, 0
x03fa:	lw	r1, [pat10]
x03fc:	nr	r1, [pat01]
x03fe:	cw	r0, 0x8000
x0400:	jes	1
x0401:	hlt	040
x0402:	cwt	r1, 0
x0403:	jes	1
x0404:	hlt	040
x0405:	lwt	r0, 0
x0406:	lw	r1, [pat01]
x0408:	nr	r1, [pat10]
x040a:	cw	r0, 0x8000
x040c:	jes	1
x040d:	hlt	040
x040e:	cwt	r1, 0
x040f:	jes	1
x0410:	hlt	040
x0411:	lwt	r0, 0
x0412:	lwt	r1, -1
x0413:	rw	r1, 0x132e
x0415:	lwt	r1, 0
x0416:	nm	r1, 0x132e
x0418:	cw	r0, 0x8000
x041a:	jes	1
x041b:	hlt	040
x041c:	lw	r1, [0x132e]
x041e:	cwt	r1, 0
x041f:	jes	1
x0420:	hlt	040
x0421:	lwt	r0, 0
x0422:	lwt	r1, -1
x0423:	rz	0x132e
x0425:	nm	r1, 0x132e
x0427:	cw	r0, 0x8000
x0429:	jes	1
x042a:	hlt	040
x042b:	lw	r1, [0x132e]
x042d:	cwt	r1, 0
x042e:	jes	1
x042f:	hlt	040
x0430:	lwt	r0, 0
x0431:	rz	0x132e
x0433:	lwt	r1, 0
x0434:	nm	r1, 0x132e
x0436:	cw	r0, 0x8000
x0438:	jes	1
x0439:	hlt	040
x043a:	lw	r1, [0x132e]
x043c:	cwt	r1, 0
x043d:	jes	1
x043e:	hlt	040
x043f:	lwt	r0, 0
x0440:	lwt	r1, -1
x0441:	rw	r1, 0x132e
x0443:	nm	r1, 0x132e
x0445:	cwt	r0, 0
x0446:	jes	1
x0447:	hlt	040
x0448:	lw	r1, [0x132e]
x044a:	cwt	r1, -1
x044b:	jes	1
x044c:	hlt	040
x044d:	lwt	r0, 0
x044e:	lw	r1, [pat01]
x0450:	rw	r1, 0x132e
x0452:	nm	r1, 0x132e
x0454:	cwt	r0, 0
x0455:	jes	1
x0456:	hlt	040
x0457:	lw	r1, [0x132e]
x0459:	cw	r1, [pat01]
x045b:	jes	1
x045c:	hlt	040
x045d:	lwt	r0, 0
x045e:	lw	r1, [pat10]
x0460:	rw	r1, 0x132e
x0462:	nm	r1, 0x132e
x0464:	cwt	r0, 0
x0465:	jes	1
x0466:	hlt	040
x0467:	lw	r1, [0x132e]
x0469:	cw	r1, [pat10]
x046b:	jes	1
x046c:	hlt	040
x046d:	lwt	r0, 0
x046e:	lw	r1, [pat10]
x0470:	rw	r1, 0x132e
x0472:	lw	r1, [pat01]
x0474:	nm	r1, 0x132e
x0476:	cw	r0, 0x8000
x0478:	jes	1
x0479:	hlt	040
x047a:	lw	r1, [0x132e]
x047c:	cwt	r1, 0
x047d:	jes	1
x047e:	hlt	040
x047f:	lwt	r0, 0
x0480:	lw	r1, [pat01]
x0482:	rw	r1, 0x132e
x0484:	lw	r1, [pat10]
x0486:	nm	r1, 0x132e
x0488:	cw	r0, 0x8000
x048a:	jes	1
x048b:	hlt	040
x048c:	lw	r1, [0x132e]
x048e:	cwt	r1, 0
x048f:	jes	1
x0490:	hlt	040
x0491:	lwt	r0, 0
x0492:	lwt	r1, 0
x0493:	lwt	r2, -1
x0494:	er	r1, r2
x0495:	cw	r0, 0x8000
x0497:	jes	1
x0498:	hlt	040
x0499:	cwt	r1, 0
x049a:	jes	1
x049b:	hlt	040
x049c:	lwt	r0, 0
x049d:	lwt	r1, -1
x049e:	lwt	r2, 0
x049f:	er	r1, r2
x04a0:	cwt	r0, 0
x04a1:	jes	1
x04a2:	hlt	040
x04a3:	cwt	r1, -1
x04a4:	jes	1
x04a5:	hlt	040
x04a6:	lwt	r0, 0
x04a7:	lwt	r1, 0
x04a8:	lwt	r2, 0
x04a9:	er	r1, r2
x04aa:	cw	r0, 0x8000
x04ac:	jes	1
x04ad:	hlt	040
x04ae:	cwt	r1, 0
x04af:	jes	1
x04b0:	hlt	040
x04b1:	lwt	r0, 0
x04b2:	lwt	r1, -1
x04b3:	lwt	r2, -1
x04b4:	er	r1, r2
x04b5:	cw	r0, 0x8000
x04b7:	jes	1
x04b8:	hlt	040
x04b9:	cwt	r1, 0
x04ba:	jes	1
x04bb:	hlt	040
x04bc:	lwt	r0, 0
x04bd:	lw	r1, [pat01]
x04bf:	lw	r2, r1
x04c0:	er	r1, r2
x04c1:	cw	r0, 0x8000
x04c3:	jes	1
x04c4:	hlt	040
x04c5:	cwt	r1, 0
x04c6:	jes	1
x04c7:	hlt	040
x04c8:	lwt	r0, 0
x04c9:	lw	r1, [pat10]
x04cb:	lw	r2, r1
x04cc:	er	r1, r2
x04cd:	cw	r0, 0x8000
x04cf:	jes	1
x04d0:	hlt	040
x04d1:	cwt	r1, 0
x04d2:	jes	1
x04d3:	hlt	040
x04d4:	lwt	r0, 0
x04d5:	lw	r1, [pat01]
x04d7:	lw	r2, [pat10]
x04d9:	er	r1, r2
x04da:	cwt	r0, 0
x04db:	jes	1
x04dc:	hlt	040
x04dd:	cw	r1, [pat01]
x04df:	jes	1
x04e0:	hlt	040
x04e1:	lwt	r0, 0
x04e2:	lw	r1, [pat10]
x04e4:	lw	r2, [pat01]
x04e6:	er	r1, r2
x04e7:	cwt	r0, 0
x04e8:	jes	1
x04e9:	hlt	040
x04ea:	cw	r1, [pat10]
x04ec:	jes	1
x04ed:	hlt	040
x04ee:	lwt	r0, 0
x04ef:	lwt	r1, -1
x04f0:	rw	r1, 0x132e
x04f2:	lwt	r1, 0
x04f3:	em	r1, 0x132e
x04f5:	cwt	r0, 0
x04f6:	jes	1
x04f7:	hlt	040
x04f8:	lw	r1, [0x132e]
x04fa:	cwt	r1, -1
x04fb:	jes	1
x04fc:	hlt	040
x04fd:	lwt	r0, 0
x04fe:	lwt	r1, 0
x04ff:	rw	r1, 0x132e
x0501:	lwt	r1, -1
x0502:	em	r1, 0x132e
x0504:	cw	r0, 0x8000
x0506:	jes	1
x0507:	hlt	040
x0508:	lw	r1, [0x132e]
x050a:	cwt	r1, 0
x050b:	jes	1
x050c:	hlt	040
x050d:	lwt	r0, 0
x050e:	lwt	r1, 0
x050f:	rw	r1, 0x132e
x0511:	em	r1, 0x132e
x0513:	cw	r0, 0x8000
x0515:	jes	1
x0516:	hlt	040
x0517:	lw	r1, [0x132e]
x0519:	cwt	r1, 0
x051a:	jes	1
x051b:	hlt	040
x051c:	lwt	r0, 0
x051d:	lwt	r1, -1
x051e:	rw	r1, 0x132e
x0520:	em	r1, 0x132e
x0522:	cw	r0, 0x8000
x0524:	jes	1
x0525:	hlt	040
x0526:	lw	r1, [0x132e]
x0528:	cwt	r1, 0
x0529:	jes	1
x052a:	hlt	040
x052b:	lwt	r0, 0
x052c:	lw	r1, [pat01]
x052e:	rw	r1, 0x132e
x0530:	em	r1, 0x132e
x0532:	cw	r0, 0x8000
x0534:	jes	1
x0535:	hlt	040
x0536:	lw	r1, [0x132e]
x0538:	cwt	r1, 0
x0539:	jes	1
x053a:	hlt	040
x053b:	lwt	r0, 0
x053c:	lw	r1, [pat10]
x053e:	rw	r1, 0x132e
x0540:	em	r1, 0x132e
x0542:	cw	r0, 0x8000
x0544:	jes	1
x0545:	hlt	040
x0546:	lw	r1, [0x132e]
x0548:	cwt	r1, 0
x0549:	jes	1
x054a:	hlt	040
x054b:	lwt	r0, 0
x054c:	lw	r1, [pat10]
x054e:	rw	r1, 0x132e
x0550:	lw	r1, [pat01]
x0552:	em	r1, 0x132e
x0554:	cwt	r0, 0
x0555:	jes	1
x0556:	hlt	040
x0557:	lw	r1, [0x132e]
x0559:	cw	r1, [pat10]
x055b:	jes	1
x055c:	hlt	040
x055d:	lwt	r0, 0
x055e:	lw	r1, [pat01]
x0560:	rw	r1, 0x132e
x0562:	lw	r1, [pat10]
x0564:	em	r1, 0x132e
x0566:	cwt	r0, 0
x0567:	jes	1
x0568:	hlt	040
x0569:	lw	r1, [0x132e]
x056b:	cw	r1, [pat01]
x056d:	jes	1
x056e:	hlt	040
x056f:	lwt	r0, 0
x0570:	lwt	r1, 0
x0571:	lwt	r2, -1
x0572:	xr	r1, r2
x0573:	cwt	r0, 0
x0574:	jes	1
x0575:	hlt	040
x0576:	cwt	r1, -1
x0577:	jes	1
x0578:	hlt	040
x0579:	lwt	r0, 0
x057a:	lwt	r1, -1
x057b:	lwt	r2, 0
x057c:	xr	r1, r2
x057d:	cwt	r0, 0
x057e:	jes	1
x057f:	hlt	040
x0580:	cwt	r1, -1
x0581:	jes	1
x0582:	hlt	040
x0583:	lwt	r0, 0
x0584:	lwt	r1, 0
x0585:	lwt	r2, 0
x0586:	xr	r1, r2
x0587:	cw	r0, 0x8000
x0589:	jes	1
x058a:	hlt	040
x058b:	cwt	r1, 0
x058c:	jes	1
x058d:	hlt	040
x058e:	lwt	r0, 0
x058f:	lwt	r1, -1
x0590:	lwt	r2, -1
x0591:	xr	r1, r2
x0592:	cw	r0, 0x8000
x0594:	jes	1
x0595:	hlt	040
x0596:	cwt	r1, 0
x0597:	jes	1
x0598:	hlt	040
x0599:	lwt	r0, 0
x059a:	lw	r1, [pat10]
x059c:	xr	r1, [pat10]
x059e:	cw	r0, 0x8000
x05a0:	jes	1
x05a1:	hlt	040
x05a2:	cwt	r1, 0
x05a3:	jes	1
x05a4:	hlt	040
x05a5:	lwt	r0, 0
x05a6:	lw	r1, [pat01]
x05a8:	xr	r1, [pat01]
x05aa:	cw	r0, 0x8000
x05ac:	jes	1
x05ad:	hlt	040
x05ae:	cwt	r1, 0
x05af:	jes	1
x05b0:	hlt	040
x05b1:	lwt	r0, 0
x05b2:	lw	r1, [pat01]
x05b4:	lw	r2, [pat10]
x05b6:	xr	r1, r2
x05b7:	cwt	r0, 0
x05b8:	jes	1
x05b9:	hlt	040
x05ba:	cwt	r1, -1
x05bb:	jes	1
x05bc:	hlt	040
x05bd:	lwt	r0, 0
x05be:	lw	r1, [pat10]
x05c0:	lw	r2, [pat01]
x05c2:	xr	r1, r2
x05c3:	cwt	r0, 0
x05c4:	jes	1
x05c5:	hlt	040
x05c6:	cwt	r1, -1
x05c7:	jes	1
x05c8:	hlt	040
x05c9:	lwt	r0, 0
x05ca:	lwt	r1, -1
x05cb:	rw	r1, 0x132e
x05cd:	lwt	r1, 0
x05ce:	xm	r1, 0x132e
x05d0:	cwt	r0, 0
x05d1:	jes	1
x05d2:	hlt	040
x05d3:	lw	r1, [0x132e]
x05d5:	cwt	r1, -1
x05d6:	jes	1
x05d7:	hlt	040
x05d8:	lwt	r0, 0
x05d9:	rz	0x132e
x05db:	lwt	r1, -1
x05dc:	xm	r1, 0x132e
x05de:	cwt	r0, 0
x05df:	jes	1
x05e0:	hlt	040
x05e1:	lw	r1, [0x132e]
x05e3:	cwt	r1, -1
x05e4:	jes	1
x05e5:	hlt	040
x05e6:	lwt	r0, 0
x05e7:	rz	0x132e
x05e9:	lwt	r1, 0
x05ea:	xm	r1, 0x132e
x05ec:	cw	r0, 0x8000
x05ee:	jes	1
x05ef:	hlt	040
x05f0:	lw	r1, [0x132e]
x05f2:	cwt	r1, 0
x05f3:	jes	1
x05f4:	hlt	040
x05f5:	lwt	r0, 0
x05f6:	lwt	r1, -1
x05f7:	rw	r1, 0x132e
x05f9:	xm	r1, 0x132e
x05fb:	cw	r0, 0x8000
x05fd:	jes	1
x05fe:	hlt	040
x05ff:	lw	r1, [0x132e]
x0601:	cwt	r1, 0
x0602:	jes	1
x0603:	hlt	040
x0604:	lwt	r0, 0
x0605:	lw	r1, [pat01]
x0607:	rw	r1, 0x132e
x0609:	xm	r1, 0x132e
x060b:	cw	r0, 0x8000
x060d:	jes	1
x060e:	hlt	040
x060f:	lw	r1, [0x132e]
x0611:	cwt	r1, 0
x0612:	jes	1
x0613:	hlt	040
x0614:	lwt	r0, 0
x0615:	lw	r1, [pat10]
x0617:	rw	r1, 0x132e
x0619:	xm	r1, 0x132e
x061b:	cw	r0, 0x8000
x061d:	jes	1
x061e:	hlt	040
x061f:	lw	r1, [0x132e]
x0621:	cwt	r1, 0
x0622:	jes	1
x0623:	hlt	040
x0624:	lwt	r0, 0
x0625:	lw	r1, [pat10]
x0627:	rw	r1, 0x132e
x0629:	lw	r1, [pat01]
x062b:	xm	r1, 0x132e
x062d:	cwt	r0, 0
x062e:	jes	1
x062f:	hlt	040
x0630:	lw	r1, [0x132e]
x0632:	cwt	r1, -1
x0633:	jes	1
x0634:	hlt	040
x0635:	lwt	r0, 0
x0636:	lw	r1, [pat01]
x0638:	rw	r1, 0x132e
x063a:	lw	r1, [pat10]
x063c:	xm	r1, 0x132e
x063e:	cwt	r0, 0
x063f:	jes	1
x0640:	hlt	040
x0641:	lw	r1, [0x132e]
x0643:	cwt	r1, -1
x0644:	jes	1
x0645:	hlt	040
x0646:	lwt	r0, 0
x0647:	lwt	r1, -1
x0648:	lwt	r2, 1
x0649:	aw	r1, r2
x064a:	cw	r0, 0x9000
x064c:	jes	1
x064d:	hlt	040
x064e:	cwt	r1, 0
x064f:	jes	1
x0650:	hlt	040
x0651:	lwt	r0, 0
x0652:	lw	r1, [pat10]
x0654:	aw	r1, r1
x0655:	cw	r0, 0x7000
x0657:	jes	1
x0658:	hlt	040
x0659:	cw	r1, 0x5554
x065b:	jes	1
x065c:	hlt	040
x065d:	lwt	r0, 0
x065e:	lw	r1, 0x5555
x0660:	aw	r1, r1
x0661:	cw	r0, 0x2000
x0663:	jes	1
x0664:	hlt	040
x0665:	cw	r1, 0xaaaa
x0667:	jes	1
x0668:	hlt	040
x0669:	lwt	r0, 0
x066a:	lwt	r1, -1
x066b:	lw	r2, 0x7fff
x066d:	aw	r1, r2
x066e:	cw	r0, 0x1000
x0670:	jes	1
x0671:	hlt	040
x0672:	cw	r1, 0x7ffe
x0674:	jes	1
x0675:	hlt	040
x0676:	lwt	r1, -1
x0677:	lwt	r0, 0
x0678:	lw	r2, r1
x0679:	aw	r1, r2
x067a:	cw	r0, 0x5000
x067c:	jes	1
x067d:	hlt	040
x067e:	cwt	r1, -2
x067f:	jes	1
x0680:	hlt	040
x0681:	lwt	r0, 0
x0682:	lw	r1, 0x7fff
x0684:	lw	r2, 0x7fff
x0686:	aw	r1, r2
x0687:	cw	r0, 0x2000
x0689:	jes	1
x068a:	hlt	040
x068b:	cwt	r1, -2
x068c:	jes	1
x068d:	hlt	040
x068e:	lwt	r0, 0
x068f:	lwt	r1, -1
x0690:	lw	r2, 0x8000
x0692:	aw	r1, r2
x0693:	cw	r0, 0x7000
x0695:	jes	1
x0696:	hlt	040
x0697:	cw	r1, 0x7fff
x0699:	jes	1
x069a:	hlt	040
x069b:	lwt	r0, 0
x069c:	lw	r1, 0x8000
x069e:	lw	r2, r1
x069f:	aw	r1, r2
x06a0:	cw	r0, 0x7000
x06a2:	jes	1
x06a3:	hlt	040
x06a4:	cwt	r1, 0
x06a5:	jes	1
x06a6:	hlt	040
x06a7:	lw	r0, 0x1000
x06a9:	lw	r1, 0xfffd
x06ab:	lwt	r2, 1
x06ac:	ac	r1, r2
x06ad:	cw	r0, 0x4000
x06af:	jes	1
x06b0:	hlt	040
x06b1:	cwt	r1, -1
x06b2:	jes	1
x06b3:	hlt	040
x06b4:	lw	r0, 0x1000
x06b6:	lw	r1, 0xfffe
x06b8:	lwt	r2, 1
x06b9:	ac	r1, r2
x06ba:	cw	r0, 0x9000
x06bc:	jes	1
x06bd:	hlt	040
x06be:	cwt	r1, 0
x06bf:	jes	1
x06c0:	hlt	040
x06c1:	lw	r0, 0x1000
x06c3:	lw	r1, 0x7fff
x06c5:	ac	r1, 0x0
x06c7:	cw	r0, 0x2000
x06c9:	jes	1
x06ca:	hlt	040
x06cb:	cw	r1, 0x8000
x06cd:	jes	1
x06ce:	hlt	040
x06cf:	lwt	r0, 0
x06d0:	lwt	r1, 0
x06d1:	lw	r2, r1
x06d2:	sw	r1, r2
x06d3:	cw	r0, 0x9000
x06d5:	jes	1
x06d6:	hlt	040
x06d7:	cwt	r1, 0
x06d8:	jes	1
x06d9:	hlt	040
x06da:	lwt	r0, 0
x06db:	lw	r1, 0x8000
x06dd:	lwt	r2, 1
x06de:	sw	r1, r2
x06df:	cw	r0, 0x7000
x06e1:	jes	1
x06e2:	hlt	040
x06e3:	cw	r1, 0x7fff
x06e5:	jes	1
x06e6:	hlt	040
x06e7:	lwt	r0, 0
x06e8:	lwt	r1, 0
x06e9:	lwt	r2, -1
x06ea:	cl	r1, r2
x06eb:	cw	r0, 0x800
x06ed:	jes	1
x06ee:	hlt	040
x06ef:	cl	r2, r1
x06f0:	cw	r0, 0x200
x06f2:	jes	1
x06f3:	hlt	040
x06f4:	lw	r1, 0x7fff
x06f6:	lw	r2, 0x8000
x06f8:	cl	r1, r2
x06f9:	cw	r0, 0x800
x06fb:	jes	1
x06fc:	hlt	040
x06fd:	lw	r1, [pat10]
x06ff:	lw	r2, r1
x0700:	cl	r1, r2
x0701:	cw	r0, 0x400
x0703:	jes	1
x0704:	hlt	040
x0705:	lwt	r1, 0
x0706:	lj	x070d
x0708:	cw	r1, 0x708
x070a:	jes	1
x070b:	hlt	040
x070c:	ujs	10
x070d:	.word	0
x070e:	lw	r1, [x070d]
x0710:	cw	r1, 0x708
x0712:	jes	1
x0713:	hlt	040
x0714:	lw	r1, 0x708
x0716:	ujs	-15; -> 0x0708
x0717:	uj	0x71a
x0719:	hlt	040
x071a:	lw	r1, 0x71e
x071c:	uj	r1
x071d:	hlt	040
x071e:	lw	r1, 0x725
x0720:	rw	r1, 0x132e
x0722:	uj	[0x132e]
x0724:	hlt	040
x0725:	lw	r1, 0x72d
x0727:	rw	r1, 0x132e
x0729:	lw	r1, 0x132e
x072b:	uj	[r1]
x072c:	hlt	040
x072d:	lj	0x854
x072f:	ld	0x1338
x0731:	cw	r1, [pat01]
x0733:	jes	1
x0734:	hlt	040
x0735:	cw	r2, [pat10]
x0737:	jes	1
x0738:	hlt	040
x0739:	cwt	r3, -1
x073a:	jes	1
x073b:	ujs	11
x073c:	cwt	r4, -1
x073d:	jes	1
x073e:	ujs	8	; -> 0x0747
x073f:	cwt	r5, -1
x0740:	jes	1
x0741:	ujs	5	; -> 0x0747
x0742:	cwt	r6, -1
x0743:	jes	1
x0744:	ujs	2	; -> 0x0747
x0745:	cwt	r7, -1
x0746:	jes	1
x0747:	hlt	040
x0748:	lj	0x854
x074a:	lf	0x1338
x074c:	cw	r1, [pat01]
x074e:	jes	1
x074f:	hlt	040
x0750:	cw	r2, [pat10]
x0752:	jes	1
x0753:	hlt	040
x0754:	cw	r3, [0x133a]
x0756:	jes	1
x0757:	hlt	040
x0758:	cwt	r4, -1
x0759:	jes	1
x075a:	ujs	8	; -> 0x0763
x075b:	cwt	r5, -1
x075c:	jes	1
x075d:	ujs	5	; -> 0x0763
x075e:	cwt	r6, -1
x075f:	jes	1
x0760:	ujs	2	; -> 0x0763
x0761:	cwt	r7, -1
x0762:	jes	1
x0763:	hlt	040
x0764:	lj	0x854
x0766:	la	0x1338
x0768:	cw	r1, [pat01]
x076a:	jes	1
x076b:	hlt	040
x076c:	cw	r2, [pat10]
x076e:	jes	1
x076f:	hlt	040
x0770:	cw	r3, [0x133a]
x0772:	jes	1
x0773:	hlt	040
x0774:	cw	r4, [0x133b]
x0776:	jes	1
x0777:	hlt	040
x0778:	cw	r5, [0x133c]
x077a:	jes	1
x077b:	hlt	040
x077c:	cw	r6, [0x133d]
x077e:	jes	1
x077f:	hlt	040
x0780:	cw	r7, [0x133e]
x0782:	jes	1
x0783:	hlt	040
x0784:	lj	0x854
x0786:	ll	0x1338
x0788:	cw	r5, [pat01]
x078a:	jes	1
x078b:	hlt	040
x078c:	cw	r6, [pat10]
x078e:	jes	1
x078f:	hlt	040
x0790:	cw	r7, [0x133a]
x0792:	jes	1
x0793:	hlt	040
x0794:	cwt	r1, -1
x0795:	jes	1
x0796:	ujs	8	; -> 0x079f
x0797:	cwt	r2, -1
x0798:	jes	1
x0799:	ujs	5	; -> 0x079f
x079a:	cwt	r3, -1
x079b:	jes	1
x079c:	ujs	2	; -> 0x079f
x079d:	cwt	r4, -1
x079e:	jes	1
x079f:	hlt	040
x07a0:	lj	0x854
x07a2:	td	0x1338
x07a4:	cw	r1, [pat01]
x07a6:	jes	1
x07a7:	hlt	040
x07a8:	cw	r2, [pat10]
x07aa:	jes	1
x07ab:	hlt	040
x07ac:	cwt	r3, -1
x07ad:	jes	1
x07ae:	ujs	11
x07af:	cwt	r4, -1
x07b0:	jes	1
x07b1:	ujs	8	; -> 0x07ba
x07b2:	cwt	r5, -1
x07b3:	jes	1
x07b4:	ujs	5	; -> 0x07ba
x07b5:	cwt	r6, -1
x07b6:	jes	1
x07b7:	ujs	2	; -> 0x07ba
x07b8:	cwt	r7, -1
x07b9:	jes	1
x07ba:	hlt	040
x07bb:	lj	0x854
x07bd:	tf	0x1338
x07bf:	cw	r1, [pat01]
x07c1:	jes	1
x07c2:	hlt	040
x07c3:	cw	r2, [pat10]
x07c5:	jes	1
x07c6:	hlt	040
x07c7:	cw	r3, [0x133a]
x07c9:	jes	1
x07ca:	hlt	040
x07cb:	cwt	r4, -1
x07cc:	jes	1
x07cd:	ujs	8	; -> 0x07d6
x07ce:	cwt	r5, -1
x07cf:	jes	1
x07d0:	ujs	5	; -> 0x07d6
x07d1:	cwt	r6, -1
x07d2:	jes	1
x07d3:	ujs	2	; -> 0x07d6
x07d4:	cwt	r7, -1
x07d5:	jes	1
x07d6:	hlt	040
x07d7:	lj	0x854
x07d9:	ta	0x1338
x07db:	cw	r1, [pat01]
x07dd:	jes	1
x07de:	hlt	040
x07df:	cw	r2, [pat10]
x07e1:	jes	1
x07e2:	hlt	040
x07e3:	cw	r3, [0x133a]
x07e5:	jes	1
x07e6:	hlt	040
x07e7:	cw	r4, [0x133b]
x07e9:	jes	1
x07ea:	hlt	040
x07eb:	cw	r5, [0x133c]
x07ed:	jes	1
x07ee:	hlt	040
x07ef:	cw	r6, [0x133d]
x07f1:	jes	1
x07f2:	hlt	040
x07f3:	cw	r7, [0x133e]
x07f5:	jes	1
x07f6:	hlt	040
x07f7:	lj	0x854
x07f9:	tl	0x1338
x07fb:	cw	r5, [pat01]
x07fd:	jes	1
x07fe:	hlt	040
x07ff:	cw	r6, [pat10]
x0801:	jes	1
x0802:	hlt	040
x0803:	cw	r7, [0x133a]
x0805:	jes	1
x0806:	hlt	040
x0807:	cwt	r1, -1
x0808:	jes	1
x0809:	ujs	8	; -> 0x0812
x080a:	cwt	r2, -1
x080b:	jes	1
x080c:	ujs	5	; -> 0x0812
x080d:	cwt	r3, -1
x080e:	jes	1
x080f:	ujs	2	; -> 0x0812
x0810:	cwt	r4, -1
x0811:	jes	1
x0812:	hlt	040
x0813:	ta	0x133a
x0815:	rd	0x132e
x0817:	ld	0x132e
x0819:	lj	0x85e
x081b:	ta	0x133a
x081d:	rf	0x132e
x081f:	lf	0x132e
x0821:	lj	0x85e
x0823:	ta	0x133a
x0825:	ra	0x132e
x0827:	la	0x132e
x0829:	lj	0x85e
x082b:	ta	0x133a
x082d:	rl	0x132e
x082f:	ll	0x132e
x0831:	lj	0x85e
x0833:	ta	0x133a
x0835:	pd	0x132e
x0837:	ld	0x132e
x0839:	lj	0x85e
x083b:	ta	0x133a
x083d:	pf	0x132e
x083f:	lf	0x132e
x0841:	lj	0x85e
x0843:	ta	0x133a
x0845:	pa	0x132e
x0847:	la	0x132e
x0849:	lj	0x85e
x084b:	ta	0x133a
x084d:	pl	0x132e
x084f:	ll	0x132e
x0851:	lj	coreg
x0853:	ujs	41 ; -> 0x087d
x0854:	.word	0
x0855:	lwt	r1, -1
x0856:	lwt	r2, -1
x0857:	lwt	r3, -1
x0858:	lwt	r4, -1
x0859:	lwt	r5, -1
x085a:	lwt	r6, -1
x085b:	lwt	r7, -1
x085c:	uj	[0x854]

coreg:	.res	1
check:	cw	r1, [0x133a]
x0861:	jes	1
x0862:	hlt	040
x0863:	cw	r2, [0x133b]
x0865:	jes	1
x0866:	hlt	040
x0867:	cw	r3, [0x133c]
x0869:	jes	1
x086a:	hlt	040
x086b:	cw	r4, [0x133d]
x086d:	jes	1
x086e:	hlt	040
x086f:	cw	r5, [0x133e]
x0871:	jes	1
x0872:	hlt	040
x0873:	cw	r6, [0x133f]
x0875:	jes	1
x0876:	hlt	040
x0877:	cw	r7, [0x1340]
x0879:	jes	1
x087a:	hlt	040
x087b:	uj	[coreg]

x087d:	lwt	r0, 0
x087e:	lwt	r1, -3
x087f:	awt	r1, 1
x0880:	cw	r0, 0x4000
x0882:	jes	1
x0883:	hlt	040
x0884:	cwt	r1, -2
x0885:	jes	1
x0886:	hlt	040
x0887:	lwt	r0, 0
x0888:	lwt	r1, -1
x0889:	awt	r1, -1
x088a:	cw	r0, 0x5000
x088c:	jes	1
x088d:	hlt	040
x088e:	cwt	r1, -2
x088f:	jes	1
x0890:	hlt	040
x0891:	lwt	r0, 0
x0892:	lwt	r1, -1
x0893:	awt	r1, 1
x0894:	cw	r0, 0x9000
x0896:	jes	1
x0897:	hlt	040
x0898:	cwt	r1, 0
x0899:	jes	1
x089a:	hlt	040
x089b:	lwt	r0, 0
x089c:	lw	r1, 0x7fff
x089e:	awt	r1, 1
x089f:	cw	r0, 0x2000
x08a1:	jes	1
x08a2:	hlt	040
x08a3:	cw	r1, 0x8000
x08a5:	jes	1
x08a6:	hlt	040
x08a7:	lwt	r1, -2
x08a8:	irb	r1, 1
x08a9:	hlt	040
x08aa:	cwt	r1, -1
x08ab:	jes	1
x08ac:	hlt	040
x08ad:	lwt	r1, -1
x08ae:	irb	r1, 1
x08af:	ujs	1
x08b0:	hlt	040
x08b1:	cwt	r1, 0
x08b2:	jes	1
x08b3:	hlt	040
x08b4:	lwt	r1, 2
x08b5:	drb	r1, 1
x08b6:	hlt	040
x08b7:	cwt	r1, 1
x08b8:	jes	1
x08b9:	hlt	040
x08ba:	lwt	r1, 1
x08bb:	drb	r1, 1
x08bc:	ujs	1
x08bd:	hlt	040
x08be:	cwt	r1, 0
x08bf:	jes	1
x08c0:	hlt	040
x08c1:	lwt	r1, -2
x08c2:	trb	r1, 1
x08c3:	ujs	1
x08c4:	hlt	040
x08c5:	cwt	r1, -1
x08c6:	jes	1
x08c7:	hlt	040
x08c8:	lwt	r1, -1
x08c9:	trb	r1, 1
x08ca:	hlt	040
x08cb:	cwt	r1, 0
x08cc:	jes	1
x08cd:	hlt	040
x08ce:	ujs	1
x08cf:	.res	1
x08d0:	lw	r1, [pat10]
x08d2:	rw	r1, 0x8cf
x08d4:	lwt	r1, -1
x08d5:	lws	r1, -7
x08d6:	cw	r1, [pat10]
x08d8:	jes	1
x08d9:	hlt	040
x08da:	lw	r1, [pat01]
x08dc:	rw	r1, 0x8e4
x08de:	lwt	r1, -1
x08df:	lws	r1, 4
x08e0:	cw	r1, [pat01]
x08e2:	jes	2	; -> 0x08e5
x08e3:	hlt	040
x08e4:	.res	1
x08e5:	ujs	1
x08e6:	.word	0
x08e7:	lw	r1, [pat01]
x08e9:	rws	r1, -4
x08ea:	cw	r1, [0x8e6]
x08ec:	jes	1
x08ed:	hlt	040
x08ee:	lw	r1, [pat10]
x08f0:	rws	r1, 4
x08f1:	cw	r1, [0x8f5]
x08f3:	jes	2	; -> 0x08f6
x08f4:	hlt	040
x08f5:	.word	0
x08f6:	lwt	r0, 0
x08f7:	jl	0x8fe
x08f9:	lw	r0, 0x800
x08fb:	jl	0x8ff
x08fd:	hlt	040
x08fe:	hlt	040
x08ff:	lw	r0, 0xf7ff
x0901:	jl	0x904
x0903:	ujs	1
x0904:	hlt	040
x0905:	lwt	r0, 0
x0906:	je	0x90d
x0908:	lw	r0, 0x400
x090a:	je	0x90e
x090c:	hlt	040
x090d:	hlt	040
x090e:	lw	r0, 0xfbff
x0910:	je	0x913
x0912:	ujs	1
x0913:	hlt	040
x0914:	lwt	r0, 0
x0915:	jg	0x91c
x0917:	lw	r0, 0x200
x0919:	jg	0x91d
x091b:	hlt	040
x091c:	hlt	040
x091d:	lw	r0, 0xfdff
x091f:	jg	0x922
x0921:	ujs	1
x0922:	hlt	040
x0923:	lwt	r0, 0
x0924:	jz	0x92b
x0926:	lw	r0, 0x8000
x0928:	jz	0x92c
x092a:	hlt	040
x092b:	hlt	040
x092c:	lw	r0, 0x7fff
x092e:	jz	0x931
x0930:	ujs	1
x0931:	hlt	040
x0932:	lwt	r0, 0
x0933:	jm	0x93a
x0935:	lw	r0, 0x4000
x0937:	jm	0x93b
x0939:	hlt	040
x093a:	hlt	040
x093b:	lw	r0, 0xbfff
x093d:	jm	0x940
x093f:	ujs	1
x0940:	hlt	040
x0941:	lw	r0, 0xfbff
x0943:	jn	0x946
x0945:	hlt	040
x0946:	lw	r0, 0x400
x0948:	jn	0x94b
x094a:	ujs	1
x094b:	hlt	040
x094c:	lwt	r0, 0
x094d:	jls	4	; -> 0x0952
x094e:	lw	r0, 0x800
x0950:	jls	2	; -> 0x0953
x0951:	hlt	040
x0952:	hlt	040
x0953:	lw	r0, 0xf7ff
x0955:	jls	1	; -> 0x0957
x0956:	ujs	1
x0957:	hlt	040
x0958:	lwt	r0, 0
x0959:	jgs	4	; -> 0x095e
x095a:	lw	r0, 0x200
x095c:	jgs	2	; -> 0x095f
x095d:	hlt	040
x095e:	hlt	040
x095f:	lw	r0, 0xfdff
x0961:	jgs	1	; -> 0x0963
x0962:	ujs	1
x0963:	hlt	040
x0964:	lwt	r0, 0
x0965:	jxs	4	; -> 0x096a
x0966:	lw	r0, 0x80
x0968:	jxs	2	; -> 0x096b
x0969:	hlt	040
x096a:	hlt	040
x096b:	lw	r0, 0xff7f
x096d:	jxs	1	; -> 0x096f
x096e:	ujs	1
x096f:	hlt	040
x0970:	lwt	r0, 0
x0971:	jys	4	; -> 0x0976
x0972:	lw	r0, 0x100
x0974:	jys	2	; -> 0x0977
x0975:	hlt	040
x0976:	hlt	040
x0977:	lw	r0, 0xfeff
x0979:	jys	1	; -> 0x097b
x097a:	ujs	1
x097b:	hlt	040
x097c:	lwt	r0, 0
x097d:	jcs	4	; -> 0x0982
x097e:	lw	r0, 0x1000
x0980:	jcs	2	; -> 0x0983
x0981:	hlt	040
x0982:	hlt	040
x0983:	lw	r0, 0xefff
x0985:	jcs	1	; -> 0x0987
x0986:	ujs	1
x0987:	hlt	040
x0988:	lwt	r0, 0
x0989:	jvs	4	; -> 0x098e
x098a:	lw	r0, 0x2000
x098c:	jvs	2	; -> 0x098f
x098d:	hlt	040
x098e:	hlt	040
x098f:	lw	r0, 0xdfff
x0991:	jvs	1	; -> 0x0993
x0992:	ujs	1
x0993:	hlt	040
x0994:	rky	r1
x0995:	rky	r2
x0996:	rky	r3
x0997:	rky	r4
x0998:	cw	r1, r2
x0999:	jes	1
x099a:	hlt	040
x099b:	cw	r3, r4
x099c:	jes	1
x099d:	hlt	040
x099e:	cw	r3, r1
x099f:	jes	1
x09a0:	hlt	040
x09a1:	lwt	r0, 0
x09a2:	lw	r1, 0x8000
x09a4:	sxu	r1
x09a5:	cw	r0, 0x80
x09a7:	jes	1
x09a8:	hlt	040
x09a9:	lwt	r0, -1
x09aa:	lw	r1, 0x7fff
x09ac:	sxu	r1
x09ad:	cw	r0, 0xff7f
x09af:	jes	1
x09b0:	hlt	040
x09b1:	lwt	r0, 0
x09b2:	lwt	r1, 1
x09b3:	sxl	r1
x09b4:	cw	r0, 0x80
x09b6:	jes	1
x09b7:	hlt	040
x09b8:	lwt	r0, -1
x09b9:	lw	r1, 0xfffe
x09bb:	sxl	r1
x09bc:	cw	r0, 0xff7f
x09be:	jes	1
x09bf:	hlt	040
x09c0:	lw	r1, [pat10]
x09c2:	lw	r2, [pat01]
x09c4:	lw	r0, 0x100
x09c6:	srz	r1
x09c7:	cwt	r0, 0
x09c8:	jes	1
x09c9:	hlt	040
x09ca:	cw	r1, r2
x09cb:	jes	1
x09cc:	hlt	040
x09cd:	lwt	r0, 0
x09ce:	srz	r2
x09cf:	cw	r0, 0x100
x09d1:	jes	1
x09d2:	hlt	040
x09d3:	cw	r2, 0x2aaa
x09d5:	jes	1
x09d6:	hlt	040
x09d7:	lwt	r0, 0
x09d8:	lw	r1, [pat10]
x09da:	lw	r2, [pat01]
x09dc:	sry	r1
x09dd:	cwt	r0, 0
x09de:	jes	1
x09df:	hlt	040
x09e0:	cw	r1, r2
x09e1:	jes	1
x09e2:	hlt	040
x09e3:	lw	r0, 0x100
x09e5:	lw	r1, [pat10]
x09e7:	sry	r1
x09e8:	cwt	r0, 0
x09e9:	jes	1
x09ea:	hlt	040
x09eb:	cw	r1, 0xd555
x09ed:	jes	1
x09ee:	hlt	040
x09ef:	lwt	r0, 0
x09f0:	sry	r2
x09f1:	cw	r0, 0x100
x09f3:	jes	1
x09f4:	hlt	040
x09f5:	cw	r2, 0x2aaa
x09f7:	jes	1
x09f8:	hlt	040
x09f9:	lw	r1, [pat01]
x09fb:	lw	r0, 0x100
x09fd:	sry	r1
x09fe:	cw	r0, 0x100
x0a00:	jes	1
x0a01:	hlt	040
x0a02:	cw	r1, [pat10]
x0a04:	jes	1
x0a05:	hlt	040
x0a06:	lwt	r0, 0
x0a07:	lw	r1, [pat10]
x0a09:	lw	r2, [pat01]
x0a0b:	srx	r1
x0a0c:	cwt	r0, 0
x0a0d:	jes	1
x0a0e:	hlt	040
x0a0f:	cw	r1, r2
x0a10:	jes	1
x0a11:	hlt	040
x0a12:	lw	r0, 0x80
x0a14:	lw	r1, [pat10]
x0a16:	srx	r1
x0a17:	cw	r0, 0x80
x0a19:	jes	1
x0a1a:	hlt	040
x0a1b:	cw	r1, 0xd555
x0a1d:	jes	1
x0a1e:	hlt	040
x0a1f:	lwt	r0, 0
x0a20:	srx	r2
x0a21:	cw	r0, 0x100
x0a23:	jes	1
x0a24:	hlt	040
x0a25:	cw	r2, 0x2aaa
x0a27:	jes	1
x0a28:	hlt	040
x0a29:	lw	r1, [pat01]
x0a2b:	lw	r0, 0x80
x0a2d:	srx	r1
x0a2e:	cw	r0, 0x180
x0a30:	jes	1
x0a31:	hlt	040
x0a32:	cw	r1, [pat10]
x0a34:	jes	1
x0a35:	hlt	040
x0a36:	lw	r1, [pat10]
x0a38:	lw	r2, [pat01]
x0a3a:	lw	r0, 0x100
x0a3c:	slz	r2
x0a3d:	cwt	r0, 0
x0a3e:	jes	1
x0a3f:	hlt	040
x0a40:	cw	r1, r2
x0a41:	jes	1
x0a42:	hlt	040
x0a43:	lwt	r0, 0
x0a44:	slz	r1
x0a45:	cw	r0, 0x100
x0a47:	jes	1
x0a48:	hlt	040
x0a49:	cw	r1, 0x5554
x0a4b:	jes	1
x0a4c:	hlt	040
x0a4d:	lw	r0, 0x80
x0a4f:	lw	r1, [pat10]
x0a51:	slx	r1
x0a52:	cw	r0, 0x180
x0a54:	jes	1
x0a55:	hlt	040
x0a56:	cw	r1, [pat01]
x0a58:	jes	1
x0a59:	hlt	040
x0a5a:	lw	r1, [pat01]
x0a5c:	lwt	r0, 0
x0a5d:	slx	r1
x0a5e:	cwt	r0, 0
x0a5f:	jes	1
x0a60:	hlt	040
x0a61:	cw	r1, [pat10]
x0a63:	jes	1
x0a64:	hlt	040
x0a65:	lw	r0, 0x180
x0a67:	lw	r1, [pat01]
x0a69:	slx	r1
x0a6a:	cw	r0, 0x80
x0a6c:	jes	1
x0a6d:	hlt	040
x0a6e:	cw	r1, 0xaaab
x0a70:	jes	1
x0a71:	hlt	040
x0a72:	lw	r0, 0x100
x0a74:	lw	r1, [pat01]
x0a76:	svx	r1
x0a77:	cw	r0, 0x2000
x0a79:	jes	1
x0a7a:	hlt	040
x0a7b:	cw	r1, [pat10]
x0a7d:	jes	1
x0a7e:	hlt	040
x0a7f:	lw	r0, 0x80
x0a81:	lw	r1, [pat10]
x0a83:	svx	r1
x0a84:	cw	r0, 0x2180
x0a86:	jes	1
x0a87:	hlt	040
x0a88:	cw	r1, [pat01]
x0a8a:	jes	1
x0a8b:	hlt	040
x0a8c:	lw	r0, 0x2100
x0a8e:	lw	r1, [pat01]
x0a90:	svz	r1
x0a91:	cw	r0, 0x2000
x0a93:	jes	1
x0a94:	hlt	040
x0a95:	cw	r1, [pat10]
x0a97:	jes	1
x0a98:	hlt	040
x0a99:	lw	r0, 0x2000
x0a9b:	lwt	r1, -1
x0a9c:	svz	r1
x0a9d:	cw	r0, 0x2100
x0a9f:	jes	1
x0aa0:	hlt	040
x0aa1:	cw	r1, 0xfffe
x0aa3:	jes	1
x0aa4:	hlt	040
x0aa5:	lwt	r0, 0
x0aa6:	lw	r1, [pat10]
x0aa8:	svz	r1
x0aa9:	cw	r0, 0x2100
x0aab:	jes	1
x0aac:	hlt	040
x0aad:	cw	r1, 0x5554
x0aaf:	jes	1
x0ab0:	hlt	040
x0ab1:	lw	r0, 0x100
x0ab3:	lw	r1, [pat01]
x0ab5:	sly	r1
x0ab6:	cwt	r0, 0
x0ab7:	jes	1
x0ab8:	hlt	040
x0ab9:	cw	r1, 0xaaab
x0abb:	jes	1
x0abc:	hlt	040
x0abd:	lwt	r0, 0
x0abe:	lw	r1, [pat01]
x0ac0:	sly	r1
x0ac1:	cwt	r0, 0
x0ac2:	jes	1
x0ac3:	hlt	040
x0ac4:	cw	r1, [pat10]
x0ac6:	jes	1
x0ac7:	hlt	040
x0ac8:	lw	r0, 0x100
x0aca:	lw	r1, [pat10]
x0acc:	sly	r1
x0acd:	cw	r0, 0x100
x0acf:	jes	1
x0ad0:	hlt	040
x0ad1:	cw	r1, [pat01]
x0ad3:	jes	1
x0ad4:	hlt	040
x0ad5:	lwt	r0, 0
x0ad6:	lw	r1, [pat01]
x0ad8:	svy	r1
x0ad9:	cw	r0, 0x2000
x0adb:	jes	1
x0adc:	hlt	040
x0add:	cw	r1, [pat10]
x0adf:	jes	1
x0ae0:	hlt	040
x0ae1:	lwt	r0, 0
x0ae2:	lw	r1, [pat10]
x0ae4:	svy	r1
x0ae5:	cw	r0, 0x2100
x0ae7:	jes	1
x0ae8:	hlt	040
x0ae9:	cw	r1, 0x5554
x0aeb:	jes	1
x0aec:	hlt	040
x0aed:	lw	r0, 0x2100
x0aef:	lw	r1, [pat01]
x0af1:	svy	r1
x0af2:	cw	r0, 0x2000
x0af4:	jes	1
x0af5:	hlt	040
x0af6:	cw	r1, 0xaaab
x0af8:	jes	1
x0af9:	hlt	040
x0afa:	lw	r0, 0x100
x0afc:	lw	r1, [pat10]
x0afe:	svy	r1
x0aff:	cw	r0, 0x2100
x0b01:	jes	1
x0b02:	hlt	040
x0b03:	cw	r1, [pat01]
x0b05:	jes	1
x0b06:	hlt	040
x0b07:	lwt	r0, 0
x0b08:	lw	r1, [pat01]
x0b0a:	shc	r1, 1
x0b0b:	cw	r1, [pat10]
x0b0d:	jes	1
x0b0e:	hlt	040
x0b0f:	lw	r1, [pat10]
x0b11:	shc	r1, 1
x0b12:	cw	r1, [pat01]
x0b14:	jes	1
x0b15:	hlt	040
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
x0b2a:	hlt	040
x0b2b:	lwt	r1, -1
x0b2c:	shc	r1, 15
x0b2d:	cwt	r1, -1
x0b2e:	jes	1
x0b2f:	hlt	040
x0b30:	lwt	r0, 0
x0b31:	lwt	r1, -1
x0b32:	ngc	r1
x0b33:	cw	r0, 0x8000
x0b35:	jes	1
x0b36:	hlt	040
x0b37:	cwt	r1, 0
x0b38:	jes	1
x0b39:	hlt	040
x0b3a:	lw	r0, 0x1000
x0b3c:	lw	r1, 0x7fff
x0b3e:	ngc	r1
x0b3f:	cw	r0, 0x4000
x0b41:	jes	1
x0b42:	hlt	040
x0b43:	cw	r1, 0x8001
x0b45:	jes	1
x0b46:	hlt	040
x0b47:	lw	r1, 0x8001
x0b49:	lw	r0, 0x1000
x0b4b:	ngc	r1
x0b4c:	cwt	r0, 0
x0b4d:	jes	1
x0b4e:	hlt	040
x0b4f:	cw	r1, 0x7fff
x0b51:	jes	1
x0b52:	hlt	040
x0b53:	lwt	r0, 0
x0b54:	lwt	r1, -1
x0b55:	nga	r1
x0b56:	cwt	r0, 0
x0b57:	jes	1
x0b58:	hlt	040
x0b59:	cwt	r1, 1
x0b5a:	jes	1
x0b5b:	hlt	040
x0b5c:	lwt	r0, 0
x0b5d:	lw	r1, 0x8000
x0b5f:	nga	r1
x0b60:	cw	r0, 0x2000
x0b62:	jes	1
x0b63:	hlt	040
x0b64:	cw	r1, 0x8000
x0b66:	jes	1
x0b67:	hlt	040
x0b68:	lwt	r0, 0
x0b69:	lwt	r1, 0
x0b6a:	nga	r1
x0b6b:	cw	r0, 0x9000
x0b6d:	jes	1
x0b6e:	hlt	040
x0b6f:	cwt	r1, 0
x0b70:	jes	1
x0b71:	hlt	040
x0b72:	lwt	r0, 0
x0b73:	lw	r1, 0x7fff
x0b75:	nga	r1
x0b76:	cw	r0, 0x4000
x0b78:	jes	1
x0b79:	hlt	040
x0b7a:	cw	r1, 0x8001
x0b7c:	jes	1
x0b7d:	hlt	040
x0b7e:	lwt	r0, 0
x0b7f:	lwt	r1, -1
x0b80:	ngl	r1
x0b81:	cw	r0, 0x8000
x0b83:	jes	1
x0b84:	hlt	040
x0b85:	cwt	r1, 0
x0b86:	jes	1
x0b87:	hlt	040
x0b88:	lwt	r1, 0
x0b89:	lwt	r0, 0
x0b8a:	ngl	r1
x0b8b:	cwt	r0, 0
x0b8c:	jes	1
x0b8d:	hlt	040
x0b8e:	cwt	r1, -1
x0b8f:	jes	1
x0b90:	hlt	040
x0b91:	lwt	r0, 0
x0b92:	lw	r1, [pat01]
x0b94:	ngl	r1
x0b95:	cwt	r0, 0
x0b96:	jes	1
x0b97:	hlt	040
x0b98:	cw	r1, [pat10]
x0b9a:	jes	1
x0b9b:	hlt	040
x0b9c:	lwt	r1, -1
x0b9d:	zrb	r1
x0b9e:	cw	r1, 0xff00
x0ba0:	jes	1
x0ba1:	hlt	040
x0ba2:	lwt	r1, -1
x0ba3:	zlb	r1
x0ba4:	cw	r1, 0xff
x0ba6:	jes	1
x0ba7:	hlt	040
x0ba8:	lwt	r1, 0
x0ba9:	lb	r1, 0x2670
x0bab:	cw	r1, 0x55
x0bad:	jes	1
x0bae:	hlt	040
x0baf:	lwt	r1, -1
x0bb0:	rz	0x132e
x0bb2:	lb	r1, 0x265d
x0bb4:	cw	r1, 0xff00
x0bb6:	jes	1
x0bb7:	hlt	040
x0bb8:	lwt	r1, -1
x0bb9:	rz	0x132e
x0bbb:	rb	r1, 0x265d
x0bbd:	lw	r2, [0x132e]
x0bbf:	cw	r2, 0xff
x0bc1:	jes	1
x0bc2:	hlt	040
x0bc3:	lwt	r1, -1
x0bc4:	rw	r1, 0x132e
x0bc6:	lwt	r1, 0
x0bc7:	rb	r1, 0x265c
x0bc9:	lw	r1, [0x132e]
x0bcb:	cw	r1, 0xff
x0bcd:	jes	1
x0bce:	hlt	040
x0bcf:	lwt	r0, 0
x0bd0:	rz	0x132e
x0bd2:	lwt	r1, 0
x0bd3:	cb	r1, 0x265c
x0bd5:	cw	r0, 0x400
x0bd7:	jes	1
x0bd8:	hlt	040
x0bd9:	lwt	r1, -1
x0bda:	cb	r1, 0x265d
x0bdc:	cw	r0, 0x200
x0bde:	jes	1
x0bdf:	hlt	040
x0be0:	lwt	r1, -1
x0be1:	rw	r1, 0x132e
x0be3:	lwt	r1, 0
x0be4:	cb	r1, 0x265c
x0be6:	cw	r0, 0x800
x0be8:	jes	1
x0be9:	hlt	040
x0bea:	lwt	r0, -1
x0beb:	blc	255<<8
x0bec:	ujs	1
x0bed:	hlt	040
x0bee:	lwt	r0, 0
x0bef:	blc	181<<8
x0bf0:	hlt	040
x0bf1:	lwt	r0, -1
x0bf2:	brc	255
x0bf3:	ujs	1
x0bf4:	hlt	040
x0bf5:	lwt	r0, 0
x0bf6:	brc	181
x0bf7:	hlt	040
x0bf8:	lwt	r1, -3
x0bf9:	rw	r1, 0x132e
x0bfb:	ib	0x132e
x0bfd:	lw	r1, [0x132e]
x0bff:	cwt	r1, -2
x0c00:	jes	1
x0c01:	ujs	14
x0c02:	ib	0x132e
x0c04:	lw	r1, [0x132e]
x0c06:	cwt	r1, -1
x0c07:	jes	1
x0c08:	ujs	7	; -> 0x0c10
x0c09:	ib	0x132e
x0c0b:	hlt	040
x0c0c:	lw	r1, [0x132e]
x0c0e:	cwt	r1, 0
x0c0f:	jes	1
x0c10:	hlt	040
x0c11:	lwt	r1, 0
x0c12:	ric	r1
x0c13:	cw	r1, 0xc13
x0c15:	jes	1
x0c16:	hlt	040
x0c17:	lwt	r0, 0
x0c18:	lwt	r1, -1
x0c19:	rpc	r1
x0c1a:	cwt	r1, 0
x0c1b:	jes	1
x0c1c:	hlt	040
x0c1d:	lwt	r0, -1
x0c1e:	lwt	r1, 0
x0c1f:	rpc	r1
x0c20:	cwt	r1, -1
x0c21:	jes	1
x0c22:	hlt	040
x0c23:	lwt	r0, 0
x0c24:	lwt	r1, -1
x0c25:	lpc	r1
x0c26:	cwt	r0, -1
x0c27:	jes	1
x0c28:	hlt	040
x0c29:	lwt	r0, -1
x0c2a:	lwt	r1, 0
x0c2b:	lpc	r1
x0c2c:	cwt	r0, 0
x0c2d:	jes	1
x0c2e:	hlt	040
x0c2f:	lw	r1, [0x2]
x0c31:	cwt	r1, 0
x0c32:	jes	5	; -> 0x0c38
x0c33:	lj	reset
x0c35:	mcl
x0c36:	uj	0x100
x0c38:	rz	0xc86
x0c3a:	fi	0xc86
x0c3c:	lwt	r1, -1
x0c3d:	rw	r1, 0xc86
x0c3f:	ki	0xc86
x0c41:	lw	r1, [0xc86]
x0c43:	cwt	r1, 0
x0c44:	jes	1
x0c45:	hlt	040
x0c46:	lw	r1, 0x7fff
x0c48:	rw	r1, 0xc86
x0c4a:	fi	0xc86
x0c4c:	rz	0xc86
x0c4e:	ki	0xc86
x0c50:	lw	r1, [0xc86]
x0c52:	cw	r1, 0x7fff
x0c54:	jes	1
x0c55:	hlt	040
x0c56:	lw	r1, 0x5555
x0c58:	rw	r1, 0xc86
x0c5a:	fi	0xc86
x0c5c:	rz	0xc86
x0c5e:	ki	0xc86
x0c60:	lw	r1, [0xc86]
x0c62:	cw	r1, 0x5555
x0c64:	jes	1
x0c65:	hlt	040
x0c66:	mcl
x0c67:	lw	r1, 0xc7a
x0c69:	rw	r1, 0x5f
x0c6b:	lw	r1, stack
x0c6d:	rw	r1, stackp
x0c6f:	lw	r1, 0x40
x0c71:	rw	r1, 0xd3c
x0c73:	im	0xd3c
x0c75:	lwt	r1, 1
x0c76:	rw	r1, 0xc86
x0c78:	fi	0xc86
x0c7a:	lw	r1, 0xfd
x0c7c:	rw	r1, 0x5f
x0c7e:	lw	r1, [0x64]
x0c80:	cw	r1, 0x40
x0c82:	jes	1
x0c83:	hlt	040
x0c84:	mcl
x0c85:	ujs	6	; -> 0x0c8c
x0c86:	.word	0
x0c87:	.word	0x0c9d
x0c88:	ib	[r7+r7]
x0c89:	ib	[0xff]
x0c8b:	.word	0x0066
x0c8c:	mcl
x0c8d:	lw	r1, stack
x0c8f:	rw	r1, stackp
x0c91:	lwt	r0, -1
x0c92:	lw	r1, 0xffc0
x0c94:	rw	r1, 0xc86
x0c96:	im	0xc86
x0c98:	lw	r1, 0xc9d
x0c9a:	rw	r1, 0x60
x0c9c:	exl	255
x0c9d:	la	0xc87
x0c9f:	cw	r1, [stack]
x0ca1:	jes	1
x0ca2:	hlt	040
x0ca3:	cw	r2, [0x63]
x0ca5:	jes	1
x0ca6:	hlt	040
x0ca7:	cw	r3, [0x64]
x0ca9:	jes	1
x0caa:	hlt	040
x0cab:	cw	r4, [0x65]
x0cad:	jes	1
x0cae:	hlt	040
x0caf:	cw	r0, 0x400
x0cb1:	jes	1
x0cb2:	hlt	040
x0cb3:	cw	r5, [0x61]
x0cb5:	jes	1
x0cb6:	hlt	040
x0cb7:	lw	r1, 0xcc2
x0cb9:	rw	r1, 0x5f
x0cbb:	lwt	r1, 1
x0cbc:	rw	r1, 0xc86
x0cbe:	fi	0xc86
x0cc0:	mcl
x0cc1:	ujs	5	; -> 0x0cc7
x0cc2:	lw	r1, 0xfd
x0cc4:	rw	r1, 0x5f
x0cc6:	hlt	040
x0cc7:	lw	r1, 0xfd
x0cc9:	rw	r1, 0x5f
x0ccb:	mcl
x0ccc:	ujs	3	; -> 0x0cd0
x0ccd:	.word	0x0cd3
x0cce:	ib	[r7+r7]
x0ccf:	ib	[0xec40]
x0cd1:	sp	0xccd
x0cd3:	cwt	r0, -1
x0cd4:	jes	1
x0cd5:	hlt	040
x0cd6:	lw	r1, 0xce3
x0cd8:	rw	r1, 0x5f
x0cda:	lw	r1, stack
x0cdc:	rw	r1, stackp
x0cde:	lwt	r1, 1
x0cdf:	rw	r1, 0xc86
x0ce1:	fi	0xc86
x0ce3:	lw	r1, 0xfd
x0ce5:	rw	r1, 0x5f
x0ce7:	lw	r1, 0xffc0
x0ce9:	cw	r1, [0x64]
x0ceb:	jes	1
x0cec:	hlt	040
x0ced:	lw	r1, 0x5f
x0cef:	rw	r1, 0x132e
x0cf1:	mcl
x0cf2:	mb	0x132e
x0cf4:	lw	r1, 0xd07
x0cf6:	rw	r1, 0x5f
x0cf8:	lw	r1, stack
x0cfa:	rw	r1, stackp
x0cfc:	lw	r1, 0x40
x0cfe:	rw	r1, 0xd3c
x0d00:	im	0xd3c
x0d02:	lwt	r1, 1
x0d03:	rw	r1, 0xd3d
x0d05:	fi	0xd3d
x0d07:	lw	r1, 0xfd
x0d09:	rw	r1, 0x5f
x0d0b:	lw	r1, [0x64]
x0d0d:	cw	r1, 0x5f
x0d0f:	jes	1
x0d10:	hlt	040
x0d11:	mcl
x0d12:	lw	r1, 0xd2b
x0d14:	rw	r1, stack
x0d16:	lwt	r1, -1
x0d17:	rw	r1, 0x63
x0d19:	rz	0x64
x0d1b:	mcl
x0d1c:	lw	r1, 0x7fff
x0d1e:	rw	r1, 0xd3d
x0d20:	fi	0xd3d
x0d22:	lw	r1, 0xd36
x0d24:	rw	r1, 0x41
x0d26:	lw	r1, 0x66
x0d28:	rw	r1, 0x61
x0d2a:	lip
x0d2b:	cwt	r0, -1
x0d2c:	jes	1
x0d2d:	hlt	040
x0d2e:	lw	r1, stack
x0d30:	cw	r1, [stackp]
x0d32:	jes	1
x0d33:	hlt	040
x0d34:	mcl
x0d35:	ujs	1
x0d36:	hlt	040

; test rozkazów współpracujących z NB niezerowym

x0d37:	lw	r1, 0xc1
x0d39:	rw	r1, 0x41
x0d3b:	ujs	2
x0d3c:	.word	0
x0d3d:	.word	0
x0d3e:	lj	reset
x0d40:	lw	r1, 0x100f
x0d42:	lw	r2, 0x61
x0d44:	ou	r1, r2
x0d45:	.word	0x0d49, 0x0d4a, 0x0d4c, 0x0d4b
x0d49:	hlt	040
x0d4a:	hlt	040
x0d4b:	hlt	040
x0d4c:	mb	0x1341
x0d4e:	lwt	r3, 0
x0d4f:	lw	r4, 0x1001
x0d51:	lw	r5, 0xf060
x0d53:	pw	r3, r4
x0d54:	awt	r4, 1
x0d55:	irb	r5, -3
x0d56:	lj	0x130f
x0d58:	tw	r2, 0x132b
x0d5a:	pw	r2, 0x132e
x0d5c:	lwt	r1, 0
x0d5d:	tw	r1, 0x132e
x0d5f:	cw	r1, r2
x0d60:	jes	1
x0d61:	hlt	040
x0d62:	tw	r2, 0x1339
x0d64:	pw	r2, 0x132e
x0d66:	tw	r1, 0x132e
x0d68:	cw	r1, r2
x0d69:	jes	1
x0d6a:	hlt	040
x0d6b:	lwt	r1, 0
x0d6c:	pw	r1, 0x132e
x0d6e:	is	r1, 0x132e
x0d70:	hlt	040
x0d71:	tw	r1, 0x132e
x0d73:	cwt	r1, 0
x0d74:	jes	1
x0d75:	hlt	040
x0d76:	lwt	r1, 0
x0d77:	pw	r1, 0x132e
x0d79:	lwt	r1, -1
x0d7a:	is	r1, 0x132e
x0d7c:	ujs	1
x0d7d:	hlt	040
x0d7e:	tw	r1, 0x132e
x0d80:	cwt	r1, -1
x0d81:	jes	1
x0d82:	hlt	040
x0d83:	lwt	r1, -1
x0d84:	pw	r1, 0x132e
x0d86:	lwt	r1, 0
x0d87:	is	r1, 0x132e
x0d89:	hlt	040
x0d8a:	tw	r1, 0x132e
x0d8c:	cwt	r1, -1
x0d8d:	jes	1
x0d8e:	hlt	040
x0d8f:	lwt	r1, -1
x0d90:	pw	r1, 0x132e
x0d92:	is	r1, 0x132e
x0d94:	hlt	040
x0d95:	tw	r1, 0x132e
x0d97:	cwt	r1, -1
x0d98:	jes	1
x0d99:	hlt	040
x0d9a:	tw	r1, 0x1338
x0d9c:	pw	r1, 0x132e
x0d9e:	is	r1, 0x132e
x0da0:	hlt	040
x0da1:	tw	r1, 0x132e
x0da3:	tw	r2, 0x1338
x0da5:	cw	r1, r2
x0da6:	jes	1
x0da7:	hlt	040
x0da8:	tw	r1, 0x1339
x0daa:	pw	r1, 0x132e
x0dac:	is	r1, 0x132e
x0dae:	hlt	040
x0daf:	tw	r1, 0x132e
x0db1:	tw	r2, 0x1339
x0db3:	cw	r1, r2
x0db4:	jes	1
x0db5:	hlt	040
x0db6:	tw	r1, 0x1338
x0db8:	tw	r2, 0x1339
x0dba:	pw	r2, 0x132e
x0dbc:	is	r1, 0x132e
x0dbe:	ujs	1
x0dbf:	hlt	040
x0dc0:	tw	r1, 0x132e
x0dc2:	cwt	r1, -1
x0dc3:	jes	1
x0dc4:	hlt	040
x0dc5:	tw	r1, 0x1338
x0dc7:	tw	r2, 0x1339
x0dc9:	pw	r1, 0x132e
x0dcb:	is	r2, 0x132e
x0dcd:	ujs	1
x0dce:	hlt	040
x0dcf:	tw	r1, 0x132e
x0dd1:	cwt	r1, -1
x0dd2:	jes	1
x0dd3:	hlt	040
x0dd4:	lwt	r1, 0
x0dd5:	pw	r1, 0x132e
x0dd7:	bm	r1, 0x132e
x0dd9:	hlt	040
x0dda:	lwt	r1, -1
x0ddb:	pw	r1, 0x132e
x0ddd:	bm	r1, 0x132e
x0ddf:	hlt	040
x0de0:	lwt	r1, -1
x0de1:	lwt	r2, 0
x0de2:	pw	r2, 0x132e
x0de4:	bm	r1, 0x132e
x0de6:	ujs	1
x0de7:	hlt	040
x0de8:	lwt	r1, 0
x0de9:	lwt	r2, -1
x0dea:	pw	r2, 0x132e
x0dec:	bm	r1, 0x132e
x0dee:	hlt	040
x0def:	tw	r1, 0x1338
x0df1:	pw	r1, 0x132e
x0df3:	bm	r1, 0x132e
x0df5:	hlt	040
x0df6:	tw	r1, 0x1339
x0df8:	pw	r1, 0x132e
x0dfa:	bm	r1, 0x132e
x0dfc:	hlt	040
x0dfd:	tw	r1, 0x1338
x0dff:	tw	r2, 0x1339
x0e01:	pw	r2, 0x132e
x0e03:	bm	r1, 0x132e
x0e05:	ujs	1
x0e06:	hlt	040
x0e07:	tw	r1, 0x1339
x0e09:	tw	r2, 0x1338
x0e0b:	pw	r2, 0x132e
x0e0d:	bm	r1, 0x132e
x0e0f:	ujs	1
x0e10:	hlt	040
x0e11:	lwt	r3, 0
x0e12:	mb	0x1328
x0e14:	lpc	r3
x0e15:	mb	0x1341
x0e17:	lwt	r1, -1
x0e18:	pw	r1, 0x132e
x0e1a:	lwt	r1, 0
x0e1b:	om	r1, 0x132e
x0e1d:	cwt	r0, 0
x0e1e:	jes	1
x0e1f:	hlt	040
x0e20:	tw	r1, 0x132e
x0e22:	cwt	r1, -1
x0e23:	jes	1
x0e24:	hlt	040
x0e25:	lwt	r3, 0
x0e26:	mb	0x1328
x0e28:	lpc	r3
x0e29:	mb	0x1341
x0e2b:	pw	r0, 0x132e
x0e2d:	lwt	r1, -1
x0e2e:	om	r1, 0x132e
x0e30:	cwt	r0, 0
x0e31:	jes	1
x0e32:	hlt	040
x0e33:	tw	r1, 0x132e
x0e35:	cwt	r1, -1
x0e36:	jes	1
x0e37:	hlt	040
x0e38:	lwt	r3, 0
x0e39:	mb	0x1328
x0e3b:	lpc	r3
x0e3c:	mb	0x1341
x0e3e:	pw	r0, 0x132e
x0e40:	lwt	r1, 0
x0e41:	om	r1, 0x132e
x0e43:	cw	r0, 0x8000
x0e45:	jes	1
x0e46:	hlt	040
x0e47:	tw	r1, 0x132e
x0e49:	cwt	r1, 0
x0e4a:	jes	1
x0e4b:	hlt	040
x0e4c:	lwt	r3, 0
x0e4d:	mb	0x1328
x0e4f:	lpc	r3
x0e50:	mb	0x1341
x0e52:	lwt	r1, -1
x0e53:	pw	r1, 0x132e
x0e55:	om	r1, 0x132e
x0e57:	cwt	r0, 0
x0e58:	jes	1
x0e59:	hlt	040
x0e5a:	tw	r1, 0x132e
x0e5c:	cwt	r1, -1
x0e5d:	jes	1
x0e5e:	hlt	040
x0e5f:	lwt	r3, 0
x0e60:	mb	0x1328
x0e62:	lpc	r3
x0e63:	mb	0x1341
x0e65:	tw	r1, 0x1338
x0e67:	pw	r1, 0x132e
x0e69:	om	r1, 0x132e
x0e6b:	cwt	r0, 0
x0e6c:	jes	1
x0e6d:	hlt	040
x0e6e:	tw	r1, 0x132e
x0e70:	tw	r2, 0x1338
x0e72:	cw	r1, r2
x0e73:	jes	1
x0e74:	hlt	040
x0e75:	lwt	r3, 0
x0e76:	mb	0x1328
x0e78:	lpc	r3
x0e79:	mb	0x1341
x0e7b:	tw	r1, 0x1339
x0e7d:	pw	r1, 0x132e
x0e7f:	om	r1, 0x132e
x0e81:	cwt	r0, 0
x0e82:	jes	1
x0e83:	hlt	040
x0e84:	tw	r1, 0x132e
x0e86:	tw	r2, 0x1339
x0e88:	cw	r1, r2
x0e89:	jes	1
x0e8a:	hlt	040
x0e8b:	lwt	r3, 0
x0e8c:	mb	0x1328
x0e8e:	lpc	r3
x0e8f:	mb	0x1341
x0e91:	tw	r1, 0x1338
x0e93:	pw	r1, 0x132e
x0e95:	tw	r1, 0x1339
x0e97:	om	r1, 0x132e
x0e99:	cwt	r0, 0
x0e9a:	jes	1
x0e9b:	hlt	040
x0e9c:	tw	r1, 0x132e
x0e9e:	cwt	r1, -1
x0e9f:	jes	1
x0ea0:	hlt	040
x0ea1:	lwt	r3, 0
x0ea2:	mb	0x1328
x0ea4:	lpc	r3
x0ea5:	mb	0x1341
x0ea7:	tw	r1, 0x1339
x0ea9:	pw	r1, 0x132e
x0eab:	tw	r1, 0x1338
x0ead:	om	r1, 0x132e
x0eaf:	cwt	r0, 0
x0eb0:	jes	1
x0eb1:	hlt	040
x0eb2:	tw	r1, 0x132e
x0eb4:	cwt	r1, -1
x0eb5:	jes	1
x0eb6:	hlt	040
x0eb7:	lwt	r3, 0
x0eb8:	mb	0x1328
x0eba:	lpc	r3
x0ebb:	mb	0x1341
x0ebd:	lwt	r1, -1
x0ebe:	pw	r1, 0x132e
x0ec0:	lwt	r1, 0
x0ec1:	nm	r1, 0x132e
x0ec3:	cw	r0, 0x8000
x0ec5:	jes	1
x0ec6:	hlt	040
x0ec7:	tw	r1, 0x132e
x0ec9:	cwt	r1, 0
x0eca:	jes	1
x0ecb:	hlt	040
x0ecc:	lwt	r3, 0
x0ecd:	mb	0x1328
x0ecf:	lpc	r3
x0ed0:	mb	0x1341
x0ed2:	lwt	r1, -1
x0ed3:	pw	r0, 0x132e
x0ed5:	nm	r1, 0x132e
x0ed7:	cw	r0, 0x8000
x0ed9:	jes	1
x0eda:	hlt	040
x0edb:	tw	r1, 0x132e
x0edd:	cwt	r1, 0
x0ede:	jes	1
x0edf:	hlt	040
x0ee0:	lwt	r3, 0
x0ee1:	mb	0x1328
x0ee3:	lpc	r3
x0ee4:	mb	0x1341
x0ee6:	pw	r0, 0x132e
x0ee8:	lwt	r1, 0
x0ee9:	nm	r1, 0x132e
x0eeb:	cw	r0, 0x8000
x0eed:	jes	1
x0eee:	hlt	040
x0eef:	tw	r1, 0x132e
x0ef1:	cwt	r1, 0
x0ef2:	jes	1
x0ef3:	hlt	040
x0ef4:	lwt	r3, 0
x0ef5:	mb	0x1328
x0ef7:	lpc	r3
x0ef8:	mb	0x1341
x0efa:	lwt	r1, -1
x0efb:	pw	r1, 0x132e
x0efd:	nm	r1, 0x132e
x0eff:	cwt	r0, 0
x0f00:	jes	1
x0f01:	hlt	040
x0f02:	tw	r1, 0x132e
x0f04:	cwt	r1, -1
x0f05:	jes	1
x0f06:	hlt	040
x0f07:	lwt	r3, 0
x0f08:	mb	0x1328
x0f0a:	lpc	r3
x0f0b:	mb	0x1341
x0f0d:	tw	r1, 0x1338
x0f0f:	pw	r1, 0x132e
x0f11:	nm	r1, 0x132e
x0f13:	cwt	r0, 0
x0f14:	jes	1
x0f15:	hlt	040
x0f16:	tw	r1, 0x132e
x0f18:	tw	r2, 0x1338
x0f1a:	cw	r1, r2
x0f1b:	jes	1
x0f1c:	hlt	040
x0f1d:	lwt	r3, 0
x0f1e:	mb	0x1328
x0f20:	lpc	r3
x0f21:	mb	0x1341
x0f23:	tw	r1, 0x1339
x0f25:	pw	r1, 0x132e
x0f27:	nm	r1, 0x132e
x0f29:	cwt	r0, 0
x0f2a:	jes	1
x0f2b:	hlt	040
x0f2c:	tw	r1, 0x132e
x0f2e:	tw	r2, 0x1339
x0f30:	cw	r1, r2
x0f31:	jes	1
x0f32:	hlt	040
x0f33:	lwt	r3, 0
x0f34:	mb	0x1328
x0f36:	lpc	r3
x0f37:	mb	0x1341
x0f39:	tw	r1, 0x1339
x0f3b:	pw	r1, 0x132e
x0f3d:	tw	r1, 0x1338
x0f3f:	nm	r1, 0x132e
x0f41:	cw	r0, 0x8000
x0f43:	jes	1
x0f44:	hlt	040
x0f45:	tw	r1, 0x132e
x0f47:	cwt	r1, 0
x0f48:	jes	1
x0f49:	hlt	040
x0f4a:	lwt	r3, 0
x0f4b:	mb	0x1328
x0f4d:	lpc	r3
x0f4e:	mb	0x1341
x0f50:	tw	r1, 0x1338
x0f52:	pw	r1, 0x132e
x0f54:	tw	r1, 0x1339
x0f56:	nm	r1, 0x132e
x0f58:	cw	r0, 0x8000
x0f5a:	jes	1
x0f5b:	hlt	040
x0f5c:	tw	r1, 0x132e
x0f5e:	cwt	r1, 0
x0f5f:	jes	1
x0f60:	hlt	040
x0f61:	lwt	r3, 0
x0f62:	mb	0x1328
x0f64:	lpc	r3
x0f65:	mb	0x1341
x0f67:	lwt	r1, -1
x0f68:	pw	r1, 0x132e
x0f6a:	lwt	r1, 0
x0f6b:	em	r1, 0x132e
x0f6d:	cwt	r0, 0
x0f6e:	jes	1
x0f6f:	hlt	040
x0f70:	tw	r1, 0x132e
x0f72:	cwt	r1, -1
x0f73:	jes	1
x0f74:	hlt	040
x0f75:	lwt	r3, 0
x0f76:	mb	0x1328
x0f78:	lpc	r3
x0f79:	mb	0x1341
x0f7b:	lwt	r1, 0
x0f7c:	pw	r1, 0x132e
x0f7e:	lwt	r1, -1
x0f7f:	em	r1, 0x132e
x0f81:	cw	r0, 0x8000
x0f83:	jes	1
x0f84:	hlt	040
x0f85:	tw	r1, 0x132e
x0f87:	cwt	r1, 0
x0f88:	jes	1
x0f89:	hlt	040
x0f8a:	lwt	r3, 0
x0f8b:	mb	0x1328
x0f8d:	lpc	r3
x0f8e:	mb	0x1341
x0f90:	lwt	r1, 0
x0f91:	pw	r1, 0x132e
x0f93:	em	r1, 0x132e
x0f95:	cw	r0, 0x8000
x0f97:	jes	1
x0f98:	hlt	040
x0f99:	tw	r1, 0x132e
x0f9b:	cwt	r1, 0
x0f9c:	jes	1
x0f9d:	hlt	040
x0f9e:	lwt	r3, 0
x0f9f:	mb	0x1328
x0fa1:	lpc	r3
x0fa2:	mb	0x1341
x0fa4:	lwt	r1, -1
x0fa5:	pw	r1, 0x132e
x0fa7:	em	r1, 0x132e
x0fa9:	cw	r0, 0x8000
x0fab:	jes	1
x0fac:	hlt	040
x0fad:	tw	r1, 0x132e
x0faf:	cwt	r1, 0
x0fb0:	jes	1
x0fb1:	hlt	040
x0fb2:	lwt	r3, 0
x0fb3:	mb	0x1328
x0fb5:	lpc	r3
x0fb6:	mb	0x1341
x0fb8:	tw	r1, 0x1338
x0fba:	pw	r1, 0x132e
x0fbc:	em	r1, 0x132e
x0fbe:	cw	r0, 0x8000
x0fc0:	jes	1
x0fc1:	hlt	040
x0fc2:	tw	r1, 0x132e
x0fc4:	cwt	r1, 0
x0fc5:	jes	1
x0fc6:	hlt	040
x0fc7:	lwt	r3, 0
x0fc8:	mb	0x1328
x0fca:	lpc	r3
x0fcb:	mb	0x1341
x0fcd:	tw	r1, 0x1339
x0fcf:	pw	r1, 0x132e
x0fd1:	em	r1, 0x132e
x0fd3:	cw	r0, 0x8000
x0fd5:	jes	1
x0fd6:	hlt	040
x0fd7:	tw	r1, 0x132e
x0fd9:	cwt	r1, 0
x0fda:	jes	1
x0fdb:	hlt	040
x0fdc:	lwt	r3, 0
x0fdd:	mb	0x1328
x0fdf:	lpc	r3
x0fe0:	mb	0x1341
x0fe2:	tw	r1, 0x1339
x0fe4:	pw	r1, 0x132e
x0fe6:	tw	r1, 0x1338
x0fe8:	em	r1, 0x132e
x0fea:	cwt	r0, 0
x0feb:	jes	1
x0fec:	hlt	040
x0fed:	tw	r1, 0x132e
x0fef:	tw	r2, 0x1339
x0ff1:	cw	r1, r2
x0ff2:	jes	1
x0ff3:	hlt	040
x0ff4:	lwt	r3, 0
x0ff5:	mb	0x1328
x0ff7:	lpc	r3
x0ff8:	mb	0x1341
x0ffa:	tw	r1, 0x1338
x0ffc:	pw	r1, 0x132e
x0ffe:	tw	r1, 0x1339
x1000:	em	r1, 0x132e
x1002:	cwt	r0, 0
x1003:	jes	1
x1004:	hlt	040
x1005:	tw	r1, 0x132e
x1007:	tw	r2, 0x1338
x1009:	cw	r1, r2
x100a:	jes	1
x100b:	hlt	040
x100c:	lwt	r3, 0
x100d:	mb	0x1328
x100f:	lpc	r3
x1010:	mb	0x1341
x1012:	lwt	r1, -1
x1013:	pw	r1, 0x132e
x1015:	lwt	r1, 0
x1016:	xm	r1, 0x132e
x1018:	cwt	r0, 0
x1019:	jes	1
x101a:	hlt	040
x101b:	tw	r1, 0x132e
x101d:	cwt	r1, -1
x101e:	jes	1
x101f:	hlt	040
x1020:	lwt	r3, 0
x1021:	mb	0x1328
x1023:	lpc	r3
x1024:	mb	0x1341
x1026:	pw	r0, 0x132e
x1028:	lwt	r1, -1
x1029:	xm	r1, 0x132e
x102b:	cwt	r0, 0
x102c:	jes	1
x102d:	hlt	040
x102e:	tw	r1, 0x132e
x1030:	cwt	r1, -1
x1031:	jes	1
x1032:	hlt	040
x1033:	lwt	r3, 0
x1034:	mb	0x1328
x1036:	lpc	r3
x1037:	mb	0x1341
x1039:	pw	r0, 0x132e
x103b:	lwt	r1, 0
x103c:	xm	r1, 0x132e
x103e:	cw	r0, 0x8000
x1040:	jes	1
x1041:	hlt	040
x1042:	tw	r1, 0x132e
x1044:	cwt	r1, 0
x1045:	jes	1
x1046:	hlt	040
x1047:	lwt	r3, 0
x1048:	mb	0x1328
x104a:	lpc	r3
x104b:	mb	0x1341
x104d:	lwt	r1, -1
x104e:	pw	r1, 0x132e
x1050:	xm	r1, 0x132e
x1052:	cw	r0, 0x8000
x1054:	jes	1
x1055:	hlt	040
x1056:	tw	r1, 0x132e
x1058:	cwt	r1, 0
x1059:	jes	1
x105a:	hlt	040
x105b:	lwt	r3, 0
x105c:	mb	0x1328
x105e:	lpc	r3
x105f:	mb	0x1341
x1061:	tw	r1, 0x1338
x1063:	pw	r1, 0x132e
x1065:	xm	r1, 0x132e
x1067:	cw	r0, 0x8000
x1069:	jes	1
x106a:	hlt	040
x106b:	tw	r1, 0x132e
x106d:	cwt	r1, 0
x106e:	jes	1
x106f:	hlt	040
x1070:	lwt	r3, 0
x1071:	mb	0x1328
x1073:	lpc	r3
x1074:	mb	0x1341
x1076:	tw	r1, 0x1339
x1078:	pw	r1, 0x132e
x107a:	xm	r1, 0x132e
x107c:	cw	r0, 0x8000
x107e:	jes	1
x107f:	hlt	040
x1080:	tw	r1, 0x132e
x1082:	cwt	r1, 0
x1083:	jes	1
x1084:	hlt	040
x1085:	lwt	r3, 0
x1086:	mb	0x1328
x1088:	lpc	r3
x1089:	mb	0x1341
x108b:	tw	r1, 0x1339
x108d:	pw	r1, 0x132e
x108f:	tw	r1, 0x1338
x1091:	xm	r1, 0x132e
x1093:	cwt	r0, 0
x1094:	jes	1
x1095:	hlt	040
x1096:	tw	r1, 0x132e
x1098:	cwt	r1, -1
x1099:	jes	1
x109a:	hlt	040
x109b:	lwt	r3, 0
x109c:	mb	0x1328
x109e:	lpc	r3
x109f:	mb	0x1341
x10a1:	tw	r1, 0x1338
x10a3:	pw	r1, 0x132e
x10a5:	tw	r1, 0x1339
x10a7:	xm	r1, 0x132e
x10a9:	cwt	r0, 0
x10aa:	jes	1
x10ab:	hlt	040
x10ac:	tw	r1, 0x132e
x10ae:	cwt	r1, -1
x10af:	jes	1
x10b0:	hlt	040
x10b1:	lwt	r1, -1
x10b2:	lwt	r2, -1
x10b3:	lwt	r3, -1
x10b4:	lwt	r4, -1
x10b5:	lwt	r5, -1
x10b6:	lwt	r6, -1
x10b7:	lwt	r7, -1
x10b8:	td	0x1338
x10ba:	cw	r1, 0x5555
x10bc:	jes	1
x10bd:	hlt	040
x10be:	cw	r2, 0xaaaa
x10c0:	jes	1
x10c1:	hlt	040
x10c2:	cwt	r3, -1
x10c3:	jes	1
x10c4:	ujs	11
x10c5:	cwt	r4, -1
x10c6:	jes	1
x10c7:	ujs	8	; -> 0x10d0
x10c8:	cwt	r5, -1
x10c9:	jes	1
x10ca:	ujs	5	; -> 0x10d0
x10cb:	cwt	r6, -1
x10cc:	jes	1
x10cd:	ujs	2	; -> 0x10d0
x10ce:	cwt	r7, -1
x10cf:	jes	1
x10d0:	hlt	040
x10d1:	lwt	r1, 0
x10d2:	lb	r1, 0x2670
x10d4:	cw	r1, 0x55
x10d6:	jes	1
x10d7:	hlt	040
x10d8:	lwt	r1, -1
x10d9:	lwt	r2, 0
x10da:	pw	r2, 0x132e
x10dc:	lb	r1, 0x265d
x10de:	cw	r1, 0xff00
x10e0:	jes	1
x10e1:	hlt	040
x10e2:	lwt	r1, -1
x10e3:	lwt	r2, 0
x10e4:	pw	r2, 0x132e
x10e6:	rb	r1, 0x265d
x10e8:	tw	r2, 0x132e
x10ea:	cw	r2, 0xff
x10ec:	jes	1
x10ed:	hlt	040
x10ee:	lwt	r1, -1
x10ef:	pw	r1, 0x132e
x10f1:	lwt	r1, 0
x10f2:	rb	r1, 0x265c
x10f4:	tw	r1, 0x132e
x10f6:	cw	r1, 0xff
x10f8:	jes	1
x10f9:	hlt	040
x10fa:	lwt	r3, 0
x10fb:	mb	0x1328
x10fd:	lpc	r3
x10fe:	mb	0x1341
x1100:	pw	r0, 0x132e
x1102:	lwt	r1, 0
x1103:	cb	r1, 0x265c
x1105:	cw	r0, 0x400
x1107:	jes	1
x1108:	hlt	040
x1109:	lwt	r1, -1
x110a:	cb	r1, 0x265d
x110c:	cw	r0, 0x200
x110e:	jes	1
x110f:	hlt	040
x1110:	lwt	r1, -1
x1111:	pw	r1, 0x132e
x1113:	lwt	r1, 0
x1114:	cb	r1, 0x265c
x1116:	cw	r0, 0x800
x1118:	jes	1
x1119:	hlt	040
x111a:	lw	r1, 0x1180
x111c:	rw	r1, 0x60
x111e:	lj	0x131c
x1120:	mb	0x1328
x1122:	sp	0x1343
x1124:	lwt	r1, 0
x1125:	lw	r1, 0x1339
x1127:	cw	r1, 0x1339
x1129:	jes	1
x112a:	hlt	040
x112b:	lwt	r1, 0
x112c:	lw	r1, [pat10]
x112e:	cw	r1, 0xaaaa
x1130:	jes	1
x1131:	hlt	040
x1132:	lwt	r1, 0
x1133:	lw	r1, [pat01]
x1135:	cw	r1, 0x5555
x1137:	jes	1
x1138:	hlt	040
x1139:	lwt	r1, 0
x113a:	lw	r1, [0x133a]
x113c:	cwt	r1, 1
x113d:	jes	1
x113e:	hlt	040
x113f:	lwt	r1, 0
x1140:	rw	r1, 0x132e
x1142:	cw	r1, [0x132e]
x1144:	jes	1
x1145:	hlt	040
x1146:	lwt	r1, -1
x1147:	rw	r1, 0x132e
x1149:	cw	r1, [0x132e]
x114b:	jes	1
x114c:	hlt	040
x114d:	lw	r1, [pat10]
x114f:	rw	r1, 0x132e
x1151:	cw	r1, [0x132e]
x1153:	jes	1
x1154:	hlt	040
x1155:	lw	r1, [pat01]
x1157:	rw	r1, 0x132e
x1159:	cw	r1, [0x132e]
x115b:	jes	1
x115c:	hlt	040
x115d:	lw	r1, 0x132e
x115f:	rw	r1, 0x132f
x1161:	lwt	r2, 0
x1162:	rw	r2, [0x132f]
x1164:	cw	r2, [0x132e]
x1166:	jes	1
x1167:	hlt	040
x1168:	lwt	r2, -1
x1169:	rw	r2, [0x132f]
x116b:	cw	r2, [0x132e]
x116d:	jes	1
x116e:	hlt	040
x116f:	lw	r2, [pat01]
x1171:	rw	r2, [0x132f]
x1173:	cw	r2, [0x132e]
x1175:	jes	1
x1176:	hlt	040
x1177:	lw	r2, [pat10]
x1179:	rw	r2, [0x132f]
x117b:	cw	r2, [0x132e]
x117d:	jes	1
x117e:	hlt	040
x117f:	exl	0
x1180:	ujs	1
x1181:	.word	0
x1182:	lw	r1, stack
x1184:	rw	r1, stackp
x1186:	mcl
x1187:	lwt	r1, 0
x1188:	lw	r2, [0x11ab+r1]
x118a:	rj	r7, 0x119e
x118c:	awt	r1, 1
x118d:	cwt	r1, 19
x118e:	jes	1
x118f:	ujs	-8 ; -> 0x1188
x1190:	lwt	r1, 0
x1191:	lw	r3, [0x11ca+r1]
x1193:	lw	r2, [0x11be+r1]
x1195:	rj	r7, 0x119e
x1197:	drb	r3, 4
x1198:	awt	r1, 1
x1199:	cwt	r1, 12
x119a:	jes	59 ; -> 0x11d6
x119b:	ujs	-11; -> 0x1191
x119c:	awt	r2, 1
x119d:	ujs	-9 ; -> 0x1195
x119e:	rws	r2, 0
x119f:	.word	0
x11a0:	ki	0x1181
x11a2:	lws	r4, -34
x11a3:	cw	r4, 0x200
x11a5:	jes	1
x11a6:	hlt	040
x11a7:	mcl
x11a8:	uj	r7
x11a9:	.word	0x03ff
x11aa:	.word	0
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
x11ca:	.word	0x0006
x11cb:	.word	0x0006
x11cc:	.word	0x0008
x11cd:	.word	0x0008
x11ce:	.word	0x0008
x11cf:	.word	0x0008
x11d0:	.word	0x0008
x11d1:	.word	0x0008
x11d2:	.word	0x0008
x11d3:	.word	0x0008
x11d4:	.word	0x0008
x11d5:	.word	0x0008

; test rozkazów w bloku użytkowym (Q=1, NB=15)

	lw	r1, illegalx
	rw	r1, IV_ILLEGAL
	lw	r1, exlx
	rw	r1, exlp
	lf	0x1213
	rf	stack
	lw	r1, 0x66
	rw	r1, stackp
	lwt	r1, 1
	lw	r2, 0x41
	lwt	r3, 0
	ou	r1, r2
	.word	nb1e1, nb1e2, nb1ok, nb1e3

x11ef:	mb	blok1
x11f1:	ld	0x1216
x11f3:	pd	0x101
x11f5:	lwt	r2, 0
x11f6:	lw	r1, [0x121f+r2]
x11f8:	pw	r1, 0x100
x11fa:	lip

illegalx:
x11fb:	awt	r2, 1
x11fc:	cwt	r2, 16
x11fd:	jes	5	; -> 0x1203
x11fe:	lw	r1, 0x100
x1200:	rw	r1, 0x62
x1202:	ujs	-13; -> 0x11f6
x1203:	lw	r1, 0xcb
x1205:	rw	r1, 0x46
x1207:	lw	r1, 0x62
x1209:	rw	r1, 0x61
x120b:	uj	0x122f

exlx:
x120d:	ki	0x1181
x120f:	lw	r4, [0x1181]
x1211:	hlt	040
x1212:	ujs	illegalx

x1213:	.word	0x0100
x1214:	.word	0x0000
x1215:	.word	0x0821
x1216:	exl	0	; .word
x1217:	ujs	-3 ; -> 0x1215
blok1:	.word	1
nb1e1:	awt	r3, 1
nb1e2:	awt	r3, 1
nb1e3:	awt	r3, 1
x121c:	hlt	040
x121d:	ujs	-52; -> 0x11ea
nb1ok:	ujs	-48; -> 0x11ef
x121f:	hlt	040
x1220:	mcl
x1221:	cit
x1222:	sil
x1223:	siu
x1224:	sit
x1225:	giu
x1226:	gil
x1227:	lip
x1228:	mb	0xfc40
x122a:	ki	0xfcc0
x122c:	sp	0x7400
x122e:	in	r0, 0xe014
x1230:	df	[r4+r7]
x1231:	df	[r5+r7]
x1232:	.word	0x0002
x1233:	.word	0x0003
x1234:	df	[r6+r7]
x1235:	df	[r7+r7]
x1236:	.word	0x0002
x1237:	.word	0x0003
x1238:	df	[r5+r7]
x1239:	df	[r5+r7]
x123a:	.word	0x0003
x123b:	.word	0x0003
x123c:	df	[r4+r7]
x123d:	df	[r4+r7]
x123e:	.word	0x0000
x123f:	.word	0x0000
x1240:	df	[r7+r7]
x1241:	df	[r7+r7]
x1242:	.word	0x0003
x1243:	.word	0x0003
x1244:	mcl
x1245:	lwt	r4, -4
x1246:	lw	r1, [0x1234+r4]
x1248:	lw	r3, [0x1238+r4]
x124a:	rws	r1, 41
x124b:	fi	0x1274
x124d:	siu
x124e:	ki	0x1274
x1250:	lws	r2, 35
x1251:	cw	r2, r3
x1252:	jes	1
x1253:	hlt	040
x1254:	irb	r4, -15
x1255:	lwt	r4, -4
x1256:	lw	r1, [0x1234+r4]
x1258:	lw	r3, [0x123c+r4]
x125a:	rws	r1, 25
x125b:	fi	0x1274
x125d:	sil
x125e:	ki	0x1274
x1260:	lws	r2, 19
x1261:	cw	r2, r3
x1262:	jes	1
x1263:	hlt	040
x1264:	irb	r4, -15
x1265:	lwt	r4, -4
x1266:	lw	r1, [0x1234+r4]
x1268:	lw	r3, [0x1240+r4]
x126a:	rws	r1, 9
x126b:	fi	0x1274
x126d:	cit
x126e:	ki	0x1274
x1270:	lws	r2, 3
x1271:	cw	r2, r3
x1272:	jes	2	; -> 0x1275
x1273:	hlt	040
x1274:	.word	0
x1275:	irb	r4, -16
x1276:	lwt	r4, -4
x1277:	lw	r1, [0x1234+r4]
x1279:	lw	r3, [0x1244+r4]
x127b:	rws	r1, -8
x127c:	fi	0x1274
x127e:	sit
x127f:	ki	0x1274
x1281:	lws	r2, -14
x1282:	cw	r2, r3
x1283:	jes	1
x1284:	hlt	040
x1285:	irb	r4, -15
x1286:	nop
x1287:	rky	r1
x1288:	bn	r1, 0x100
x128a:	ujs	28 ; -> 0x12a7
x128b:	uj	0x135e
x128d:	.word	0
x128e:	.word	0
x128f:	ib	[0x12cc]
x1291:	ib	[r7+r7]
x1292:	ib	[0xff]
x1294:	.word	0x0066
x1295:	.word	0xe7ff
x1296:	ad	r4
x1297:	sd	r4
x1298:	mw	r4
x1299:	dw	r4
x129a:	af	r4
x129b:	sf	r4
x129c:	mf	r4
x129d:	df	r4
x129e:	.word	0x12d7
x129f:	.word	0x12d6
x12a0:	.word	0x12d5
x12a1:	.word	0x12d4
x12a2:	.word	0x12d3
x12a3:	.word	0x12d2
x12a4:	.word	0x12d1
x12a5:	.word	0x12d0
x12a6:	.word	0x12cf
x12a7:	mcl
x12a8:	la	0x129e
x12aa:	ra	0x67
x12ac:	ld	0x12a5
x12ae:	rd	0x6e
x12b0:	lwt	r6, 9
x12b1:	rws	r6, -36
x12b2:	lj	0x12b8
x12b4:	lws	r6, -39
x12b5:	drb	r6, -5
x12b6:	uj	0x135e

x12b8:	.res	1
x12b9:	lw	r2, stack
x12bb:	rw	r2, stackp
x12bd:	lwt	r3, -4
x12be:	ri	r2, 0
x12c0:	irb	r3, -3
x12c1:	im	0x128f
x12c3:	lwt	r0, -1
x12c4:	lw	r1, [0x1294+r6]
x12c6:	rw	r1, 0x12cb
x12c8:	lw	r4, 0xff
x12ca:	lwt	r7, 0
x12cb:	.word	0
x12cc:	hlt	040
x12cd:	uj	[0x12b8]

x12cf:	awt	r7, 1
x12d0:	awt	r7, 1
x12d1:	awt	r7, 1
x12d2:	awt	r7, 1
x12d3:	awt	r7, 1
x12d4:	awt	r7, 1
x12d5:	awt	r7, 1
x12d6:	awt	r7, 1
x12d7:	awt	r7, 1
x12d8:	lws	r4, -14
x12d9:	rpc	r2
x12da:	cwt	r2, 0
x12db:	jes	2	; -> 0x12de
x12dc:	hlt	040
x12dd:	lwt	r0, 0
x12de:	cw	r7, r6
x12df:	or	r0, r6
x12e0:	jes	1
x12e1:	hlt	040
x12e2:	lf	0x62
x12e4:	ll	0x1290
x12e6:	cw	r1, r5
x12e7:	jes	1
x12e8:	hlt	040
x12e9:	cw	r2, r6
x12ea:	jes	1
x12eb:	hlt	040
x12ec:	cw	r3, r7
x12ed:	jes	1
x12ee:	hlt	040
x12ef:	lw	r3, [0x65]
x12f1:	ld	0x1293
x12f3:	cw	r3, r1
x12f4:	jes	1
x12f5:	hlt	040
x12f6:	lw	r1, [0x61]
x12f8:	cw	r1, r2
x12f9:	jes	1
x12fa:	hlt	040
x12fb:	lw	r1, 0x1306
x12fd:	rw	r1, 0x5f
x12ff:	lwt	r1, 1
x1300:	rw	r1, 0x128d
x1302:	fi	0x128d
x1304:	mcl
x1305:	ujs	3	; -> 0x1309
x1306:	lw	r0, [0x128e]
x1308:	hlt	040
x1309:	lw	r1, 0xfd
x130b:	rw	r1, 0x5f
x130d:	uj	[0x12b8]

x130f:	.res	1
x1310:	lw	r1, 0x1346
x1312:	lwt	r2, -24
x1313:	lw	r3, 0x1329
x1315:	lw	r4, [r1]
x1316:	pw	r4, r3
x1317:	awt	r3, 1
x1318:	awt	r1, 1
x1319:	irb	r2, -5
x131a:	uj	[0x130f]

x131c:	.res	1
x131d:	lw	r1, 0x1124
x131f:	lw	r2, [r1]
x1320:	pw	r2, r1
x1321:	awt	r1, 1
x1322:	cw	r1, 0x1180
x1324:	jes	1
x1325:	ujs	-7 ; -> 0x131f
x1326:	uj	[0x131c]

x1328:	.word	0x0000
x1329:	.word	0x0000
x132a:	.word	0x0001
x132b:	.word	0b1010101010101010
x132c:	.word	0
x132d:	.word	0
x132e:	.word	0
x132f:	.word	0
x1330:	.word	0
x1331:	.word	0
x1332:	.word	0
x1333:	.word	0
x1334:	.word	0
x1335:	.word	0
x1336:	.word	0
x1337:	.word	0
pat01:	.word	0b0101010101010101
pat10:	.word	0b1010101010101010
x133a:	.word	0x0001
x133b:	.word	0x0002
x133c:	.word	0x0003
x133d:	.word	0x0004
x133e:	.word	0x0005
x133f:	.word	0x0006
x1340:	.word	0x0007
x1341:	.word	0x100f
x1342:	.word	0x0041
x1343:	.word	0x1124
x1344:	.word	0x0000
x1345:	.word	0x002f
x1346:	.word	0x0000
x1347:	.word	0x0001
x1348:	.word	0b1010101010101010
x1349:	.word	0x0000
x134a:	.word	0x0000
x134b:	.word	0x0000
x134c:	.word	0x0000
x134d:	.word	0x0000
x134e:	.word	0x0000
x134f:	.word	0x0000
x1350:	.word	0x0000
x1351:	.word	0x0000
x1352:	.word	0x0000
x1353:	.word	0x0000
x1354:	.word	0x0000
x1355:	.word	0b0101010101010101
x1356:	.word	0b1010101010101010
x1357:	.word	0x0001
x1358:	.word	0x0002
x1359:	.word	0x0003
x135a:	.word	0x0004
x135b:	.word	0x0005
x135c:	.word	0x0006
x135d:	.word	0x0007
x135e:	ujs	1
x135f:	.word	0
x1360:	lw	r2, 0x62
x1362:	lw	r1, 0xffa2
x1364:	ri	r2, 0x0
x1366:	irb	r1, -3
x1367:	lw	r1, 0x62
x1369:	rw	r1, 0x61
x136b:	mcl
x136c:	ujs	6	; -> 0x1373
x136d:	.word	0
x136e:	.word	0
x136f:	.word	0
x1370:	ib	[r7+r7]
x1371:	pw	r5, r5+r2
x1372:	xr	r2, [r2+r5]
x1373:	lw	r1, [0x136f]
x1375:	rw	r1, 0x136e
x1377:	lj	0x13b0
x1379:	lj	0x1395
x137b:	lw	r1, [0x1370]
x137d:	rw	r1, 0x136e
x137f:	lj	0x13b0
x1381:	lj	0x1395
x1383:	lw	r1, [0x1371]
x1385:	rw	r1, 0x136e
x1387:	lj	0x13b0
x1389:	lj	0x1395
x138b:	lw	r1, [0x1372]
x138d:	rw	r1, 0x136e
x138f:	lj	0x13b0
x1391:	lj	0x1395
x1393:	uj	0x145b

x1395:	.word	0
x1396:	lw	r1, [0x136f]
x1398:	rw	r1, 0x136d
x139a:	lj	0x13c3
x139c:	lw	r1, [0x1370]
x139e:	rw	r1, 0x136d
x13a0:	lj	0x13c3
x13a2:	lw	r1, [0x1371]
x13a4:	rw	r1, 0x136d
x13a6:	lj	0x13c3
x13a8:	lw	r1, [0x1372]
x13aa:	rw	r1, 0x136d
x13ac:	lj	0x13c3
x13ae:	uj	[0x1395]

x13b0:	.word	0
x13b1:	lw	r1, [0x136e]
x13b3:	lw	r2, [0x136e]
x13b5:	lw	r3, [0x136e]
x13b7:	lw	r4, [0x136e]
x13b9:	lw	r5, [0x136e]
x13bb:	lw	r6, [0x136e]
x13bd:	lw	r7, [0x136e]
x13bf:	lw	r0, [0x136e]
x13c1:	uj	[0x13b0]

x13c3:	.word	0
x13c4:	lw	r1, [0x136e]
x13c6:	lw	r1, [0x136d]
x13c8:	cw	r1, [0x136d]
x13ca:	jes	1
x13cb:	hlt	040
x13cc:	lw	r1, [0x136e]
x13ce:	lj	0x1418
x13d0:	lw	r2, [0x136d]
x13d2:	cw	r2, [0x136d]
x13d4:	jes	1
x13d5:	hlt	040
x13d6:	lw	r2, [0x136e]
x13d8:	lj	0x1418
x13da:	lw	r3, [0x136d]
x13dc:	cw	r3, [0x136d]
x13de:	jes	1
x13df:	hlt	040
x13e0:	lw	r3, [0x136e]
x13e2:	lj	0x1418
x13e4:	lw	r4, [0x136d]
x13e6:	cw	r4, [0x136d]
x13e8:	jes	1
x13e9:	hlt	040
x13ea:	lw	r4, [0x136e]
x13ec:	lj	0x1418
x13ee:	lw	r5, [0x136d]
x13f0:	cw	r5, [0x136d]
x13f2:	jes	1
x13f3:	hlt	040
x13f4:	lw	r5, [0x136e]
x13f6:	lj	0x1418
x13f8:	lw	r6, [0x136d]
x13fa:	cw	r6, [0x136d]
x13fc:	jes	1
x13fd:	hlt	040
x13fe:	lw	r6, [0x136e]
x1400:	lj	0x1418
x1402:	lw	r7, [0x136d]
x1404:	cw	r7, [0x136d]
x1406:	jes	1
x1407:	hlt	040
x1408:	lw	r7, [0x136e]
x140a:	lj	0x1418
x140c:	lw	r0, [0x136d]
x140e:	cw	r0, [0x136d]
x1410:	jes	1
x1411:	hlt	040
x1412:	lw	r0, [0x136e]
x1414:	lj	0x1418
x1416:	uj	[0x13c3]

x1418:	.word	0
x1419:	cw	r1, [0x136e]
x141b:	jes	1
x141c:	hlt	040
x141d:	cw	r2, [0x136e]
x141f:	jes	1
x1420:	hlt	040
x1421:	cw	r3, [0x136e]
x1423:	jes	1
x1424:	hlt	040
x1425:	cw	r4, [0x136e]
x1427:	jes	1
x1428:	hlt	040
x1429:	cw	r5, [0x136e]
x142b:	jes	1
x142c:	hlt	040
x142d:	cw	r6, [0x136e]
x142f:	jes	1
x1430:	hlt	040
x1431:	cw	r7, [0x136e]
x1433:	jes	1
x1434:	hlt	040
x1435:	rw	r0, 0x145a
x1437:	lw	r7, [0x145a]
x1439:	cw	r1, [0x136f]
x143b:	jes	10
x143c:	cw	r1, [0x1370]
x143e:	jes	11
x143f:	cw	r1, [0x1371]
x1441:	jes	12
x1442:	cw	r7, [0x1459]
x1444:	jes	13
x1445:	ujs	11
x1446:	cw	r7, [0x1456]
x1448:	jes	9	; -> 0x1452
x1449:	ujs	7	; -> 0x1451
x144a:	cw	r7, [0x1457]
x144c:	jes	5	; -> 0x1452
x144d:	ujs	3	; -> 0x1451
x144e:	cw	r1, [0x1458]
x1450:	jes	1
x1451:	hlt	040
x1452:	lw	r7, [0x136e]
x1454:	uj	[0x1418]

x1456:	.word	0x0400
x1457:	tl	r7+r7
x1458:	pw	r5, r5+r2
x1459:	em	r2, r2+r5
x145a:	.word	0
x145b:	lw	r1, [0x136f]
x145d:	rw	r1, 0x136e
x145f:	lj	0x13b0
x1461:	lj	0x147d
x1463:	lw	r1, [0x1370]
x1465:	rw	r1, 0x136e
x1467:	lj	0x13b0
x1469:	lj	0x147d
x146b:	lw	r1, [0x1371]
x146d:	rw	r1, 0x136e
x146f:	lj	0x13b0
x1471:	lj	0x147d
x1473:	lw	r1, [0x1372]
x1475:	rw	r1, 0x136e
x1477:	lj	0x13b0
x1479:	lj	0x147d
x147b:	uj	0x14ef

x147d:	.res	1
x147e:	lw	r1, [0x136f]
x1480:	rw	r1, 0x136d
x1482:	lj	0x1498
x1484:	lw	r1, [0x1370]
x1486:	rw	r1, 0x136d
x1488:	lj	0x1498
x148a:	lw	r1, [0x1371]
x148c:	rw	r1, 0x136d
x148e:	lj	0x1498
x1490:	lw	r1, [0x1372]
x1492:	rw	r1, 0x136d
x1494:	lj	0x1498
x1496:	uj	[0x147d]

x1498:	.res	1
x1499:	lw	r1, [0x136d]
x149b:	lw	r2, r1
x149c:	cw	r1, r2
x149d:	jes	1
x149e:	hlt	040
x149f:	lw	r1, [0x136e]
x14a1:	lw	r2, [0x136e]
x14a3:	lj	0x1418
x14a5:	lw	r2, [0x136d]
x14a7:	lw	r3, r2
x14a8:	cw	r2, r3
x14a9:	jes	1
x14aa:	hlt	040
x14ab:	lw	r2, [0x136e]
x14ad:	lw	r3, [0x136e]
x14af:	lj	0x1418
x14b1:	lw	r3, [0x136d]
x14b3:	lw	r4, r3
x14b4:	cw	r3, r4
x14b5:	jes	1
x14b6:	hlt	040
x14b7:	lw	r3, [0x136e]
x14b9:	lw	r4, [0x136e]
x14bb:	lj	0x1418
x14bd:	lw	r4, [0x136d]
x14bf:	lw	r5, r4
x14c0:	cw	r4, r5
x14c1:	jes	1
x14c2:	hlt	040
x14c3:	lw	r4, [0x136e]
x14c5:	lw	r5, [0x136e]
x14c7:	lj	0x1418
x14c9:	lw	r5, [0x136d]
x14cb:	lw	r6, r5
x14cc:	cw	r5, r6
x14cd:	jes	1
x14ce:	hlt	040
x14cf:	lw	r5, [0x136e]
x14d1:	lw	r6, [0x136e]
x14d3:	lj	0x1418
x14d5:	lw	r6, [0x136d]
x14d7:	lw	r7, r6
x14d8:	cw	r6, r7
x14d9:	jes	1
x14da:	hlt	040
x14db:	lw	r6, [0x136e]
x14dd:	lw	r7, [0x136e]
x14df:	lj	0x1418
x14e1:	lw	r7, [0x136d]
x14e3:	lw	r1, r7
x14e4:	cw	r7, r1
x14e5:	jes	1
x14e6:	hlt	040
x14e7:	lw	r7, [0x136e]
x14e9:	lw	r1, [0x136e]
x14eb:	lj	0x1418
x14ed:	uj	[0x1498]

x14ef:	ujs	1
x14f0:	.word	0
x14f1:	lj	reset
x14f3:	ujs	6	; -> 0x14fa
x14f4:	.word	0x000d
x14f5:	.word	0x000f
x14f6:	.word	0x001c
x14f7:	.word	0x0038
x14f8:	.word	0x14f9
x14f9:	ib	[r7+r7]
x14fa:	ric	r1
x14fb:	cw	r1, 0x14fb
x14fd:	jes	1
x14fe:	hlt	040
x14ff:	lw	r2, [0x14f5]
x1501:	lw	r3, [0x14f4]
x1503:	md	r2
x1504:	lw	r1, r3
x1505:	cw	r1, [0x14f6]
x1507:	jes	1
x1508:	hlt	040
x1509:	lw	r2, [0x14f4]
x150b:	lw	r3, [0x14f5]
x150d:	lw	r1, r2+r3
x150e:	cw	r1, [0x14f6]
x1510:	jes	1
x1511:	hlt	040
x1512:	lw	r4, [0x14f6]
x1514:	lw	r2, [0x14f4]
x1516:	lw	r3, [0x14f5]
x1518:	md	r1
x1519:	lw	r1, r2+r3
x151a:	cw	r1, [0x14f7]
x151c:	jes	1
x151d:	hlt	040
x151e:	lw	r2, 0x14f4
x1520:	lw	r1, [r2]
x1521:	cw	r1, [0x14f4]
x1523:	jes	1
x1524:	hlt	040
x1525:	lwt	r1, 1
x1526:	lw	r2, 0x14f5
x1528:	md	r1
x1529:	lw	r1, [r2]
x152a:	cw	r1, [0x14f6]
x152c:	jes	1
x152d:	hlt	040
x152e:	lwt	r3, 1
x152f:	lw	r2, 0x14f6
x1531:	lw	r1, [r2+r3]
x1532:	cw	r1, [0x14f7]
x1534:	jes	1
x1535:	hlt	040
x1536:	lwt	r1, 1
x1537:	lwt	r3, 1
x1538:	lw	r2, 0x14f5
x153a:	md	r1
x153b:	lw	r1, [r3+r2]
x153c:	cw	r1, [0x14f7]
x153e:	jes	1
x153f:	hlt	040
x1540:	lw	r1, -1
x1542:	rw	r1, 0x14f9
x1544:	lw	r2, 0x14f9
x1546:	rz	r2
x1547:	lw	r1, [0x14f9]
x1549:	cwt	r1, 0
x154a:	jes	1
x154b:	hlt	040
x154c:	lw	r1, -1
x154e:	rw	r1, 0x14f9
x1550:	lw	r2, 0x14f8
x1552:	lwt	r3, 1
x1553:	md	r3
x1554:	rz	r2
x1555:	lw	r1, [0x14f9]
x1557:	cwt	r1, 0
x1558:	jes	1
x1559:	hlt	040
x155a:	lw	r1, -1
x155c:	rw	r1, 0x14f9
x155e:	lwt	r3, 1
x155f:	lw	r2, 0x14f8
x1561:	rz	r2+r3
x1562:	lw	r1, [0x14f9]
x1564:	cwt	r1, 0
x1565:	jes	1
x1566:	hlt	040
x1567:	lw	r1, -1
x1569:	rw	r1, 0x14f9
x156b:	lwt	r3, 1
x156c:	lwt	r1, 1
x156d:	lw	r2, 0x14f7
x156f:	md	r1
x1570:	rz	r2+r3
x1571:	lw	r1, [0x14f9]
x1573:	cwt	r1, 0
x1574:	jes	1
x1575:	hlt	040
x1576:	lw	r1, -1
x1578:	rw	r1, 0x14f9
x157a:	lw	r2, 0x14f8
x157c:	rz	[r2]
x157d:	lw	r1, [0x14f9]
x157f:	cwt	r1, 0
x1580:	jes	1
x1581:	hlt	040
x1582:	lw	r1, -1
x1584:	rw	r1, 0x14f9
x1586:	lw	r2, 0x14f7
x1588:	lwt	r3, 1
x1589:	md	r3
x158a:	rz	[r2]
x158b:	lw	r1, [0x14f9]
x158d:	cwt	r1, 0
x158e:	jes	1
x158f:	hlt	040
x1590:	lw	r1, -1
x1592:	rw	r1, 0x14f9
x1594:	lw	r2, 0x14f7
x1596:	lwt	r3, 1
x1597:	rz	[r2+r3]
x1598:	lw	r1, [0x14f9]
x159a:	cwt	r1, 0
x159b:	jes	1
x159c:	hlt	040
x159d:	lw	r1, -1
x159f:	rw	r1, 0x14f9
x15a1:	lw	r2, 0x14f6
x15a3:	lwt	r3, 1
x15a4:	lwt	r1, 1
x15a5:	md	r1
x15a6:	rz	[r2+r3]
x15a7:	lw	r1, [0x14f9]
x15a9:	cwt	r1, 0
x15aa:	jes	1
x15ab:	hlt	040
x15ac:	lw	r3, 0x14f7
x15ae:	md	r3
x15af:	lwt	r1, 2
x15b0:	cw	r1, 0x14f9
x15b2:	jes	1
x15b3:	hlt	040
x15b4:	lwt	r0, 0
x15b5:	brc	64
x15b6:	hlt	040
x15b7:	lw	r1, 0xff
x15b9:	lw	r0, 0xff
x15bb:	md	r1
x15bc:	brc	64
x15bd:	hlt	040
x15be:	lwt	r1, 1
x15bf:	md	r1
x15c0:	lw	r2, 0x14f4
x15c2:	cw	r2, 0x14f5
x15c4:	jes	1
x15c5:	hlt	040
x15c6:	lwt	r1, 1
x15c7:	lw	r2, 0x14f4+r1
x15c9:	cw	r2, 0x14f5
x15cb:	jes	1
x15cc:	hlt	040
x15cd:	lwt	r1, 1
x15ce:	lwt	r3, 1
x15cf:	md	r1
x15d0:	lw	r2, 0x14f4+r3
x15d2:	cw	r2, 0x14f6
x15d4:	jes	1
x15d5:	hlt	040
x15d6:	lwt	r1, 1
x15d7:	md	r1
x15d8:	lw	r2, [0x14f7]
x15da:	cw	r2, 0x14f9
x15dc:	jes	1
x15dd:	hlt	040
x15de:	lwt	r1, 1
x15df:	lw	r2, [0x14f7+r1]
x15e1:	cw	r2, 0x14f9
x15e3:	jes	1
x15e4:	hlt	040
x15e5:	lwt	r1, 1
x15e6:	lwt	r3, 1
x15e7:	md	r1
x15e8:	lw	r2, [0x14f6+r3]
x15ea:	cw	r2, 0x14f9
x15ec:	jes	1
x15ed:	hlt	040
x15ee:	lw	r1, -1
x15f0:	rw	r1, 0x14f9
x15f2:	lwt	r1, 1
x15f3:	md	r1
x15f4:	rz	0x14f8
x15f6:	lw	r2, [0x14f9]
x15f8:	cwt	r2, 0
x15f9:	jes	1
x15fa:	hlt	040
x15fb:	lw	r1, -1
x15fd:	rw	r1, 0x14f9
x15ff:	lwt	r1, 1
x1600:	rz	0x14f8+r1
x1602:	lw	r2, [0x14f9]
x1604:	cwt	r2, 0
x1605:	jes	1
x1606:	hlt	040
x1607:	lw	r1, -1
x1609:	rw	r1, 0x14f9
x160b:	lwt	r1, 1
x160c:	lwt	r3, 1
x160d:	md	r1
x160e:	rz	0x14f7+r3
x1610:	lw	r2, [0x14f9]
x1612:	cwt	r2, 0
x1613:	jes	1
x1614:	hlt	040
x1615:	lw	r1, -1
x1617:	rw	r1, 0x14f9
x1619:	lwt	r1, 1
x161a:	md	r1
x161b:	rz	[0x14f7]
x161d:	lw	r2, [0x14f9]
x161f:	cwt	r2, 0
x1620:	jes	1
x1621:	hlt	040
x1622:	lw	r1, -1
x1624:	rw	r1, 0x14f9
x1626:	lwt	r1, 1
x1627:	rz	[0x14f7+r1]
x1629:	lw	r2, [0x14f9]
x162b:	cwt	r2, 0
x162c:	jes	1
x162d:	hlt	040
x162e:	lw	r1, -1
x1630:	rw	r1, 0x14f9
x1632:	lwt	r1, 1
x1633:	lwt	r3, 1
x1634:	md	r1
x1635:	rz	[0x14f6+r3]
x1637:	lw	r2, [0x14f9]
x1639:	cwt	r2, 0
x163a:	jes	1
x163b:	hlt	040
x163c:	mcl
x163d:	ujs	3	; -> 0x1641
int00:	hlt	040
x163f:	ujs	int00
x1640:	hlt	040
x1641:	lwt	r3, 1
x1642:	lw	r4, 0x41
x1644:	ou	r3, r4
x1645:	.word	0x1640, 0x1649, 0x1650, 0x164b
x1649:	hlt	040
x164a:	ujs	-7 ; -> 0x1644
x164b:	hlt	040
x164c:	ujs	-9 ; -> 0x1644
x164d:	.word	0
x164e:	.word	0
int_store:
	.res	1
x1650:	lj	reset
x1652:	rz	0x164d
x1654:	im	0x164d
x1656:	lw	r7, 0x7fff
x1658:	rw	r7, 0x164e
x165a:	fi	0x164e
x165c:	lw	r1, 0x1668
x165e:	rw	r1, 0x41
x1660:	lwt	r0, -1
x1661:	lw	r1, 0x8000
x1663:	rw	r1, 0x164d
x1665:	im	0x164d
x1667:	ujs	39 ; -> 0x168f
x1668:	la	stack
x166a:	cw	r1, 0x1667
x166c:	jes	1
x166d:	hlt	040
x166e:	cwt	r2, -1
x166f:	jes	1
x1670:	hlt	040
x1671:	cw	r3, 0x8000
x1673:	jes	1
x1674:	hlt	040
x1675:	cwt	r4, 0
x1676:	jes	1
x1677:	hlt	040
x1678:	lw	r1, [0x61]
x167a:	cw	r1, 0x66
x167c:	jes	1
x167d:	hlt	040
x167e:	cw	r0, 0x400
x1680:	jes	1
x1681:	hlt	040
x1682:	ki	int_store
x1684:	lw	r1, [int_store]
x1686:	cw	r1, 0x3fff
x1688:	jes	1
x1689:	hlt	040
x168a:	lw	r1, 0xc1
x168c:	rw	r1, 0x41
x168e:	lip
x168f:	lw	r1, 0x169b
x1691:	rw	r1, 0x42
x1693:	lw	r1, 0x4000
x1695:	rw	r1, 0x164d
x1697:	lwt	r0, -1
x1698:	im	0x164d
x169a:	ujs	39 ; -> 0x16c2
x169b:	la	stack
x169d:	cw	r1, 0x169a
x169f:	jes	1
x16a0:	hlt	040
x16a1:	cwt	r2, -1
x16a2:	jes	1
x16a3:	hlt	040
x16a4:	cw	r3, 0x4000
x16a6:	jes	1
x16a7:	hlt	040
x16a8:	cwt	r4, 0
x16a9:	jes	1
x16aa:	hlt	040
x16ab:	lw	r1, [0x61]
x16ad:	cw	r1, 0x66
x16af:	jes	1
x16b0:	hlt	040
x16b1:	cw	r0, 0x400
x16b3:	jes	1
x16b4:	hlt	040
x16b5:	ki	int_store
x16b7:	lw	r1, [int_store]
x16b9:	cw	r1, 0x1fff
x16bb:	jes	1
x16bc:	hlt	040
x16bd:	lw	r1, 0xc3
x16bf:	rw	r1, 0x42
x16c1:	lip

x16c2:	lw	r1, 0x16ce
x16c4:	rw	r1, 0x43
x16c6:	lw	r1, 0x2000
x16c8:	rw	r1, 0x164d
x16ca:	lwt	r0, -1
x16cb:	im	0x164d
x16cd:	ujs	39 ; -> 0x16f5
x16ce:	la	stack
x16d0:	cw	r1, 0x16cd
x16d2:	jes	1
x16d3:	hlt	040
x16d4:	cwt	r2, -1
x16d5:	jes	1
x16d6:	hlt	040
x16d7:	cw	r3, 0x2000
x16d9:	jes	1
x16da:	hlt	040
x16db:	cwt	r4, 0
x16dc:	jes	1
x16dd:	hlt	040
x16de:	lw	r1, [0x61]
x16e0:	cw	r1, 0x66
x16e2:	jes	1
x16e3:	hlt	040
x16e4:	cw	r0, 0x400
x16e6:	jes	1
x16e7:	hlt	040
x16e8:	ki	int_store
x16ea:	lw	r1, [int_store]
x16ec:	cw	r1, 0xfff
x16ee:	jes	1
x16ef:	hlt	040
x16f0:	lw	r1, 0xc5
x16f2:	rw	r1, 0x43
x16f4:	lip

x16f5:	lw	r1, 0x1701
x16f7:	rw	r1, 0x44
x16f9:	lw	r1, 0x1000
x16fb:	rw	r1, 0x164d
x16fd:	lwt	r0, -1
x16fe:	im	0x164d
x1700:	ujs	39 ; -> 0x1728
x1701:	la	stack
x1703:	cw	r1, 0x1700
x1705:	jes	1
x1706:	hlt	040
x1707:	cwt	r2, -1
x1708:	jes	1
x1709:	hlt	040
x170a:	cw	r3, 0x1000
x170c:	jes	1
x170d:	hlt	040
x170e:	cwt	r4, 0
x170f:	jes	1
x1710:	hlt	040
x1711:	lw	r1, [0x61]
x1713:	cw	r1, 0x66
x1715:	jes	1
x1716:	hlt	040
x1717:	cw	r0, 0x400
x1719:	jes	1
x171a:	hlt	040
x171b:	ki	int_store
x171d:	lw	r1, [int_store]
x171f:	cw	r1, 0x7ff
x1721:	jes	1
x1722:	hlt	040
x1723:	lw	r1, 0xc7
x1725:	rw	r1, 0x44
x1727:	lip

x1728:	lw	r1, 0x1741
x172a:	lw	r2, 0x1768
x172c:	lw	r3, 0x178f
x172e:	lw	r4, 0x17b6
x1730:	lw	r5, 0x17dd
x1732:	lw	r6, 0x1804
x1734:	lw	r7, 0x182b
x1736:	ra	0x45
x1738:	lw	r1, 0x800
x173a:	rw	r1, 0x164d
x173c:	lwt	r0, -1
x173d:	im	0x164d
x173f:	uj	0x1852
x1741:	la	stack
x1743:	cw	r1, 0x173f
x1745:	jes	1
x1746:	hlt	040
x1747:	cwt	r2, -1
x1748:	jes	1
x1749:	hlt	040
x174a:	cw	r3, 0x800
x174c:	jes	1
x174d:	hlt	040
x174e:	cwt	r4, 0
x174f:	jes	1
x1750:	hlt	040
x1751:	lw	r1, [0x61]
x1753:	cw	r1, 0x66
x1755:	jes	1
x1756:	hlt	040
x1757:	cw	r0, 0x400
x1759:	jes	1
x175a:	hlt	040
x175b:	ki	int_store
x175d:	lw	r1, [int_store]
x175f:	cw	r1, 0x3ff
x1761:	jes	1
x1762:	hlt	040
x1763:	lw	r1, 0xc9
x1765:	rw	r1, 0x45
x1767:	lip

x1768:	la	stack
x176a:	cw	r1, 0x173f
x176c:	jes	1
x176d:	hlt	040
x176e:	cwt	r2, -1
x176f:	jes	1
x1770:	hlt	040
x1771:	cw	r3, 0x800
x1773:	jes	1
x1774:	hlt	040
x1775:	cwt	r4, 0
x1776:	jes	1
x1777:	hlt	040
x1778:	lw	r1, [0x61]
x177a:	cw	r1, 0x66
x177c:	jes	1
x177d:	hlt	040
x177e:	cw	r0, 0x400
x1780:	jes	1
x1781:	hlt	040
x1782:	ki	int_store
x1784:	lw	r1, [int_store]
x1786:	cw	r1, 0x1ff
x1788:	jes	1
x1789:	hlt	040
x178a:	lw	r1, 0xcb
x178c:	rw	r1, 0x46
x178e:	lip

x178f:	la	stack
x1791:	cw	r1, 0x173f
x1793:	jes	1
x1794:	hlt	040
x1795:	cwt	r2, -1
x1796:	jes	1
x1797:	hlt	040
x1798:	cw	r3, 0x800
x179a:	jes	1
x179b:	hlt	040
x179c:	cwt	r4, 0
x179d:	jes	1
x179e:	hlt	040
x179f:	lw	r1, [0x61]
x17a1:	cw	r1, 0x66
x17a3:	jes	1
x17a4:	hlt	040
x17a5:	cw	r0, 0x400
x17a7:	jes	1
x17a8:	hlt	040
x17a9:	ki	int_store
x17ab:	lw	r1, [int_store]
x17ad:	cw	r1, 0xff
x17af:	jes	1
x17b0:	hlt	040
x17b1:	lw	r1, 0xcd
x17b3:	rw	r1, 0x47
x17b5:	lip

x17b6:	la	stack
x17b8:	cw	r1, 0x173f
x17ba:	jes	1
x17bb:	hlt	040
x17bc:	cwt	r2, -1
x17bd:	jes	1
x17be:	hlt	040
x17bf:	cw	r3, 0x800
x17c1:	jes	1
x17c2:	hlt	040
x17c3:	cwt	r4, 0
x17c4:	jes	1
x17c5:	hlt	040
x17c6:	lw	r1, [0x61]
x17c8:	cw	r1, 0x66
x17ca:	jes	1
x17cb:	hlt	040
x17cc:	cw	r0, 0x400
x17ce:	jes	1
x17cf:	hlt	040
x17d0:	ki	int_store
x17d2:	lw	r1, [int_store]
x17d4:	cw	r1, 0x7f
x17d6:	jes	1
x17d7:	hlt	040
x17d8:	lw	r1, 0xcf
x17da:	rw	r1, 0x48
x17dc:	lip

x17dd:	la	stack
x17df:	cw	r1, 0x173f
x17e1:	jes	1
x17e2:	hlt	040
x17e3:	cwt	r2, -1
x17e4:	jes	1
x17e5:	hlt	040
x17e6:	cw	r3, 0x800
x17e8:	jes	1
x17e9:	hlt	040
x17ea:	cwt	r4, 0
x17eb:	jes	1
x17ec:	hlt	040
x17ed:	lw	r1, [0x61]
x17ef:	cw	r1, 0x66
x17f1:	jes	1
x17f2:	hlt	040
x17f3:	cw	r0, 0x400
x17f5:	jes	1
x17f6:	hlt	040
x17f7:	ki	int_store
x17f9:	lw	r1, [int_store]
x17fb:	cw	r1, 0x3f
x17fd:	jes	1
x17fe:	hlt	040
x17ff:	lw	r1, 0xd1
x1801:	rw	r1, 0x49
x1803:	lip

x1804:	la	stack
x1806:	cw	r1, 0x173f
x1808:	jes	1
x1809:	hlt	040
x180a:	cwt	r2, -1
x180b:	jes	1
x180c:	hlt	040
x180d:	cw	r3, 0x800
x180f:	jes	1
x1810:	hlt	040
x1811:	cwt	r4, 0
x1812:	jes	1
x1813:	hlt	040
x1814:	lw	r1, [0x61]
x1816:	cw	r1, 0x66
x1818:	jes	1
x1819:	hlt	040
x181a:	cw	r0, 0x400
x181c:	jes	1
x181d:	hlt	040
x181e:	ki	int_store
x1820:	lw	r1, [int_store]
x1822:	cw	r1, 0x1f
x1824:	jes	1
x1825:	hlt	040
x1826:	lw	r1, 0xd3
x1828:	rw	r1, 0x4a
x182a:	lip

x182b:	la	stack
x182d:	cw	r1, 0x173f
x182f:	jes	1
x1830:	hlt	040
x1831:	cwt	r2, -1
x1832:	jes	1
x1833:	hlt	040
x1834:	cw	r3, 0x800
x1836:	jes	1
x1837:	hlt	040
x1838:	cwt	r4, 0
x1839:	jes	1
x183a:	hlt	040
x183b:	lw	r1, [0x61]
x183d:	cw	r1, 0x66
x183f:	jes	1
x1840:	hlt	040
x1841:	cw	r0, 0x400
x1843:	jes	1
x1844:	hlt	040
x1845:	ki	int_store
x1847:	lw	r1, [int_store]
x1849:	cw	r1, 0xf
x184b:	jes	1
x184c:	hlt	040
x184d:	lw	r1, 0xd5
x184f:	rw	r1, 0x4b
x1851:	lip

x1852:	lw	r1, 0x1867
x1854:	lw	r2, 0x188d
x1856:	lw	r3, 0x18b3
x1858:	lw	r4, 0x18d9
x185a:	rf	0x5c
x185c:	rw	r4, 0x5f
x185e:	lw	r1, 0x40
x1860:	rw	r1, 0x164d
x1862:	lwt	r0, -1
x1863:	im	0x164d
x1865:	uj	0x18ff
x1867:	la	stack
x1869:	cw	r1, 0x1865
x186b:	jes	1
x186c:	hlt	040
x186d:	cwt	r2, -1
x186e:	jes	1
x186f:	hlt	040
x1870:	cw	r3, 0x40
x1872:	jes	1
x1873:	hlt	040
x1874:	cwt	r4, 0
x1875:	jes	1
x1876:	hlt	040
x1877:	lw	r1, [0x61]
x1879:	cw	r1, 0x66
x187b:	jes	1
x187c:	hlt	040
x187d:	cw	r0, 0x400
x187f:	jes	1
x1880:	hlt	040
x1881:	ki	int_store
x1883:	lw	r1, [int_store]
x1885:	cwt	r1, 7
x1886:	jes	1
x1887:	hlt	040
x1888:	lw	r1, 0xf7
x188a:	rw	r1, 0x5c
x188c:	lip

x188d:	la	stack
x188f:	cw	r1, 0x1865
x1891:	jes	1
x1892:	hlt	040
x1893:	cwt	r2, -1
x1894:	jes	1
x1895:	hlt	040
x1896:	cw	r3, 0x40
x1898:	jes	1
x1899:	hlt	040
x189a:	cwt	r4, 0
x189b:	jes	1
x189c:	hlt	040
x189d:	lw	r1, [0x61]
x189f:	cw	r1, 0x66
x18a1:	jes	1
x18a2:	hlt	040
x18a3:	cw	r0, 0x400
x18a5:	jes	1
x18a6:	hlt	040
x18a7:	ki	int_store
x18a9:	lw	r1, [int_store]
x18ab:	cwt	r1, 3
x18ac:	jes	1
x18ad:	hlt	040
x18ae:	lw	r1, 0xf9
x18b0:	rw	r1, 0x5d
x18b2:	lip

x18b3:	la	stack
x18b5:	cw	r1, 0x1865
x18b7:	jes	1
x18b8:	hlt	040
x18b9:	cwt	r2, -1
x18ba:	jes	1
x18bb:	hlt	040
x18bc:	cw	r3, 0x40
x18be:	jes	1
x18bf:	hlt	040
x18c0:	cwt	r4, 0
x18c1:	jes	1
x18c2:	hlt	040
x18c3:	lw	r1, [0x61]
x18c5:	cw	r1, 0x66
x18c7:	jes	1
x18c8:	hlt	040
x18c9:	cw	r0, 0x400
x18cb:	jes	1
x18cc:	hlt	040
x18cd:	ki	int_store
x18cf:	lw	r1, [int_store]
x18d1:	cwt	r1, 1
x18d2:	jes	1
x18d3:	hlt	040
x18d4:	lw	r1, 0xfb
x18d6:	rw	r1, 0x5e
x18d8:	lip

x18d9:	la	stack
x18db:	cw	r1, 0x1865
x18dd:	jes	1
x18de:	hlt	040
x18df:	cwt	r2, -1
x18e0:	jes	1
x18e1:	hlt	040
x18e2:	cw	r3, 0x40
x18e4:	jes	1
x18e5:	hlt	040
x18e6:	cwt	r4, 0
x18e7:	jes	1
x18e8:	hlt	040
x18e9:	lw	r1, [0x61]
x18eb:	cw	r1, 0x66
x18ed:	jes	1
x18ee:	hlt	040
x18ef:	cw	r0, 0x400
x18f1:	jes	1
x18f2:	hlt	040
x18f3:	ki	int_store
x18f5:	lw	r1, [int_store]
x18f7:	cwt	r1, 0
x18f8:	jes	1
x18f9:	hlt	040
x18fa:	lw	r1, 0xfd
x18fc:	rw	r1, 0x5f
x18fe:	lip

x18ff:	lw	r1, 0x1906
x1901:	rw	r1, 0x5c
x1903:	lwt	r0, -1
x1904:	fi	0x164e
x1906:	lw	r1, [stack]
x1908:	cw	r1, 0x1906
x190a:	jes	1
x190b:	hlt	040
x190c:	ki	int_store
x190e:	lw	r1, [int_store]
x1910:	cw	r1, 0x7ff7
x1912:	jes	1
x1913:	hlt	040
x1914:	lw	r1, 0xf7
x1916:	rw	r1, 0x5c
x1918:	lw	r1, 0x1923
x191a:	rw	r1, 0x5d
x191c:	lwt	r0, -1
x191d:	lw	r1, 0x40
x191f:	rw	r1, 0x164d
x1921:	im	0x164d
x1923:	lw	r1, [0x66]
x1925:	cw	r1, 0x1923
x1927:	jes	1
x1928:	hlt	040
x1929:	ki	int_store
x192b:	lw	r1, [int_store]
x192d:	cw	r1, 0x7ff3
x192f:	jes	1
x1930:	hlt	040
x1931:	lw	r1, 0xf9
x1933:	rw	r1, 0x5d
x1935:	lw	r1, 0x193c
x1937:	rw	r1, 0x5e
x1939:	lwt	r0, -1
x193a:	im	0x164d
x193c:	lw	r1, [0x6a]
x193e:	cw	r1, 0x193c
x1940:	jes	1
x1941:	hlt	040
x1942:	ki	int_store
x1944:	lw	r1, [int_store]
x1946:	cw	r1, 0x7ff1
x1948:	jes	1
x1949:	hlt	040
x194a:	lw	r1, 0xfb
x194c:	rw	r1, 0x5e
x194e:	lw	r1, 0x1955
x1950:	rw	r1, 0x5f
x1952:	lwt	r0, -1
x1953:	im	0x164d
x1955:	lw	r1, [0x6e]
x1957:	cw	r1, 0x1955
x1959:	jes	1
x195a:	hlt	040
x195b:	ki	int_store
x195d:	lw	r1, [int_store]
x195f:	cw	r1, 0x7ff0
x1961:	jes	1
x1962:	hlt	040
x1963:	lw	r1, 0x196e
x1965:	rw	r1, 0x6e
x1967:	lw	r1, 0xfd
x1969:	rw	r1, 0x5f
x196b:	rz	0x70
x196d:	lip

x196e:	lw	r1, [0x61]
x1970:	cw	r1, 0x6e
x1972:	jes	1
x1973:	hlt	040
x1974:	lw	r1, 0x1981
x1976:	rw	r1, 0x45
x1978:	lwt	r0, -1
x1979:	lw	r1, 0x800
x197b:	rw	r1, 0x164d
x197d:	fi	0x164e
x197f:	im	0x164d
x1981:	lw	r1, [0x6e]
x1983:	cw	r1, 0x1981
x1985:	jes	1
x1986:	hlt	040
x1987:	ki	int_store
x1989:	lw	r1, [int_store]
x198b:	cw	r1, 0x7bff
x198d:	jes	1
x198e:	hlt	040
x198f:	lw	r1, 0xc9
x1991:	rw	r1, 0x45
x1993:	lw	r1, 0x199a
x1995:	rw	r1, 0x46
x1997:	lwt	r0, -1
x1998:	im	0x164d
x199a:	lw	r1, [0x72]
x199c:	cw	r1, 0x199a
x199e:	jes	1
x199f:	hlt	040
x19a0:	ki	int_store
x19a2:	lw	r1, [int_store]
x19a4:	cw	r1, 0x79ff
x19a6:	jes	1
x19a7:	hlt	040
x19a8:	lw	r1, 0xcb
x19aa:	rw	r1, 0x46
x19ac:	lw	r1, 0x19b3
x19ae:	rw	r1, 0x47
x19b0:	lwt	r0, -1
x19b1:	im	0x164d
x19b3:	lw	r1, [0x76]
x19b5:	cw	r1, 0x19b3
x19b7:	jes	1
x19b8:	hlt	040
x19b9:	ki	int_store
x19bb:	lw	r1, [int_store]
x19bd:	cw	r1, 0x78ff
x19bf:	jes	1
x19c0:	hlt	040
x19c1:	lw	r1, 0xcd
x19c3:	rw	r1, 0x47
x19c5:	lw	r1, 0x19cc
x19c7:	rw	r1, 0x48
x19c9:	lwt	r0, -1
x19ca:	im	0x164d
x19cc:	lw	r1, [0x7a]
x19ce:	cw	r1, 0x19cc
x19d0:	jes	1
x19d1:	hlt	040
x19d2:	ki	int_store
x19d4:	lw	r1, [int_store]
x19d6:	cw	r1, 0x787f
x19d8:	jes	1
x19d9:	hlt	040
x19da:	lw	r1, 0xcf
x19dc:	rw	r1, 0x48
x19de:	lw	r1, 0x19e5
x19e0:	rw	r1, 0x49
x19e2:	lwt	r0, -1
x19e3:	im	0x164d
x19e5:	lw	r1, [0x7e]
x19e7:	cw	r1, 0x19e5
x19e9:	jes	1
x19ea:	hlt	040
x19eb:	ki	int_store
x19ed:	lw	r1, [int_store]
x19ef:	cw	r1, 0x783f
x19f1:	jes	1
x19f2:	hlt	040
x19f3:	lw	r1, 0xd1
x19f5:	rw	r1, 0x49
x19f7:	lw	r1, 0x19fe
x19f9:	rw	r1, 0x4a
x19fb:	lwt	r0, -1
x19fc:	im	0x164d
x19fe:	lw	r1, [0x82]
x1a00:	cw	r1, 0x19fe
x1a02:	jes	1
x1a03:	hlt	040
x1a04:	ki	int_store
x1a06:	lw	r1, [int_store]
x1a08:	cw	r1, 0x781f
x1a0a:	jes	1
x1a0b:	hlt	040
x1a0c:	lw	r1, 0xd3
x1a0e:	rw	r1, 0x4a
x1a10:	lw	r1, 0x1a17
x1a12:	rw	r1, 0x4b
x1a14:	lwt	r0, -1
x1a15:	im	0x164d
x1a17:	lw	r1, [0x86]
x1a19:	cw	r1, 0x1a17
x1a1b:	jes	1
x1a1c:	hlt	040
x1a1d:	ki	int_store
x1a1f:	lw	r1, [int_store]
x1a21:	cw	r1, 0x780f
x1a23:	jes	1
x1a24:	hlt	040
x1a25:	lw	r1, 0x1a30
x1a27:	rw	r1, 0x86
x1a29:	lw	r1, 0xd5
x1a2b:	rw	r1, 0x4b
x1a2d:	rz	0x88
x1a2f:	lip

x1a30:	lw	r1, [0x61]
x1a32:	cw	r1, 0x86
x1a34:	jes	1
x1a35:	hlt	040
x1a36:	lw	r1, 0x1a43
x1a38:	rw	r1, 0x44
x1a3a:	lwt	r0, -1
x1a3b:	lw	r1, 0x1000
x1a3d:	rw	r1, 0x164d
x1a3f:	fi	0x164e
x1a41:	im	0x164d
x1a43:	lw	r1, [0x86]
x1a45:	cw	r1, 0x1a43
x1a47:	jes	1
x1a48:	hlt	040
x1a49:	ki	int_store
x1a4b:	lw	r1, [int_store]
x1a4d:	cw	r1, 0x77ff
x1a4f:	jes	1
x1a50:	hlt	040
x1a51:	lw	r1, 0xc7
x1a53:	rw	r1, 0x44
x1a55:	lw	r1, 0x1a5a
x1a57:	rw	r1, 0x86
x1a59:	lip

x1a5a:	lw	r1, 0x1a65
x1a5c:	rw	r1, 0x43
x1a5e:	lwt	r0, -1
x1a5f:	lw	r1, 0x2000
x1a61:	rw	r1, 0x164d
x1a63:	im	0x164d
x1a65:	lw	r1, [0x86]
x1a67:	cw	r1, 0x1a65
x1a69:	jes	1
x1a6a:	hlt	040
x1a6b:	ki	int_store
x1a6d:	lw	r1, [int_store]
x1a6f:	cw	r1, 0x67ff
x1a71:	jes	1
x1a72:	hlt	040
x1a73:	lw	r1, 0xc5
x1a75:	rw	r1, 0x43
x1a77:	lw	r1, 0x1a7c
x1a79:	rw	r1, 0x86
x1a7b:	lip
x1a7c:	lw	r1, 0x1a87
x1a7e:	rw	r1, 0x42
x1a80:	lwt	r0, -1
x1a81:	lw	r1, 0x4000
x1a83:	rw	r1, 0x164d
x1a85:	im	0x164d
x1a87:	lw	r1, [0x86]
x1a89:	cw	r1, 0x1a87
x1a8b:	jes	1
x1a8c:	hlt	040
x1a8d:	ki	int_store
x1a8f:	lw	r1, [int_store]
x1a91:	cw	r1, 0x47ff
x1a93:	jes	1
x1a94:	hlt	040
x1a95:	lw	r1, 0xc3
x1a97:	rw	r1, 0x42
x1a99:	lw	r1, 0x1aa0
x1a9b:	rw	r1, 0x86
x1a9d:	rz	0x88
x1a9f:	lip
x1aa0:	lw	r1, 0x1ab5
x1aa2:	rw	r1, 0x40
x1aa4:	rz	0x164e
x1aa6:	fi	0x164e
x1aa8:	lw	r1, 0x8000
x1aaa:	rw	r1, 0x164e
x1aac:	lwt	r0, -1
x1aad:	lw	r1, 0xffc0
x1aaf:	rw	r1, 0x164d
x1ab1:	im	0x164d
x1ab3:	fi	0x164e
x1ab5:	la	0x86
x1ab7:	cw	r1, 0x1ab5
x1ab9:	jes	1
x1aba:	hlt	040
x1abb:	cwt	r2, -1
x1abc:	jes	1
x1abd:	hlt	040
x1abe:	cw	r3, 0xffc0
x1ac0:	jes	1
x1ac1:	hlt	040
x1ac2:	cwt	r4, 0
x1ac3:	jes	1
x1ac4:	hlt	040
x1ac5:	lw	r1, [0x61]
x1ac7:	cw	r1, 0x8a
x1ac9:	jes	1
x1aca:	hlt	040
x1acb:	cw	r0, 0x400
x1acd:	jes	1
x1ace:	hlt	040
x1acf:	ki	int_store
x1ad1:	lw	r1, [int_store]
x1ad3:	cwt	r1, 0
x1ad4:	jes	1
x1ad5:	hlt	040
x1ad6:	lw	r1, int00
x1ad8:	rw	r1, 0x40
x1ada:	lj	reset
x1adc:	lw	r1, 0xffc0
x1ade:	rw	r1, mask
x1ae0:	im	mask
x1ae2:	hlt	077

reset:	.res	1
	lw	r2, stack
	lw	r1, stack-intproc+1
resetloop:
	ri	r2, 0
	irb	r1, resetloop
	lw	r1, stack
	rw	r1, stackp
	mcl
	uj	[reset]

; XPCT ir : 0xec3f
