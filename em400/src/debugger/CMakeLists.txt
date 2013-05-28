BISON_TARGET(parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
FLEX_TARGET(scanner scanner.l ${CMAKE_CURRENT_BINARY_DIR}/scanner.c)
ADD_FLEX_BISON_DEPENDENCY(scanner parser)

ADD_library(debugger awin.c dasm.c debugger.c eval.c dasm_iset.c cmd.c ui.c
log.c keywords.c decode.c
${BISON_parser_OUTPUTS}
${FLEX_scanner_OUTPUTS}
)
