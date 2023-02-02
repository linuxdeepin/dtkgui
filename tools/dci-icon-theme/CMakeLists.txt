set(BIN_NAME dci-icon-theme)

add_executable(${BIN_NAME}
    main.cpp
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt5::Gui
    ${LIB_NAME}
)

install(TARGETS ${BIN_NAME} DESTINATION "${TOOL_INSTALL_DIR}")
