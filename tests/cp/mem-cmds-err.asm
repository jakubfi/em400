; Error paths of mem, memw and memmap.

	hlt	077

; XPCT_CMD mem : "ERR: Missing argument (memory segment)"
; XPCT_CMD mem 16 0 1 : "ERR: Wrong segment number: 16"
; XPCT_CMD mem 0 : "ERR: Missing argument (start address)"
; XPCT_CMD mem 0 foo 1 : "ERR: Invalid address: -1"
; XPCT_CMD mem 0 0 : "ERR: Missing argument (word count)"
; XPCT_CMD mem 0 0 0 : "ERR: Invalid word count: 0"

; XPCT_CMD memw : "ERR: Missing argument (memory segment)"
; XPCT_CMD memw 16 0 1 : "ERR: Wrong segment number: 16"
; XPCT_CMD memw 0 : "ERR: Missing argument (start address)"
; XPCT_CMD memw 0 foo 1 : "ERR: Invalid address: -1"
; XPCT_CMD memw 0 0 : "ERR: Missing argument (value)"
; XPCT_CMD memw 0 0 1 foo : "ERR: Value on position 1 is not a valid 16-bit integer: -1"
; XPCT_CMD memw 1 0 1 : "ERR: Memory write failed when writing 1 words at address 0x0000"

; XPCT_CMD memmap : "ERR: Missing argument (memory segment)"
; XPCT_CMD memmap 16 : "ERR: Wrong segment number: 16"
