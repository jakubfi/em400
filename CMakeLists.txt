cmake_minimum_required(VERSION 3.9.6)
project(em400
	DESCRIPTION "MERA-400 emulator"
	LANGUAGES C CXX
)

include(GNUInstallDirs)
include(cmake/appversion.cmake)

# ---- UI curses ---------------------------------------------------------

find_package(Curses)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

set(UI_CURSES OFF)
if(CURSES_FOUND)
	if(WIN32)
		message(STATUS "Curses library found, but curses UI is disabled on Windows")
	else()
		set(UI_CURSES ON)
		message(STATUS "Curses library found. Curses UI enabled.")
	endif()
else()
	message(STATUS "Curses library not found. Curses UI disabled.")
endif()

if(UI_CURSES)
	set(CURSES_NEED_NCURSES TRUE)
	find_path(READLINE_INCLUDE_DIR readline/readline.h)
	find_library(READLINE_LIBRARY readline)
	find_library(TINFO_LIBRARY tinfo)

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

	add_library(ui-curses STATIC
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

	target_link_libraries(ui-curses
		${CURSES_LIBRARIES}
		${TINFO_LIBRARY}
		${READLINE_LIBRARY}
		${FLEX_LIBRARIES}
		${BISON_LIBRARIES}
	)

	target_include_directories(ui-curses PRIVATE ${CMAKE_SOURCE_DIR}/src)
	target_include_directories(ui-curses PRIVATE ${CMAKE_SOURCE_DIR}/include)
	target_include_directories(ui-curses PRIVATE ${CMAKE_BINARY_DIR})
endif(UI_CURSES)

# ---- Target: em400 -----------------------------------------------------

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Threads REQUIRED)
find_package(emdas REQUIRED)
find_package(emcrk REQUIRED)
find_package(emawp 3.0 REQUIRED)
if(NOT WIN32)
	find_library(MATH_LIBRARY m)
	find_library(RT_LIBRARY rt)
endif()

# --- IPO support

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
if(IPO_SUPPORTED)
	message(STATUS "LTO supported and enabled")
else()
	message(STATUS "LTO disabled: ${IPO_ERROR}")
endif()

# --- Sound

include(FindALSA)
include(cmake/FindPulseAudio.cmake)
find_library(PULSEAUDIO_SIMPLE_LIBRARY pulse-simple)

if(PULSEAUDIO_FOUND OR ALSA_FOUND)
	message(STATUS "Sound enabled: ALSA: ${ALSA_FOUND}, PulseAudio: ${PULSEAUDIO_FOUND}")
else()
	message(STATUS "No supported sound libraries found, disabling sound")
endif()

# --- QT

set(UI_QT OFF)
find_package(Qt5 COMPONENTS Widgets)
find_package(Qt5 COMPONENTS Multimedia)

if (Qt5Widgets_FOUND)
	message(STATUS "Found QT: ${Qt5Widgets_VERSION_STRING}")
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(UI_QT ON)
else()
	message(STATUS "QT not found")
endif()

# --- EM400

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
	src/utils/serial.c
	src/utils/serial.h

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
	src/mem/mega_prom.c
	src/mem/mega_prom.h
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
	src/ectl/parser.y
	src/ectl/scanner.l
	include/ectl.h

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
)

if(ALSA_FOUND)
	list(APPEND SOURCES_EM400 src/sound/alsa.c)
endif()
if(PULSEAUDIO_FOUND)
	list(APPEND SOURCES_EM400 src/sound/pulseaudio.c)
endif()

if (UI_QT)
	list(APPEND SOURCES_EM400 src/ui/qt5/qt5.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt5/mainwindow.ui)
	list(APPEND SOURCES_EM400 src/ui/qt5/resources.qrc)
	list(APPEND SOURCES_EM400 src/ui/qt5/mainwindow.h src/ui/qt5/mainwindow.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt5/emumodel.h src/ui/qt5/emumodel.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt5/dasmview.h src/ui/qt5/dasmview.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt5/memview.h src/ui/qt5/memview.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt5/asmline.h src/ui/qt5/asmline.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt5/controlpanel.h src/ui/qt5/controlpanel.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt5/switch.h src/ui/qt5/switch.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt5/led.h src/ui/qt5/led.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt5/rotary.h src/ui/qt5/rotary.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt5/ignition.h src/ui/qt5/ignition.cpp)
endif()

# --- internal parser

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

# --- em400 target

add_executable(em400
	${SOURCES_EM400}
	${BISON_ectl_parser_OUTPUTS}
	${FLEX_ectl_scanner_OUTPUTS}
)

set_property(TARGET em400 PROPERTY C_STANDARD 11)
target_compile_options(em400 PRIVATE -Wall)
target_include_directories(em400 PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(em400 PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(em400 PRIVATE ${CMAKE_BINARY_DIR})
target_compile_definitions(em400 PRIVATE EM400_VERSION="${APP_VERSION}")
if(UI_CURSES)
	target_compile_definitions(em400 PRIVATE UI_CURSES)
	target_link_libraries(em400 ui-curses)
endif(UI_CURSES)

if(IPO_SUPPORTED)
	set_property(TARGET em400 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if (UI_QT)
	set_property(TARGET em400 PROPERTY AUTOMOC ON)
	set_property(TARGET em400 PROPERTY AUTOUIC ON)
	set_property(TARGET em400 PROPERTY AUTORCC ON)
	set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/ectl_parser.c PROPERTY SKIP_AUTOGEN ON)
	set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/ectl_parser.h PROPERTY SKIP_AUTOGEN ON)
	set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/ectl_scanner.c PROPERTY SKIP_AUTOGEN ON)
	set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/uicurses_parser.c PROPERTY SKIP_AUTOGEN ON)
	set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/uicurses_parser.h PROPERTY SKIP_AUTOGEN ON)
	set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/uicurses_scanner.c PROPERTY SKIP_AUTOGEN ON)
	target_include_directories(em400 PRIVATE ${Qt5Widgets_INCLUDE_DIRS})
	target_compile_definitions(em400 PRIVATE UI_QT)
	target_link_libraries(em400 ${Qt5Widgets_LIBRARIES})
	target_link_libraries(em400 ${Qt5Multimedia_LIBRARIES})
endif()

target_link_libraries(em400
	emdas
	emcrk
	emawp
	${FLEX_LIBRARIES}
	${BISON_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
)

if(NOT WIN32)
	target_link_libraries(em400
		${RT_LIBRARY}
		${MATH_LIBRARY}
	)
endif()

if(ALSA_FOUND)
	target_include_directories(em400 PUBLIC SYSTEM ${ALSA_INCLUDE_DIRS})
	target_link_libraries(em400 ${ALSA_LIBRARIES})
	target_compile_definitions(em400 PRIVATE HAVE_ALSA)
endif()
if(PULSEAUDIO_FOUND)
	target_include_directories(em400 PUBLIC SYSTEM ${PULSEAUDIO_INCLUDE_DIRS})
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
if(WIN32)
	target_link_libraries(emitool ws2_32)
endif()
set_property(TARGET emitool PROPERTY C_STANDARD 11)
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
if(NOT WIN32)
	target_sources(embin PRIVATE src/utils/serial.c)
else()
	target_link_libraries(embin ws2_32)
endif()
set_property(TARGET embin PROPERTY C_STANDARD 11)
target_include_directories(embin PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_options(embin PUBLIC -Wall)

install(TARGETS embin
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# vim: tabstop=4
