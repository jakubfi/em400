add_library(ui
	ui.c
)

add_subdirectory(cmd)

target_link_libraries(ui
	ui_cmd
)

# vim: tabstop=4
