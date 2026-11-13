# SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(wayland_test)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(AUTOMOC_COMPILER_PREDEFINES ON)

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui WaylandClient)
if(${QT_VERSION_MAJOR} STREQUAL "5")
    find_package(Qt5 REQUIRED COMPONENTS XkbCommonSupport)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)

find_package(TreelandProtocols 0.7.0 REQUIRED)
set(TreelandProtocols_FOUND ${TreelandProtocols_FOUND})
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${ECM_MODULE_PATH}")
find_package(WaylandProtocols 1.45 REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

if("${QT_VERSION_MAJOR}" STREQUAL "6")
    qt6_generate_wayland_protocol_client_sources(${PROJECT_NAME} FILES
        ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-appearance-unstable-v1.xml
        ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-decoration-unstable-v1.xml
        ${WaylandProtocols_DATADIR}/staging/ext-background-effect/ext-background-effect-v1.xml
    )

    if (${Qt6Core_VERSION} VERSION_GREATER_EQUAL "6.10.0")
        set(QT_NO_PRIVATE_MODULE_WARNING ON)
        find_package(Qt6 REQUIRED COMPONENTS GuiPrivate CorePrivate WaylandClientPrivate)
    endif()
else()
    # ECM setup
    include(FeatureSummary)
    find_package(QtWaylandScanner)
    ecm_add_qtwayland_client_protocol(CLIENT_SRCS
        PROTOCOL ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-appearance-unstable-v1.xml
        BASENAME treeland-appearance-unstable-v1
    )
    ecm_add_qtwayland_client_protocol(CLIENT_SRCS
        PROTOCOL ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-decoration-unstable-v1.xml
        BASENAME treeland-decoration-unstable-v1
    )
    ecm_add_qtwayland_client_protocol(CLIENT_SRCS
        PROTOCOL ${WaylandProtocols_DATADIR}/staging/ext-background-effect/ext-background-effect-v1.xml
        BASENAME ext-background-effect-v1
    )
    target_sources(${PROJECT_NAME} PRIVATE ${CLIENT_SRCS})

    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::XkbCommonSupportPrivate
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::CorePrivate
    Qt${QT_VERSION_MAJOR}::WaylandClientPrivate
    ${WAYLAND_CLIENT_LIBRARIES}
)
