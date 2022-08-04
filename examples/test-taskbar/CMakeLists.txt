set(CMAKE_AUTOMOC ON)
find_package(DtkCore REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt5 REQUIRED COMPONENTS Gui)

set(BIN taskbar)

add_executable(${BIN}
  testtaskbarwindow.h 
  testtaskbarwindow.cpp
  main.cpp
)

target_link_libraries(
  ${BIN} PRIVATE
  Qt5::Widgets 
  Qt5::Gui 
  dtkgui
  ${DtkCore_LIBRARIES}
)
