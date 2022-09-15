set(BIN_NAME deepin-gui-settings)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(DtkCore REQUIRED)

add_executable(${BIN_NAME} 
  main.cpp 
)

target_link_libraries(${BIN_NAME}
  Qt5::Widgets 
  dtkgui
  ${DtkCore_LIBRARIES}
)
target_include_directories(${BIN_NAME} PUBLIC
  ../../include/kernel/
  ../../include/DtkGui/
  ../../include/util/
  ../../include/
  ${DtkCore_INCLUDE_DIRS}
)
install(TARGETS ${BIN_NAME} DESTINATION "${TOOL_INSTALL_DIR}")
