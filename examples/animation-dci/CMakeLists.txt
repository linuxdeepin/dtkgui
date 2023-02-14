set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(DtkCore REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(TARGET animation-dci)

add_executable(${TARGET}
  main.cpp
  test_dci_icon.qrc
)

target_link_libraries(
  ${TARGET} PRIVATE
  Qt5::Widgets
  dtkgui
  ${DtkCore_LIBRARIES}
)
