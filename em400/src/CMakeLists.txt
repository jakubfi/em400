ADD_EXECUTABLE(em400 em400.c instructions.c io.c memory.c timer.c cpu.c errors.c interrupts.c iset.c registers.c utils.c alu.c)

target_link_libraries(em400
drv
${FLEX_LIBRARIES} ${BISON_LIBRARIES} ${DEBUGGER_LIB}
readline ncurses rt
)
