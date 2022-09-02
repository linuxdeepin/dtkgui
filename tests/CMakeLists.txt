set(BIN_NAME "ut-${PROJECT_NAME}")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(DtkCore REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Gui)
find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt5 REQUIRED COMPONENTS DBus)
find_package(Qt5 REQUIRED COMPONENTS Network)
find_package(Qt5 REQUIRED COMPONENTS Test)
find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)
#pkg_check_modules(GMOCK REQUIRED gmock)
add_definitions(-DTEST)

set(CMAKE_CXX_FLAGS "-fno-access-control")
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

include(../src/dbus/dbus.cmake)
include(../src/kernel/kernel.cmake)
include(../src/util/util.cmake)
include(../src/private/private.cmake)

file(GLOB test_SRC 
  res.qrc
  test.h
  main.cpp 
  src/*.cpp
)
add_executable(${BIN_NAME}
  ${dbus_SRC}
  ${kernel_SRC}
  ${util_SRC}
  ${private_SRC}
  ${test_SRC}
)
set(LIBRARIES 
    ${DtkCore_LIBRARIES}
    Qt5::Gui 
    Qt5::Widgets
    Qt5::DBus 
    Qt5::Network 
    Qt5::Test 
    ${librsvg_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
    -lpthread
    -lgmock
    -lm 
    -lgcov
)

if(NOT DTK_DISABLE_LIBXDG)
  list(APPEND LIBRARIES Qt5XdgIconLoader)
endif()

target_link_libraries(${BIN_NAME} PRIVATE 
  ${LIBRARIES}
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${BIN_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(${BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
endif()
target_include_directories(${BIN_NAME} PUBLIC
  ../include/util/
  ../include/DtkGui/
  ../include/global/
  ../include/kernel/
  ../include/filedrag/
  ../src/private/
  ../src/
  ${librsvg_INCLUDE_DIRS}
  ${GMOCK_INCLUDE_DIRS}
  ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)
add_test(NAME ${BIN_NAME} COMMAND ${BIN_NAME})
