; OPTS -c configs/minimal.cfg
;
; MERA-400 CPU test
;
; Based on the original "TP" test provided by the manufacturer.
; The only difference is that this version uses changed HLT codes.

	.equ interrupts 0x40
	.equ intproc 0xc1
	.equ start 0x100
	.equ int_illegal 0x46

	UJ    start

mask:	.word 0

	.org interrupts
	.word int00, int01, int02, int03, int04, int05, int06, int07
	.word int08, int09, int10, int11, int12, int13, int14, int15
	.word int16, int17, int18, int19, int20, int21, int22, int23
	.word int24, int25, int26, int27, int28, int29, int30, int31

exlp:	.word 0
stackp:	.word 0
stack:

	.org intproc
int01:	HLT   040	LIP
int02:	HLT   040	LIP
int03:	HLT   040	LIP
int04:	HLT   040	LIP
int05:	HLT   040	LIP
int06:	HLT   040	LIP
int07:	HLT   040	LIP
int08:	HLT   040	LIP
int09:	HLT   040	LIP
int10:	HLT   040	LIP
int11:	HLT   040	LIP
int12:	HLT   040	LIP
int13:	HLT   040	LIP
int14:	HLT   040	LIP
int15:	HLT   040	LIP
int16:	HLT   040	LIP
int17:	HLT   040	LIP
int18:	HLT   040	LIP
int19:	HLT   040	LIP
int20:	HLT   040	LIP
int21:	HLT   040	LIP
int22:	HLT   040	LIP
int23:	HLT   040	LIP
int24:	HLT   040	LIP
int25:	HLT   040	LIP
int26:	HLT   040	LIP
int27:	HLT   040	LIP
int28:	HLT   040	LIP
int29:	HLT   040	LIP
int30:	HLT   040	LIP
int31:	HLT   040	LIP

; test rozkazow w bloku podstawowym (Q=0, NB=0)

	.org start
x0100:	LW    r1, -1
x0102:	CW    r1, -1
x0104:	JES   1
x0105:	HLT   040
x0106:	LW    r1, -1
x0108:	CWT   r1, -1
x0109:	JES   1
x010a:	HLT   040
x010b:	LWT   r1, -1
x010c:	CW    r1, -1
x010e:	JES   1
x010f:	HLT   040
x0110:	LWT   r1, -1
x0111:	CWT   r1, -1
x0112:	JES   1
x0113:	HLT   040
x0114:	LWT   r1, 0
x0115:	LW    r1, [0x132b]
x0117:	CW    r1, 0xaaaa
x0119:	JES   1
x011a:	HLT   040
x011b:	LWT   r2, 0
x011c:	LW    r2, 0xaaaa
x011e:	CW    r2, 0xaaaa
x0120:	JES   1
x0121:	HLT   040
x0122:	LWT   r1, 0
x0123:	LW    r1, r2
x0124:	CW    r1, 0xaaaa
x0126:	JES   1
x0127:	HLT   040
x0128:	LW    r1, 0xaaaa
x012a:	LW    r2, [0x132b]
x012c:	CW    r2, r1
x012d:	JES   1
x012e:	HLT   040
x012f:	LW    r1, 0xaaaa
x0131:	RW    r1, 0x132e
x0133:	LW    r2, [0x132e]
x0135:	CW    r2, 0xaaaa
x0137:	JES   1
x0138:	HLT   040
x0139:	LW    r1, 0xaaaa
x013b:	CW    r1, [0x132b]
x013d:	JES   1
x013e:	HLT   040
x013f:	RZ    0x132b
x0141:	LWT   r1, 0
x0142:	CW    r1, [0x132b]
x0144:	JES   1
x0145:	HLT   040
x0146:	LW    r1, 0xaaaa
x0148:	RW    r1, 0x132b
x014a:	RZ    mask
x014c:	MCL
x014d:	CWT   r0, 0
x014e:	JES   1
x014f:	HLT   040

restart:
x0150:	LW    r2, [0x132b]
x0152:	RW    r2, 0x132e
x0154:	LWT   r1, 0
x0155:	TW    r1, 0x132e
x0157:	CW    r1, [0x132b]
x0159:	JES   1
x015a:	HLT   040
x015b:	LW    r2, [pat10]
x015d:	RZ    0x132e
x015f:	PW    r2, 0x132e
x0161:	LW    r1, [0x132e]
x0163:	CW    r1, [pat10]
x0165:	JES   1
x0166:	HLT   040
x0167:	LWT   r1, 0
x0168:	LWT   r7, 0
x0169:	LS    r1, -1
x016b:	CWT   r1, 0
x016c:	JES   1
x016d:	HLT   040
x016e:	LW    r1, [pat10]
x0170:	LWT   r7, -1
x0171:	LS    r1, [pat01]
x0173:	CW    r1, [pat01]
x0175:	JES   1
x0176:	HLT   040
x0177:	LWT   r1, -1
x0178:	LW    r7, [pat01]
x017a:	LS    r1, -1
x017c:	CWT   r1, -1
x017d:	JES   1
x017e:	HLT   040
x017f:	LWT   r1, -1
x0180:	LW    r7, [pat10]
x0182:	LS    r1, -1
x0184:	CWT   r1, -1
x0185:	JES   1
x0186:	HLT   040
x0187:	LW    r1, 0x132e
x0189:	RZ    0x132e
x018b:	RZ    0x132f
x018d:	RI    r1, [pat10]
x018f:	LW    r2, [pat10]
x0191:	CW    r2, [0x132e]
x0193:	JES   1
x0194:	HLT   040
x0195:	CW    r1, 0x132f
x0197:	JES   1
x0198:	HLT   040
x0199:	RI    r1, [pat10]
x019b:	LW    r2, [pat10]
x019d:	CW    r2, [0x132f]
x019f:	JES   1
x01a0:	HLT   040
x01a1:	CW    r1, 0x1330
x01a3:	JES   1
x01a4:	HLT   040
x01a5:	RJ    r7, 0x1a7
x01a7:	CW    r7, 0x1a7
x01a9:	JES   1
x01aa:	HLT   040
x01ab:	RZ    0x132e
x01ad:	LWT   r1, 0
x01ae:	IS    r1, 0x132e
x01b0:	HLT   040
x01b1:	LW    r1, [0x132e]
x01b3:	CWT   r1, 0
x01b4:	JES   1
x01b5:	HLT   040
x01b6:	RZ    0x132e
x01b8:	LWT   r1, -1
x01b9:	IS    r1, 0x132e
x01bb:	UJS   1
x01bc:	HLT   040
x01bd:	LW    r1, [0x132e]
x01bf:	CWT   r1, -1
x01c0:	JES   1
x01c1:	HLT   040
x01c2:	LWT   r1, -1
x01c3:	RW    r1, 0x132e
x01c5:	LWT   r1, 0
x01c6:	IS    r1, 0x132e
x01c8:	HLT   040
x01c9:	LW    r1, [0x132e]
x01cb:	CWT   r1, -1
x01cc:	JES   1
x01cd:	HLT   040
x01ce:	LWT   r1, -1
x01cf:	RW    r1, 0x132e
x01d1:	IS    r1, 0x132e
x01d3:	HLT   040
x01d4:	LW    r1, [0x132e]
x01d6:	CWT   r1, -1
x01d7:	JES   1
x01d8:	HLT   040
x01d9:	LW    r1, [pat01]
x01db:	RW    r1, 0x132e
x01dd:	IS    r1, 0x132e
x01df:	HLT   040
x01e0:	LW    r1, [0x132e]
x01e2:	CW    r1, [pat01]
x01e4:	JES   1
x01e5:	HLT   040
x01e6:	LW    r1, [pat10]
x01e8:	RW    r1, 0x132e
x01ea:	IS    r1, 0x132e
x01ec:	HLT   040
x01ed:	LW    r1, [0x132e]
x01ef:	CW    r1, [pat10]
x01f1:	JES   1
x01f2:	HLT   040
x01f3:	LW    r1, [pat01]
x01f5:	LW    r2, [pat10]
x01f7:	RW    r2, 0x132e
x01f9:	IS    r1, 0x132e
x01fb:	UJS   1
x01fc:	HLT   040
x01fd:	LW    r1, [0x132e]
x01ff:	CWT   r1, -1
x0200:	JES   1
x0201:	HLT   040
x0202:	LW    r1, [pat01]
x0204:	LW    r2, [pat10]
x0206:	RW    r1, 0x132e
x0208:	IS    r2, 0x132e
x020a:	UJS   1
x020b:	HLT   040
x020c:	LW    r1, [0x132e]
x020e:	CWT   r1, -1
x020f:	JES   1
x0210:	HLT   040
x0211:	LWT   r1, 0
x0212:	LWT   r2, 0
x0213:	BC    r1, r2
x0214:	UJS   1
x0215:	HLT   040
x0216:	LWT   r1, -1
x0217:	LWT   r2, -1
x0218:	BC    r1, r2
x0219:	UJS   1
x021a:	HLT   040
x021b:	LWT   r1, -1
x021c:	LWT   r2, 0
x021d:	BC    r1, r2
x021e:	UJS   1
x021f:	HLT   040
x0220:	LWT   r1, 0
x0221:	LWT   r2, -1
x0222:	BC    r1, r2
x0223:	HLT   040
x0224:	LW    r1, [pat01]
x0226:	LW    r2, r1
x0227:	BC    r1, r2
x0228:	UJS   1
x0229:	HLT   040
x022a:	LW    r1, [pat10]
x022c:	LW    r2, r1
x022d:	BC    r1, r2
x022e:	UJS   1
x022f:	HLT   040
x0230:	LW    r1, [pat01]
x0232:	LW    r2, [pat10]
x0234:	BC    r1, r2
x0235:	HLT   040
x0236:	LW    r1, [pat10]
x0238:	LW    r2, [pat01]
x023a:	BC    r1, r2
x023b:	HLT   040
x023c:	LWT   r1, 0
x023d:	RW    r1, 0x132e
x023f:	BM    r1, 0x132e
x0241:	HLT   040
x0242:	LWT   r1, -1
x0243:	RW    r1, 0x132e
x0245:	BM    r1, 0x132e
x0247:	HLT   040
x0248:	LWT   r1, -1
x0249:	RZ    0x132e
x024b:	BM    r1, 0x132e
x024d:	UJS   1
x024e:	HLT   040
x024f:	LWT   r1, 0
x0250:	LWT   r2, -1
x0251:	RW    r2, 0x132e
x0253:	BM    r1, 0x132e
x0255:	HLT   040
x0256:	LW    r1, [pat01]
x0258:	RW    r1, 0x132e
x025a:	BM    r1, 0x132e
x025c:	HLT   040
x025d:	LW    r1, [pat10]
x025f:	RW    r1, 0x132e
x0261:	BM    r1, 0x132e
x0263:	HLT   040
x0264:	LW    r1, [pat01]
x0266:	LW    r2, [pat10]
x0268:	RW    r2, 0x132e
x026a:	BM    r1, 0x132e
x026c:	UJS   1
x026d:	HLT   040
x026e:	LW    r1, [pat10]
x0270:	LW    r2, [pat01]
x0272:	RW    r2, 0x132e
x0274:	BM    r1, 0x132e
x0276:	UJS   1
x0277:	HLT   040
x0278:	LWT   r1, 0
x0279:	LW    r2, r1
x027a:	BN    r1, r2
x027b:	HLT   040
x027c:	LWT   r1, -1
x027d:	LW    r2, r1
x027e:	BN    r1, r2
x027f:	UJS   1
x0280:	HLT   040
x0281:	LWT   r1, -1
x0282:	LWT   r2, 0
x0283:	BN    r1, r2
x0284:	HLT   040
x0285:	LWT   r1, 0
x0286:	LWT   r2, -1
x0287:	BN    r1, r2
x0288:	HLT   040
x0289:	LW    r1, [pat01]
x028b:	LW    r2, r1
x028c:	BN    r1, r2
x028d:	UJS   1
x028e:	HLT   040
x028f:	LW    r1, [pat10]
x0291:	LW    r2, r1
x0292:	BN    r1, r2
x0293:	UJS   1
x0294:	HLT   040
x0295:	LW    r1, [pat01]
x0297:	LW    r2, [pat10]
x0299:	BN    r1, r2
x029a:	HLT   040
x029b:	LW    r1, [pat10]
x029d:	LW    r2, [pat01]
x029f:	BN    r1, r2
x02a0:	HLT   040
x02a1:	LWT   r1, 0
x02a2:	LWT   r2, 0
x02a3:	BB    r1, r2
x02a4:	HLT   040
x02a5:	LWT   r1, -1
x02a6:	LW    r2, r1
x02a7:	BB    r1, r2
x02a8:	HLT   040
x02a9:	LWT   r1, -1
x02aa:	LWT   r2, 0
x02ab:	BB    r1, r2
x02ac:	HLT   040
x02ad:	LWT   r1, 0
x02ae:	LWT   r2, -1
x02af:	BB    r1, r2
x02b0:	UJS   1
x02b1:	HLT   040
x02b2:	LW    r1, [pat01]
x02b4:	LW    r2, r1
x02b5:	BB    r1, r2
x02b6:	HLT   040
x02b7:	LW    r1, [pat10]
x02b9:	LW    r2, r1
x02ba:	BB    r1, r2
x02bb:	HLT   040
x02bc:	LW    r1, [pat01]
x02be:	LW    r2, [pat10]
x02c0:	BB    r1, r2
x02c1:	UJS   1
x02c2:	HLT   040
x02c3:	LW    r1, [pat10]
x02c5:	LW    r2, [pat01]
x02c7:	BB    r1, r2
x02c8:	UJS   1
x02c9:	HLT   040
x02ca:	LWT   r1, 0
x02cb:	LWT   r7, 0
x02cc:	LWT   r2, -1
x02cd:	BS    r1, r2
x02ce:	HLT   040
x02cf:	LW    r1, [pat10]
x02d1:	LWT   r7, -1
x02d2:	LW    r2, [pat01]
x02d4:	BS    r1, r2
x02d5:	UJS   1
x02d6:	HLT   040
x02d7:	LWT   r1, -1
x02d8:	LWT   r2, -1
x02d9:	LW    r7, [pat01]
x02db:	BS    r1, r2
x02dc:	HLT   040
x02dd:	LWT   r1, -1
x02de:	LWT   r2, -1
x02df:	LW    r7, [pat10]
x02e1:	BS    r1, r2
x02e2:	HLT   040
x02e3:	LWT   r0, 0
x02e4:	LWT   r1, 0
x02e5:	LWT   r2, -1
x02e6:	OR    r1, r2
x02e7:	CWT   r0, 0
x02e8:	JES   1
x02e9:	HLT   040
x02ea:	CWT   r1, -1
x02eb:	JES   1
x02ec:	HLT   040
x02ed:	LWT   r0, 0
x02ee:	LWT   r1, -1
x02ef:	LWT   r2, 0
x02f0:	OR    r1, r2
x02f1:	CWT   r0, 0
x02f2:	JES   1
x02f3:	HLT   040
x02f4:	CWT   r1, -1
x02f5:	JES   1
x02f6:	HLT   040
x02f7:	LWT   r0, 0
x02f8:	LWT   r1, 0
x02f9:	LWT   r2, 0
x02fa:	OR    r1, r2
x02fb:	CW    r0, 0x8000
x02fd:	JES   1
x02fe:	HLT   040
x02ff:	CWT   r1, 0
x0300:	JES   1
x0301:	HLT   040
x0302:	LWT   r0, 0
x0303:	LWT   r1, -1
x0304:	LWT   r2, -1
x0305:	OR    r1, r2
x0306:	CWT   r0, 0
x0307:	JES   1
x0308:	HLT   040
x0309:	CWT   r1, -1
x030a:	JES   1
x030b:	HLT   040
x030c:	LWT   r0, 0
x030d:	LW    r1, [pat01]
x030f:	LW    r2, r1
x0310:	OR    r1, r2
x0311:	CWT   r0, 0
x0312:	JES   1
x0313:	HLT   040
x0314:	CW    r1, [pat01]
x0316:	JES   1
x0317:	HLT   040
x0318:	LWT   r0, 0
x0319:	LW    r1, [pat10]
x031b:	LW    r2, r1
x031c:	OR    r1, r2
x031d:	CWT   r0, 0
x031e:	JES   1
x031f:	HLT   040
x0320:	CW    r1, [pat10]
x0322:	JES   1
x0323:	HLT   040
x0324:	LWT   r0, 0
x0325:	LW    r1, [pat10]
x0327:	OR    r1, [pat01]
x0329:	CWT   r0, 0
x032a:	JES   1
x032b:	HLT   040
x032c:	CWT   r1, -1
x032d:	JES   1
x032e:	HLT   040
x032f:	LWT   r0, 0
x0330:	LW    r1, [pat01]
x0332:	OR    r1, [pat10]
x0334:	CWT   r0, 0
x0335:	JES   1
x0336:	HLT   040
x0337:	CWT   r1, -1
x0338:	JES   1
x0339:	HLT   040
x033a:	LWT   r0, 0
x033b:	LWT   r1, -1
x033c:	RW    r1, 0x132e
x033e:	LWT   r1, 0
x033f:	OM    r1, 0x132e
x0341:	CWT   r0, 0
x0342:	JES   1
x0343:	HLT   040
x0344:	LW    r1, [0x132e]
x0346:	CWT   r1, -1
x0347:	JES   1
x0348:	HLT   040
x0349:	LWT   r0, 0
x034a:	RZ    0x132e
x034c:	LWT   r1, -1
x034d:	OM    r1, 0x132e
x034f:	CWT   r0, 0
x0350:	JES   1
x0351:	HLT   040
x0352:	LW    r1, [0x132e]
x0354:	CWT   r1, -1
x0355:	JES   1
x0356:	HLT   040
x0357:	LWT   r0, 0
x0358:	RZ    0x132e
x035a:	LWT   r1, 0
x035b:	OM    r1, 0x132e
x035d:	CW    r0, 0x8000
x035f:	JES   1
x0360:	HLT   040
x0361:	LW    r1, [0x132e]
x0363:	CWT   r1, 0
x0364:	JES   1
x0365:	HLT   040
x0366:	LWT   r0, 0
x0367:	LWT   r1, -1
x0368:	RW    r1, 0x132e
x036a:	OM    r1, 0x132e
x036c:	CWT   r0, 0
x036d:	JES   1
x036e:	HLT   040
x036f:	LW    r1, [0x132e]
x0371:	CWT   r1, -1
x0372:	JES   1
x0373:	HLT   040
x0374:	LWT   r0, 0
x0375:	LW    r1, [pat01]
x0377:	RW    r1, 0x132e
x0379:	OM    r1, 0x132e
x037b:	CWT   r0, 0
x037c:	JES   1
x037d:	HLT   040
x037e:	LW    r1, [0x132e]
x0380:	CW    r1, [pat01]
x0382:	JES   1
x0383:	HLT   040
x0384:	LWT   r0, 0
x0385:	LW    r1, [pat10]
x0387:	RW    r1, 0x132e
x0389:	OM    r1, 0x132e
x038b:	CWT   r0, 0
x038c:	JES   1
x038d:	HLT   040
x038e:	LW    r1, [0x132e]
x0390:	CW    r1, [pat10]
x0392:	JES   1
x0393:	HLT   040
x0394:	LWT   r0, 0
x0395:	LW    r1, [pat01]
x0397:	RW    r1, 0x132e
x0399:	LW    r1, [pat10]
x039b:	OM    r1, 0x132e
x039d:	CWT   r0, 0
x039e:	JES   1
x039f:	HLT   040
x03a0:	LW    r1, [0x132e]
x03a2:	CWT   r1, -1
x03a3:	JES   1
x03a4:	HLT   040
x03a5:	LWT   r0, 0
x03a6:	LW    r1, [pat10]
x03a8:	RW    r1, 0x132e
x03aa:	LW    r1, [pat01]
x03ac:	OM    r1, 0x132e
x03ae:	CWT   r0, 0
x03af:	JES   1
x03b0:	HLT   040
x03b1:	LW    r1, [0x132e]
x03b3:	CWT   r1, -1
x03b4:	JES   1
x03b5:	HLT   040
x03b6:	LWT   r0, 0
x03b7:	LWT   r1, 0
x03b8:	NR    r1, -1
x03ba:	CW    r0, 0x8000
x03bc:	JES   1
x03bd:	HLT   040
x03be:	CWT   r1, 0
x03bf:	JES   1
x03c0:	HLT   040
x03c1:	LWT   r0, 0
x03c2:	LWT   r1, -1
x03c3:	NR    r1, 0x0
x03c5:	CW    r0, 0x8000
x03c7:	JES   1
x03c8:	HLT   040
x03c9:	CWT   r1, 0
x03ca:	JES   1
x03cb:	HLT   040
x03cc:	LWT   r0, 0
x03cd:	LWT   r1, 0
x03ce:	LW    r2, r1
x03cf:	NR    r1, r2
x03d0:	CW    r0, 0x8000
x03d2:	JES   1
x03d3:	HLT   040
x03d4:	CWT   r1, 0
x03d5:	JES   1
x03d6:	HLT   040
x03d7:	LWT   r0, 0
x03d8:	LWT   r1, -1
x03d9:	LW    r2, r1
x03da:	NR    r1, r2
x03db:	CWT   r0, 0
x03dc:	JES   1
x03dd:	HLT   040
x03de:	CWT   r1, -1
x03df:	JES   1
x03e0:	HLT   040
x03e1:	LWT   r0, 0
x03e2:	LW    r1, [pat01]
x03e4:	LW    r2, r1
x03e5:	NR    r1, r2
x03e6:	CWT   r0, 0
x03e7:	JES   1
x03e8:	HLT   040
x03e9:	CW    r1, [pat01]
x03eb:	JES   1
x03ec:	HLT   040
x03ed:	LWT   r0, 0
x03ee:	LW    r1, [pat10]
x03f0:	LW    r2, r1
x03f1:	NR    r1, r2
x03f2:	CWT   r0, 0
x03f3:	JES   1
x03f4:	HLT   040
x03f5:	CW    r1, [pat10]
x03f7:	JES   1
x03f8:	HLT   040
x03f9:	LWT   r0, 0
x03fa:	LW    r1, [pat10]
x03fc:	NR    r1, [pat01]
x03fe:	CW    r0, 0x8000
x0400:	JES   1
x0401:	HLT   040
x0402:	CWT   r1, 0
x0403:	JES   1
x0404:	HLT   040
x0405:	LWT   r0, 0
x0406:	LW    r1, [pat01]
x0408:	NR    r1, [pat10]
x040a:	CW    r0, 0x8000
x040c:	JES   1
x040d:	HLT   040
x040e:	CWT   r1, 0
x040f:	JES   1
x0410:	HLT   040
x0411:	LWT   r0, 0
x0412:	LWT   r1, -1
x0413:	RW    r1, 0x132e
x0415:	LWT   r1, 0
x0416:	NM    r1, 0x132e
x0418:	CW    r0, 0x8000
x041a:	JES   1
x041b:	HLT   040
x041c:	LW    r1, [0x132e]
x041e:	CWT   r1, 0
x041f:	JES   1
x0420:	HLT   040
x0421:	LWT   r0, 0
x0422:	LWT   r1, -1
x0423:	RZ    0x132e
x0425:	NM    r1, 0x132e
x0427:	CW    r0, 0x8000
x0429:	JES   1
x042a:	HLT   040
x042b:	LW    r1, [0x132e]
x042d:	CWT   r1, 0
x042e:	JES   1
x042f:	HLT   040
x0430:	LWT   r0, 0
x0431:	RZ    0x132e
x0433:	LWT   r1, 0
x0434:	NM    r1, 0x132e
x0436:	CW    r0, 0x8000
x0438:	JES   1
x0439:	HLT   040
x043a:	LW    r1, [0x132e]
x043c:	CWT   r1, 0
x043d:	JES   1
x043e:	HLT   040
x043f:	LWT   r0, 0
x0440:	LWT   r1, -1
x0441:	RW    r1, 0x132e
x0443:	NM    r1, 0x132e
x0445:	CWT   r0, 0
x0446:	JES   1
x0447:	HLT   040
x0448:	LW    r1, [0x132e]
x044a:	CWT   r1, -1
x044b:	JES   1
x044c:	HLT   040
x044d:	LWT   r0, 0
x044e:	LW    r1, [pat01]
x0450:	RW    r1, 0x132e
x0452:	NM    r1, 0x132e
x0454:	CWT   r0, 0
x0455:	JES   1
x0456:	HLT   040
x0457:	LW    r1, [0x132e]
x0459:	CW    r1, [pat01]
x045b:	JES   1
x045c:	HLT   040
x045d:	LWT   r0, 0
x045e:	LW    r1, [pat10]
x0460:	RW    r1, 0x132e
x0462:	NM    r1, 0x132e
x0464:	CWT   r0, 0
x0465:	JES   1
x0466:	HLT   040
x0467:	LW    r1, [0x132e]
x0469:	CW    r1, [pat10]
x046b:	JES   1
x046c:	HLT   040
x046d:	LWT   r0, 0
x046e:	LW    r1, [pat10]
x0470:	RW    r1, 0x132e
x0472:	LW    r1, [pat01]
x0474:	NM    r1, 0x132e
x0476:	CW    r0, 0x8000
x0478:	JES   1
x0479:	HLT   040
x047a:	LW    r1, [0x132e]
x047c:	CWT   r1, 0
x047d:	JES   1
x047e:	HLT   040
x047f:	LWT   r0, 0
x0480:	LW    r1, [pat01]
x0482:	RW    r1, 0x132e
x0484:	LW    r1, [pat10]
x0486:	NM    r1, 0x132e
x0488:	CW    r0, 0x8000
x048a:	JES   1
x048b:	HLT   040
x048c:	LW    r1, [0x132e]
x048e:	CWT   r1, 0
x048f:	JES   1
x0490:	HLT   040
x0491:	LWT   r0, 0
x0492:	LWT   r1, 0
x0493:	LWT   r2, -1
x0494:	ER    r1, r2
x0495:	CW    r0, 0x8000
x0497:	JES   1
x0498:	HLT   040
x0499:	CWT   r1, 0
x049a:	JES   1
x049b:	HLT   040
x049c:	LWT   r0, 0
x049d:	LWT   r1, -1
x049e:	LWT   r2, 0
x049f:	ER    r1, r2
x04a0:	CWT   r0, 0
x04a1:	JES   1
x04a2:	HLT   040
x04a3:	CWT   r1, -1
x04a4:	JES   1
x04a5:	HLT   040
x04a6:	LWT   r0, 0
x04a7:	LWT   r1, 0
x04a8:	LWT   r2, 0
x04a9:	ER    r1, r2
x04aa:	CW    r0, 0x8000
x04ac:	JES   1
x04ad:	HLT   040
x04ae:	CWT   r1, 0
x04af:	JES   1
x04b0:	HLT   040
x04b1:	LWT   r0, 0
x04b2:	LWT   r1, -1
x04b3:	LWT   r2, -1
x04b4:	ER    r1, r2
x04b5:	CW    r0, 0x8000
x04b7:	JES   1
x04b8:	HLT   040
x04b9:	CWT   r1, 0
x04ba:	JES   1
x04bb:	HLT   040
x04bc:	LWT   r0, 0
x04bd:	LW    r1, [pat01]
x04bf:	LW    r2, r1
x04c0:	ER    r1, r2
x04c1:	CW    r0, 0x8000
x04c3:	JES   1
x04c4:	HLT   040
x04c5:	CWT   r1, 0
x04c6:	JES   1
x04c7:	HLT   040
x04c8:	LWT   r0, 0
x04c9:	LW    r1, [pat10]
x04cb:	LW    r2, r1
x04cc:	ER    r1, r2
x04cd:	CW    r0, 0x8000
x04cf:	JES   1
x04d0:	HLT   040
x04d1:	CWT   r1, 0
x04d2:	JES   1
x04d3:	HLT   040
x04d4:	LWT   r0, 0
x04d5:	LW    r1, [pat01]
x04d7:	LW    r2, [pat10]
x04d9:	ER    r1, r2
x04da:	CWT   r0, 0
x04db:	JES   1
x04dc:	HLT   040
x04dd:	CW    r1, [pat01]
x04df:	JES   1
x04e0:	HLT   040
x04e1:	LWT   r0, 0
x04e2:	LW    r1, [pat10]
x04e4:	LW    r2, [pat01]
x04e6:	ER    r1, r2
x04e7:	CWT   r0, 0
x04e8:	JES   1
x04e9:	HLT   040
x04ea:	CW    r1, [pat10]
x04ec:	JES   1
x04ed:	HLT   040
x04ee:	LWT   r0, 0
x04ef:	LWT   r1, -1
x04f0:	RW    r1, 0x132e
x04f2:	LWT   r1, 0
x04f3:	EM    r1, 0x132e
x04f5:	CWT   r0, 0
x04f6:	JES   1
x04f7:	HLT   040
x04f8:	LW    r1, [0x132e]
x04fa:	CWT   r1, -1
x04fb:	JES   1
x04fc:	HLT   040
x04fd:	LWT   r0, 0
x04fe:	LWT   r1, 0
x04ff:	RW    r1, 0x132e
x0501:	LWT   r1, -1
x0502:	EM    r1, 0x132e
x0504:	CW    r0, 0x8000
x0506:	JES   1
x0507:	HLT   040
x0508:	LW    r1, [0x132e]
x050a:	CWT   r1, 0
x050b:	JES   1
x050c:	HLT   040
x050d:	LWT   r0, 0
x050e:	LWT   r1, 0
x050f:	RW    r1, 0x132e
x0511:	EM    r1, 0x132e
x0513:	CW    r0, 0x8000
x0515:	JES   1
x0516:	HLT   040
x0517:	LW    r1, [0x132e]
x0519:	CWT   r1, 0
x051a:	JES   1
x051b:	HLT   040
x051c:	LWT   r0, 0
x051d:	LWT   r1, -1
x051e:	RW    r1, 0x132e
x0520:	EM    r1, 0x132e
x0522:	CW    r0, 0x8000
x0524:	JES   1
x0525:	HLT   040
x0526:	LW    r1, [0x132e]
x0528:	CWT   r1, 0
x0529:	JES   1
x052a:	HLT   040
x052b:	LWT   r0, 0
x052c:	LW    r1, [pat01]
x052e:	RW    r1, 0x132e
x0530:	EM    r1, 0x132e
x0532:	CW    r0, 0x8000
x0534:	JES   1
x0535:	HLT   040
x0536:	LW    r1, [0x132e]
x0538:	CWT   r1, 0
x0539:	JES   1
x053a:	HLT   040
x053b:	LWT   r0, 0
x053c:	LW    r1, [pat10]
x053e:	RW    r1, 0x132e
x0540:	EM    r1, 0x132e
x0542:	CW    r0, 0x8000
x0544:	JES   1
x0545:	HLT   040
x0546:	LW    r1, [0x132e]
x0548:	CWT   r1, 0
x0549:	JES   1
x054a:	HLT   040
x054b:	LWT   r0, 0
x054c:	LW    r1, [pat10]
x054e:	RW    r1, 0x132e
x0550:	LW    r1, [pat01]
x0552:	EM    r1, 0x132e
x0554:	CWT   r0, 0
x0555:	JES   1
x0556:	HLT   040
x0557:	LW    r1, [0x132e]
x0559:	CW    r1, [pat10]
x055b:	JES   1
x055c:	HLT   040
x055d:	LWT   r0, 0
x055e:	LW    r1, [pat01]
x0560:	RW    r1, 0x132e
x0562:	LW    r1, [pat10]
x0564:	EM    r1, 0x132e
x0566:	CWT   r0, 0
x0567:	JES   1
x0568:	HLT   040
x0569:	LW    r1, [0x132e]
x056b:	CW    r1, [pat01]
x056d:	JES   1
x056e:	HLT   040
x056f:	LWT   r0, 0
x0570:	LWT   r1, 0
x0571:	LWT   r2, -1
x0572:	XR    r1, r2
x0573:	CWT   r0, 0
x0574:	JES   1
x0575:	HLT   040
x0576:	CWT   r1, -1
x0577:	JES   1
x0578:	HLT   040
x0579:	LWT   r0, 0
x057a:	LWT   r1, -1
x057b:	LWT   r2, 0
x057c:	XR    r1, r2
x057d:	CWT   r0, 0
x057e:	JES   1
x057f:	HLT   040
x0580:	CWT   r1, -1
x0581:	JES   1
x0582:	HLT   040
x0583:	LWT   r0, 0
x0584:	LWT   r1, 0
x0585:	LWT   r2, 0
x0586:	XR    r1, r2
x0587:	CW    r0, 0x8000
x0589:	JES   1
x058a:	HLT   040
x058b:	CWT   r1, 0
x058c:	JES   1
x058d:	HLT   040
x058e:	LWT   r0, 0
x058f:	LWT   r1, -1
x0590:	LWT   r2, -1
x0591:	XR    r1, r2
x0592:	CW    r0, 0x8000
x0594:	JES   1
x0595:	HLT   040
x0596:	CWT   r1, 0
x0597:	JES   1
x0598:	HLT   040
x0599:	LWT   r0, 0
x059a:	LW    r1, [pat10]
x059c:	XR    r1, [pat10]
x059e:	CW    r0, 0x8000
x05a0:	JES   1
x05a1:	HLT   040
x05a2:	CWT   r1, 0
x05a3:	JES   1
x05a4:	HLT   040
x05a5:	LWT   r0, 0
x05a6:	LW    r1, [pat01]
x05a8:	XR    r1, [pat01]
x05aa:	CW    r0, 0x8000
x05ac:	JES   1
x05ad:	HLT   040
x05ae:	CWT   r1, 0
x05af:	JES   1
x05b0:	HLT   040
x05b1:	LWT   r0, 0
x05b2:	LW    r1, [pat01]
x05b4:	LW    r2, [pat10]
x05b6:	XR    r1, r2
x05b7:	CWT   r0, 0
x05b8:	JES   1
x05b9:	HLT   040
x05ba:	CWT   r1, -1
x05bb:	JES   1
x05bc:	HLT   040
x05bd:	LWT   r0, 0
x05be:	LW    r1, [pat10]
x05c0:	LW    r2, [pat01]
x05c2:	XR    r1, r2
x05c3:	CWT   r0, 0
x05c4:	JES   1
x05c5:	HLT   040
x05c6:	CWT   r1, -1
x05c7:	JES   1
x05c8:	HLT   040
x05c9:	LWT   r0, 0
x05ca:	LWT   r1, -1
x05cb:	RW    r1, 0x132e
x05cd:	LWT   r1, 0
x05ce:	XM    r1, 0x132e
x05d0:	CWT   r0, 0
x05d1:	JES   1
x05d2:	HLT   040
x05d3:	LW    r1, [0x132e]
x05d5:	CWT   r1, -1
x05d6:	JES   1
x05d7:	HLT   040
x05d8:	LWT   r0, 0
x05d9:	RZ    0x132e
x05db:	LWT   r1, -1
x05dc:	XM    r1, 0x132e
x05de:	CWT   r0, 0
x05df:	JES   1
x05e0:	HLT   040
x05e1:	LW    r1, [0x132e]
x05e3:	CWT   r1, -1
x05e4:	JES   1
x05e5:	HLT   040
x05e6:	LWT   r0, 0
x05e7:	RZ    0x132e
x05e9:	LWT   r1, 0
x05ea:	XM    r1, 0x132e
x05ec:	CW    r0, 0x8000
x05ee:	JES   1
x05ef:	HLT   040
x05f0:	LW    r1, [0x132e]
x05f2:	CWT   r1, 0
x05f3:	JES   1
x05f4:	HLT   040
x05f5:	LWT   r0, 0
x05f6:	LWT   r1, -1
x05f7:	RW    r1, 0x132e
x05f9:	XM    r1, 0x132e
x05fb:	CW    r0, 0x8000
x05fd:	JES   1
x05fe:	HLT   040
x05ff:	LW    r1, [0x132e]
x0601:	CWT   r1, 0
x0602:	JES   1
x0603:	HLT   040
x0604:	LWT   r0, 0
x0605:	LW    r1, [pat01]
x0607:	RW    r1, 0x132e
x0609:	XM    r1, 0x132e
x060b:	CW    r0, 0x8000
x060d:	JES   1
x060e:	HLT   040
x060f:	LW    r1, [0x132e]
x0611:	CWT   r1, 0
x0612:	JES   1
x0613:	HLT   040
x0614:	LWT   r0, 0
x0615:	LW    r1, [pat10]
x0617:	RW    r1, 0x132e
x0619:	XM    r1, 0x132e
x061b:	CW    r0, 0x8000
x061d:	JES   1
x061e:	HLT   040
x061f:	LW    r1, [0x132e]
x0621:	CWT   r1, 0
x0622:	JES   1
x0623:	HLT   040
x0624:	LWT   r0, 0
x0625:	LW    r1, [pat10]
x0627:	RW    r1, 0x132e
x0629:	LW    r1, [pat01]
x062b:	XM    r1, 0x132e
x062d:	CWT   r0, 0
x062e:	JES   1
x062f:	HLT   040
x0630:	LW    r1, [0x132e]
x0632:	CWT   r1, -1
x0633:	JES   1
x0634:	HLT   040
x0635:	LWT   r0, 0
x0636:	LW    r1, [pat01]
x0638:	RW    r1, 0x132e
x063a:	LW    r1, [pat10]
x063c:	XM    r1, 0x132e
x063e:	CWT   r0, 0
x063f:	JES   1
x0640:	HLT   040
x0641:	LW    r1, [0x132e]
x0643:	CWT   r1, -1
x0644:	JES   1
x0645:	HLT   040
x0646:	LWT   r0, 0
x0647:	LWT   r1, -1
x0648:	LWT   r2, 1
x0649:	AW    r1, r2
x064a:	CW    r0, 0x9000
x064c:	JES   1
x064d:	HLT   040
x064e:	CWT   r1, 0
x064f:	JES   1
x0650:	HLT   040
x0651:	LWT   r0, 0
x0652:	LW    r1, [pat10]
x0654:	AW    r1, r1
x0655:	CW    r0, 0x7000
x0657:	JES   1
x0658:	HLT   040
x0659:	CW    r1, 0x5554
x065b:	JES   1
x065c:	HLT   040
x065d:	LWT   r0, 0
x065e:	LW    r1, 0x5555
x0660:	AW    r1, r1
x0661:	CW    r0, 0x2000
x0663:	JES   1
x0664:	HLT   040
x0665:	CW    r1, 0xaaaa
x0667:	JES   1
x0668:	HLT   040
x0669:	LWT   r0, 0
x066a:	LWT   r1, -1
x066b:	LW    r2, 0x7fff
x066d:	AW    r1, r2
x066e:	CW    r0, 0x1000
x0670:	JES   1
x0671:	HLT   040
x0672:	CW    r1, 0x7ffe
x0674:	JES   1
x0675:	HLT   040
x0676:	LWT   r1, -1
x0677:	LWT   r0, 0
x0678:	LW    r2, r1
x0679:	AW    r1, r2
x067a:	CW    r0, 0x5000
x067c:	JES   1
x067d:	HLT   040
x067e:	CWT   r1, -2
x067f:	JES   1
x0680:	HLT   040
x0681:	LWT   r0, 0
x0682:	LW    r1, 0x7fff
x0684:	LW    r2, 0x7fff
x0686:	AW    r1, r2
x0687:	CW    r0, 0x2000
x0689:	JES   1
x068a:	HLT   040
x068b:	CWT   r1, -2
x068c:	JES   1
x068d:	HLT   040
x068e:	LWT   r0, 0
x068f:	LWT   r1, -1
x0690:	LW    r2, 0x8000
x0692:	AW    r1, r2
x0693:	CW    r0, 0x7000
x0695:	JES   1
x0696:	HLT   040
x0697:	CW    r1, 0x7fff
x0699:	JES   1
x069a:	HLT   040
x069b:	LWT   r0, 0
x069c:	LW    r1, 0x8000
x069e:	LW    r2, r1
x069f:	AW    r1, r2
x06a0:	CW    r0, 0x7000
x06a2:	JES   1
x06a3:	HLT   040
x06a4:	CWT   r1, 0
x06a5:	JES   1
x06a6:	HLT   040
x06a7:	LW    r0, 0x1000
x06a9:	LW    r1, 0xfffd
x06ab:	LWT   r2, 1
x06ac:	AC    r1, r2
x06ad:	CW    r0, 0x4000
x06af:	JES   1
x06b0:	HLT   040
x06b1:	CWT   r1, -1
x06b2:	JES   1
x06b3:	HLT   040
x06b4:	LW    r0, 0x1000
x06b6:	LW    r1, 0xfffe
x06b8:	LWT   r2, 1
x06b9:	AC    r1, r2
x06ba:	CW    r0, 0x9000
x06bc:	JES   1
x06bd:	HLT   040
x06be:	CWT   r1, 0
x06bf:	JES   1
x06c0:	HLT   040
x06c1:	LW    r0, 0x1000
x06c3:	LW    r1, 0x7fff
x06c5:	AC    r1, 0x0
x06c7:	CW    r0, 0x2000
x06c9:	JES   1
x06ca:	HLT   040
x06cb:	CW    r1, 0x8000
x06cd:	JES   1
x06ce:	HLT   040
x06cf:	LWT   r0, 0
x06d0:	LWT   r1, 0
x06d1:	LW    r2, r1
x06d2:	SW    r1, r2
x06d3:	CW    r0, 0x9000
x06d5:	JES   1
x06d6:	HLT   040
x06d7:	CWT   r1, 0
x06d8:	JES   1
x06d9:	HLT   040
x06da:	LWT   r0, 0
x06db:	LW    r1, 0x8000
x06dd:	LWT   r2, 1
x06de:	SW    r1, r2
x06df:	CW    r0, 0x7000
x06e1:	JES   1
x06e2:	HLT   040
x06e3:	CW    r1, 0x7fff
x06e5:	JES   1
x06e6:	HLT   040
x06e7:	LWT   r0, 0
x06e8:	LWT   r1, 0
x06e9:	LWT   r2, -1
x06ea:	CL    r1, r2
x06eb:	CW    r0, 0x800
x06ed:	JES   1
x06ee:	HLT   040
x06ef:	CL    r2, r1
x06f0:	CW    r0, 0x200
x06f2:	JES   1
x06f3:	HLT   040
x06f4:	LW    r1, 0x7fff
x06f6:	LW    r2, 0x8000
x06f8:	CL    r1, r2
x06f9:	CW    r0, 0x800
x06fb:	JES   1
x06fc:	HLT   040
x06fd:	LW    r1, [pat10]
x06ff:	LW    r2, r1
x0700:	CL    r1, r2
x0701:	CW    r0, 0x400
x0703:	JES   1
x0704:	HLT   040
x0705:	LWT   r1, 0
x0706:	LJ    x070d
x0708:	CW    r1, 0x708
x070a:	JES   1
x070b:	HLT   040
x070c:	UJS   10
x070d:	.word 0
x070e:	LW    r1, [x070d]
x0710:	CW    r1, 0x708
x0712:	JES   1
x0713:	HLT   040
x0714:	LW    r1, 0x708
x0716:	UJS   -15; -> 0x0708
x0717:	UJ    0x71a
x0719:	HLT   040
x071a:	LW    r1, 0x71e
x071c:	UJ    r1
x071d:	HLT   040
x071e:	LW    r1, 0x725
x0720:	RW    r1, 0x132e
x0722:	UJ    [0x132e]
x0724:	HLT   040
x0725:	LW    r1, 0x72d
x0727:	RW    r1, 0x132e
x0729:	LW    r1, 0x132e
x072b:	UJ    [r1]
x072c:	HLT   040
x072d:	LJ    0x854
x072f:	LD    0x1338
x0731:	CW    r1, [pat01]
x0733:	JES   1
x0734:	HLT   040
x0735:	CW    r2, [pat10]
x0737:	JES   1
x0738:	HLT   040
x0739:	CWT   r3, -1
x073a:	JES   1
x073b:	UJS   11
x073c:	CWT   r4, -1
x073d:	JES   1
x073e:	UJS   8  ; -> 0x0747
x073f:	CWT   r5, -1
x0740:	JES   1
x0741:	UJS   5  ; -> 0x0747
x0742:	CWT   r6, -1
x0743:	JES   1
x0744:	UJS   2  ; -> 0x0747
x0745:	CWT   r7, -1
x0746:	JES   1
x0747:	HLT   040
x0748:	LJ    0x854
x074a:	LF    0x1338
x074c:	CW    r1, [pat01]
x074e:	JES   1
x074f:	HLT   040
x0750:	CW    r2, [pat10]
x0752:	JES   1
x0753:	HLT   040
x0754:	CW    r3, [0x133a]
x0756:	JES   1
x0757:	HLT   040
x0758:	CWT   r4, -1
x0759:	JES   1
x075a:	UJS   8  ; -> 0x0763
x075b:	CWT   r5, -1
x075c:	JES   1
x075d:	UJS   5  ; -> 0x0763
x075e:	CWT   r6, -1
x075f:	JES   1
x0760:	UJS   2  ; -> 0x0763
x0761:	CWT   r7, -1
x0762:	JES   1
x0763:	HLT   040
x0764:	LJ    0x854
x0766:	LA    0x1338
x0768:	CW    r1, [pat01]
x076a:	JES   1
x076b:	HLT   040
x076c:	CW    r2, [pat10]
x076e:	JES   1
x076f:	HLT   040
x0770:	CW    r3, [0x133a]
x0772:	JES   1
x0773:	HLT   040
x0774:	CW    r4, [0x133b]
x0776:	JES   1
x0777:	HLT   040
x0778:	CW    r5, [0x133c]
x077a:	JES   1
x077b:	HLT   040
x077c:	CW    r6, [0x133d]
x077e:	JES   1
x077f:	HLT   040
x0780:	CW    r7, [0x133e]
x0782:	JES   1
x0783:	HLT   040
x0784:	LJ    0x854
x0786:	LL    0x1338
x0788:	CW    r5, [pat01]
x078a:	JES   1
x078b:	HLT   040
x078c:	CW    r6, [pat10]
x078e:	JES   1
x078f:	HLT   040
x0790:	CW    r7, [0x133a]
x0792:	JES   1
x0793:	HLT   040
x0794:	CWT   r1, -1
x0795:	JES   1
x0796:	UJS   8  ; -> 0x079f
x0797:	CWT   r2, -1
x0798:	JES   1
x0799:	UJS   5  ; -> 0x079f
x079a:	CWT   r3, -1
x079b:	JES   1
x079c:	UJS   2  ; -> 0x079f
x079d:	CWT   r4, -1
x079e:	JES   1
x079f:	HLT   040
x07a0:	LJ    0x854
x07a2:	TD    0x1338
x07a4:	CW    r1, [pat01]
x07a6:	JES   1
x07a7:	HLT   040
x07a8:	CW    r2, [pat10]
x07aa:	JES   1
x07ab:	HLT   040
x07ac:	CWT   r3, -1
x07ad:	JES   1
x07ae:	UJS   11
x07af:	CWT   r4, -1
x07b0:	JES   1
x07b1:	UJS   8  ; -> 0x07ba
x07b2:	CWT   r5, -1
x07b3:	JES   1
x07b4:	UJS   5  ; -> 0x07ba
x07b5:	CWT   r6, -1
x07b6:	JES   1
x07b7:	UJS   2  ; -> 0x07ba
x07b8:	CWT   r7, -1
x07b9:	JES   1
x07ba:	HLT   040
x07bb:	LJ    0x854
x07bd:	TF    0x1338
x07bf:	CW    r1, [pat01]
x07c1:	JES   1
x07c2:	HLT   040
x07c3:	CW    r2, [pat10]
x07c5:	JES   1
x07c6:	HLT   040
x07c7:	CW    r3, [0x133a]
x07c9:	JES   1
x07ca:	HLT   040
x07cb:	CWT   r4, -1
x07cc:	JES   1
x07cd:	UJS   8  ; -> 0x07d6
x07ce:	CWT   r5, -1
x07cf:	JES   1
x07d0:	UJS   5  ; -> 0x07d6
x07d1:	CWT   r6, -1
x07d2:	JES   1
x07d3:	UJS   2  ; -> 0x07d6
x07d4:	CWT   r7, -1
x07d5:	JES   1
x07d6:	HLT   040
x07d7:	LJ    0x854
x07d9:	TA    0x1338
x07db:	CW    r1, [pat01]
x07dd:	JES   1
x07de:	HLT   040
x07df:	CW    r2, [pat10]
x07e1:	JES   1
x07e2:	HLT   040
x07e3:	CW    r3, [0x133a]
x07e5:	JES   1
x07e6:	HLT   040
x07e7:	CW    r4, [0x133b]
x07e9:	JES   1
x07ea:	HLT   040
x07eb:	CW    r5, [0x133c]
x07ed:	JES   1
x07ee:	HLT   040
x07ef:	CW    r6, [0x133d]
x07f1:	JES   1
x07f2:	HLT   040
x07f3:	CW    r7, [0x133e]
x07f5:	JES   1
x07f6:	HLT   040
x07f7:	LJ    0x854
x07f9:	TL    0x1338
x07fb:	CW    r5, [pat01]
x07fd:	JES   1
x07fe:	HLT   040
x07ff:	CW    r6, [pat10]
x0801:	JES   1
x0802:	HLT   040
x0803:	CW    r7, [0x133a]
x0805:	JES   1
x0806:	HLT   040
x0807:	CWT   r1, -1
x0808:	JES   1
x0809:	UJS   8  ; -> 0x0812
x080a:	CWT   r2, -1
x080b:	JES   1
x080c:	UJS   5  ; -> 0x0812
x080d:	CWT   r3, -1
x080e:	JES   1
x080f:	UJS   2  ; -> 0x0812
x0810:	CWT   r4, -1
x0811:	JES   1
x0812:	HLT   040
x0813:	TA    0x133a
x0815:	RD    0x132e
x0817:	LD    0x132e
x0819:	LJ    0x85e
x081b:	TA    0x133a
x081d:	RF    0x132e
x081f:	LF    0x132e
x0821:	LJ    0x85e
x0823:	TA    0x133a
x0825:	RA    0x132e
x0827:	LA    0x132e
x0829:	LJ    0x85e
x082b:	TA    0x133a
x082d:	RL    0x132e
x082f:	LL    0x132e
x0831:	LJ    0x85e
x0833:	TA    0x133a
x0835:	PD    0x132e
x0837:	LD    0x132e
x0839:	LJ    0x85e
x083b:	TA    0x133a
x083d:	PF    0x132e
x083f:	LF    0x132e
x0841:	LJ    0x85e
x0843:	TA    0x133a
x0845:	PA    0x132e
x0847:	LA    0x132e
x0849:	LJ    0x85e
x084b:	TA    0x133a
x084d:	PL    0x132e
x084f:	LL    0x132e
x0851:	LJ    coreg
x0853:	UJS   41 ; -> 0x087d
x0854:	.word 0
x0855:	LWT   r1, -1
x0856:	LWT   r2, -1
x0857:	LWT   r3, -1
x0858:	LWT   r4, -1
x0859:	LWT   r5, -1
x085a:	LWT   r6, -1
x085b:	LWT   r7, -1
x085c:	UJ    [0x854]

coreg:	.res 1
check:	CW    r1, [0x133a]
x0861:	JES   1
x0862:	HLT   040
x0863:	CW    r2, [0x133b]
x0865:	JES   1
x0866:	HLT   040
x0867:	CW    r3, [0x133c]
x0869:	JES   1
x086a:	HLT   040
x086b:	CW    r4, [0x133d]
x086d:	JES   1
x086e:	HLT   040
x086f:	CW    r5, [0x133e]
x0871:	JES   1
x0872:	HLT   040
x0873:	CW    r6, [0x133f]
x0875:	JES   1
x0876:	HLT   040
x0877:	CW    r7, [0x1340]
x0879:	JES   1
x087a:	HLT   040
x087b:	UJ    [coreg]

x087d:	LWT   r0, 0
x087e:	LWT   r1, -3
x087f:	AWT   r1, 1
x0880:	CW    r0, 0x4000
x0882:	JES   1
x0883:	HLT   040
x0884:	CWT   r1, -2
x0885:	JES   1
x0886:	HLT   040
x0887:	LWT   r0, 0
x0888:	LWT   r1, -1
x0889:	AWT   r1, -1
x088a:	CW    r0, 0x5000
x088c:	JES   1
x088d:	HLT   040
x088e:	CWT   r1, -2
x088f:	JES   1
x0890:	HLT   040
x0891:	LWT   r0, 0
x0892:	LWT   r1, -1
x0893:	AWT   r1, 1
x0894:	CW    r0, 0x9000
x0896:	JES   1
x0897:	HLT   040
x0898:	CWT   r1, 0
x0899:	JES   1
x089a:	HLT   040
x089b:	LWT   r0, 0
x089c:	LW    r1, 0x7fff
x089e:	AWT   r1, 1
x089f:	CW    r0, 0x2000
x08a1:	JES   1
x08a2:	HLT   040
x08a3:	CW    r1, 0x8000
x08a5:	JES   1
x08a6:	HLT   040
x08a7:	LWT   r1, -2
x08a8:	IRB   r1, 1
x08a9:	HLT   040
x08aa:	CWT   r1, -1
x08ab:	JES   1
x08ac:	HLT   040
x08ad:	LWT   r1, -1
x08ae:	IRB   r1, 1
x08af:	UJS   1
x08b0:	HLT   040
x08b1:	CWT   r1, 0
x08b2:	JES   1
x08b3:	HLT   040
x08b4:	LWT   r1, 2
x08b5:	DRB   r1, 1
x08b6:	HLT   040
x08b7:	CWT   r1, 1
x08b8:	JES   1
x08b9:	HLT   040
x08ba:	LWT   r1, 1
x08bb:	DRB   r1, 1
x08bc:	UJS   1
x08bd:	HLT   040
x08be:	CWT   r1, 0
x08bf:	JES   1
x08c0:	HLT   040
x08c1:	LWT   r1, -2
x08c2:	TRB   r1, 1
x08c3:	UJS   1
x08c4:	HLT   040
x08c5:	CWT   r1, -1
x08c6:	JES   1
x08c7:	HLT   040
x08c8:	LWT   r1, -1
x08c9:	TRB   r1, 1
x08ca:	HLT   040
x08cb:	CWT   r1, 0
x08cc:	JES   1
x08cd:	HLT   040
x08ce:	UJS   1
x08cf:	.res 1
x08d0:	LW    r1, [pat10]
x08d2:	RW    r1, 0x8cf
x08d4:	LWT   r1, -1
x08d5:	LWS   r1, -7
x08d6:	CW    r1, [pat10]
x08d8:	JES   1
x08d9:	HLT   040
x08da:	LW    r1, [pat01]
x08dc:	RW    r1, 0x8e4
x08de:	LWT   r1, -1
x08df:	LWS   r1, 4
x08e0:	CW    r1, [pat01]
x08e2:	JES   2  ; -> 0x08e5
x08e3:	HLT   040
x08e4:	.res 1
x08e5:	UJS   1
x08e6:	.word 0
x08e7:	LW    r1, [pat01]
x08e9:	RWS   r1, -4
x08ea:	CW    r1, [0x8e6]
x08ec:	JES   1
x08ed:	HLT   040
x08ee:	LW    r1, [pat10]
x08f0:	RWS   r1, 4
x08f1:	CW    r1, [0x8f5]
x08f3:	JES   2  ; -> 0x08f6
x08f4:	HLT   040
x08f5:	.word 0
x08f6:	LWT   r0, 0
x08f7:	JL    0x8fe
x08f9:	LW    r0, 0x800
x08fb:	JL    0x8ff
x08fd:	HLT   040
x08fe:	HLT   040
x08ff:	LW    r0, 0xf7ff
x0901:	JL    0x904
x0903:	UJS   1
x0904:	HLT   040
x0905:	LWT   r0, 0
x0906:	JE    0x90d
x0908:	LW    r0, 0x400
x090a:	JE    0x90e
x090c:	HLT   040
x090d:	HLT   040
x090e:	LW    r0, 0xfbff
x0910:	JE    0x913
x0912:	UJS   1
x0913:	HLT   040
x0914:	LWT   r0, 0
x0915:	JG    0x91c
x0917:	LW    r0, 0x200
x0919:	JG    0x91d
x091b:	HLT   040
x091c:	HLT   040
x091d:	LW    r0, 0xfdff
x091f:	JG    0x922
x0921:	UJS   1
x0922:	HLT   040
x0923:	LWT   r0, 0
x0924:	JZ    0x92b
x0926:	LW    r0, 0x8000
x0928:	JZ    0x92c
x092a:	HLT   040
x092b:	HLT   040
x092c:	LW    r0, 0x7fff
x092e:	JZ    0x931
x0930:	UJS   1
x0931:	HLT   040
x0932:	LWT   r0, 0
x0933:	JM    0x93a
x0935:	LW    r0, 0x4000
x0937:	JM    0x93b
x0939:	HLT   040
x093a:	HLT   040
x093b:	LW    r0, 0xbfff
x093d:	JM    0x940
x093f:	UJS   1
x0940:	HLT   040
x0941:	LW    r0, 0xfbff
x0943:	JN    0x946
x0945:	HLT   040
x0946:	LW    r0, 0x400
x0948:	JN    0x94b
x094a:	UJS   1
x094b:	HLT   040
x094c:	LWT   r0, 0
x094d:	JLS   4  ; -> 0x0952
x094e:	LW    r0, 0x800
x0950:	JLS   2  ; -> 0x0953
x0951:	HLT   040
x0952:	HLT   040
x0953:	LW    r0, 0xf7ff
x0955:	JLS   1  ; -> 0x0957
x0956:	UJS   1
x0957:	HLT   040
x0958:	LWT   r0, 0
x0959:	JGS   4  ; -> 0x095e
x095a:	LW    r0, 0x200
x095c:	JGS   2  ; -> 0x095f
x095d:	HLT   040
x095e:	HLT   040
x095f:	LW    r0, 0xfdff
x0961:	JGS   1  ; -> 0x0963
x0962:	UJS   1
x0963:	HLT   040
x0964:	LWT   r0, 0
x0965:	JXS   4  ; -> 0x096a
x0966:	LW    r0, 0x80
x0968:	JXS   2  ; -> 0x096b
x0969:	HLT   040
x096a:	HLT   040
x096b:	LW    r0, 0xff7f
x096d:	JXS   1  ; -> 0x096f
x096e:	UJS   1
x096f:	HLT   040
x0970:	LWT   r0, 0
x0971:	JYS   4  ; -> 0x0976
x0972:	LW    r0, 0x100
x0974:	JYS   2  ; -> 0x0977
x0975:	HLT   040
x0976:	HLT   040
x0977:	LW    r0, 0xfeff
x0979:	JYS   1  ; -> 0x097b
x097a:	UJS   1
x097b:	HLT   040
x097c:	LWT   r0, 0
x097d:	JCS   4  ; -> 0x0982
x097e:	LW    r0, 0x1000
x0980:	JCS   2  ; -> 0x0983
x0981:	HLT   040
x0982:	HLT   040
x0983:	LW    r0, 0xefff
x0985:	JCS   1  ; -> 0x0987
x0986:	UJS   1
x0987:	HLT   040
x0988:	LWT   r0, 0
x0989:	JVS   4  ; -> 0x098e
x098a:	LW    r0, 0x2000
x098c:	JVS   2  ; -> 0x098f
x098d:	HLT   040
x098e:	HLT   040
x098f:	LW    r0, 0xdfff
x0991:	JVS   1  ; -> 0x0993
x0992:	UJS   1
x0993:	HLT   040
x0994:	RKY   r1
x0995:	RKY   r2
x0996:	RKY   r3
x0997:	RKY   r4
x0998:	CW    r1, r2
x0999:	JES   1
x099a:	HLT   040
x099b:	CW    r3, r4
x099c:	JES   1
x099d:	HLT   040
x099e:	CW    r3, r1
x099f:	JES   1
x09a0:	HLT   040
x09a1:	LWT   r0, 0
x09a2:	LW    r1, 0x8000
x09a4:	SXU   r1
x09a5:	CW    r0, 0x80
x09a7:	JES   1
x09a8:	HLT   040
x09a9:	LWT   r0, -1
x09aa:	LW    r1, 0x7fff
x09ac:	SXU   r1
x09ad:	CW    r0, 0xff7f
x09af:	JES   1
x09b0:	HLT   040
x09b1:	LWT   r0, 0
x09b2:	LWT   r1, 1
x09b3:	SXL   r1
x09b4:	CW    r0, 0x80
x09b6:	JES   1
x09b7:	HLT   040
x09b8:	LWT   r0, -1
x09b9:	LW    r1, 0xfffe
x09bb:	SXL   r1
x09bc:	CW    r0, 0xff7f
x09be:	JES   1
x09bf:	HLT   040
x09c0:	LW    r1, [pat10]
x09c2:	LW    r2, [pat01]
x09c4:	LW    r0, 0x100
x09c6:	SRZ   r1
x09c7:	CWT   r0, 0
x09c8:	JES   1
x09c9:	HLT   040
x09ca:	CW    r1, r2
x09cb:	JES   1
x09cc:	HLT   040
x09cd:	LWT   r0, 0
x09ce:	SRZ   r2
x09cf:	CW    r0, 0x100
x09d1:	JES   1
x09d2:	HLT   040
x09d3:	CW    r2, 0x2aaa
x09d5:	JES   1
x09d6:	HLT   040
x09d7:	LWT   r0, 0
x09d8:	LW    r1, [pat10]
x09da:	LW    r2, [pat01]
x09dc:	SRY   r1
x09dd:	CWT   r0, 0
x09de:	JES   1
x09df:	HLT   040
x09e0:	CW    r1, r2
x09e1:	JES   1
x09e2:	HLT   040
x09e3:	LW    r0, 0x100
x09e5:	LW    r1, [pat10]
x09e7:	SRY   r1
x09e8:	CWT   r0, 0
x09e9:	JES   1
x09ea:	HLT   040
x09eb:	CW    r1, 0xd555
x09ed:	JES   1
x09ee:	HLT   040
x09ef:	LWT   r0, 0
x09f0:	SRY   r2
x09f1:	CW    r0, 0x100
x09f3:	JES   1
x09f4:	HLT   040
x09f5:	CW    r2, 0x2aaa
x09f7:	JES   1
x09f8:	HLT   040
x09f9:	LW    r1, [pat01]
x09fb:	LW    r0, 0x100
x09fd:	SRY   r1
x09fe:	CW    r0, 0x100
x0a00:	JES   1
x0a01:	HLT   040
x0a02:	CW    r1, [pat10]
x0a04:	JES   1
x0a05:	HLT   040
x0a06:	LWT   r0, 0
x0a07:	LW    r1, [pat10]
x0a09:	LW    r2, [pat01]
x0a0b:	SRX   r1
x0a0c:	CWT   r0, 0
x0a0d:	JES   1
x0a0e:	HLT   040
x0a0f:	CW    r1, r2
x0a10:	JES   1
x0a11:	HLT   040
x0a12:	LW    r0, 0x80
x0a14:	LW    r1, [pat10]
x0a16:	SRX   r1
x0a17:	CW    r0, 0x80
x0a19:	JES   1
x0a1a:	HLT   040
x0a1b:	CW    r1, 0xd555
x0a1d:	JES   1
x0a1e:	HLT   040
x0a1f:	LWT   r0, 0
x0a20:	SRX   r2
x0a21:	CW    r0, 0x100
x0a23:	JES   1
x0a24:	HLT   040
x0a25:	CW    r2, 0x2aaa
x0a27:	JES   1
x0a28:	HLT   040
x0a29:	LW    r1, [pat01]
x0a2b:	LW    r0, 0x80
x0a2d:	SRX   r1
x0a2e:	CW    r0, 0x180
x0a30:	JES   1
x0a31:	HLT   040
x0a32:	CW    r1, [pat10]
x0a34:	JES   1
x0a35:	HLT   040
x0a36:	LW    r1, [pat10]
x0a38:	LW    r2, [pat01]
x0a3a:	LW    r0, 0x100
x0a3c:	SLZ   r2
x0a3d:	CWT   r0, 0
x0a3e:	JES   1
x0a3f:	HLT   040
x0a40:	CW    r1, r2
x0a41:	JES   1
x0a42:	HLT   040
x0a43:	LWT   r0, 0
x0a44:	SLZ   r1
x0a45:	CW    r0, 0x100
x0a47:	JES   1
x0a48:	HLT   040
x0a49:	CW    r1, 0x5554
x0a4b:	JES   1
x0a4c:	HLT   040
x0a4d:	LW    r0, 0x80
x0a4f:	LW    r1, [pat10]
x0a51:	SLX   r1
x0a52:	CW    r0, 0x180
x0a54:	JES   1
x0a55:	HLT   040
x0a56:	CW    r1, [pat01]
x0a58:	JES   1
x0a59:	HLT   040
x0a5a:	LW    r1, [pat01]
x0a5c:	LWT   r0, 0
x0a5d:	SLX   r1
x0a5e:	CWT   r0, 0
x0a5f:	JES   1
x0a60:	HLT   040
x0a61:	CW    r1, [pat10]
x0a63:	JES   1
x0a64:	HLT   040
x0a65:	LW    r0, 0x180
x0a67:	LW    r1, [pat01]
x0a69:	SLX   r1
x0a6a:	CW    r0, 0x80
x0a6c:	JES   1
x0a6d:	HLT   040
x0a6e:	CW    r1, 0xaaab
x0a70:	JES   1
x0a71:	HLT   040
x0a72:	LW    r0, 0x100
x0a74:	LW    r1, [pat01]
x0a76:	SVX   r1
x0a77:	CW    r0, 0x2000
x0a79:	JES   1
x0a7a:	HLT   040
x0a7b:	CW    r1, [pat10]
x0a7d:	JES   1
x0a7e:	HLT   040
x0a7f:	LW    r0, 0x80
x0a81:	LW    r1, [pat10]
x0a83:	SVX   r1
x0a84:	CW    r0, 0x2180
x0a86:	JES   1
x0a87:	HLT   040
x0a88:	CW    r1, [pat01]
x0a8a:	JES   1
x0a8b:	HLT   040
x0a8c:	LW    r0, 0x2100
x0a8e:	LW    r1, [pat01]
x0a90:	SVZ   r1
x0a91:	CW    r0, 0x2000
x0a93:	JES   1
x0a94:	HLT   040
x0a95:	CW    r1, [pat10]
x0a97:	JES   1
x0a98:	HLT   040
x0a99:	LW    r0, 0x2000
x0a9b:	LWT   r1, -1
x0a9c:	SVZ   r1
x0a9d:	CW    r0, 0x2100
x0a9f:	JES   1
x0aa0:	HLT   040
x0aa1:	CW    r1, 0xfffe
x0aa3:	JES   1
x0aa4:	HLT   040
x0aa5:	LWT   r0, 0
x0aa6:	LW    r1, [pat10]
x0aa8:	SVZ   r1
x0aa9:	CW    r0, 0x2100
x0aab:	JES   1
x0aac:	HLT   040
x0aad:	CW    r1, 0x5554
x0aaf:	JES   1
x0ab0:	HLT   040
x0ab1:	LW    r0, 0x100
x0ab3:	LW    r1, [pat01]
x0ab5:	SLY   r1
x0ab6:	CWT   r0, 0
x0ab7:	JES   1
x0ab8:	HLT   040
x0ab9:	CW    r1, 0xaaab
x0abb:	JES   1
x0abc:	HLT   040
x0abd:	LWT   r0, 0
x0abe:	LW    r1, [pat01]
x0ac0:	SLY   r1
x0ac1:	CWT   r0, 0
x0ac2:	JES   1
x0ac3:	HLT   040
x0ac4:	CW    r1, [pat10]
x0ac6:	JES   1
x0ac7:	HLT   040
x0ac8:	LW    r0, 0x100
x0aca:	LW    r1, [pat10]
x0acc:	SLY   r1
x0acd:	CW    r0, 0x100
x0acf:	JES   1
x0ad0:	HLT   040
x0ad1:	CW    r1, [pat01]
x0ad3:	JES   1
x0ad4:	HLT   040
x0ad5:	LWT   r0, 0
x0ad6:	LW    r1, [pat01]
x0ad8:	SVY   r1
x0ad9:	CW    r0, 0x2000
x0adb:	JES   1
x0adc:	HLT   040
x0add:	CW    r1, [pat10]
x0adf:	JES   1
x0ae0:	HLT   040
x0ae1:	LWT   r0, 0
x0ae2:	LW    r1, [pat10]
x0ae4:	SVY   r1
x0ae5:	CW    r0, 0x2100
x0ae7:	JES   1
x0ae8:	HLT   040
x0ae9:	CW    r1, 0x5554
x0aeb:	JES   1
x0aec:	HLT   040
x0aed:	LW    r0, 0x2100
x0aef:	LW    r1, [pat01]
x0af1:	SVY   r1
x0af2:	CW    r0, 0x2000
x0af4:	JES   1
x0af5:	HLT   040
x0af6:	CW    r1, 0xaaab
x0af8:	JES   1
x0af9:	HLT   040
x0afa:	LW    r0, 0x100
x0afc:	LW    r1, [pat10]
x0afe:	SVY   r1
x0aff:	CW    r0, 0x2100
x0b01:	JES   1
x0b02:	HLT   040
x0b03:	CW    r1, [pat01]
x0b05:	JES   1
x0b06:	HLT   040
x0b07:	LWT   r0, 0
x0b08:	LW    r1, [pat01]
x0b0a:	SHC   r1, 1
x0b0b:	CW    r1, [pat10]
x0b0d:	JES   1
x0b0e:	HLT   040
x0b0f:	LW    r1, [pat10]
x0b11:	SHC   r1, 1
x0b12:	CW    r1, [pat01]
x0b14:	JES   1
x0b15:	HLT   040
x0b16:	LW    r1, [pat01]
x0b18:	SHC   r1, 1
x0b19:	SHC   r1, 2
x0b1a:	SHC   r1, 3
x0b1b:	SHC   r1, 4
x0b1c:	SHC   r1, 5
x0b1d:	SHC   r1, 6
x0b1e:	SHC   r1, 7
x0b1f:	SHC   r1, 8
x0b20:	SHC   r1, 9
x0b21:	SHC   r1, 10
x0b22:	SHC   r1, 11
x0b23:	SHC   r1, 12
x0b24:	SHC   r1, 13
x0b25:	SHC   r1, 14
x0b26:	SHC   r1, 15
x0b27:	CW    r1, [pat01]
x0b29:	JES   1
x0b2a:	HLT   040
x0b2b:	LWT   r1, -1
x0b2c:	SHC   r1, 15
x0b2d:	CWT   r1, -1
x0b2e:	JES   1
x0b2f:	HLT   040
x0b30:	LWT   r0, 0
x0b31:	LWT   r1, -1
x0b32:	NGC   r1
x0b33:	CW    r0, 0x8000
x0b35:	JES   1
x0b36:	HLT   040
x0b37:	CWT   r1, 0
x0b38:	JES   1
x0b39:	HLT   040
x0b3a:	LW    r0, 0x1000
x0b3c:	LW    r1, 0x7fff
x0b3e:	NGC   r1
x0b3f:	CW    r0, 0x4000
x0b41:	JES   1
x0b42:	HLT   040
x0b43:	CW    r1, 0x8001
x0b45:	JES   1
x0b46:	HLT   040
x0b47:	LW    r1, 0x8001
x0b49:	LW    r0, 0x1000
x0b4b:	NGC   r1
x0b4c:	CWT   r0, 0
x0b4d:	JES   1
x0b4e:	HLT   040
x0b4f:	CW    r1, 0x7fff
x0b51:	JES   1
x0b52:	HLT   040
x0b53:	LWT   r0, 0
x0b54:	LWT   r1, -1
x0b55:	NGA   r1
x0b56:	CWT   r0, 0
x0b57:	JES   1
x0b58:	HLT   040
x0b59:	CWT   r1, 1
x0b5a:	JES   1
x0b5b:	HLT   040
x0b5c:	LWT   r0, 0
x0b5d:	LW    r1, 0x8000
x0b5f:	NGA   r1
x0b60:	CW    r0, 0x2000
x0b62:	JES   1
x0b63:	HLT   040
x0b64:	CW    r1, 0x8000
x0b66:	JES   1
x0b67:	HLT   040
x0b68:	LWT   r0, 0
x0b69:	LWT   r1, 0
x0b6a:	NGA   r1
x0b6b:	CW    r0, 0x9000
x0b6d:	JES   1
x0b6e:	HLT   040
x0b6f:	CWT   r1, 0
x0b70:	JES   1
x0b71:	HLT   040
x0b72:	LWT   r0, 0
x0b73:	LW    r1, 0x7fff
x0b75:	NGA   r1
x0b76:	CW    r0, 0x4000
x0b78:	JES   1
x0b79:	HLT   040
x0b7a:	CW    r1, 0x8001
x0b7c:	JES   1
x0b7d:	HLT   040
x0b7e:	LWT   r0, 0
x0b7f:	LWT   r1, -1
x0b80:	NGL   r1
x0b81:	CW    r0, 0x8000
x0b83:	JES   1
x0b84:	HLT   040
x0b85:	CWT   r1, 0
x0b86:	JES   1
x0b87:	HLT   040
x0b88:	LWT   r1, 0
x0b89:	LWT   r0, 0
x0b8a:	NGL   r1
x0b8b:	CWT   r0, 0
x0b8c:	JES   1
x0b8d:	HLT   040
x0b8e:	CWT   r1, -1
x0b8f:	JES   1
x0b90:	HLT   040
x0b91:	LWT   r0, 0
x0b92:	LW    r1, [pat01]
x0b94:	NGL   r1
x0b95:	CWT   r0, 0
x0b96:	JES   1
x0b97:	HLT   040
x0b98:	CW    r1, [pat10]
x0b9a:	JES   1
x0b9b:	HLT   040
x0b9c:	LWT   r1, -1
x0b9d:	ZRB   r1
x0b9e:	CW    r1, 0xff00
x0ba0:	JES   1
x0ba1:	HLT   040
x0ba2:	LWT   r1, -1
x0ba3:	ZLB   r1
x0ba4:	CW    r1, 0xff
x0ba6:	JES   1
x0ba7:	HLT   040
x0ba8:	LWT   r1, 0
x0ba9:	LB    r1, 0x2670
x0bab:	CW    r1, 0x55
x0bad:	JES   1
x0bae:	HLT   040
x0baf:	LWT   r1, -1
x0bb0:	RZ    0x132e
x0bb2:	LB    r1, 0x265d
x0bb4:	CW    r1, 0xff00
x0bb6:	JES   1
x0bb7:	HLT   040
x0bb8:	LWT   r1, -1
x0bb9:	RZ    0x132e
x0bbb:	RB    r1, 0x265d
x0bbd:	LW    r2, [0x132e]
x0bbf:	CW    r2, 0xff
x0bc1:	JES   1
x0bc2:	HLT   040
x0bc3:	LWT   r1, -1
x0bc4:	RW    r1, 0x132e
x0bc6:	LWT   r1, 0
x0bc7:	RB    r1, 0x265c
x0bc9:	LW    r1, [0x132e]
x0bcb:	CW    r1, 0xff
x0bcd:	JES   1
x0bce:	HLT   040
x0bcf:	LWT   r0, 0
x0bd0:	RZ    0x132e
x0bd2:	LWT   r1, 0
x0bd3:	CB    r1, 0x265c
x0bd5:	CW    r0, 0x400
x0bd7:	JES   1
x0bd8:	HLT   040
x0bd9:	LWT   r1, -1
x0bda:	CB    r1, 0x265d
x0bdc:	CW    r0, 0x200
x0bde:	JES   1
x0bdf:	HLT   040
x0be0:	LWT   r1, -1
x0be1:	RW    r1, 0x132e
x0be3:	LWT   r1, 0
x0be4:	CB    r1, 0x265c
x0be6:	CW    r0, 0x800
x0be8:	JES   1
x0be9:	HLT   040
x0bea:	LWT   r0, -1
x0beb:	BLC   255<<8
x0bec:	UJS   1
x0bed:	HLT   040
x0bee:	LWT   r0, 0
x0bef:	BLC   181<<8
x0bf0:	HLT   040
x0bf1:	LWT   r0, -1
x0bf2:	BRC   255
x0bf3:	UJS   1
x0bf4:	HLT   040
x0bf5:	LWT   r0, 0
x0bf6:	BRC   181
x0bf7:	HLT   040
x0bf8:	LWT   r1, -3
x0bf9:	RW    r1, 0x132e
x0bfb:	IB    0x132e
x0bfd:	LW    r1, [0x132e]
x0bff:	CWT   r1, -2
x0c00:	JES   1
x0c01:	UJS   14
x0c02:	IB    0x132e
x0c04:	LW    r1, [0x132e]
x0c06:	CWT   r1, -1
x0c07:	JES   1
x0c08:	UJS   7  ; -> 0x0c10
x0c09:	IB    0x132e
x0c0b:	HLT   040
x0c0c:	LW    r1, [0x132e]
x0c0e:	CWT   r1, 0
x0c0f:	JES   1
x0c10:	HLT   040
x0c11:	LWT   r1, 0
x0c12:	RIC   r1
x0c13:	CW    r1, 0xc13
x0c15:	JES   1
x0c16:	HLT   040
x0c17:	LWT   r0, 0
x0c18:	LWT   r1, -1
x0c19:	RPC   r1
x0c1a:	CWT   r1, 0
x0c1b:	JES   1
x0c1c:	HLT   040
x0c1d:	LWT   r0, -1
x0c1e:	LWT   r1, 0
x0c1f:	RPC   r1
x0c20:	CWT   r1, -1
x0c21:	JES   1
x0c22:	HLT   040
x0c23:	LWT   r0, 0
x0c24:	LWT   r1, -1
x0c25:	LPC   r1
x0c26:	CWT   r0, -1
x0c27:	JES   1
x0c28:	HLT   040
x0c29:	LWT   r0, -1
x0c2a:	LWT   r1, 0
x0c2b:	LPC   r1
x0c2c:	CWT   r0, 0
x0c2d:	JES   1
x0c2e:	HLT   040
x0c2f:	LW    r1, [0x2]
x0c31:	CWT   r1, 0
x0c32:	JES   5  ; -> 0x0c38
x0c33:	LJ    reset
x0c35:	MCL
x0c36:	UJ    0x100
x0c38:	RZ    0xc86
x0c3a:	FI    0xc86
x0c3c:	LWT   r1, -1
x0c3d:	RW    r1, 0xc86
x0c3f:	KI    0xc86
x0c41:	LW    r1, [0xc86]
x0c43:	CWT   r1, 0
x0c44:	JES   1
x0c45:	HLT   040
x0c46:	LW    r1, 0x7fff
x0c48:	RW    r1, 0xc86
x0c4a:	FI    0xc86
x0c4c:	RZ    0xc86
x0c4e:	KI    0xc86
x0c50:	LW    r1, [0xc86]
x0c52:	CW    r1, 0x7fff
x0c54:	JES   1
x0c55:	HLT   040
x0c56:	LW    r1, 0x5555
x0c58:	RW    r1, 0xc86
x0c5a:	FI    0xc86
x0c5c:	RZ    0xc86
x0c5e:	KI    0xc86
x0c60:	LW    r1, [0xc86]
x0c62:	CW    r1, 0x5555
x0c64:	JES   1
x0c65:	HLT   040
x0c66:	MCL
x0c67:	LW    r1, 0xc7a
x0c69:	RW    r1, 0x5f
x0c6b:	LW    r1, stack
x0c6d:	RW    r1, stackp
x0c6f:	LW    r1, 0x40
x0c71:	RW    r1, 0xd3c
x0c73:	IM    0xd3c
x0c75:	LWT   r1, 1
x0c76:	RW    r1, 0xc86
x0c78:	FI    0xc86
x0c7a:	LW    r1, 0xfd
x0c7c:	RW    r1, 0x5f
x0c7e:	LW    r1, [0x64]
x0c80:	CW    r1, 0x40
x0c82:	JES   1
x0c83:	HLT   040
x0c84:	MCL
x0c85:	UJS   6  ; -> 0x0c8c
x0c86:	.word 0
x0c87:	.word 0x0c9d
x0c88:	IB    [r7+r7]
x0c89:	IB    [0xff]
x0c8b:	.word 0x0066
x0c8c:	MCL
x0c8d:	LW    r1, stack
x0c8f:	RW    r1, stackp
x0c91:	LWT   r0, -1
x0c92:	LW    r1, 0xffc0
x0c94:	RW    r1, 0xc86
x0c96:	IM    0xc86
x0c98:	LW    r1, 0xc9d
x0c9a:	RW    r1, 0x60
x0c9c:	EXL   255
x0c9d:	LA    0xc87
x0c9f:	CW    r1, [stack]
x0ca1:	JES   1
x0ca2:	HLT   040
x0ca3:	CW    r2, [0x63]
x0ca5:	JES   1
x0ca6:	HLT   040
x0ca7:	CW    r3, [0x64]
x0ca9:	JES   1
x0caa:	HLT   040
x0cab:	CW    r4, [0x65]
x0cad:	JES   1
x0cae:	HLT   040
x0caf:	CW    r0, 0x400
x0cb1:	JES   1
x0cb2:	HLT   040
x0cb3:	CW    r5, [0x61]
x0cb5:	JES   1
x0cb6:	HLT   040
x0cb7:	LW    r1, 0xcc2
x0cb9:	RW    r1, 0x5f
x0cbb:	LWT   r1, 1
x0cbc:	RW    r1, 0xc86
x0cbe:	FI    0xc86
x0cc0:	MCL
x0cc1:	UJS   5  ; -> 0x0cc7
x0cc2:	LW    r1, 0xfd
x0cc4:	RW    r1, 0x5f
x0cc6:	HLT   040
x0cc7:	LW    r1, 0xfd
x0cc9:	RW    r1, 0x5f
x0ccb:	MCL
x0ccc:	UJS   3  ; -> 0x0cd0
x0ccd:	.word 0x0cd3
x0cce:	IB    [r7+r7]
x0ccf:	IB    [0xec40]
x0cd1:	SP    0xccd
x0cd3:	CWT   r0, -1
x0cd4:	JES   1
x0cd5:	HLT   040
x0cd6:	LW    r1, 0xce3
x0cd8:	RW    r1, 0x5f
x0cda:	LW    r1, stack
x0cdc:	RW    r1, stackp
x0cde:	LWT   r1, 1
x0cdf:	RW    r1, 0xc86
x0ce1:	FI    0xc86
x0ce3:	LW    r1, 0xfd
x0ce5:	RW    r1, 0x5f
x0ce7:	LW    r1, 0xffc0
x0ce9:	CW    r1, [0x64]
x0ceb:	JES   1
x0cec:	HLT   040
x0ced:	LW    r1, 0x5f
x0cef:	RW    r1, 0x132e
x0cf1:	MCL
x0cf2:	MB    0x132e
x0cf4:	LW    r1, 0xd07
x0cf6:	RW    r1, 0x5f
x0cf8:	LW    r1, stack
x0cfa:	RW    r1, stackp
x0cfc:	LW    r1, 0x40
x0cfe:	RW    r1, 0xd3c
x0d00:	IM    0xd3c
x0d02:	LWT   r1, 1
x0d03:	RW    r1, 0xd3d
x0d05:	FI    0xd3d
x0d07:	LW    r1, 0xfd
x0d09:	RW    r1, 0x5f
x0d0b:	LW    r1, [0x64]
x0d0d:	CW    r1, 0x5f
x0d0f:	JES   1
x0d10:	HLT   040
x0d11:	MCL
x0d12:	LW    r1, 0xd2b
x0d14:	RW    r1, stack
x0d16:	LWT   r1, -1
x0d17:	RW    r1, 0x63
x0d19:	RZ    0x64
x0d1b:	MCL
x0d1c:	LW    r1, 0x7fff
x0d1e:	RW    r1, 0xd3d
x0d20:	FI    0xd3d
x0d22:	LW    r1, 0xd36
x0d24:	RW    r1, 0x41
x0d26:	LW    r1, 0x66
x0d28:	RW    r1, 0x61
x0d2a:	LIP
x0d2b:	CWT   r0, -1
x0d2c:	JES   1
x0d2d:	HLT   040
x0d2e:	LW    r1, stack
x0d30:	CW    r1, [stackp]
x0d32:	JES   1
x0d33:	HLT   040
x0d34:	MCL
x0d35:	UJS   1
x0d36:	HLT   040

; test rozkazow wspolpracujacych z NB niezerowym

x0d37:	LW    r1, 0xc1
x0d39:	RW    r1, 0x41
x0d3b:	UJS   2
x0d3c:	.word 0
x0d3d:	.word 0
x0d3e:	LJ    reset
x0d40:	LW    r1, 0x100f
x0d42:	LW    r2, 0x61
x0d44:	OU    r1, r2
x0d45:	.word 0x0d49, 0x0d4a, 0x0d4c, 0x0d4b
x0d49:	HLT   040
x0d4a:	HLT   040
x0d4b:	HLT   040
x0d4c:	MB    0x1341
x0d4e:	LWT   r3, 0
x0d4f:	LW    r4, 0x1001
x0d51:	LW    r5, 0xf060
x0d53:	PW    r3, r4
x0d54:	AWT   r4, 1
x0d55:	IRB   r5, -3
x0d56:	LJ    0x130f
x0d58:	TW    r2, 0x132b
x0d5a:	PW    r2, 0x132e
x0d5c:	LWT   r1, 0
x0d5d:	TW    r1, 0x132e
x0d5f:	CW    r1, r2
x0d60:	JES   1
x0d61:	HLT   040
x0d62:	TW    r2, 0x1339
x0d64:	PW    r2, 0x132e
x0d66:	TW    r1, 0x132e
x0d68:	CW    r1, r2
x0d69:	JES   1
x0d6a:	HLT   040
x0d6b:	LWT   r1, 0
x0d6c:	PW    r1, 0x132e
x0d6e:	IS    r1, 0x132e
x0d70:	HLT   040
x0d71:	TW    r1, 0x132e
x0d73:	CWT   r1, 0
x0d74:	JES   1
x0d75:	HLT   040
x0d76:	LWT   r1, 0
x0d77:	PW    r1, 0x132e
x0d79:	LWT   r1, -1
x0d7a:	IS    r1, 0x132e
x0d7c:	UJS   1
x0d7d:	HLT   040
x0d7e:	TW    r1, 0x132e
x0d80:	CWT   r1, -1
x0d81:	JES   1
x0d82:	HLT   040
x0d83:	LWT   r1, -1
x0d84:	PW    r1, 0x132e
x0d86:	LWT   r1, 0
x0d87:	IS    r1, 0x132e
x0d89:	HLT   040
x0d8a:	TW    r1, 0x132e
x0d8c:	CWT   r1, -1
x0d8d:	JES   1
x0d8e:	HLT   040
x0d8f:	LWT   r1, -1
x0d90:	PW    r1, 0x132e
x0d92:	IS    r1, 0x132e
x0d94:	HLT   040
x0d95:	TW    r1, 0x132e
x0d97:	CWT   r1, -1
x0d98:	JES   1
x0d99:	HLT   040
x0d9a:	TW    r1, 0x1338
x0d9c:	PW    r1, 0x132e
x0d9e:	IS    r1, 0x132e
x0da0:	HLT   040
x0da1:	TW    r1, 0x132e
x0da3:	TW    r2, 0x1338
x0da5:	CW    r1, r2
x0da6:	JES   1
x0da7:	HLT   040
x0da8:	TW    r1, 0x1339
x0daa:	PW    r1, 0x132e
x0dac:	IS    r1, 0x132e
x0dae:	HLT   040
x0daf:	TW    r1, 0x132e
x0db1:	TW    r2, 0x1339
x0db3:	CW    r1, r2
x0db4:	JES   1
x0db5:	HLT   040
x0db6:	TW    r1, 0x1338
x0db8:	TW    r2, 0x1339
x0dba:	PW    r2, 0x132e
x0dbc:	IS    r1, 0x132e
x0dbe:	UJS   1
x0dbf:	HLT   040
x0dc0:	TW    r1, 0x132e
x0dc2:	CWT   r1, -1
x0dc3:	JES   1
x0dc4:	HLT   040
x0dc5:	TW    r1, 0x1338
x0dc7:	TW    r2, 0x1339
x0dc9:	PW    r1, 0x132e
x0dcb:	IS    r2, 0x132e
x0dcd:	UJS   1
x0dce:	HLT   040
x0dcf:	TW    r1, 0x132e
x0dd1:	CWT   r1, -1
x0dd2:	JES   1
x0dd3:	HLT   040
x0dd4:	LWT   r1, 0
x0dd5:	PW    r1, 0x132e
x0dd7:	BM    r1, 0x132e
x0dd9:	HLT   040
x0dda:	LWT   r1, -1
x0ddb:	PW    r1, 0x132e
x0ddd:	BM    r1, 0x132e
x0ddf:	HLT   040
x0de0:	LWT   r1, -1
x0de1:	LWT   r2, 0
x0de2:	PW    r2, 0x132e
x0de4:	BM    r1, 0x132e
x0de6:	UJS   1
x0de7:	HLT   040
x0de8:	LWT   r1, 0
x0de9:	LWT   r2, -1
x0dea:	PW    r2, 0x132e
x0dec:	BM    r1, 0x132e
x0dee:	HLT   040
x0def:	TW    r1, 0x1338
x0df1:	PW    r1, 0x132e
x0df3:	BM    r1, 0x132e
x0df5:	HLT   040
x0df6:	TW    r1, 0x1339
x0df8:	PW    r1, 0x132e
x0dfa:	BM    r1, 0x132e
x0dfc:	HLT   040
x0dfd:	TW    r1, 0x1338
x0dff:	TW    r2, 0x1339
x0e01:	PW    r2, 0x132e
x0e03:	BM    r1, 0x132e
x0e05:	UJS   1
x0e06:	HLT   040
x0e07:	TW    r1, 0x1339
x0e09:	TW    r2, 0x1338
x0e0b:	PW    r2, 0x132e
x0e0d:	BM    r1, 0x132e
x0e0f:	UJS   1
x0e10:	HLT   040
x0e11:	LWT   r3, 0
x0e12:	MB    0x1328
x0e14:	LPC   r3
x0e15:	MB    0x1341
x0e17:	LWT   r1, -1
x0e18:	PW    r1, 0x132e
x0e1a:	LWT   r1, 0
x0e1b:	OM    r1, 0x132e
x0e1d:	CWT   r0, 0
x0e1e:	JES   1
x0e1f:	HLT   040
x0e20:	TW    r1, 0x132e
x0e22:	CWT   r1, -1
x0e23:	JES   1
x0e24:	HLT   040
x0e25:	LWT   r3, 0
x0e26:	MB    0x1328
x0e28:	LPC   r3
x0e29:	MB    0x1341
x0e2b:	PW    r0, 0x132e
x0e2d:	LWT   r1, -1
x0e2e:	OM    r1, 0x132e
x0e30:	CWT   r0, 0
x0e31:	JES   1
x0e32:	HLT   040
x0e33:	TW    r1, 0x132e
x0e35:	CWT   r1, -1
x0e36:	JES   1
x0e37:	HLT   040
x0e38:	LWT   r3, 0
x0e39:	MB    0x1328
x0e3b:	LPC   r3
x0e3c:	MB    0x1341
x0e3e:	PW    r0, 0x132e
x0e40:	LWT   r1, 0
x0e41:	OM    r1, 0x132e
x0e43:	CW    r0, 0x8000
x0e45:	JES   1
x0e46:	HLT   040
x0e47:	TW    r1, 0x132e
x0e49:	CWT   r1, 0
x0e4a:	JES   1
x0e4b:	HLT   040
x0e4c:	LWT   r3, 0
x0e4d:	MB    0x1328
x0e4f:	LPC   r3
x0e50:	MB    0x1341
x0e52:	LWT   r1, -1
x0e53:	PW    r1, 0x132e
x0e55:	OM    r1, 0x132e
x0e57:	CWT   r0, 0
x0e58:	JES   1
x0e59:	HLT   040
x0e5a:	TW    r1, 0x132e
x0e5c:	CWT   r1, -1
x0e5d:	JES   1
x0e5e:	HLT   040
x0e5f:	LWT   r3, 0
x0e60:	MB    0x1328
x0e62:	LPC   r3
x0e63:	MB    0x1341
x0e65:	TW    r1, 0x1338
x0e67:	PW    r1, 0x132e
x0e69:	OM    r1, 0x132e
x0e6b:	CWT   r0, 0
x0e6c:	JES   1
x0e6d:	HLT   040
x0e6e:	TW    r1, 0x132e
x0e70:	TW    r2, 0x1338
x0e72:	CW    r1, r2
x0e73:	JES   1
x0e74:	HLT   040
x0e75:	LWT   r3, 0
x0e76:	MB    0x1328
x0e78:	LPC   r3
x0e79:	MB    0x1341
x0e7b:	TW    r1, 0x1339
x0e7d:	PW    r1, 0x132e
x0e7f:	OM    r1, 0x132e
x0e81:	CWT   r0, 0
x0e82:	JES   1
x0e83:	HLT   040
x0e84:	TW    r1, 0x132e
x0e86:	TW    r2, 0x1339
x0e88:	CW    r1, r2
x0e89:	JES   1
x0e8a:	HLT   040
x0e8b:	LWT   r3, 0
x0e8c:	MB    0x1328
x0e8e:	LPC   r3
x0e8f:	MB    0x1341
x0e91:	TW    r1, 0x1338
x0e93:	PW    r1, 0x132e
x0e95:	TW    r1, 0x1339
x0e97:	OM    r1, 0x132e
x0e99:	CWT   r0, 0
x0e9a:	JES   1
x0e9b:	HLT   040
x0e9c:	TW    r1, 0x132e
x0e9e:	CWT   r1, -1
x0e9f:	JES   1
x0ea0:	HLT   040
x0ea1:	LWT   r3, 0
x0ea2:	MB    0x1328
x0ea4:	LPC   r3
x0ea5:	MB    0x1341
x0ea7:	TW    r1, 0x1339
x0ea9:	PW    r1, 0x132e
x0eab:	TW    r1, 0x1338
x0ead:	OM    r1, 0x132e
x0eaf:	CWT   r0, 0
x0eb0:	JES   1
x0eb1:	HLT   040
x0eb2:	TW    r1, 0x132e
x0eb4:	CWT   r1, -1
x0eb5:	JES   1
x0eb6:	HLT   040
x0eb7:	LWT   r3, 0
x0eb8:	MB    0x1328
x0eba:	LPC   r3
x0ebb:	MB    0x1341
x0ebd:	LWT   r1, -1
x0ebe:	PW    r1, 0x132e
x0ec0:	LWT   r1, 0
x0ec1:	NM    r1, 0x132e
x0ec3:	CW    r0, 0x8000
x0ec5:	JES   1
x0ec6:	HLT   040
x0ec7:	TW    r1, 0x132e
x0ec9:	CWT   r1, 0
x0eca:	JES   1
x0ecb:	HLT   040
x0ecc:	LWT   r3, 0
x0ecd:	MB    0x1328
x0ecf:	LPC   r3
x0ed0:	MB    0x1341
x0ed2:	LWT   r1, -1
x0ed3:	PW    r0, 0x132e
x0ed5:	NM    r1, 0x132e
x0ed7:	CW    r0, 0x8000
x0ed9:	JES   1
x0eda:	HLT   040
x0edb:	TW    r1, 0x132e
x0edd:	CWT   r1, 0
x0ede:	JES   1
x0edf:	HLT   040
x0ee0:	LWT   r3, 0
x0ee1:	MB    0x1328
x0ee3:	LPC   r3
x0ee4:	MB    0x1341
x0ee6:	PW    r0, 0x132e
x0ee8:	LWT   r1, 0
x0ee9:	NM    r1, 0x132e
x0eeb:	CW    r0, 0x8000
x0eed:	JES   1
x0eee:	HLT   040
x0eef:	TW    r1, 0x132e
x0ef1:	CWT   r1, 0
x0ef2:	JES   1
x0ef3:	HLT   040
x0ef4:	LWT   r3, 0
x0ef5:	MB    0x1328
x0ef7:	LPC   r3
x0ef8:	MB    0x1341
x0efa:	LWT   r1, -1
x0efb:	PW    r1, 0x132e
x0efd:	NM    r1, 0x132e
x0eff:	CWT   r0, 0
x0f00:	JES   1
x0f01:	HLT   040
x0f02:	TW    r1, 0x132e
x0f04:	CWT   r1, -1
x0f05:	JES   1
x0f06:	HLT   040
x0f07:	LWT   r3, 0
x0f08:	MB    0x1328
x0f0a:	LPC   r3
x0f0b:	MB    0x1341
x0f0d:	TW    r1, 0x1338
x0f0f:	PW    r1, 0x132e
x0f11:	NM    r1, 0x132e
x0f13:	CWT   r0, 0
x0f14:	JES   1
x0f15:	HLT   040
x0f16:	TW    r1, 0x132e
x0f18:	TW    r2, 0x1338
x0f1a:	CW    r1, r2
x0f1b:	JES   1
x0f1c:	HLT   040
x0f1d:	LWT   r3, 0
x0f1e:	MB    0x1328
x0f20:	LPC   r3
x0f21:	MB    0x1341
x0f23:	TW    r1, 0x1339
x0f25:	PW    r1, 0x132e
x0f27:	NM    r1, 0x132e
x0f29:	CWT   r0, 0
x0f2a:	JES   1
x0f2b:	HLT   040
x0f2c:	TW    r1, 0x132e
x0f2e:	TW    r2, 0x1339
x0f30:	CW    r1, r2
x0f31:	JES   1
x0f32:	HLT   040
x0f33:	LWT   r3, 0
x0f34:	MB    0x1328
x0f36:	LPC   r3
x0f37:	MB    0x1341
x0f39:	TW    r1, 0x1339
x0f3b:	PW    r1, 0x132e
x0f3d:	TW    r1, 0x1338
x0f3f:	NM    r1, 0x132e
x0f41:	CW    r0, 0x8000
x0f43:	JES   1
x0f44:	HLT   040
x0f45:	TW    r1, 0x132e
x0f47:	CWT   r1, 0
x0f48:	JES   1
x0f49:	HLT   040
x0f4a:	LWT   r3, 0
x0f4b:	MB    0x1328
x0f4d:	LPC   r3
x0f4e:	MB    0x1341
x0f50:	TW    r1, 0x1338
x0f52:	PW    r1, 0x132e
x0f54:	TW    r1, 0x1339
x0f56:	NM    r1, 0x132e
x0f58:	CW    r0, 0x8000
x0f5a:	JES   1
x0f5b:	HLT   040
x0f5c:	TW    r1, 0x132e
x0f5e:	CWT   r1, 0
x0f5f:	JES   1
x0f60:	HLT   040
x0f61:	LWT   r3, 0
x0f62:	MB    0x1328
x0f64:	LPC   r3
x0f65:	MB    0x1341
x0f67:	LWT   r1, -1
x0f68:	PW    r1, 0x132e
x0f6a:	LWT   r1, 0
x0f6b:	EM    r1, 0x132e
x0f6d:	CWT   r0, 0
x0f6e:	JES   1
x0f6f:	HLT   040
x0f70:	TW    r1, 0x132e
x0f72:	CWT   r1, -1
x0f73:	JES   1
x0f74:	HLT   040
x0f75:	LWT   r3, 0
x0f76:	MB    0x1328
x0f78:	LPC   r3
x0f79:	MB    0x1341
x0f7b:	LWT   r1, 0
x0f7c:	PW    r1, 0x132e
x0f7e:	LWT   r1, -1
x0f7f:	EM    r1, 0x132e
x0f81:	CW    r0, 0x8000
x0f83:	JES   1
x0f84:	HLT   040
x0f85:	TW    r1, 0x132e
x0f87:	CWT   r1, 0
x0f88:	JES   1
x0f89:	HLT   040
x0f8a:	LWT   r3, 0
x0f8b:	MB    0x1328
x0f8d:	LPC   r3
x0f8e:	MB    0x1341
x0f90:	LWT   r1, 0
x0f91:	PW    r1, 0x132e
x0f93:	EM    r1, 0x132e
x0f95:	CW    r0, 0x8000
x0f97:	JES   1
x0f98:	HLT   040
x0f99:	TW    r1, 0x132e
x0f9b:	CWT   r1, 0
x0f9c:	JES   1
x0f9d:	HLT   040
x0f9e:	LWT   r3, 0
x0f9f:	MB    0x1328
x0fa1:	LPC   r3
x0fa2:	MB    0x1341
x0fa4:	LWT   r1, -1
x0fa5:	PW    r1, 0x132e
x0fa7:	EM    r1, 0x132e
x0fa9:	CW    r0, 0x8000
x0fab:	JES   1
x0fac:	HLT   040
x0fad:	TW    r1, 0x132e
x0faf:	CWT   r1, 0
x0fb0:	JES   1
x0fb1:	HLT   040
x0fb2:	LWT   r3, 0
x0fb3:	MB    0x1328
x0fb5:	LPC   r3
x0fb6:	MB    0x1341
x0fb8:	TW    r1, 0x1338
x0fba:	PW    r1, 0x132e
x0fbc:	EM    r1, 0x132e
x0fbe:	CW    r0, 0x8000
x0fc0:	JES   1
x0fc1:	HLT   040
x0fc2:	TW    r1, 0x132e
x0fc4:	CWT   r1, 0
x0fc5:	JES   1
x0fc6:	HLT   040
x0fc7:	LWT   r3, 0
x0fc8:	MB    0x1328
x0fca:	LPC   r3
x0fcb:	MB    0x1341
x0fcd:	TW    r1, 0x1339
x0fcf:	PW    r1, 0x132e
x0fd1:	EM    r1, 0x132e
x0fd3:	CW    r0, 0x8000
x0fd5:	JES   1
x0fd6:	HLT   040
x0fd7:	TW    r1, 0x132e
x0fd9:	CWT   r1, 0
x0fda:	JES   1
x0fdb:	HLT   040
x0fdc:	LWT   r3, 0
x0fdd:	MB    0x1328
x0fdf:	LPC   r3
x0fe0:	MB    0x1341
x0fe2:	TW    r1, 0x1339
x0fe4:	PW    r1, 0x132e
x0fe6:	TW    r1, 0x1338
x0fe8:	EM    r1, 0x132e
x0fea:	CWT   r0, 0
x0feb:	JES   1
x0fec:	HLT   040
x0fed:	TW    r1, 0x132e
x0fef:	TW    r2, 0x1339
x0ff1:	CW    r1, r2
x0ff2:	JES   1
x0ff3:	HLT   040
x0ff4:	LWT   r3, 0
x0ff5:	MB    0x1328
x0ff7:	LPC   r3
x0ff8:	MB    0x1341
x0ffa:	TW    r1, 0x1338
x0ffc:	PW    r1, 0x132e
x0ffe:	TW    r1, 0x1339
x1000:	EM    r1, 0x132e
x1002:	CWT   r0, 0
x1003:	JES   1
x1004:	HLT   040
x1005:	TW    r1, 0x132e
x1007:	TW    r2, 0x1338
x1009:	CW    r1, r2
x100a:	JES   1
x100b:	HLT   040
x100c:	LWT   r3, 0
x100d:	MB    0x1328
x100f:	LPC   r3
x1010:	MB    0x1341
x1012:	LWT   r1, -1
x1013:	PW    r1, 0x132e
x1015:	LWT   r1, 0
x1016:	XM    r1, 0x132e
x1018:	CWT   r0, 0
x1019:	JES   1
x101a:	HLT   040
x101b:	TW    r1, 0x132e
x101d:	CWT   r1, -1
x101e:	JES   1
x101f:	HLT   040
x1020:	LWT   r3, 0
x1021:	MB    0x1328
x1023:	LPC   r3
x1024:	MB    0x1341
x1026:	PW    r0, 0x132e
x1028:	LWT   r1, -1
x1029:	XM    r1, 0x132e
x102b:	CWT   r0, 0
x102c:	JES   1
x102d:	HLT   040
x102e:	TW    r1, 0x132e
x1030:	CWT   r1, -1
x1031:	JES   1
x1032:	HLT   040
x1033:	LWT   r3, 0
x1034:	MB    0x1328
x1036:	LPC   r3
x1037:	MB    0x1341
x1039:	PW    r0, 0x132e
x103b:	LWT   r1, 0
x103c:	XM    r1, 0x132e
x103e:	CW    r0, 0x8000
x1040:	JES   1
x1041:	HLT   040
x1042:	TW    r1, 0x132e
x1044:	CWT   r1, 0
x1045:	JES   1
x1046:	HLT   040
x1047:	LWT   r3, 0
x1048:	MB    0x1328
x104a:	LPC   r3
x104b:	MB    0x1341
x104d:	LWT   r1, -1
x104e:	PW    r1, 0x132e
x1050:	XM    r1, 0x132e
x1052:	CW    r0, 0x8000
x1054:	JES   1
x1055:	HLT   040
x1056:	TW    r1, 0x132e
x1058:	CWT   r1, 0
x1059:	JES   1
x105a:	HLT   040
x105b:	LWT   r3, 0
x105c:	MB    0x1328
x105e:	LPC   r3
x105f:	MB    0x1341
x1061:	TW    r1, 0x1338
x1063:	PW    r1, 0x132e
x1065:	XM    r1, 0x132e
x1067:	CW    r0, 0x8000
x1069:	JES   1
x106a:	HLT   040
x106b:	TW    r1, 0x132e
x106d:	CWT   r1, 0
x106e:	JES   1
x106f:	HLT   040
x1070:	LWT   r3, 0
x1071:	MB    0x1328
x1073:	LPC   r3
x1074:	MB    0x1341
x1076:	TW    r1, 0x1339
x1078:	PW    r1, 0x132e
x107a:	XM    r1, 0x132e
x107c:	CW    r0, 0x8000
x107e:	JES   1
x107f:	HLT   040
x1080:	TW    r1, 0x132e
x1082:	CWT   r1, 0
x1083:	JES   1
x1084:	HLT   040
x1085:	LWT   r3, 0
x1086:	MB    0x1328
x1088:	LPC   r3
x1089:	MB    0x1341
x108b:	TW    r1, 0x1339
x108d:	PW    r1, 0x132e
x108f:	TW    r1, 0x1338
x1091:	XM    r1, 0x132e
x1093:	CWT   r0, 0
x1094:	JES   1
x1095:	HLT   040
x1096:	TW    r1, 0x132e
x1098:	CWT   r1, -1
x1099:	JES   1
x109a:	HLT   040
x109b:	LWT   r3, 0
x109c:	MB    0x1328
x109e:	LPC   r3
x109f:	MB    0x1341
x10a1:	TW    r1, 0x1338
x10a3:	PW    r1, 0x132e
x10a5:	TW    r1, 0x1339
x10a7:	XM    r1, 0x132e
x10a9:	CWT   r0, 0
x10aa:	JES   1
x10ab:	HLT   040
x10ac:	TW    r1, 0x132e
x10ae:	CWT   r1, -1
x10af:	JES   1
x10b0:	HLT   040
x10b1:	LWT   r1, -1
x10b2:	LWT   r2, -1
x10b3:	LWT   r3, -1
x10b4:	LWT   r4, -1
x10b5:	LWT   r5, -1
x10b6:	LWT   r6, -1
x10b7:	LWT   r7, -1
x10b8:	TD    0x1338
x10ba:	CW    r1, 0x5555
x10bc:	JES   1
x10bd:	HLT   040
x10be:	CW    r2, 0xaaaa
x10c0:	JES   1
x10c1:	HLT   040
x10c2:	CWT   r3, -1
x10c3:	JES   1
x10c4:	UJS   11
x10c5:	CWT   r4, -1
x10c6:	JES   1
x10c7:	UJS   8  ; -> 0x10d0
x10c8:	CWT   r5, -1
x10c9:	JES   1
x10ca:	UJS   5  ; -> 0x10d0
x10cb:	CWT   r6, -1
x10cc:	JES   1
x10cd:	UJS   2  ; -> 0x10d0
x10ce:	CWT   r7, -1
x10cf:	JES   1
x10d0:	HLT   040
x10d1:	LWT   r1, 0
x10d2:	LB    r1, 0x2670
x10d4:	CW    r1, 0x55
x10d6:	JES   1
x10d7:	HLT   040
x10d8:	LWT   r1, -1
x10d9:	LWT   r2, 0
x10da:	PW    r2, 0x132e
x10dc:	LB    r1, 0x265d
x10de:	CW    r1, 0xff00
x10e0:	JES   1
x10e1:	HLT   040
x10e2:	LWT   r1, -1
x10e3:	LWT   r2, 0
x10e4:	PW    r2, 0x132e
x10e6:	RB    r1, 0x265d
x10e8:	TW    r2, 0x132e
x10ea:	CW    r2, 0xff
x10ec:	JES   1
x10ed:	HLT   040
x10ee:	LWT   r1, -1
x10ef:	PW    r1, 0x132e
x10f1:	LWT   r1, 0
x10f2:	RB    r1, 0x265c
x10f4:	TW    r1, 0x132e
x10f6:	CW    r1, 0xff
x10f8:	JES   1
x10f9:	HLT   040
x10fa:	LWT   r3, 0
x10fb:	MB    0x1328
x10fd:	LPC   r3
x10fe:	MB    0x1341
x1100:	PW    r0, 0x132e
x1102:	LWT   r1, 0
x1103:	CB    r1, 0x265c
x1105:	CW    r0, 0x400
x1107:	JES   1
x1108:	HLT   040
x1109:	LWT   r1, -1
x110a:	CB    r1, 0x265d
x110c:	CW    r0, 0x200
x110e:	JES   1
x110f:	HLT   040
x1110:	LWT   r1, -1
x1111:	PW    r1, 0x132e
x1113:	LWT   r1, 0
x1114:	CB    r1, 0x265c
x1116:	CW    r0, 0x800
x1118:	JES   1
x1119:	HLT   040
x111a:	LW    r1, 0x1180
x111c:	RW    r1, 0x60
x111e:	LJ    0x131c
x1120:	MB    0x1328
x1122:	SP    0x1343
x1124:	LWT   r1, 0
x1125:	LW    r1, 0x1339
x1127:	CW    r1, 0x1339
x1129:	JES   1
x112a:	HLT   040
x112b:	LWT   r1, 0
x112c:	LW    r1, [pat10]
x112e:	CW    r1, 0xaaaa
x1130:	JES   1
x1131:	HLT   040
x1132:	LWT   r1, 0
x1133:	LW    r1, [pat01]
x1135:	CW    r1, 0x5555
x1137:	JES   1
x1138:	HLT   040
x1139:	LWT   r1, 0
x113a:	LW    r1, [0x133a]
x113c:	CWT   r1, 1
x113d:	JES   1
x113e:	HLT   040
x113f:	LWT   r1, 0
x1140:	RW    r1, 0x132e
x1142:	CW    r1, [0x132e]
x1144:	JES   1
x1145:	HLT   040
x1146:	LWT   r1, -1
x1147:	RW    r1, 0x132e
x1149:	CW    r1, [0x132e]
x114b:	JES   1
x114c:	HLT   040
x114d:	LW    r1, [pat10]
x114f:	RW    r1, 0x132e
x1151:	CW    r1, [0x132e]
x1153:	JES   1
x1154:	HLT   040
x1155:	LW    r1, [pat01]
x1157:	RW    r1, 0x132e
x1159:	CW    r1, [0x132e]
x115b:	JES   1
x115c:	HLT   040
x115d:	LW    r1, 0x132e
x115f:	RW    r1, 0x132f
x1161:	LWT   r2, 0
x1162:	RW    r2, [0x132f]
x1164:	CW    r2, [0x132e]
x1166:	JES   1
x1167:	HLT   040
x1168:	LWT   r2, -1
x1169:	RW    r2, [0x132f]
x116b:	CW    r2, [0x132e]
x116d:	JES   1
x116e:	HLT   040
x116f:	LW    r2, [pat01]
x1171:	RW    r2, [0x132f]
x1173:	CW    r2, [0x132e]
x1175:	JES   1
x1176:	HLT   040
x1177:	LW    r2, [pat10]
x1179:	RW    r2, [0x132f]
x117b:	CW    r2, [0x132e]
x117d:	JES   1
x117e:	HLT   040
x117f:	EXL   0
x1180:	UJS   1
x1181:	.word 0
x1182:	LW    r1, stack
x1184:	RW    r1, stackp
x1186:	MCL
x1187:	LWT   r1, 0
x1188:	LW    r2, [0x11ab+r1]
x118a:	RJ    r7, 0x119e
x118c:	AWT   r1, 1
x118d:	CWT   r1, 19
x118e:	JES   1
x118f:	UJS   -8 ; -> 0x1188
x1190:	LWT   r1, 0
x1191:	LW    r3, [0x11ca+r1]
x1193:	LW    r2, [0x11be+r1]
x1195:	RJ    r7, 0x119e
x1197:	DRB   r3, 4
x1198:	AWT   r1, 1
x1199:	CWT   r1, 12
x119a:	JES   59 ; -> 0x11d6
x119b:	UJS   -11; -> 0x1191
x119c:	AWT   r2, 1
x119d:	UJS   -9 ; -> 0x1195
x119e:	RWS   r2, 0
x119f:	.word 0
x11a0:	KI    0x1181
x11a2:	LWS   r4, -34
x11a3:	CW    r4, 0x200
x11a5:	JES   1
x11a6:	HLT   040
x11a7:	MCL
x11a8:	UJ    r7
x11a9:	.word 0x03ff
x11aa:	.word 0
x11ab:	.word 0
x11ac:	.word 0x0400
x11ad:	.word 0x0800
x11ae:	.word 0x0c00
x11af:	.word 0x1000
x11b0:	.word 0x1400
x11b1:	.word 0x1800
x11b2:	.word 0x1c00
x11b3:	.word 0x2000
x11b4:	.word 0x2400
x11b5:	.word 0x2800
x11b6:	.word 0x2c00
x11b7:	.word 0x3000
x11b8:	.word 0x3400
x11b9:	.word 0x3800
x11ba:	.word 0x3c00
x11bb:	.word 0xed40
x11bc:	.word 0xed80
x11bd:	.word 0xedc0
x11be:	.word 0xe80a
x11bf:	.word 0xea0a
x11c0:	.word 0xe818
x11c1:	.word 0xea18
x11c2:	.word 0xe820
x11c3:	.word 0xea20
x11c4:	.word 0xe828
x11c5:	.word 0xea28
x11c6:	.word 0xe830
x11c7:	.word 0xea30
x11c8:	.word 0xe838
x11c9:	.word 0xea38
x11ca:	.word 0x0006
x11cb:	.word 0x0006
x11cc:	.word 0x0008
x11cd:	.word 0x0008
x11ce:	.word 0x0008
x11cf:	.word 0x0008
x11d0:	.word 0x0008
x11d1:	.word 0x0008
x11d2:	.word 0x0008
x11d3:	.word 0x0008
x11d4:	.word 0x0008
x11d5:	.word 0x0008

; test rozkazow w bloku uzytkowym (Q=1, NB=15)

	LW    r1, illegalx
	RW    r1, int_illegal
	LW    r1, exlx
	RW    r1, exlp
	LF    0x1213
	RF    stack
	LW    r1, 0x66
	RW    r1, stackp
	LWT   r1, 1
	LW    r2, 0x41
	LWT   r3, 0
	OU    r1, r2
	.word nb1e1, nb1e2, nb1ok, nb1e3

x11ef:	MB    blok1
x11f1:	LD    0x1216
x11f3:	PD    0x101
x11f5:	LWT   r2, 0
x11f6:	LW    r1, [0x121f+r2]
x11f8:	PW    r1, 0x100
x11fa:	LIP

illegalx:
x11fb:	AWT   r2, 1
x11fc:	CWT   r2, 16
x11fd:	JES   5  ; -> 0x1203
x11fe:	LW    r1, 0x100
x1200:	RW    r1, 0x62
x1202:	UJS   -13; -> 0x11f6
x1203:	LW    r1, 0xcb
x1205:	RW    r1, 0x46
x1207:	LW    r1, 0x62
x1209:	RW    r1, 0x61
x120b:	UJ    0x122f

exlx:
x120d:	KI    0x1181
x120f:	LW    r4, [0x1181]
x1211:	HLT   040
x1212:	UJS   illegalx

x1213:	.word 0x0100
x1214:	.word 0x0000
x1215:	.word 0x0821
x1216:	EXL   0  ; .word
x1217:	UJS   -3 ; -> 0x1215
blok1:	.word 1
nb1e1:	AWT   r3, 1
nb1e2:	AWT   r3, 1
nb1e3:	AWT   r3, 1
x121c:	HLT   040
x121d:	UJS   -52; -> 0x11ea
nb1ok:	UJS   -48; -> 0x11ef
x121f:	HLT   040
x1220:	MCL
x1221:	CIT
x1222:	SIL
x1223:	SIU
x1224:	SIT
x1225:	GIU
x1226:	GIL
x1227:	LIP
x1228:	MB    0xfc40
x122a:	KI    0xfcc0
x122c:	SP    0x7400
x122e:	IN    r0, 0xe014
x1230:	DF    [r4+r7]
x1231:	DF    [r5+r7]
x1232:	.word 0x0002
x1233:	.word 0x0003
x1234:	DF    [r6+r7]
x1235:	DF    [r7+r7]
x1236:	.word 0x0002
x1237:	.word 0x0003
x1238:	DF    [r5+r7]
x1239:	DF    [r5+r7]
x123a:	.word 0x0003
x123b:	.word 0x0003
x123c:	DF    [r4+r7]
x123d:	DF    [r4+r7]
x123e:	.word 0x0000
x123f:	.word 0x0000
x1240:	DF    [r7+r7]
x1241:	DF    [r7+r7]
x1242:	.word 0x0003
x1243:	.word 0x0003
x1244:	MCL
x1245:	LWT   r4, -4
x1246:	LW    r1, [0x1234+r4]
x1248:	LW    r3, [0x1238+r4]
x124a:	RWS   r1, 41
x124b:	FI    0x1274
x124d:	SIU
x124e:	KI    0x1274
x1250:	LWS   r2, 35
x1251:	CW    r2, r3
x1252:	JES   1
x1253:	HLT   040
x1254:	IRB   r4, -15
x1255:	LWT   r4, -4
x1256:	LW    r1, [0x1234+r4]
x1258:	LW    r3, [0x123c+r4]
x125a:	RWS   r1, 25
x125b:	FI    0x1274
x125d:	SIL
x125e:	KI    0x1274
x1260:	LWS   r2, 19
x1261:	CW    r2, r3
x1262:	JES   1
x1263:	HLT   040
x1264:	IRB   r4, -15
x1265:	LWT   r4, -4
x1266:	LW    r1, [0x1234+r4]
x1268:	LW    r3, [0x1240+r4]
x126a:	RWS   r1, 9
x126b:	FI    0x1274
x126d:	CIT
x126e:	KI    0x1274
x1270:	LWS   r2, 3
x1271:	CW    r2, r3
x1272:	JES   2  ; -> 0x1275
x1273:	HLT   040
x1274:	.word 0
x1275:	IRB   r4, -16
x1276:	LWT   r4, -4
x1277:	LW    r1, [0x1234+r4]
x1279:	LW    r3, [0x1244+r4]
x127b:	RWS   r1, -8
x127c:	FI    0x1274
x127e:	SIT
x127f:	KI    0x1274
x1281:	LWS   r2, -14
x1282:	CW    r2, r3
x1283:	JES   1
x1284:	HLT   040
x1285:	IRB   r4, -15
x1286:	NOP
x1287:	RKY   r1
x1288:	BN    r1, 0x100
x128a:	UJS   28 ; -> 0x12a7
x128b:	UJ    0x135e
x128d:	.word 0
x128e:	.word 0
x128f:	IB    [0x12cc]
x1291:	IB    [r7+r7]
x1292:	IB    [0xff]
x1294:	.word 0x0066
x1295:	.word 0xe7ff
x1296:	AD    r4
x1297:	SD    r4
x1298:	MW    r4
x1299:	DW    r4
x129a:	AF    r4
x129b:	SF    r4
x129c:	MF    r4
x129d:	DF    r4
x129e:	.word 0x12d7
x129f:	.word 0x12d6
x12a0:	.word 0x12d5
x12a1:	.word 0x12d4
x12a2:	.word 0x12d3
x12a3:	.word 0x12d2
x12a4:	.word 0x12d1
x12a5:	.word 0x12d0
x12a6:	.word 0x12cf
x12a7:	MCL
x12a8:	LA    0x129e
x12aa:	RA    0x67
x12ac:	LD    0x12a5
x12ae:	RD    0x6e
x12b0:	LWT   r6, 9
x12b1:	RWS   r6, -36
x12b2:	LJ    0x12b8
x12b4:	LWS   r6, -39
x12b5:	DRB   r6, -5
x12b6:	UJ    0x135e

x12b8:	.res 1
x12b9:	LW    r2, stack
x12bb:	RW    r2, stackp
x12bd:	LWT   r3, -4
x12be:	RI    r2, 0
x12c0:	IRB   r3, -3
x12c1:	IM    0x128f
x12c3:	LWT   r0, -1
x12c4:	LW    r1, [0x1294+r6]
x12c6:	RW    r1, 0x12cb
x12c8:	LW    r4, 0xff
x12ca:	LWT   r7, 0
x12cb:	.word 0
x12cc:	HLT   040
x12cd:	UJ    [0x12b8]

x12cf:	AWT   r7, 1
x12d0:	AWT   r7, 1
x12d1:	AWT   r7, 1
x12d2:	AWT   r7, 1
x12d3:	AWT   r7, 1
x12d4:	AWT   r7, 1
x12d5:	AWT   r7, 1
x12d6:	AWT   r7, 1
x12d7:	AWT   r7, 1
x12d8:	LWS   r4, -14
x12d9:	RPC   r2
x12da:	CWT   r2, 0
x12db:	JES   2  ; -> 0x12de
x12dc:	HLT   040
x12dd:	LWT   r0, 0
x12de:	CW    r7, r6
x12df:	OR    r0, r6
x12e0:	JES   1
x12e1:	HLT   040
x12e2:	LF    0x62
x12e4:	LL    0x1290
x12e6:	CW    r1, r5
x12e7:	JES   1
x12e8:	HLT   040
x12e9:	CW    r2, r6
x12ea:	JES   1
x12eb:	HLT   040
x12ec:	CW    r3, r7
x12ed:	JES   1
x12ee:	HLT   040
x12ef:	LW    r3, [0x65]
x12f1:	LD    0x1293
x12f3:	CW    r3, r1
x12f4:	JES   1
x12f5:	HLT   040
x12f6:	LW    r1, [0x61]
x12f8:	CW    r1, r2
x12f9:	JES   1
x12fa:	HLT   040
x12fb:	LW    r1, 0x1306
x12fd:	RW    r1, 0x5f
x12ff:	LWT   r1, 1
x1300:	RW    r1, 0x128d
x1302:	FI    0x128d
x1304:	MCL
x1305:	UJS   3  ; -> 0x1309
x1306:	LW    r0, [0x128e]
x1308:	HLT   040
x1309:	LW    r1, 0xfd
x130b:	RW    r1, 0x5f
x130d:	UJ    [0x12b8]

x130f:	.res 1
x1310:	LW    r1, 0x1346
x1312:	LWT   r2, -24
x1313:	LW    r3, 0x1329
x1315:	LW    r4, [r1]
x1316:	PW    r4, r3
x1317:	AWT   r3, 1
x1318:	AWT   r1, 1
x1319:	IRB   r2, -5
x131a:	UJ    [0x130f]

x131c:	.res 1
x131d:	LW    r1, 0x1124
x131f:	LW    r2, [r1]
x1320:	PW    r2, r1
x1321:	AWT   r1, 1
x1322:	CW    r1, 0x1180
x1324:	JES   1
x1325:	UJS   -7 ; -> 0x131f
x1326:	UJ    [0x131c]

x1328:	.word 0x0000
x1329:	.word 0x0000
x132a:	.word 0x0001
x132b:	.word 0b1010101010101010
x132c:	.word 0
x132d:	.word 0
x132e:	.word 0
x132f:	.word 0
x1330:	.word 0
x1331:	.word 0
x1332:	.word 0
x1333:	.word 0
x1334:	.word 0
x1335:	.word 0
x1336:	.word 0
x1337:	.word 0
pat01:	.word 0b0101010101010101
pat10:	.word 0b1010101010101010
x133a:	.word 0x0001
x133b:	.word 0x0002
x133c:	.word 0x0003
x133d:	.word 0x0004
x133e:	.word 0x0005
x133f:	.word 0x0006
x1340:	.word 0x0007
x1341:	.word 0x100f
x1342:	.word 0x0041
x1343:	.word 0x1124
x1344:	.word 0x0000
x1345:	.word 0x002f
x1346:	.word 0x0000
x1347:	.word 0x0001
x1348:	.word 0b1010101010101010
x1349:	.word 0x0000
x134a:	.word 0x0000
x134b:	.word 0x0000
x134c:	.word 0x0000
x134d:	.word 0x0000
x134e:	.word 0x0000
x134f:	.word 0x0000
x1350:	.word 0x0000
x1351:	.word 0x0000
x1352:	.word 0x0000
x1353:	.word 0x0000
x1354:	.word 0x0000
x1355:	.word 0b0101010101010101
x1356:	.word 0b1010101010101010
x1357:	.word 0x0001
x1358:	.word 0x0002
x1359:	.word 0x0003
x135a:	.word 0x0004
x135b:	.word 0x0005
x135c:	.word 0x0006
x135d:	.word 0x0007
x135e:	UJS   1
x135f:	.word 0
x1360:	LW    r2, 0x62
x1362:	LW    r1, 0xffa2
x1364:	RI    r2, 0x0
x1366:	IRB   r1, -3
x1367:	LW    r1, 0x62
x1369:	RW    r1, 0x61
x136b:	MCL
x136c:	UJS   6  ; -> 0x1373
x136d:	.word 0
x136e:	.word 0
x136f:	.word 0
x1370:	IB    [r7+r7]
x1371:	PW    r5, r5+r2
x1372:	XR    r2, [r2+r5]
x1373:	LW    r1, [0x136f]
x1375:	RW    r1, 0x136e
x1377:	LJ    0x13b0
x1379:	LJ    0x1395
x137b:	LW    r1, [0x1370]
x137d:	RW    r1, 0x136e
x137f:	LJ    0x13b0
x1381:	LJ    0x1395
x1383:	LW    r1, [0x1371]
x1385:	RW    r1, 0x136e
x1387:	LJ    0x13b0
x1389:	LJ    0x1395
x138b:	LW    r1, [0x1372]
x138d:	RW    r1, 0x136e
x138f:	LJ    0x13b0
x1391:	LJ    0x1395
x1393:	UJ    0x145b

x1395:	.word 0
x1396:	LW    r1, [0x136f]
x1398:	RW    r1, 0x136d
x139a:	LJ    0x13c3
x139c:	LW    r1, [0x1370]
x139e:	RW    r1, 0x136d
x13a0:	LJ    0x13c3
x13a2:	LW    r1, [0x1371]
x13a4:	RW    r1, 0x136d
x13a6:	LJ    0x13c3
x13a8:	LW    r1, [0x1372]
x13aa:	RW    r1, 0x136d
x13ac:	LJ    0x13c3
x13ae:	UJ    [0x1395]

x13b0:	.word 0
x13b1:	LW    r1, [0x136e]
x13b3:	LW    r2, [0x136e]
x13b5:	LW    r3, [0x136e]
x13b7:	LW    r4, [0x136e]
x13b9:	LW    r5, [0x136e]
x13bb:	LW    r6, [0x136e]
x13bd:	LW    r7, [0x136e]
x13bf:	LW    r0, [0x136e]
x13c1:	UJ    [0x13b0]

x13c3:	.word 0
x13c4:	LW    r1, [0x136e]
x13c6:	LW    r1, [0x136d]
x13c8:	CW    r1, [0x136d]
x13ca:	JES   1
x13cb:	HLT   040
x13cc:	LW    r1, [0x136e]
x13ce:	LJ    0x1418
x13d0:	LW    r2, [0x136d]
x13d2:	CW    r2, [0x136d]
x13d4:	JES   1
x13d5:	HLT   040
x13d6:	LW    r2, [0x136e]
x13d8:	LJ    0x1418
x13da:	LW    r3, [0x136d]
x13dc:	CW    r3, [0x136d]
x13de:	JES   1
x13df:	HLT   040
x13e0:	LW    r3, [0x136e]
x13e2:	LJ    0x1418
x13e4:	LW    r4, [0x136d]
x13e6:	CW    r4, [0x136d]
x13e8:	JES   1
x13e9:	HLT   040
x13ea:	LW    r4, [0x136e]
x13ec:	LJ    0x1418
x13ee:	LW    r5, [0x136d]
x13f0:	CW    r5, [0x136d]
x13f2:	JES   1
x13f3:	HLT   040
x13f4:	LW    r5, [0x136e]
x13f6:	LJ    0x1418
x13f8:	LW    r6, [0x136d]
x13fa:	CW    r6, [0x136d]
x13fc:	JES   1
x13fd:	HLT   040
x13fe:	LW    r6, [0x136e]
x1400:	LJ    0x1418
x1402:	LW    r7, [0x136d]
x1404:	CW    r7, [0x136d]
x1406:	JES   1
x1407:	HLT   040
x1408:	LW    r7, [0x136e]
x140a:	LJ    0x1418
x140c:	LW    r0, [0x136d]
x140e:	CW    r0, [0x136d]
x1410:	JES   1
x1411:	HLT   040
x1412:	LW    r0, [0x136e]
x1414:	LJ    0x1418
x1416:	UJ    [0x13c3]

x1418:	.word 0
x1419:	CW    r1, [0x136e]
x141b:	JES   1
x141c:	HLT   040
x141d:	CW    r2, [0x136e]
x141f:	JES   1
x1420:	HLT   040
x1421:	CW    r3, [0x136e]
x1423:	JES   1
x1424:	HLT   040
x1425:	CW    r4, [0x136e]
x1427:	JES   1
x1428:	HLT   040
x1429:	CW    r5, [0x136e]
x142b:	JES   1
x142c:	HLT   040
x142d:	CW    r6, [0x136e]
x142f:	JES   1
x1430:	HLT   040
x1431:	CW    r7, [0x136e]
x1433:	JES   1
x1434:	HLT   040
x1435:	RW    r0, 0x145a
x1437:	LW    r7, [0x145a]
x1439:	CW    r1, [0x136f]
x143b:	JES   10
x143c:	CW    r1, [0x1370]
x143e:	JES   11
x143f:	CW    r1, [0x1371]
x1441:	JES   12
x1442:	CW    r7, [0x1459]
x1444:	JES   13
x1445:	UJS   11
x1446:	CW    r7, [0x1456]
x1448:	JES   9  ; -> 0x1452
x1449:	UJS   7  ; -> 0x1451
x144a:	CW    r7, [0x1457]
x144c:	JES   5  ; -> 0x1452
x144d:	UJS   3  ; -> 0x1451
x144e:	CW    r1, [0x1458]
x1450:	JES   1
x1451:	HLT   040
x1452:	LW    r7, [0x136e]
x1454:	UJ    [0x1418]

x1456:	.word 0x0400
x1457:	TL    r7+r7
x1458:	PW    r5, r5+r2
x1459:	EM    r2, r2+r5
x145a:	.word 0
x145b:	LW    r1, [0x136f]
x145d:	RW    r1, 0x136e
x145f:	LJ    0x13b0
x1461:	LJ    0x147d
x1463:	LW    r1, [0x1370]
x1465:	RW    r1, 0x136e
x1467:	LJ    0x13b0
x1469:	LJ    0x147d
x146b:	LW    r1, [0x1371]
x146d:	RW    r1, 0x136e
x146f:	LJ    0x13b0
x1471:	LJ    0x147d
x1473:	LW    r1, [0x1372]
x1475:	RW    r1, 0x136e
x1477:	LJ    0x13b0
x1479:	LJ    0x147d
x147b:	UJ    0x14ef

x147d:	.res 1
x147e:	LW    r1, [0x136f]
x1480:	RW    r1, 0x136d
x1482:	LJ    0x1498
x1484:	LW    r1, [0x1370]
x1486:	RW    r1, 0x136d
x1488:	LJ    0x1498
x148a:	LW    r1, [0x1371]
x148c:	RW    r1, 0x136d
x148e:	LJ    0x1498
x1490:	LW    r1, [0x1372]
x1492:	RW    r1, 0x136d
x1494:	LJ    0x1498
x1496:	UJ    [0x147d]

x1498:	.res 1
x1499:	LW    r1, [0x136d]
x149b:	LW    r2, r1
x149c:	CW    r1, r2
x149d:	JES   1
x149e:	HLT   040
x149f:	LW    r1, [0x136e]
x14a1:	LW    r2, [0x136e]
x14a3:	LJ    0x1418
x14a5:	LW    r2, [0x136d]
x14a7:	LW    r3, r2
x14a8:	CW    r2, r3
x14a9:	JES   1
x14aa:	HLT   040
x14ab:	LW    r2, [0x136e]
x14ad:	LW    r3, [0x136e]
x14af:	LJ    0x1418
x14b1:	LW    r3, [0x136d]
x14b3:	LW    r4, r3
x14b4:	CW    r3, r4
x14b5:	JES   1
x14b6:	HLT   040
x14b7:	LW    r3, [0x136e]
x14b9:	LW    r4, [0x136e]
x14bb:	LJ    0x1418
x14bd:	LW    r4, [0x136d]
x14bf:	LW    r5, r4
x14c0:	CW    r4, r5
x14c1:	JES   1
x14c2:	HLT   040
x14c3:	LW    r4, [0x136e]
x14c5:	LW    r5, [0x136e]
x14c7:	LJ    0x1418
x14c9:	LW    r5, [0x136d]
x14cb:	LW    r6, r5
x14cc:	CW    r5, r6
x14cd:	JES   1
x14ce:	HLT   040
x14cf:	LW    r5, [0x136e]
x14d1:	LW    r6, [0x136e]
x14d3:	LJ    0x1418
x14d5:	LW    r6, [0x136d]
x14d7:	LW    r7, r6
x14d8:	CW    r6, r7
x14d9:	JES   1
x14da:	HLT   040
x14db:	LW    r6, [0x136e]
x14dd:	LW    r7, [0x136e]
x14df:	LJ    0x1418
x14e1:	LW    r7, [0x136d]
x14e3:	LW    r1, r7
x14e4:	CW    r7, r1
x14e5:	JES   1
x14e6:	HLT   040
x14e7:	LW    r7, [0x136e]
x14e9:	LW    r1, [0x136e]
x14eb:	LJ    0x1418
x14ed:	UJ    [0x1498]

x14ef:	UJS   1
x14f0:	.word 0
x14f1:	LJ    reset
x14f3:	UJS   6  ; -> 0x14fa
x14f4:	.word 0x000d
x14f5:	.word 0x000f
x14f6:	.word 0x001c
x14f7:	.word 0x0038
x14f8:	.word 0x14f9
x14f9:	IB    [r7+r7]
x14fa:	RIC   r1
x14fb:	CW    r1, 0x14fb
x14fd:	JES   1
x14fe:	HLT   040
x14ff:	LW    r2, [0x14f5]
x1501:	LW    r3, [0x14f4]
x1503:	MD    r2
x1504:	LW    r1, r3
x1505:	CW    r1, [0x14f6]
x1507:	JES   1
x1508:	HLT   040
x1509:	LW    r2, [0x14f4]
x150b:	LW    r3, [0x14f5]
x150d:	LW    r1, r2+r3
x150e:	CW    r1, [0x14f6]
x1510:	JES   1
x1511:	HLT   040
x1512:	LW    r4, [0x14f6]
x1514:	LW    r2, [0x14f4]
x1516:	LW    r3, [0x14f5]
x1518:	MD    r1
x1519:	LW    r1, r2+r3
x151a:	CW    r1, [0x14f7]
x151c:	JES   1
x151d:	HLT   040
x151e:	LW    r2, 0x14f4
x1520:	LW    r1, [r2]
x1521:	CW    r1, [0x14f4]
x1523:	JES   1
x1524:	HLT   040
x1525:	LWT   r1, 1
x1526:	LW    r2, 0x14f5
x1528:	MD    r1
x1529:	LW    r1, [r2]
x152a:	CW    r1, [0x14f6]
x152c:	JES   1
x152d:	HLT   040
x152e:	LWT   r3, 1
x152f:	LW    r2, 0x14f6
x1531:	LW    r1, [r2+r3]
x1532:	CW    r1, [0x14f7]
x1534:	JES   1
x1535:	HLT   040
x1536:	LWT   r1, 1
x1537:	LWT   r3, 1
x1538:	LW    r2, 0x14f5
x153a:	MD    r1
x153b:	LW    r1, [r3+r2]
x153c:	CW    r1, [0x14f7]
x153e:	JES   1
x153f:	HLT   040
x1540:	LW    r1, -1
x1542:	RW    r1, 0x14f9
x1544:	LW    r2, 0x14f9
x1546:	RZ    r2
x1547:	LW    r1, [0x14f9]
x1549:	CWT   r1, 0
x154a:	JES   1
x154b:	HLT   040
x154c:	LW    r1, -1
x154e:	RW    r1, 0x14f9
x1550:	LW    r2, 0x14f8
x1552:	LWT   r3, 1
x1553:	MD    r3
x1554:	RZ    r2
x1555:	LW    r1, [0x14f9]
x1557:	CWT   r1, 0
x1558:	JES   1
x1559:	HLT   040
x155a:	LW    r1, -1
x155c:	RW    r1, 0x14f9
x155e:	LWT   r3, 1
x155f:	LW    r2, 0x14f8
x1561:	RZ    r2+r3
x1562:	LW    r1, [0x14f9]
x1564:	CWT   r1, 0
x1565:	JES   1
x1566:	HLT   040
x1567:	LW    r1, -1
x1569:	RW    r1, 0x14f9
x156b:	LWT   r3, 1
x156c:	LWT   r1, 1
x156d:	LW    r2, 0x14f7
x156f:	MD    r1
x1570:	RZ    r2+r3
x1571:	LW    r1, [0x14f9]
x1573:	CWT   r1, 0
x1574:	JES   1
x1575:	HLT   040
x1576:	LW    r1, -1
x1578:	RW    r1, 0x14f9
x157a:	LW    r2, 0x14f8
x157c:	RZ    [r2]
x157d:	LW    r1, [0x14f9]
x157f:	CWT   r1, 0
x1580:	JES   1
x1581:	HLT   040
x1582:	LW    r1, -1
x1584:	RW    r1, 0x14f9
x1586:	LW    r2, 0x14f7
x1588:	LWT   r3, 1
x1589:	MD    r3
x158a:	RZ    [r2]
x158b:	LW    r1, [0x14f9]
x158d:	CWT   r1, 0
x158e:	JES   1
x158f:	HLT   040
x1590:	LW    r1, -1
x1592:	RW    r1, 0x14f9
x1594:	LW    r2, 0x14f7
x1596:	LWT   r3, 1
x1597:	RZ    [r2+r3]
x1598:	LW    r1, [0x14f9]
x159a:	CWT   r1, 0
x159b:	JES   1
x159c:	HLT   040
x159d:	LW    r1, -1
x159f:	RW    r1, 0x14f9
x15a1:	LW    r2, 0x14f6
x15a3:	LWT   r3, 1
x15a4:	LWT   r1, 1
x15a5:	MD    r1
x15a6:	RZ    [r2+r3]
x15a7:	LW    r1, [0x14f9]
x15a9:	CWT   r1, 0
x15aa:	JES   1
x15ab:	HLT   040
x15ac:	LW    r3, 0x14f7
x15ae:	MD    r3
x15af:	LWT   r1, 2
x15b0:	CW    r1, 0x14f9
x15b2:	JES   1
x15b3:	HLT   040
x15b4:	LWT   r0, 0
x15b5:	BRC   64
x15b6:	HLT   040
x15b7:	LW    r1, 0xff
x15b9:	LW    r0, 0xff
x15bb:	MD    r1
x15bc:	BRC   64
x15bd:	HLT   040
x15be:	LWT   r1, 1
x15bf:	MD    r1
x15c0:	LW    r2, 0x14f4
x15c2:	CW    r2, 0x14f5
x15c4:	JES   1
x15c5:	HLT   040
x15c6:	LWT   r1, 1
x15c7:	LW    r2, 0x14f4+r1
x15c9:	CW    r2, 0x14f5
x15cb:	JES   1
x15cc:	HLT   040
x15cd:	LWT   r1, 1
x15ce:	LWT   r3, 1
x15cf:	MD    r1
x15d0:	LW    r2, 0x14f4+r3
x15d2:	CW    r2, 0x14f6
x15d4:	JES   1
x15d5:	HLT   040
x15d6:	LWT   r1, 1
x15d7:	MD    r1
x15d8:	LW    r2, [0x14f7]
x15da:	CW    r2, 0x14f9
x15dc:	JES   1
x15dd:	HLT   040
x15de:	LWT   r1, 1
x15df:	LW    r2, [0x14f7+r1]
x15e1:	CW    r2, 0x14f9
x15e3:	JES   1
x15e4:	HLT   040
x15e5:	LWT   r1, 1
x15e6:	LWT   r3, 1
x15e7:	MD    r1
x15e8:	LW    r2, [0x14f6+r3]
x15ea:	CW    r2, 0x14f9
x15ec:	JES   1
x15ed:	HLT   040
x15ee:	LW    r1, -1
x15f0:	RW    r1, 0x14f9
x15f2:	LWT   r1, 1
x15f3:	MD    r1
x15f4:	RZ    0x14f8
x15f6:	LW    r2, [0x14f9]
x15f8:	CWT   r2, 0
x15f9:	JES   1
x15fa:	HLT   040
x15fb:	LW    r1, -1
x15fd:	RW    r1, 0x14f9
x15ff:	LWT   r1, 1
x1600:	RZ    0x14f8+r1
x1602:	LW    r2, [0x14f9]
x1604:	CWT   r2, 0
x1605:	JES   1
x1606:	HLT   040
x1607:	LW    r1, -1
x1609:	RW    r1, 0x14f9
x160b:	LWT   r1, 1
x160c:	LWT   r3, 1
x160d:	MD    r1
x160e:	RZ    0x14f7+r3
x1610:	LW    r2, [0x14f9]
x1612:	CWT   r2, 0
x1613:	JES   1
x1614:	HLT   040
x1615:	LW    r1, -1
x1617:	RW    r1, 0x14f9
x1619:	LWT   r1, 1
x161a:	MD    r1
x161b:	RZ    [0x14f7]
x161d:	LW    r2, [0x14f9]
x161f:	CWT   r2, 0
x1620:	JES   1
x1621:	HLT   040
x1622:	LW    r1, -1
x1624:	RW    r1, 0x14f9
x1626:	LWT   r1, 1
x1627:	RZ    [0x14f7+r1]
x1629:	LW    r2, [0x14f9]
x162b:	CWT   r2, 0
x162c:	JES   1
x162d:	HLT   040
x162e:	LW    r1, -1
x1630:	RW    r1, 0x14f9
x1632:	LWT   r1, 1
x1633:	LWT   r3, 1
x1634:	MD    r1
x1635:	RZ    [0x14f6+r3]
x1637:	LW    r2, [0x14f9]
x1639:	CWT   r2, 0
x163a:	JES   1
x163b:	HLT   040
x163c:	MCL
x163d:	UJS   3  ; -> 0x1641
int00:	HLT   040
x163f:	UJS   int00
x1640:	HLT   040
x1641:	LWT   r3, 1
x1642:	LW    r4, 0x41
x1644:	OU    r3, r4
x1645:	.word 0x1640, 0x1649, 0x1650, 0x164b
x1649:	HLT   040
x164a:	UJS   -7 ; -> 0x1644
x164b:	HLT   040
x164c:	UJS   -9 ; -> 0x1644
x164d:	.word 0
x164e:	.word 0
int_store:
	.res 1
x1650:	LJ    reset
x1652:	RZ    0x164d
x1654:	IM    0x164d
x1656:	LW    r7, 0x7fff
x1658:	RW    r7, 0x164e
x165a:	FI    0x164e
x165c:	LW    r1, 0x1668
x165e:	RW    r1, 0x41
x1660:	LWT   r0, -1
x1661:	LW    r1, 0x8000
x1663:	RW    r1, 0x164d
x1665:	IM    0x164d
x1667:	UJS   39 ; -> 0x168f
x1668:	LA    stack
x166a:	CW    r1, 0x1667
x166c:	JES   1
x166d:	HLT   040
x166e:	CWT   r2, -1
x166f:	JES   1
x1670:	HLT   040
x1671:	CW    r3, 0x8000
x1673:	JES   1
x1674:	HLT   040
x1675:	CWT   r4, 0
x1676:	JES   1
x1677:	HLT   040
x1678:	LW    r1, [0x61]
x167a:	CW    r1, 0x66
x167c:	JES   1
x167d:	HLT   040
x167e:	CW    r0, 0x400
x1680:	JES   1
x1681:	HLT   040
x1682:	KI    int_store
x1684:	LW    r1, [int_store]
x1686:	CW    r1, 0x3fff
x1688:	JES   1
x1689:	HLT   040
x168a:	LW    r1, 0xc1
x168c:	RW    r1, 0x41
x168e:	LIP
x168f:	LW    r1, 0x169b
x1691:	RW    r1, 0x42
x1693:	LW    r1, 0x4000
x1695:	RW    r1, 0x164d
x1697:	LWT   r0, -1
x1698:	IM    0x164d
x169a:	UJS   39 ; -> 0x16c2
x169b:	LA    stack
x169d:	CW    r1, 0x169a
x169f:	JES   1
x16a0:	HLT   040
x16a1:	CWT   r2, -1
x16a2:	JES   1
x16a3:	HLT   040
x16a4:	CW    r3, 0x4000
x16a6:	JES   1
x16a7:	HLT   040
x16a8:	CWT   r4, 0
x16a9:	JES   1
x16aa:	HLT   040
x16ab:	LW    r1, [0x61]
x16ad:	CW    r1, 0x66
x16af:	JES   1
x16b0:	HLT   040
x16b1:	CW    r0, 0x400
x16b3:	JES   1
x16b4:	HLT   040
x16b5:	KI    int_store
x16b7:	LW    r1, [int_store]
x16b9:	CW    r1, 0x1fff
x16bb:	JES   1
x16bc:	HLT   040
x16bd:	LW    r1, 0xc3
x16bf:	RW    r1, 0x42
x16c1:	LIP

x16c2:	LW    r1, 0x16ce
x16c4:	RW    r1, 0x43
x16c6:	LW    r1, 0x2000
x16c8:	RW    r1, 0x164d
x16ca:	LWT   r0, -1
x16cb:	IM    0x164d
x16cd:	UJS   39 ; -> 0x16f5
x16ce:	LA    stack
x16d0:	CW    r1, 0x16cd
x16d2:	JES   1
x16d3:	HLT   040
x16d4:	CWT   r2, -1
x16d5:	JES   1
x16d6:	HLT   040
x16d7:	CW    r3, 0x2000
x16d9:	JES   1
x16da:	HLT   040
x16db:	CWT   r4, 0
x16dc:	JES   1
x16dd:	HLT   040
x16de:	LW    r1, [0x61]
x16e0:	CW    r1, 0x66
x16e2:	JES   1
x16e3:	HLT   040
x16e4:	CW    r0, 0x400
x16e6:	JES   1
x16e7:	HLT   040
x16e8:	KI    int_store
x16ea:	LW    r1, [int_store]
x16ec:	CW    r1, 0xfff
x16ee:	JES   1
x16ef:	HLT   040
x16f0:	LW    r1, 0xc5
x16f2:	RW    r1, 0x43
x16f4:	LIP

x16f5:	LW    r1, 0x1701
x16f7:	RW    r1, 0x44
x16f9:	LW    r1, 0x1000
x16fb:	RW    r1, 0x164d
x16fd:	LWT   r0, -1
x16fe:	IM    0x164d
x1700:	UJS   39 ; -> 0x1728
x1701:	LA    stack
x1703:	CW    r1, 0x1700
x1705:	JES   1
x1706:	HLT   040
x1707:	CWT   r2, -1
x1708:	JES   1
x1709:	HLT   040
x170a:	CW    r3, 0x1000
x170c:	JES   1
x170d:	HLT   040
x170e:	CWT   r4, 0
x170f:	JES   1
x1710:	HLT   040
x1711:	LW    r1, [0x61]
x1713:	CW    r1, 0x66
x1715:	JES   1
x1716:	HLT   040
x1717:	CW    r0, 0x400
x1719:	JES   1
x171a:	HLT   040
x171b:	KI    int_store
x171d:	LW    r1, [int_store]
x171f:	CW    r1, 0x7ff
x1721:	JES   1
x1722:	HLT   040
x1723:	LW    r1, 0xc7
x1725:	RW    r1, 0x44
x1727:	LIP

x1728:	LW    r1, 0x1741
x172a:	LW    r2, 0x1768
x172c:	LW    r3, 0x178f
x172e:	LW    r4, 0x17b6
x1730:	LW    r5, 0x17dd
x1732:	LW    r6, 0x1804
x1734:	LW    r7, 0x182b
x1736:	RA    0x45
x1738:	LW    r1, 0x800
x173a:	RW    r1, 0x164d
x173c:	LWT   r0, -1
x173d:	IM    0x164d
x173f:	UJ    0x1852
x1741:	LA    stack
x1743:	CW    r1, 0x173f
x1745:	JES   1
x1746:	HLT   040
x1747:	CWT   r2, -1
x1748:	JES   1
x1749:	HLT   040
x174a:	CW    r3, 0x800
x174c:	JES   1
x174d:	HLT   040
x174e:	CWT   r4, 0
x174f:	JES   1
x1750:	HLT   040
x1751:	LW    r1, [0x61]
x1753:	CW    r1, 0x66
x1755:	JES   1
x1756:	HLT   040
x1757:	CW    r0, 0x400
x1759:	JES   1
x175a:	HLT   040
x175b:	KI    int_store
x175d:	LW    r1, [int_store]
x175f:	CW    r1, 0x3ff
x1761:	JES   1
x1762:	HLT   040
x1763:	LW    r1, 0xc9
x1765:	RW    r1, 0x45
x1767:	LIP

x1768:	LA    stack
x176a:	CW    r1, 0x173f
x176c:	JES   1
x176d:	HLT   040
x176e:	CWT   r2, -1
x176f:	JES   1
x1770:	HLT   040
x1771:	CW    r3, 0x800
x1773:	JES   1
x1774:	HLT   040
x1775:	CWT   r4, 0
x1776:	JES   1
x1777:	HLT   040
x1778:	LW    r1, [0x61]
x177a:	CW    r1, 0x66
x177c:	JES   1
x177d:	HLT   040
x177e:	CW    r0, 0x400
x1780:	JES   1
x1781:	HLT   040
x1782:	KI    int_store
x1784:	LW    r1, [int_store]
x1786:	CW    r1, 0x1ff
x1788:	JES   1
x1789:	HLT   040
x178a:	LW    r1, 0xcb
x178c:	RW    r1, 0x46
x178e:	LIP

x178f:	LA    stack
x1791:	CW    r1, 0x173f
x1793:	JES   1
x1794:	HLT   040
x1795:	CWT   r2, -1
x1796:	JES   1
x1797:	HLT   040
x1798:	CW    r3, 0x800
x179a:	JES   1
x179b:	HLT   040
x179c:	CWT   r4, 0
x179d:	JES   1
x179e:	HLT   040
x179f:	LW    r1, [0x61]
x17a1:	CW    r1, 0x66
x17a3:	JES   1
x17a4:	HLT   040
x17a5:	CW    r0, 0x400
x17a7:	JES   1
x17a8:	HLT   040
x17a9:	KI    int_store
x17ab:	LW    r1, [int_store]
x17ad:	CW    r1, 0xff
x17af:	JES   1
x17b0:	HLT   040
x17b1:	LW    r1, 0xcd
x17b3:	RW    r1, 0x47
x17b5:	LIP

x17b6:	LA    stack
x17b8:	CW    r1, 0x173f
x17ba:	JES   1
x17bb:	HLT   040
x17bc:	CWT   r2, -1
x17bd:	JES   1
x17be:	HLT   040
x17bf:	CW    r3, 0x800
x17c1:	JES   1
x17c2:	HLT   040
x17c3:	CWT   r4, 0
x17c4:	JES   1
x17c5:	HLT   040
x17c6:	LW    r1, [0x61]
x17c8:	CW    r1, 0x66
x17ca:	JES   1
x17cb:	HLT   040
x17cc:	CW    r0, 0x400
x17ce:	JES   1
x17cf:	HLT   040
x17d0:	KI    int_store
x17d2:	LW    r1, [int_store]
x17d4:	CW    r1, 0x7f
x17d6:	JES   1
x17d7:	HLT   040
x17d8:	LW    r1, 0xcf
x17da:	RW    r1, 0x48
x17dc:	LIP

x17dd:	LA    stack
x17df:	CW    r1, 0x173f
x17e1:	JES   1
x17e2:	HLT   040
x17e3:	CWT   r2, -1
x17e4:	JES   1
x17e5:	HLT   040
x17e6:	CW    r3, 0x800
x17e8:	JES   1
x17e9:	HLT   040
x17ea:	CWT   r4, 0
x17eb:	JES   1
x17ec:	HLT   040
x17ed:	LW    r1, [0x61]
x17ef:	CW    r1, 0x66
x17f1:	JES   1
x17f2:	HLT   040
x17f3:	CW    r0, 0x400
x17f5:	JES   1
x17f6:	HLT   040
x17f7:	KI    int_store
x17f9:	LW    r1, [int_store]
x17fb:	CW    r1, 0x3f
x17fd:	JES   1
x17fe:	HLT   040
x17ff:	LW    r1, 0xd1
x1801:	RW    r1, 0x49
x1803:	LIP

x1804:	LA    stack
x1806:	CW    r1, 0x173f
x1808:	JES   1
x1809:	HLT   040
x180a:	CWT   r2, -1
x180b:	JES   1
x180c:	HLT   040
x180d:	CW    r3, 0x800
x180f:	JES   1
x1810:	HLT   040
x1811:	CWT   r4, 0
x1812:	JES   1
x1813:	HLT   040
x1814:	LW    r1, [0x61]
x1816:	CW    r1, 0x66
x1818:	JES   1
x1819:	HLT   040
x181a:	CW    r0, 0x400
x181c:	JES   1
x181d:	HLT   040
x181e:	KI    int_store
x1820:	LW    r1, [int_store]
x1822:	CW    r1, 0x1f
x1824:	JES   1
x1825:	HLT   040
x1826:	LW    r1, 0xd3
x1828:	RW    r1, 0x4a
x182a:	LIP

x182b:	LA    stack
x182d:	CW    r1, 0x173f
x182f:	JES   1
x1830:	HLT   040
x1831:	CWT   r2, -1
x1832:	JES   1
x1833:	HLT   040
x1834:	CW    r3, 0x800
x1836:	JES   1
x1837:	HLT   040
x1838:	CWT   r4, 0
x1839:	JES   1
x183a:	HLT   040
x183b:	LW    r1, [0x61]
x183d:	CW    r1, 0x66
x183f:	JES   1
x1840:	HLT   040
x1841:	CW    r0, 0x400
x1843:	JES   1
x1844:	HLT   040
x1845:	KI    int_store
x1847:	LW    r1, [int_store]
x1849:	CW    r1, 0xf
x184b:	JES   1
x184c:	HLT   040
x184d:	LW    r1, 0xd5
x184f:	RW    r1, 0x4b
x1851:	LIP

x1852:	LW    r1, 0x1867
x1854:	LW    r2, 0x188d
x1856:	LW    r3, 0x18b3
x1858:	LW    r4, 0x18d9
x185a:	RF    0x5c
x185c:	RW    r4, 0x5f
x185e:	LW    r1, 0x40
x1860:	RW    r1, 0x164d
x1862:	LWT   r0, -1
x1863:	IM    0x164d
x1865:	UJ    0x18ff
x1867:	LA    stack
x1869:	CW    r1, 0x1865
x186b:	JES   1
x186c:	HLT   040
x186d:	CWT   r2, -1
x186e:	JES   1
x186f:	HLT   040
x1870:	CW    r3, 0x40
x1872:	JES   1
x1873:	HLT   040
x1874:	CWT   r4, 0
x1875:	JES   1
x1876:	HLT   040
x1877:	LW    r1, [0x61]
x1879:	CW    r1, 0x66
x187b:	JES   1
x187c:	HLT   040
x187d:	CW    r0, 0x400
x187f:	JES   1
x1880:	HLT   040
x1881:	KI    int_store
x1883:	LW    r1, [int_store]
x1885:	CWT   r1, 7
x1886:	JES   1
x1887:	HLT   040
x1888:	LW    r1, 0xf7
x188a:	RW    r1, 0x5c
x188c:	LIP

x188d:	LA    stack
x188f:	CW    r1, 0x1865
x1891:	JES   1
x1892:	HLT   040
x1893:	CWT   r2, -1
x1894:	JES   1
x1895:	HLT   040
x1896:	CW    r3, 0x40
x1898:	JES   1
x1899:	HLT   040
x189a:	CWT   r4, 0
x189b:	JES   1
x189c:	HLT   040
x189d:	LW    r1, [0x61]
x189f:	CW    r1, 0x66
x18a1:	JES   1
x18a2:	HLT   040
x18a3:	CW    r0, 0x400
x18a5:	JES   1
x18a6:	HLT   040
x18a7:	KI    int_store
x18a9:	LW    r1, [int_store]
x18ab:	CWT   r1, 3
x18ac:	JES   1
x18ad:	HLT   040
x18ae:	LW    r1, 0xf9
x18b0:	RW    r1, 0x5d
x18b2:	LIP

x18b3:	LA    stack
x18b5:	CW    r1, 0x1865
x18b7:	JES   1
x18b8:	HLT   040
x18b9:	CWT   r2, -1
x18ba:	JES   1
x18bb:	HLT   040
x18bc:	CW    r3, 0x40
x18be:	JES   1
x18bf:	HLT   040
x18c0:	CWT   r4, 0
x18c1:	JES   1
x18c2:	HLT   040
x18c3:	LW    r1, [0x61]
x18c5:	CW    r1, 0x66
x18c7:	JES   1
x18c8:	HLT   040
x18c9:	CW    r0, 0x400
x18cb:	JES   1
x18cc:	HLT   040
x18cd:	KI    int_store
x18cf:	LW    r1, [int_store]
x18d1:	CWT   r1, 1
x18d2:	JES   1
x18d3:	HLT   040
x18d4:	LW    r1, 0xfb
x18d6:	RW    r1, 0x5e
x18d8:	LIP

x18d9:	LA    stack
x18db:	CW    r1, 0x1865
x18dd:	JES   1
x18de:	HLT   040
x18df:	CWT   r2, -1
x18e0:	JES   1
x18e1:	HLT   040
x18e2:	CW    r3, 0x40
x18e4:	JES   1
x18e5:	HLT   040
x18e6:	CWT   r4, 0
x18e7:	JES   1
x18e8:	HLT   040
x18e9:	LW    r1, [0x61]
x18eb:	CW    r1, 0x66
x18ed:	JES   1
x18ee:	HLT   040
x18ef:	CW    r0, 0x400
x18f1:	JES   1
x18f2:	HLT   040
x18f3:	KI    int_store
x18f5:	LW    r1, [int_store]
x18f7:	CWT   r1, 0
x18f8:	JES   1
x18f9:	HLT   040
x18fa:	LW    r1, 0xfd
x18fc:	RW    r1, 0x5f
x18fe:	LIP

x18ff:	LW    r1, 0x1906
x1901:	RW    r1, 0x5c
x1903:	LWT   r0, -1
x1904:	FI    0x164e
x1906:	LW    r1, [stack]
x1908:	CW    r1, 0x1906
x190a:	JES   1
x190b:	HLT   040
x190c:	KI    int_store
x190e:	LW    r1, [int_store]
x1910:	CW    r1, 0x7ff7
x1912:	JES   1
x1913:	HLT   040
x1914:	LW    r1, 0xf7
x1916:	RW    r1, 0x5c
x1918:	LW    r1, 0x1923
x191a:	RW    r1, 0x5d
x191c:	LWT   r0, -1
x191d:	LW    r1, 0x40
x191f:	RW    r1, 0x164d
x1921:	IM    0x164d
x1923:	LW    r1, [0x66]
x1925:	CW    r1, 0x1923
x1927:	JES   1
x1928:	HLT   040
x1929:	KI    int_store
x192b:	LW    r1, [int_store]
x192d:	CW    r1, 0x7ff3
x192f:	JES   1
x1930:	HLT   040
x1931:	LW    r1, 0xf9
x1933:	RW    r1, 0x5d
x1935:	LW    r1, 0x193c
x1937:	RW    r1, 0x5e
x1939:	LWT   r0, -1
x193a:	IM    0x164d
x193c:	LW    r1, [0x6a]
x193e:	CW    r1, 0x193c
x1940:	JES   1
x1941:	HLT   040
x1942:	KI    int_store
x1944:	LW    r1, [int_store]
x1946:	CW    r1, 0x7ff1
x1948:	JES   1
x1949:	HLT   040
x194a:	LW    r1, 0xfb
x194c:	RW    r1, 0x5e
x194e:	LW    r1, 0x1955
x1950:	RW    r1, 0x5f
x1952:	LWT   r0, -1
x1953:	IM    0x164d
x1955:	LW    r1, [0x6e]
x1957:	CW    r1, 0x1955
x1959:	JES   1
x195a:	HLT   040
x195b:	KI    int_store
x195d:	LW    r1, [int_store]
x195f:	CW    r1, 0x7ff0
x1961:	JES   1
x1962:	HLT   040
x1963:	LW    r1, 0x196e
x1965:	RW    r1, 0x6e
x1967:	LW    r1, 0xfd
x1969:	RW    r1, 0x5f
x196b:	RZ    0x70
x196d:	LIP

x196e:	LW    r1, [0x61]
x1970:	CW    r1, 0x6e
x1972:	JES   1
x1973:	HLT   040
x1974:	LW    r1, 0x1981
x1976:	RW    r1, 0x45
x1978:	LWT   r0, -1
x1979:	LW    r1, 0x800
x197b:	RW    r1, 0x164d
x197d:	FI    0x164e
x197f:	IM    0x164d
x1981:	LW    r1, [0x6e]
x1983:	CW    r1, 0x1981
x1985:	JES   1
x1986:	HLT   040
x1987:	KI    int_store
x1989:	LW    r1, [int_store]
x198b:	CW    r1, 0x7bff
x198d:	JES   1
x198e:	HLT   040
x198f:	LW    r1, 0xc9
x1991:	RW    r1, 0x45
x1993:	LW    r1, 0x199a
x1995:	RW    r1, 0x46
x1997:	LWT   r0, -1
x1998:	IM    0x164d
x199a:	LW    r1, [0x72]
x199c:	CW    r1, 0x199a
x199e:	JES   1
x199f:	HLT   040
x19a0:	KI    int_store
x19a2:	LW    r1, [int_store]
x19a4:	CW    r1, 0x79ff
x19a6:	JES   1
x19a7:	HLT   040
x19a8:	LW    r1, 0xcb
x19aa:	RW    r1, 0x46
x19ac:	LW    r1, 0x19b3
x19ae:	RW    r1, 0x47
x19b0:	LWT   r0, -1
x19b1:	IM    0x164d
x19b3:	LW    r1, [0x76]
x19b5:	CW    r1, 0x19b3
x19b7:	JES   1
x19b8:	HLT   040
x19b9:	KI    int_store
x19bb:	LW    r1, [int_store]
x19bd:	CW    r1, 0x78ff
x19bf:	JES   1
x19c0:	HLT   040
x19c1:	LW    r1, 0xcd
x19c3:	RW    r1, 0x47
x19c5:	LW    r1, 0x19cc
x19c7:	RW    r1, 0x48
x19c9:	LWT   r0, -1
x19ca:	IM    0x164d
x19cc:	LW    r1, [0x7a]
x19ce:	CW    r1, 0x19cc
x19d0:	JES   1
x19d1:	HLT   040
x19d2:	KI    int_store
x19d4:	LW    r1, [int_store]
x19d6:	CW    r1, 0x787f
x19d8:	JES   1
x19d9:	HLT   040
x19da:	LW    r1, 0xcf
x19dc:	RW    r1, 0x48
x19de:	LW    r1, 0x19e5
x19e0:	RW    r1, 0x49
x19e2:	LWT   r0, -1
x19e3:	IM    0x164d
x19e5:	LW    r1, [0x7e]
x19e7:	CW    r1, 0x19e5
x19e9:	JES   1
x19ea:	HLT   040
x19eb:	KI    int_store
x19ed:	LW    r1, [int_store]
x19ef:	CW    r1, 0x783f
x19f1:	JES   1
x19f2:	HLT   040
x19f3:	LW    r1, 0xd1
x19f5:	RW    r1, 0x49
x19f7:	LW    r1, 0x19fe
x19f9:	RW    r1, 0x4a
x19fb:	LWT   r0, -1
x19fc:	IM    0x164d
x19fe:	LW    r1, [0x82]
x1a00:	CW    r1, 0x19fe
x1a02:	JES   1
x1a03:	HLT   040
x1a04:	KI    int_store
x1a06:	LW    r1, [int_store]
x1a08:	CW    r1, 0x781f
x1a0a:	JES   1
x1a0b:	HLT   040
x1a0c:	LW    r1, 0xd3
x1a0e:	RW    r1, 0x4a
x1a10:	LW    r1, 0x1a17
x1a12:	RW    r1, 0x4b
x1a14:	LWT   r0, -1
x1a15:	IM    0x164d
x1a17:	LW    r1, [0x86]
x1a19:	CW    r1, 0x1a17
x1a1b:	JES   1
x1a1c:	HLT   040
x1a1d:	KI    int_store
x1a1f:	LW    r1, [int_store]
x1a21:	CW    r1, 0x780f
x1a23:	JES   1
x1a24:	HLT   040
x1a25:	LW    r1, 0x1a30
x1a27:	RW    r1, 0x86
x1a29:	LW    r1, 0xd5
x1a2b:	RW    r1, 0x4b
x1a2d:	RZ    0x88
x1a2f:	LIP

x1a30:	LW    r1, [0x61]
x1a32:	CW    r1, 0x86
x1a34:	JES   1
x1a35:	HLT   040
x1a36:	LW    r1, 0x1a43
x1a38:	RW    r1, 0x44
x1a3a:	LWT   r0, -1
x1a3b:	LW    r1, 0x1000
x1a3d:	RW    r1, 0x164d
x1a3f:	FI    0x164e
x1a41:	IM    0x164d
x1a43:	LW    r1, [0x86]
x1a45:	CW    r1, 0x1a43
x1a47:	JES   1
x1a48:	HLT   040
x1a49:	KI    int_store
x1a4b:	LW    r1, [int_store]
x1a4d:	CW    r1, 0x77ff
x1a4f:	JES   1
x1a50:	HLT   040
x1a51:	LW    r1, 0xc7
x1a53:	RW    r1, 0x44
x1a55:	LW    r1, 0x1a5a
x1a57:	RW    r1, 0x86
x1a59:	LIP

x1a5a:	LW    r1, 0x1a65
x1a5c:	RW    r1, 0x43
x1a5e:	LWT   r0, -1
x1a5f:	LW    r1, 0x2000
x1a61:	RW    r1, 0x164d
x1a63:	IM    0x164d
x1a65:	LW    r1, [0x86]
x1a67:	CW    r1, 0x1a65
x1a69:	JES   1
x1a6a:	HLT   040
x1a6b:	KI    int_store
x1a6d:	LW    r1, [int_store]
x1a6f:	CW    r1, 0x67ff
x1a71:	JES   1
x1a72:	HLT   040
x1a73:	LW    r1, 0xc5
x1a75:	RW    r1, 0x43
x1a77:	LW    r1, 0x1a7c
x1a79:	RW    r1, 0x86
x1a7b:	LIP
x1a7c:	LW    r1, 0x1a87
x1a7e:	RW    r1, 0x42
x1a80:	LWT   r0, -1
x1a81:	LW    r1, 0x4000
x1a83:	RW    r1, 0x164d
x1a85:	IM    0x164d
x1a87:	LW    r1, [0x86]
x1a89:	CW    r1, 0x1a87
x1a8b:	JES   1
x1a8c:	HLT   040
x1a8d:	KI    int_store
x1a8f:	LW    r1, [int_store]
x1a91:	CW    r1, 0x47ff
x1a93:	JES   1
x1a94:	HLT   040
x1a95:	LW    r1, 0xc3
x1a97:	RW    r1, 0x42
x1a99:	LW    r1, 0x1aa0
x1a9b:	RW    r1, 0x86
x1a9d:	RZ    0x88
x1a9f:	LIP
x1aa0:	LW    r1, 0x1ab5
x1aa2:	RW    r1, 0x40
x1aa4:	RZ    0x164e
x1aa6:	FI    0x164e
x1aa8:	LW    r1, 0x8000
x1aaa:	RW    r1, 0x164e
x1aac:	LWT   r0, -1
x1aad:	LW    r1, 0xffc0
x1aaf:	RW    r1, 0x164d
x1ab1:	IM    0x164d
x1ab3:	FI    0x164e
x1ab5:	LA    0x86
x1ab7:	CW    r1, 0x1ab5
x1ab9:	JES   1
x1aba:	HLT   040
x1abb:	CWT   r2, -1
x1abc:	JES   1
x1abd:	HLT   040
x1abe:	CW    r3, 0xffc0
x1ac0:	JES   1
x1ac1:	HLT   040
x1ac2:	CWT   r4, 0
x1ac3:	JES   1
x1ac4:	HLT   040
x1ac5:	LW    r1, [0x61]
x1ac7:	CW    r1, 0x8a
x1ac9:	JES   1
x1aca:	HLT   040
x1acb:	CW    r0, 0x400
x1acd:	JES   1
x1ace:	HLT   040
x1acf:	KI    int_store
x1ad1:	LW    r1, [int_store]
x1ad3:	CWT   r1, 0
x1ad4:	JES   1
x1ad5:	HLT   040
x1ad6:	LW    r1, int00
x1ad8:	RW    r1, 0x40
x1ada:	LJ    reset
x1adc:	LW    r1, 0xffc0
x1ade:	RW    r1, mask
x1ae0:	IM    mask
x1ae2:	HLT   077

reset:	.res 1
	LW    r2, stack
	LW    r1, stack-intproc+1
resetloop:
	RI    r2, 0
	IRB   r1, resetloop
	LW    r1, stack
	RW    r1, stackp
	MCL
	UJ    [reset]

; XPCT ir&0x3f : 0o77
