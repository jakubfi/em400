BISON_TARGET(cfg_parser cfg_parser.y ${CMAKE_CURRENT_BINARY_DIR}/cfg_parser.c COMPILE_FLAGS "-p cyy")
FLEX_TARGET(cfg_scanner cfg_scanner.l ${CMAKE_CURRENT_BINARY_DIR}/cfg_scanner.c COMPILE_FLAGS "-P cyy")
ADD_FLEX_BISON_DEPENDENCY(cfg_scanner cfg_parser)

ADD_EXECUTABLE(em400 em400.c instructions.c io.c memory.c timer.c cpu.c errors.c interrupts.c iset.c registers.c utils.c alu.c cfg.c
${BISON_cfg_parser_OUTPUTS}
${FLEX_cfg_scanner_OUTPUTS}
)

target_link_libraries(em400
drv
${FLEX_LIBRARIES} ${BISON_LIBRARIES} ${DEBUGGER_LIB}
readline ncurses rt
)
