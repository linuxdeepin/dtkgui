set(BIN_NAME dci-image-converter)
find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(Qt5 REQUIRED COMPONENTS Gui)

add_executable(${BIN_NAME}
  main.cpp
)
target_link_libraries(${BIN_NAME} PRIVATE
  Qt5::Core
  Qt5::Gui
)
