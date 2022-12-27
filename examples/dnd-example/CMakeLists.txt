find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(BIN1 dnd-test-client)

add_executable(${BIN1}
    dnd-test-client.cpp
)

target_link_libraries(${BIN1} PRIVATE
    Qt5::Widgets
    Qt5::DBus
    ${LIB_NAME}
    Dtk::Core
)

install(TARGETS ${BIN1} DESTINATION "${TOOL_INSTALL_DIR}")

set(BIN2 dnd-test-server)

add_executable(${BIN2}
    dnd-test-server.cpp
)

target_link_libraries(${BIN2} PRIVATE
    Qt5::Widgets
    Qt5::DBus
    ${LIB_NAME}
    Dtk::Core
)

install(TARGETS ${BIN2} DESTINATION "${TOOL_INSTALL_DIR}")
