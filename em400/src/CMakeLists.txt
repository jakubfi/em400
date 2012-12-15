ADD_EXECUTABLE(em400 em400.c instructions.c io.c memory.c timer.c cpu.c errors.c interrupts.c iset.c registers.c utils.c)

target_link_libraries(em400 readline ncurses rt ${FLEX_LIBRARIES}
${BISON_LIBRARIES} ${DEBUGGER_LIB})
