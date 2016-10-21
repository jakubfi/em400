bison_target(ectl_parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/ectl_parser.c COMPILE_FLAGS "-p ectl_yy")
flex_target(ectl_scanner scanner.l ${CMAKE_CURRENT_BINARY_DIR}/ectl_scanner.c COMPILE_FLAGS "-P ectl_yy --never-interactive")
add_flex_bison_dependency(ectl_scanner ectl_parser)

include_directories(${CMAKE_BINARY_DIR}/src/ectl)

add_library(ectl
    cpu.c
	est.c
    ${BISON_ectl_parser_OUTPUTS}
    ${FLEX_ectl_scanner_OUTPUTS}
)

# vim: tabstop=4
