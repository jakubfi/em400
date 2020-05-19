cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(em400
	DESCRIPTION "MERA-400 emulator"
	LANGUAGES C
)

include(GNUInstallDirs)
include(cmake/appversion.cmake)

# ---- Libraries ---------------------------------------------------------

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Threads REQUIRED)
find_package(emdas REQUIRED)
find_package(emcrk REQUIRED)
find_package(emawp REQUIRED)
find_library(MATH_LIBRARY m)
find_library(RT_LIBRARY rt)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)
find_path(READLINE_INCLUDE_DIR readline/readline.h)
find_library(READLINE_LIBRARY readline)

include(FindALSA)
include(cmake/FindPulseAudio.cmake)
find_library(PULSEAUDIO_SIMPLE_LIBRARY pulse-simple)

if(PULSEAUDIO_FOUND OR ALSA_FOUND)
	message(STATUS "Sound enabled: ALSA: ${ALSA_FOUND}, PulseAudio: ${PULSEAUDIO_FOUND}")
else()
	message(STATUS "No supported sound libraries found, disabling sound")
endif()

# ---- Sources -----------------------------------------------------------

set(SOURCES_EM400
	src/em400.c
	src/log.c
	src/log_crk.c
	src/cfg.c

	src/utils/utils.c
	src/utils/elst.c
	src/utils/fdbridge.c

	src/cpu/instructions.c
	src/cpu/clock.c
	src/cpu/cpu.c
	src/cpu/interrupts.c
	src/cpu/iset.c
	src/cpu/alu.c
	src/cpu/cp.c

	src/mem/mem.c
	src/mem/mega.c
	src/mem/elwro.c

	src/fpga/iobus.c

	src/io/io.c
	src/io/chan.c
	src/io/cchar.c
	src/io/cchar_term.c
	src/io/cchar_flop8.c
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

	src/sound/file.c
	src/sound/sound.c
	src/external/biquad/biquad.c
	src/cpu/buzzer.c

	src/external/iniparser/dictionary.c
	src/external/iniparser/iniparser.c
)

if(ALSA_FOUND)
	list(APPEND SOURCES_EM400 src/sound/alsa.c)
endif()
if(PULSEAUDIO_FOUND)
	list(APPEND SOURCES_EM400 src/sound/pulseaudio.c)
endif()

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

# TODO: cleanup
bison_target(debugger_parser
	src/ui/curses/parser.y
	${CMAKE_CURRENT_BINARY_DIR}/debugger_parser.c
)
flex_target(debugger_scanner
	src/ui/curses/scanner.l
	${CMAKE_CURRENT_BINARY_DIR}/debugger_scanner.c
	COMPILE_FLAGS "--never-interactive"
)
add_flex_bison_dependency(debugger_scanner debugger_parser)
set(SOURCES_DEBUGGER
	src/ui/curses/awin.c
	src/ui/curses/awinch.c
	src/ui/curses/debugger.c
	src/ui/curses/eval.c
	src/ui/curses/cmd.c
	src/ui/curses/ui.c
	src/ui/curses/keywords.c
	src/ui/curses/decode.c
	${BISON_debugger_parser_OUTPUTS}
	${FLEX_debugger_scanner_OUTPUTS}
)
set(DEBUGGER_LIBS ${CURSES_LIBRARIES} ${READLINE_LIBRARY})

# ---- Target: em400 -----------------------------------------------------

add_executable(em400
	${SOURCES_EM400}
	${SOURCES_DEBUGGER}
	${BISON_ectl_parser_OUTPUTS}
	${FLEX_ectl_scanner_OUTPUTS}
)

set_property(TARGET em400 PROPERTY C_STANDARD 99)
target_include_directories(em400 PUBLIC SYSTEM ${EMCRK_INCLUDE_DIRS})
target_include_directories(em400 PUBLIC SYSTEM ${EMAWP_INCLUDE_DIRS})
target_include_directories(em400 PUBLIC SYSTEM ${EMDAS_INCLUDE_DIRS})
target_include_directories(em400 PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(em400 PUBLIC ${CMAKE_SOURCE_DIR}/include)
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
	${RT_LIBRARY}
	${MATH_LIBRARY}
)

if(ALSA_FOUND)
	target_include_directories(em400 PUBLIC SYSTEM ${ALSA_INCLUDE_DIRS})
	target_link_libraries(em400 ${ALSA_LIBRARIES})
	target_compile_definitions(em400 PRIVATE HAVE_ALSA)
endif()
if(PULSEAUDIO_FOUND)
	target_include_directories(em400 PUBLIC SYSTEM  ${PULSEAUDIO_INCLUDE_DIRS})
	target_link_libraries(em400 ${PULSEAUDIO_LIBRARIES} ${PULSEAUDIO_SIMPLE_LIBRARY})
	target_compile_definitions(em400 PRIVATE HAVE_PULSEAUDIO)
endif()

install(TARGETS em400
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ---- Target: emitool ---------------------------------------------------

add_executable(emitool
	src/emitool.c
	src/io/dev/e4image.c
)
set_property(TARGET emitool PROPERTY C_STANDARD 99)
target_include_directories(emitool PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_options(emitool PUBLIC -Wall)

install(TARGETS emitool
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ---- Target: embin -----------------------------------------------------

add_executable(embin
	src/embin.c
	src/utils/utils.c
)
set_property(TARGET embin PROPERTY C_STANDARD 99)
target_include_directories(embin PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_options(embin PUBLIC -Wall)

install(TARGETS embin
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# vim: tabstop=4
