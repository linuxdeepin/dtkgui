set(CMAKE_AUTOMOC ON)
find_package(Qt5 REQUIRED COMPONENTS Gui X11Extras)
include(FindPkgConfig)
pkg_check_modules(XCB xcb)

set(TARGET frameless-window)

add_executable(${TARGET}
  main.cpp
)

target_link_libraries(
  ${TARGET} PRIVATE
  Qt5::Gui
  Qt5::X11Extras
  ${XCB_LIBRARIES}
)

target_include_directories(${TARGET} PUBLIC
  ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)
