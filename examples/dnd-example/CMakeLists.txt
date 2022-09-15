set(CMAKE_AUTOMOC ON)
find_package(DtkCore REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt5 REQUIRED COMPONENTS DBus)

set(BIN1 dnd-test-client)

add_executable(${BIN1}
  dnd-test-client.cpp
)

target_link_libraries(
  ${BIN1} PRIVATE
  Qt5::Widgets 
  Qt5::DBus 
  dtkgui
  ${DtkCore_LIBRARIES}
)

install(TARGETS ${BIN1} DESTINATION "${TOOL_INSTALL_DIR}")
set(BIN2 dnd-test-server)

add_executable(${BIN2}
  dnd-test-server.cpp
)

target_link_libraries(
  ${BIN2} PRIVATE
  Qt5::Widgets 
  Qt5::DBus 
  dtkgui
  ${DtkCore_LIBRARIES}
)


install(TARGETS ${BIN2} DESTINATION "${TOOL_INSTALL_DIR}")
