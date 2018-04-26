cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(em400
	LANGUAGES C
)

include(GNUInstallDirs)
include(appversion.cmake)

# ---- Settings ----------------------------------------------------------

# It's still here due to the old UI, conveniently marking all the places where
# the old UI keeps its tentacles squeezed into the emulation core.
set(WITH_DEBUGGER 1)
# This needs to be changed into build types
set(DEBUG 0)

if(DEBUG)
	add_definitions(-DDEBUG -g -O0)
else(DEBUG)
	add_definitions(-DNDEBUG -O3)
endif(DEBUG)

# ---- Libraries ---------------------------------------------------------

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Threads REQUIRED)
find_package(emdas REQUIRED)
find_package(emcrk REQUIRED)
find_package(emawp REQUIRED)
if(WITH_DEBUGGER)
	set(CURSES_NEED_NCURSES TRUE)
	find_package(Curses REQUIRED)
	find_path(READLINE_INCLUDE_DIR readline/readline.h)
	find_library(READLINE_LIBRARY NAMES readline)
endif(WITH_DEBUGGER)

# ---- Sources -----------------------------------------------------------

set(SOURCES_EM400
	src/em400.c
	src/cfg.c
 	src/log.c
	src/log_crk.c

	src/utils/utils.c
	src/utils/elst.c
	src/utils/fdbridge.c

	src/cpu/instructions.c
 	src/cpu/timer.c
 	src/cpu/cpu.c
	src/cpu/interrupts.c
	src/cpu/iset.c
	src/cpu/alu.c
	src/cpu/cp.c

	src/mem/mem.c
	src/mem/mem_mega.c
	src/mem/mem_elwro.c

	src/fpga/iobus.c

	src/io/io.c
	src/io/chan.c
	src/io/cchar.c
	src/io/cchar_term.c
	src/io/iotester.c

	src/io/dev/dev.c
	src/io/dev/e4image.c
	src/io/dev/term.c
	src/io/dev/winchester.c
	src/io/dev/flop5.c
	src/io/dev/punchrd.c
	src/io/dev/puncher.c
	src/io/dev/terminal.c
	src/io/dev/printer.c

	src/io/mx/mx.c
	src/io/mx/cmds.c
	src/io/mx/irq.c
	src/io/mx/event.c
	src/io/mx/line.c
	src/io/mx/proto_common.c
	src/io/mx/proto_winchester.c
	src/io/mx/proto_floppy.c
	src/io/mx/proto_tape.c
	src/io/mx/proto_puncher.c
	src/io/mx/proto_punchrd.c
	src/io/mx/proto_terminal.c
	src/io/mx/proto_som_puncher.c
	src/io/mx/proto_som_punchrd.c
	src/io/mx/proto_som_terminal.c

	src/ui/ui.c

	src/ui/cmd/cmd.c
	src/ui/cmd/commands.c
	src/ui/cmd/utils.c

	src/ectl/cpu.c
	src/ectl/est.c
	src/ectl/brk.c
)

bison_target(cfg_parser
	src/cfg_parser.y
	${CMAKE_CURRENT_BINARY_DIR}/cfg_parser.c
	DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/cfg_parser.h
	COMPILE_FLAGS "-p cyy"
)
flex_target(cfg_scanner
	src/cfg_scanner.l
	${CMAKE_CURRENT_BINARY_DIR}/cfg_scanner.c
	COMPILE_FLAGS "-P cyy --never-interactive"
)
add_flex_bison_dependency(cfg_scanner cfg_parser)

bison_target(ectl_parser
	src/ectl/parser.y
	${CMAKE_CURRENT_BINARY_DIR}/ectl_parser.c
	DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/ectl_parser.h
	COMPILE_FLAGS "-p ectl_yy"
)
flex_target(ectl_scanner
	src/ectl/scanner.l
	${CMAKE_CURRENT_BINARY_DIR}/ectl_scanner.c
	COMPILE_FLAGS "-P ectl_yy --never-interactive"
)
add_flex_bison_dependency(ectl_scanner ectl_parser)

if(WITH_DEBUGGER)
	bison_target(debugger_parser
		src/debugger/parser.y
		${CMAKE_CURRENT_BINARY_DIR}/debugger_parser.c
	)
	flex_target(debugger_scanner
		src/debugger/scanner.l
		${CMAKE_CURRENT_BINARY_DIR}/debugger_scanner.c
		COMPILE_FLAGS "--never-interactive"
	)
	add_flex_bison_dependency(debugger_scanner debugger_parser)
	set(SOURCES_DEBUGGER
		src/debugger/awin.c
		src/debugger/awinch.c
		src/debugger/debugger.c
		src/debugger/eval.c
		src/debugger/cmd.c
		src/debugger/ui.c
		src/debugger/keywords.c
		src/debugger/decode.c
		${BISON_debugger_parser_OUTPUTS}
		${FLEX_debugger_scanner_OUTPUTS}
	)
	add_definitions(-DWITH_DEBUGGER)
	set(DEBUGGER_LIBS ${CURSES_LIBRARIES} ${READLINE_LIBRARY})
endif(WITH_DEBUGGER)

# ---- Target: em400 -----------------------------------------------------

add_executable(em400
	${SOURCES_EM400}
	${SOURCES_DEBUGGER}
	${BISON_cfg_parser_OUTPUTS}
	${FLEX_cfg_scanner_OUTPUTS}
	${BISON_ectl_parser_OUTPUTS}
	${FLEX_ectl_scanner_OUTPUTS}
)

set_property(TARGET em400 PROPERTY C_STANDARD 99)
target_include_directories(em400 PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(em400 PRIVATE ${CMAKE_BINARY_DIR})
target_compile_definitions(em400 PRIVATE EM400_VERSION="${APP_VERSION}")
target_compile_options(em400 PUBLIC -Wall)

target_link_libraries(em400
	${DEBUGGER_LIBS}
	${EMDAS_LIBRARIES}
	${EMCRK_LIBRARIES}
	${EMAWP_LIBRARIES}
	${FLEX_LIBRARIES} ${BISON_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	rt m
)

install(TARGETS em400
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ---- Target: emitool ---------------------------------------------------

add_executable(emitool
	src/emitool.c
	src/io/dev/e4image.c
)
set_property(TARGET emitool PROPERTY C_STANDARD 99)
target_include_directories(emitool PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_options(emitool PUBLIC -Wall)

install(TARGETS emitool
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# vim: tabstop=4
