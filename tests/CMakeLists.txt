set(BIN_NAME ut-DtkGui)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Test)
find_package(GTest REQUIRED)

include(../src/dbus/dbus.cmake)
include(../src/kernel/kernel.cmake)
include(../src/util/util.cmake)
include(../src/private/private.cmake)
include(../src/filedrag/dfiledrag.cmake)

file(GLOB test_SRC
    res.qrc
    test.h
    main.cpp
    src/*.cpp
)
if(DTK_DISABLE_LIBXDG)
    list(REMOVE_ITEM test_SRC "${CMAKE_CURRENT_LIST_DIR}/src/ut_xdgiconproxyengine.cpp")
endif()

set(test-plugin "minimal-plugin")
add_subdirectory(platform-plugin-test)

add_executable(${BIN_NAME}
    ${dbus_SRC}
    ${kernel_SRC}
    ${util_SRC}
    ${private_SRC}
    ${filedrag_SRC}
    ${test_SRC}
)

add_dependencies(${BIN_NAME} ${test-plugin})

target_compile_options(${BIN_NAME} PRIVATE -fno-access-control -fsanitize=address)
target_link_options(${BIN_NAME} PRIVATE -fsanitize=address)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Test
    Dtk${DTK_VERSION_MAJOR}::Core
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
        Qt${QT_VERSION_MAJOR}XdgIconLoader
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
