set(BIN_NAME deepin-gui-settings)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

add_executable(${BIN_NAME} 
  	main.cpp
)

target_link_libraries(${BIN_NAME}
	Qt5::Widgets
	Dtk::Core
	${LIB_NAME}
)

install(TARGETS ${BIN_NAME} DESTINATION "${TOOL_INSTALL_DIR}")
