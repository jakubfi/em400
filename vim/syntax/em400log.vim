" Vim syntax file
" Language:	EM400 log file (MERA-400 emulator)
" Orig Author:	Jakub Filipowicz <jakubf@gmail.com>
" Maintainer:	Jakub Filipowicz <jakubf@gmail.com>
" Last Change:	Date: 2015/04/09
" Revision:	1.0

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

syn match em400logHALT			"HALT \d\+[ \t]*(alarm:.*)" contained

syn match em400logREG			"^[ \t]*REG |.*"
syn match em400logMEM			"^[ \t]*MEM |.*"
syn match em400logCPU			"^[ \t]*CPU |.*"
syn match em400logOP			"^[ \t]*OP |.*" contains=em400logHALT
syn match em400logINT			"^[ \t]*INT |.*"

syn match em400logIO			"^[ \t]*IO |.*"

syn match em400logMX			"^[ \t]*MX |.*"
syn match em400logPX			"^[ \t]*PX |.*"
syn match em400logCCHR			"^[ \t]*CCHR |.*"
syn match em400logCMEM			"^[ \t]*CMEM |.*"

syn match em400logTERM			"^[ \t]*TERM |.*"
syn match em400log9425			"^[ \t]*9425 |.*"
syn match em400logWNCH			"^[ \t]*WNCH |.*"
syn match em400logFLOP			"^[ \t]*FLOP |.*"
syn match em400logPNCH			"^[ \t]*PNCH |.*"
syn match em400logPNRD			"^[ \t]*PNRD |.*"
syn match em400logCRK5			"^[ \t]*CRK5 |.*"
syn match em400logEM4H			"^[ \t]*EM4H |.*"

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_em400log_syntax_inits")
  if version < 508
    let did_em400log_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

	HiLink em400logHALT		Error

	HiLink em400logREG		Label
	HiLink em400logMEM		Special
	HiLink em400logCPU		Label
	HiLink em400logOP		Label
	HiLink em400logINT		Constant

	HiLink em400logIO		Identifier

	HiLink em400logMX		Type
	HiLink em400logPX		Type
	HiLink em400logCCHR		Type
	HiLink em400logCMEM		Type

	HiLink em400logTERM		PreProc
	HiLink em400log9425		PreProc
	HiLink em400logWNCH		PreProc
	HiLink em400logFLOP		PreProc
	HiLink em400logPNCH		PreProc
	HiLink em400logPNRD		PreProc

	HiLink em400logCRK5		Statement
	HiLink em400logEM4H		Label

  syntax sync minlines=50

  delcommand HiLink
endif

let b:current_syntax = "em400log"

let &cpo = s:cpo_save
unlet s:cpo_save

" vim: ts=4
