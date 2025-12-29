set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Dtk${DTK_NAME_SUFFIX}Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(BIN_NAME animation-dci)
set(TARGET ${BIN_NAME}${DTK_NAME_SUFFIX})
add_executable(${TARGET}
  main.cpp
  test_dci_icon.qrc
)

target_link_libraries(
  ${TARGET} PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
  dtk${DTK_NAME_SUFFIX}gui
)
set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME ${BIN_NAME})
