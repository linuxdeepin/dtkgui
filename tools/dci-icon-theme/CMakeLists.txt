set(BIN_NAME dci-icon-theme)
find_package(Qt5 REQUIRED COMPONENTS Core Gui)
find_package(Dtk REQUIRED COMPONENTS Core)

add_executable(${BIN_NAME}
  main.cpp
)
target_link_libraries(${BIN_NAME} PRIVATE
  Qt5::Core
  Qt5::Gui
  ${DtkCore_LIBRARIES}
)

install(TARGETS ${BIN_NAME} DESTINATION "${TOOL_INSTALL_DIR}")
