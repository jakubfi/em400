au! BufNewFile,BufRead *.[Ll][Oo][Gg] call s:FTem400log()
	function! s:FTem400log()

		let n = 1
		let level = 0

		while n <= 100 && n <= line("$")
			let ln = getline(n)

			" log levels
			if 1 == 2
			" component | thread | function() | ... (the LOG_F_COMP/LOG_F_FUN header)
			elseif (ln =~? '^\s*[A-Z0-9]\+ |\s\+\w\+ |\s\+\w\+() |')
				let level = level + 1
			" emulator initialization
			elseif (ln =~ 'Effective configuration')
				let level = level + 1
			" emulator initialization
			elseif (ln =~? 'Program to load')
				let level = level + 1
			" emulator initialization
			elseif (ln =~? 'CPU emulation')
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
