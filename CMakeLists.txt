cmake_minimum_required(VERSION 3.9.6)
project(em400
	DESCRIPTION "MERA-400 emulator"
	LANGUAGES C
)

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)

if(IPO_SUPPORTED)
	message(STATUS "IPO supported")
else()
	message(STATUS "IPO not supported, disabling: ${error}")
endif()

include(GNUInstallDirs)
include(cmake/appversion.cmake)

# ---- Libraries ---------------------------------------------------------

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Threads REQUIRED)
find_package(emdas REQUIRED)
find_package(emcrk REQUIRED)
find_package(emawp 3.0 REQUIRED)
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
	src/atomic.h
	src/em400.c
	src/em400.h
	src/log.c
	src/log.h
	src/log_io.h
	src/log_crk.c
	src/log_crk.h
	src/cfg.c
	src/cfg.h

	src/utils/utils.c
	src/utils/utils.h
	src/utils/elst.c
	src/utils/elst.h

	src/cpu/instructions.c
	src/cpu/instructions.h
	src/cpu/clock.c
	src/cpu/clock.h
	src/cpu/cpu.c
	src/cpu/cpu.h
	src/cpu/interrupts.c
	src/cpu/interrupts.h
	src/cpu/iset.c
	src/cpu/iset.h
	src/cpu/alu.c
	src/cpu/alu.h
	src/cpu/cp.c
	src/cpu/cp.h

	src/mem/mem.c
	src/mem/mem.h
	src/mem/mega.c
	src/mem/mega.h
	src/mem/elwro.c
	src/mem/elwro.h

	src/fpga/iobus.c
	src/fpga/iobus.h

	src/io/defs.h
	src/io/io.c
	src/io/io.h
	src/io/chan.c
	src/io/chan.h
	src/io/cchar.c
	src/io/cchar.h
	src/io/cchar_term.c
	src/io/cchar_term.h
	src/io/cchar_flop8.c
	src/io/cchar_flop8.h
	src/io/iotester.c

	src/io/dev/dev.c
	src/io/dev/dev.h
	src/io/dev/e4image.c
	src/io/dev/e4image.h
	src/io/dev/winchester.c
	src/io/dev/flop5.c
	src/io/dev/punchrd.c
	src/io/dev/puncher.c
	src/io/dev/terminal.c
	src/io/dev/printer.c
	src/io/dev/fdbridge.c
	src/io/dev/fdbridge.h

	src/io/mx/mx.c
	src/io/mx/mx.h
	src/io/mx/cmds.c
	src/io/mx/cmds.h
	src/io/mx/irq.c
	src/io/mx/irq.h
	src/io/mx/event.c
	src/io/mx/event.h
	src/io/mx/line.c
	src/io/mx/line.h
	src/io/mx/proto_common.c
	src/io/mx/proto_common.h
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
	src/ui/ui.h

	src/ui/cmd/cmd.c
	src/ui/cmd/commands.c
	src/ui/cmd/commands.h
	src/ui/cmd/utils.c
	src/ui/cmd/utils.h

	src/ectl/cpu.c
	src/ectl/est.c
	src/ectl/est.h
	src/ectl/brk.c
	src/ectl/brk.h

	src/sound/file.c
	src/sound/sound.c
	src/sound/sound.h
	src/external/biquad/biquad.c
	src/external/biquad/biquad.h
	src/cpu/buzzer.c
	src/cpu/buzzer.h

	src/external/iniparser/dictionary.c
	src/external/iniparser/dictionary.h
	src/external/iniparser/iniparser.c
	src/external/iniparser/iniparser.h

	src/ectl/parser.y
	src/ectl/scanner.l
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
bison_target(ui_curses_parser
	src/ui/curses/parser.y
	${CMAKE_CURRENT_BINARY_DIR}/ui_curses_parser.c
)
flex_target(ui_curses_scanner
	src/ui/curses/scanner.l
	${CMAKE_CURRENT_BINARY_DIR}/ui_curses_scanner.c
	COMPILE_FLAGS "--never-interactive"
)
add_flex_bison_dependency(ui_curses_scanner ui_curses_parser)
set(SOURCES_DEBUGGER
	src/ui/curses/awin.c
	src/ui/curses/awin.h
	src/ui/curses/awinch.c
	src/ui/curses/debugger.c
	src/ui/curses/debugger.h
	src/ui/curses/eval.c
	src/ui/curses/eval.h
	src/ui/curses/cmd.c
	src/ui/curses/cmd.h
	src/ui/curses/ui.c
	src/ui/curses/ui.h
	src/ui/curses/keywords.c
	src/ui/curses/keywords.h
	src/ui/curses/decode.c
	src/ui/curses/decode.h
	src/ui/curses/parser.y
	src/ui/curses/scanner.l
	${BISON_ui_curses_parser_OUTPUTS}
	${FLEX_ui_curses_scanner_OUTPUTS}
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
target_include_directories(em400 PUBLIC SYSTEM ${EMAWP_INCLUDE_DIRS})
target_include_directories(em400 PUBLIC SYSTEM ${EMDAS_INCLUDE_DIRS})
target_include_directories(em400 PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(em400 PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(em400 PRIVATE ${CMAKE_BINARY_DIR})
target_compile_definitions(em400 PRIVATE EM400_VERSION="${APP_VERSION}")
target_compile_options(em400 PUBLIC -Wall)
if(IPO_SUPPORTED)
	set_property(TARGET em400 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_link_libraries(em400
	${DEBUGGER_LIBS}
	${EMDAS_LIBRARIES}
	emcrk
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

install(FILES
	${CMAKE_SOURCE_DIR}/tools/e4term
	PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
				OWNER_READ GROUP_READ WORLD_READ
				OWNER_WRITE
	DESTINATION ${CMAKE_INSTALL_BINDIR}
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
