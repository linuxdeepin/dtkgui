set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui EventDispatcherSupport ServiceSupport)
find_package(Dtk${DTK_VERSION_MAJOR}Core REQUIRED)

file(GLOB plugin_SRC
    minimal.json
    *.h
    *.cpp
)

add_library(${test-plugin} SHARED ${plugin_SRC})

target_link_libraries(${test-plugin} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::CorePrivate
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::EventDispatcherSupportPrivate
    Qt${QT_VERSION_MAJOR}::ServiceSupportPrivate
    Dtk${DTK_VERSION_MAJOR}::Core)

target_include_directories(${test-plugin} PUBLIC
    ${PROJECT_SOURCE_DIR}/include/global
    ${PROJECT_SOURCE_DIR}/include/kernel
    ${PROJECT_SOURCE_DIR}/include/filedrag
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/../plugins/platforms)
