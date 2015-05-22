; OPTS -c configs/multix.cfg

; Test "set configuration" command:
;
;  - physical/logical line count
;  - physical line types
;  - physical line directions
;  - line type vs. direction match
;  - incomplete physical line configurations
;  - logical lines for unused physical lines
;  - logical lines for physical lines already configured as logical lines
;  - protocols
;  - physical line numbers are 5-bit in logical line configuration
;  - physical line direction vs. protocol
;  - protocol parameters
;  - configuration of already configured MULTIX
;  - correct full configuration

	.cpu	mx16

	.include hw.inc
	.include io.inc
	.include mx.inc

	UJ	start

msk_mx:	.word	IMASK_CH0_1

	.org	OS_MEM_BEG

; ------------------------------------------------------------------------

; setconf fields for each test

dummy:	.word	0, 0 ; dummy to acknowledge IWYZE

; ---------------------------------------------------------------
; line count tests
; ---------------------------------------------------------------

num0:	.word	33\7 + 1\15, 0 ; wrong number of physical line descriptions (>32)
num1:	.word	0\7 + 1\15, 0 ; wrong number of physical line descriptions (=0)
num2:	.word	1\7 + 33\15, 0 ; wrong number of logical lines (>32)
num3:	.word	1\7 + 0\15, 0 ; wrong number of logical lines (=0)
num4:	.word	2\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTA + 30 ; 31 phy lines
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTA + 1 ; +2 phy lines = exceeds number of total 32 physical lines

; ---------------------------------------------------------------
; physical line configuration tests
; ---------------------------------------------------------------

; wrong line type

type0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_UNUSED + MX_LTYPE_ERR + 0 ; wrong line 0 type
type1:	.word	2\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_UNUSED + MX_LTYPE_USARTS + 0
	.word	MX_LDIR_IN + MX_LINE_UNUSED + MX_LTYPE_ERR + 0 ; wrong line 1 type

; wrong (unknown) directions

dir1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_ERR1 + MX_LINE_UNUSED + MX_LTYPE_USARTA + 0 ; wrong line 0 direction (1)
dir3:	.word	2\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTA + 0
	.word	MX_LDIR_ERR3 + MX_LINE_UNUSED + MX_LTYPE_USARTA + 0 ; wrong line 1 direction (3)
dir5:	.word	3\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTA + 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTA + 0
	.word	MX_LDIR_ERR5 + MX_LINE_UNUSED + MX_LTYPE_USARTA + 0 ; wrong line 2 direction (5)

; wrong line type vs. direction combinations

pdir0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_UNUSED + MX_LTYPE_8255 + 0 ; wrong 8255 line 0 direction (none)
pdir1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_8255 + 0 ; wrong 8255 line 0 direction (full-duplex)
pdir2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_UNUSED + MX_LTYPE_8255 + 0 ; wrong 8255 line 0 direction (half-duplex)
wdir0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_UNUSED + MX_LTYPE_WINCH + 0 ; wrong winchester line 0 direction (input)
wdir1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_UNUSED + MX_LTYPE_WINCH + 0 ; wrong winchester line 0 direction (output)
wdir2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_UNUSED + MX_LTYPE_WINCH + 0 ; wrong winchester line 0 direction (half-duplex)
wdir3:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_WINCH + 0 ; wrong winchester line 0 direction (full-duplex)
fdir0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_UNUSED + MX_LTYPE_FLOPPY + 0 ; wrong floppy line 0 direction (input)
fdir1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_UNUSED + MX_LTYPE_FLOPPY + 0 ; wrong floppy line 0 direction (output)
fdir2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_UNUSED + MX_LTYPE_FLOPPY + 0 ; wrong floppy line 0 direction (half-duplex)
fdir3:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_FLOPPY + 0 ; wrong floppy line 0 direction (full-duplex)
tdir0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_UNUSED + MX_LTYPE_TAPE + 0 ; wrong tape line 0 direction (input)
tdir1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_UNUSED + MX_LTYPE_TAPE + 0 ; wrong tape line 0 direction (output)
tdir2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_UNUSED + MX_LTYPE_TAPE + 0 ; wrong tape line 0 direction (half-duplex)
tdir3:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_TAPE + 0 ; wrong tape line 0 direction (full-duplex)

; incomplete physical line configurations

cmpl0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTS + 2 ; incomplete configuration for line 3 (only 3 lines in group)
cmpl1:	.word	2\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTA + 3
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTS + 2 ; incomplete configuration for line 7 (only 3 lines in group)
cmpl2:	.word	6\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTA + 11
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTS + 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTS + 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTS + 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTS + 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTS + 0 ; incomplete configuration for line 17 (only 1 line in group)

; only one tape controller group (4 drives)

tap0:	.word	2\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3 ; incomplete configuration for line 4

; ---------------------------------------------------------------
; logical line configuration tests
; ---------------------------------------------------------------

; logical lines for unused physical lines

unus0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_MONITOR + 0\15, 0, 0, 0 ; logical line 0 for unused physical line
unus1:	.word	3\7 + 4\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTA + 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTA + 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTA + 1
	.word	MX_LPROTO_MONITOR + 2\15, 0, 0, 0
	.word	MX_LPROTO_MONITOR + 0\15, 0, 0, 0
	.word	MX_LPROTO_MONITOR + 3\15, 0, 0, 0
	.word	MX_LPROTO_MONITOR + 1\15, 0, 0, 0 ; logical line 3 for unused physical line

; logical lines for physical lines already configured as logical lines

us0:	.word	1\7 + 2\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_MONITOR + 0\15, 0, 0, 0
	.word	MX_LPROTO_MONITOR + 0\15, 0, 0, 0 ; logical line 1 for already used physical line 0
us1:	.word	1\7 + 4\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_MONITOR + 2\15, 0, 0, 0
	.word	MX_LPROTO_MONITOR + 0\15, 0, 0, 0
	.word	MX_LPROTO_MONITOR + 1\15, 0, 0, 0
	.word	MX_LPROTO_MONITOR + 1\15, 0, 0, 0 ; logical line 3 for already used physical line 2

; unknown protocol

uprot0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_ERR + 0\15, 0, 0, 0 ; unknown protocol for logical line 0
uprot1:	.word	1\7 + 2\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_MONITOR + 1\15, 0, 0, 0
	.word	MX_LPROTO_ERR + 2\15, 0, 0, 0 ; unknown protocol for logical line 1

; physical line numbers are 5-bit
; (3 most significant bits in 8-bit physical line number are ignored)

p5bit:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_ERR + 0b111\10 + 1\15, 0, 0, 0 ; unknown protocol for logical line 0 for physical line 1

; wrong directions for punch reader

dpr0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (out) for punch reader on line 0
dpr1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (out) for punch reader on line 0
dpr2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (out) for punch reader on line 0
dpr3:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3
	.word	MX_LPROTO_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (none) for punch reader on line 0
dpr4:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3
	.word	MX_LPROTO_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (none) for punch reader on line 0
dpr5:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_FLOPPY + 3
	.word	MX_LPROTO_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (none) for punch reader on line 0

dpu0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (in) for puncher on line 0
dpu1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (in) for puncher on line 0
dpu2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (in) for puncher on line 0
dpu3:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3
	.word	MX_LPROTO_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (none) for puncher on line 0
dpu4:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3
	.word	MX_LPROTO_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (none) for puncher on line 0
dpu5:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_FLOPPY + 3
	.word	MX_LPROTO_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (none) for puncher on line 0

; wrong directions for monitor

dm0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_MONITOR + 2\15, 0, 0, 0 ; wrong direction (in) for monitor on line 0
dm1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_MONITOR + 2\15, 0, 0, 0 ; wrong direction (out) for monitor on line 0
dm2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3
	.word	MX_LPROTO_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for monitor on line 0
dm3:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3
	.word	MX_LPROTO_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for monitor on line 0
dm4:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_FLOPPY + 3
	.word	MX_LPROTO_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for monitor on line 0

dm5:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for monitor on line 0
dm6:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for monitor on line 0
dm7:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for monitor on line 0
dm8:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for monitor on line 0

; wrong directions for punch reader (SOM)

dpr0s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (out) for SOM punch reader on line 0
dpr1s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (out) for SOM punch reader on line 0
dpr2s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (out) for SOM punch reader on line 0
dpr3s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (none) for SOM punch reader on line 0
dpr4s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (none) for SOM punch reader on line 0
dpr5s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_FLOPPY + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (none) for SOM punch reader on line 0

; wrong directions for puncher (SOM)

dpu0s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_SOM_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (in) for SOM puncher on line 0
dpu1s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_SOM_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (in) for SOM puncher on line 0
dpu2s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (in) for SOM puncher on line 0
dpu3s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3
	.word	MX_LPROTO_SOM_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (none) for SOM puncher on line 0
dpu4s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3
	.word	MX_LPROTO_SOM_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (none) for SOM puncher on line 0
dpu5s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_FLOPPY + 3
	.word	MX_LPROTO_SOM_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (none) for SOM puncher on line 0

; wrong directions for monitor (SOM)

dm0s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (in) for SOM monitor on line 0
dm1s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (out) for SOM monitor on line 0
dm2s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for SOM monitor on line 0
dm3s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for SOM monitor on line 0
dm4s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_FLOPPY + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for SOM monitor on line 0

dm5s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for SOM monitor on line 0
dm6s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for SOM monitor on line 0
dm7s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for SOM monitor on line 0
dm8s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for SOM monitor on line 0

; wrong line types for winchester

tw0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_WINCH + 2\15, 0, 0, 0 ; wrong line type for winchester
tw1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_WINCH + 2\15, 0, 0, 0 ; wrong line type for winchester
tw2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_WINCH + 2\15, 0, 0, 0 ; wrong line type for winchester
tw3:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_FLOPPY + 3
	.word	MX_LPROTO_WINCH + 2\15, 0, 0, 0 ; wrong line type for winchester
tw4:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3
	.word	MX_LPROTO_WINCH + 2\15, 0, 0, 0 ; wrong line type for winchester

; wrong line types for floppy

tf0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_FLOPPY + 2\15, 0, 0, 0 ; wrong line type for floppy
tf1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_FLOPPY + 2\15, 0, 0, 0 ; wrong line type for floppy
tf2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_FLOPPY + 2\15, 0, 0, 0 ; wrong line type for floppy
tf3:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3
	.word	MX_LPROTO_FLOPPY + 2\15, 0, 0, 0 ; wrong line type for floppy
tf4:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3
	.word	MX_LPROTO_FLOPPY + 2\15, 0, 0, 0 ; wrong line type for floppy

; wrong line types for tape

tt0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_TAPE + 2\15, 0, 0, 0 ; wrong line type for tape
tt1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_TAPE + 2\15, 0, 0, 0 ; wrong line type for tape
tt2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_TAPE + 2\15, 0, 0, 0 ; wrong line type for tape
tt3:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3
	.word	MX_LPROTO_TAPE + 2\15, 0, 0, 0 ; wrong line type for tape
tt4:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_FLOPPY + 3
	.word	MX_LPROTO_TAPE + 2\15, 0, 0, 0 ; wrong line type for tape

; wrong line types for punch reader

tpu0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_PUNCHRD + 2\15, 0, 0, 0 ; wrong line type for punch reader
tpu1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_PUNCHRD + 2\15, 0, 0, 0 ; wrong line type for punch reader
tpu2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_PUNCHRD + 2\15, 0, 0, 0 ; wrong line type for punch reader

; wrong line types for puncher

tpr0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_PUNCHER + 2\15, 0, 0, 0 ; wrong line type for puncher
tpr1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_PUNCHER + 2\15, 0, 0, 0 ; wrong line type for puncher
tpr2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_PUNCHER + 2\15, 0, 0, 0 ; wrong line type for puncher

; wrong line types for terminal

tm0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_MONITOR + 2\15, 0, 0, 0 ; wrong line type for monitor
tm1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_MONITOR + 2\15, 0, 0, 0 ; wrong line type for monitor


; wrong line types for SOM punch reader

tpu0s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong line type for SOM punch reader
tpu1s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong line type for SOM punch reader
tpu2s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong line type for SOM punch reader

; wrong line types for SOM puncher

tpr0s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_PUNCHER + 2\15, 0, 0, 0 ; wrong line type for SOM puncher
tpr1s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_PUNCHER + 2\15, 0, 0, 0 ; wrong line type for SOM puncher
tpr2s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_PUNCHER + 2\15, 0, 0, 0 ; wrong line type for SOM puncher

; wrong line types for SOM terminal

tm0s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong line type for SOM monitor
tm1s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong line type for SOM monitor

; ---------------------------------------------------------------
; protocol configuration errors
; ---------------------------------------------------------------

; floppy: unknown floppy drive type

fl0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_FLOPPY + 3
	.word	MX_LPROTO_FLOPPY + 0\15, MX_FLOPPY_ERR, 0, 0

; ---------------------------------------------------------------
; correct configuration
; ---------------------------------------------------------------

cfgok:	.word	13\7 + 22\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTA + 0		; 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTA + 0	; 1
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTA + 2		; 2, 3, 4
	.word	MX_LDIR_HD + MX_LINE_USED + MX_LTYPE_USARTA + 2		; 5, 6, 7
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_8255 + 1		; 8, 9
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_8255 + 1		; 10, 11
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3		; 12, 13, 14, 15
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_FLOPPY + 3	; 16, 17, 18, 19
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3	; 20, 21, 22, 23
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTA + 0		; 24
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTA + 0	; 25
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTA + 2		; 26, 27, 28
	.word	MX_LDIR_HD + MX_LINE_USED + MX_LTYPE_USARTA + 2		; 29, 30, 31

	.word	MX_LPROTO_PUNCHRD + 0, 0, 0, 0
	.word	MX_LPROTO_PUNCHER + 1, 0, 0, 0
	.word	MX_LPROTO_PUNCHRD + 2, 0, 0, 0
	.word	MX_LPROTO_PUNCHER + 3, 0, 0, 0
	.word	MX_LPROTO_MONITOR + 4, 0, 0, 0
	.word	MX_LPROTO_PUNCHRD + 5, 0, 0, 0
	.word	MX_LPROTO_PUNCHER + 6, 0, 0, 0
	.word	MX_LPROTO_MONITOR + 7, 0, 0, 0
	.word	MX_LPROTO_PUNCHRD + 8, 0, 0, 0
;	.word	MX_LPROTO_SOM_PUNCHRD + 9, 0, 0, 0
	.word	MX_LPROTO_PUNCHER + 10, 0, 0, 0
;	.word	MX_LPROTO_SOM_PUNCHER + 11, 0, 0, 0
	.word	MX_LPROTO_TAPE + 0\8 + 12, 0, 0, 0
	.word	MX_LPROTO_TAPE + 1\8 + 13, 0, 0, 0
	.word	MX_LPROTO_TAPE + 0\8 + 14, 0, 0, 0
	.word	MX_LPROTO_TAPE + 1\8 + 15, 0, 0, 0
	.word	MX_LPROTO_FLOPPY + 16, MX_FLOPPY_SD, 0, 0
	.word	MX_LPROTO_FLOPPY + 17, MX_FLOPPY_DD, 0, 0
	.word	MX_LPROTO_FLOPPY + 18, MX_FLOPPY_HD, 0, 0
	.word	MX_LPROTO_FLOPPY + 19, MX_FLOPPY_HD + MX_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH + 20, 0\7 + MX_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH + 21, 1\7 + MX_LONG_DISK_ADDR, 0, 0
	.word	MX_LPROTO_WINCH + 22, 3\7 + MX_FORMAT_PROTECT + MX_LONG_DISK_ADDR, 0, 0
	.word	MX_LPROTO_WINCH + 23, 7\7 + MX_FORMAT_PROTECT, 0, 0	; any winchester configuration is OK for MULTIX
;	.word	MX_LPROTO_SOM_PUNCHRD + 24, 0, 0, 0
;	.word	MX_LPROTO_SOM_PUNCHER + 25, 0, 0, 0
;	.word	MX_LPROTO_SOM_PUNCHRD + 26, 0, 0, 0
;	.word	MX_LPROTO_SOM_PUNCHER + 27, 0, 0, 0
;	.word	MX_LPROTO_SOM_MONITOR + 28, 0, 0, 0
;	.word	MX_LPROTO_SOM_PUNCHRD + 29, 0, 0, 0
;	.word	MX_LPROTO_SOM_PUNCHER + 30, 0, 0, 0
;	.word	MX_LPROTO_SOM_MONITOR + 31, 0, 0, 0

; ---------------------------------------------------------------
; reconfiguration of properly configured MULTIX
; ---------------------------------------------------------------

recfg:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3
	.word	MX_LPROTO_WINCH + 0\15, 0, 0, 0

; ------------------------------------------------------------------------

; test blocks
; NOTE: all SOM protocols are disabled (MULTIX emulation don't support them atm)

	.const	test_size 3

seq:	.word	dummy,	MX_IWYZE\7,	0
	.word	0x6666,	MX_INKOT\7,	0
	.word	num0,	MX_INKON\7,	MX_SCERR_NUM
	.word	num1,	MX_INKON\7,	MX_SCERR_NUM
	.word	num2,	MX_INKON\7,	MX_SCERR_NUM
	.word	num3,	MX_INKON\7,	MX_SCERR_NUM
	.word	num4,	MX_INKON\7,	MX_SCERR_NUM
	.word	type0,	MX_INKON\7,	MX_SCERR_TYPE + 0
	.word	type1,	MX_INKON\7,	MX_SCERR_TYPE + 1
	.word	dir1,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	dir3,	MX_INKON\7,	MX_SCERR_DIR + 1
	.word	dir5,	MX_INKON\7,	MX_SCERR_DIR + 2
	.word	pdir0,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	pdir1,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	pdir2,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	wdir0,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	wdir1,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	wdir2,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	wdir3,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	fdir0,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	fdir1,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	fdir2,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	fdir3,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	tdir0,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	tdir1,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	tdir2,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	tdir3,	MX_INKON\7,	MX_SCERR_DIR + 0
	.word	cmpl0,	MX_INKON\7,	MX_SCERR_INCOMPLETE + 3
	.word	cmpl1,	MX_INKON\7,	MX_SCERR_INCOMPLETE + 7
	.word	cmpl2,	MX_INKON\7,	MX_SCERR_INCOMPLETE + 17
	.word	tap0,	MX_INKON\7,	MX_SCERR_INCOMPLETE + 4

	.word	unus0,	MX_INKON\7,	MX_SCERR_UNUSED + 0
	.word	unus1,	MX_INKON\7,	MX_SCERR_UNUSED + 3
	.word	us0,	MX_INKON\7,	MX_SCERR_USED + 1
	.word	us1,	MX_INKON\7,	MX_SCERR_USED + 3
	.word	uprot0,	MX_INKON\7,	MX_SCERR_PROTO + 0
	.word	uprot1,	MX_INKON\7,	MX_SCERR_PROTO + 1
	.word	p5bit,	MX_INKON\7,	MX_SCERR_PROTO + 0
	.word	dpr0,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dpr1,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dpr2,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dpr3,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dpr4,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dpr5,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dpu0,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dpu1,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dpu2,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dpu3,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dpu4,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dpu5,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dm0,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dm1,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dm2,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dm3,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dm4,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dm5,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dm6,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dm7,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	dm8,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpr0s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpr1s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpr2s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpr3s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpr4s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpr5s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpu0s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpu1s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpu2s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpu3s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpu4s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpu5s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm0s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm1s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm2s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm3s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm4s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm5s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm6s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm7s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm8s,	MX_INKON\7,	MX_SCERR_DIR_MISMATCH + 0
	.word	tw0,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tw1,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tw2,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tw3,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tw4,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tf0,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tf1,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tf2,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tf3,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tf4,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tt0,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tt1,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tt2,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tt3,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tt4,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tpu0,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tpu1,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tpu2,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tpr0,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tpr1,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tpr2,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tm0,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
	.word	tm1,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tpu0s,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tpu1s,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tpu2s,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tpr0s,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tpr1s,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tpr2s,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tm0s,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tm1s,	MX_INKON\7,	MX_SCERR_TYPE_MISMATCH + 0

	.word	fl0,	MX_INKON\7,	MX_SCERR_PARAMS + 0

	.word	cfgok,	MX_IUKON\7,	0
	.word	recfg,	MX_INKON\7,	MX_SCERR_CONFSET
seqe:

; ------------------------------------------------------------------------
; expects:
;  r1 - configuration field address
;  r4 - RJ return adress
setcfg:
	OU	r1, 0b101\2 + MX_CHAN_DEFAULT
	.word	no, en, ok, pe
no:	HLT	041
ok:	UJ	r4
en:	UJS	setcfg
pe:	HLT	042

; ------------------------------------------------------------------------
mx_proc:
	LW	r1, [STACKP]
	LW	r1, [r1-1]	; intspec from mx

	CW	r1, [r2+1]	; is intspec as expected?
	BB	r0, ?E
	HLT	040		; bad intspec

	CW	r1, MX_INKOT\7	; skip setconf result if INKOT
	JES	next

	LW	r1, [r2+2]	; load expected setconf return value
	LW	r3, [r2]
	LW	r5, [r3+1]
	CW	r1, r5		; load setconf return value
	BB	r0, ?E
	HLT	043		; bad setconf result

next:
	AWT	r2, test_size	; next test
	CW	r2, seqe	; all tests finished?
	BLC	?E
	HLT	077		; all tests OK

	LW	r1, [r2]
	RJ	r4, setcfg

	LIP

; ------------------------------------------------------------------------
start:
	LW	r1, stack
	RW	r1, STACKP
	LW	r1, mx_proc
	RW	r1, MX_IV

	LW	r2, seq		; start of all tests
	IM	msk_mx
loop:
	HLT
	UJS	loop

stack:

; XPCT int(rz[15]) : 0
; XPCT int(rz[6]) : 0
; XPCT int(alarm) : 0
; XPCT oct(ir[10-15]) : 077
