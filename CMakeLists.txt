cmake_minimum_required(VERSION 3.25)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" DTK_FILE_VERSION)
string(STRIP "${DTK_FILE_VERSION}" DTK_FILE_VERSION)
set(DTK_VERSION "${DTK_FILE_VERSION}" CACHE STRING "define project version")
project(DtkGui
  VERSION ${DTK_VERSION}
  DESCRIPTION "DTK Gui module"
  HOMEPAGE_URL "https://github.com/linuxdeepin/dtkgui"
  LANGUAGES CXX C
)

if("${PROJECT_VERSION_MAJOR}" STREQUAL "5")
    set(QT_VERSION_MAJOR "5")
elseif("${PROJECT_VERSION_MAJOR}" STREQUAL "6")
    set(QT_VERSION_MAJOR "6")
    set(DTK_VERSION_MAJOR "6")
else()
    message(SEND_ERROR "not support Prject Version ${PROJECT_VERSION}.")
endif()

include(dtkgui.cmake)
