set(BIN_NAME ut-dtkgui)

find_package(Qt5 REQUIRED COMPONENTS Widgets Test)
find_package(GTest REQUIRED)

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

target_compile_options(${BIN_NAME} PRIVATE -fno-access-control -fsanitize=address)
target_link_options(${BIN_NAME} PRIVATE -fsanitize=address)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt5::Gui
    Qt5::GuiPrivate
    Qt5::Widgets
    Qt5::DBus
    Qt5::Network
    Qt5::Test
    Dtk::Core
    PkgConfig::librsvg
    GTest::GTest
    gmock
    pthread
    m
    gcov
)

if(NOT DTK_DISABLE_EX_IMAGE_FORMAT AND EX_IMAGE_FORMAT_LIBS_FOUND) 
target_link_libraries(${BIN_NAME} PRIVATE
	PkgConfig::libraw
    FreeImage::FreeImage
)
endif()

if(NOT DTK_DISABLE_LIBXDG)
    target_link_libraries(${BIN_NAME} PRIVATE 
        Qt5XdgIconLoader
    )
endif()

target_include_directories(${BIN_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include/util
    ${PROJECT_SOURCE_DIR}/include/DtkGui
    ${PROJECT_SOURCE_DIR}/include/global
    ${PROJECT_SOURCE_DIR}/include/kernel
    ${PROJECT_SOURCE_DIR}/include/filedrag
    ${PROJECT_SOURCE_DIR}/src/private
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/dbus
    ${PROJECT_SOURCE_DIR}/src/util/private
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${BIN_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(${BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
endif()

add_test(NAME ${BIN_NAME} COMMAND ${BIN_NAME})
