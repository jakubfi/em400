; Exercise the evaluator's full-RZ access (bare 'rz', via int_get_nchan)
; alongside the single-bit form rz[n]. Only non-channel interrupts are set
; (0..11 and 28..31); none is interrupt 0 (non-maskable), and rm defaults to
; 0 so every set interrupt stays masked and pending across the hlt.
;
; int_get_nchan packs the non-channel bits into a 16-bit word:
;   int 0..11  -> word bits 15..4
;   int 28..31 -> word bits 3..0
; So: int1->0x4000, int11->0x0010, int28->0x0008, int31->0x0001 = 0x4019

; PRECMD int 1
; PRECMD int 11
; PRECMD int 28
; PRECMD int 31

	hlt	077

; full 16-bit RZ value
; XPCT rz : 0x4019

; individual bits (rz[n] reads the same bit 'int n' sets)
; XPCT rz[1] : 1
; XPCT rz[11] : 1
; XPCT rz[28] : 1
; XPCT rz[31] : 1
; XPCT rz[0] : 0
; XPCT rz[15] : 0

; combining the full value with operators
; XPCT rz & 0x4000 : 0x4000
; XPCT rz >> 14 : 1
