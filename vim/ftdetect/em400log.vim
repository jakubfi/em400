au! BufNewFile,BufRead *.[Ll][Oo][Gg] call s:FTem400log()
	function! s:FTem400log()

		let n = 1
		let level = 0

		while n <= 100 && n <= line("$")
			let ln = getline(n)

			" log levels
			if 1 == 2
			elseif (ln =~? '^\(EM4H\| MEM\| CPU\| INT  IO\|CCHR\|TERM\|  MX\) [0-9] |')
				let level = level + 1
			" emulator initialization
			elseif (ln =~ 'Emulation speed')
				let level = level + 1
			" emulator initialization
			elseif (ln =~? 'CPU modifications')
				let level = level + 1
			" emulator initialization
			elseif (ln =~? 'Hardware AWP')
				let level = level + 1
			endif

			if (level > 10)
				break
			endif

			let n = n + 1
		endwhile

		if (level > 10)
			set filetype=em400log
		endif

	endfunction

" vim: tabstop=4
