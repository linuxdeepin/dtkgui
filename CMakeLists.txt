cmake_minimum_required(VERSION 3.25)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" FILE_VERSION)
string(STRIP "${FILE_VERSION}" FILE_VERSION)

project(DtkGui
  VERSION ${FILE_VERSION}
  DESCRIPTION "DTK Gui module"
  HOMEPAGE_URL "https://github.com/linuxdeepin/dtkgui"
  LANGUAGES CXX C
)

# 引入 DTK5/DTK6 构建选项
option(DTK5 "Build DTK5." ON)
if(DTK5)
    set(DTK_VERSION_MAJOR "5")
    set(DTK_NAME_SUFFIX "")     # DTK5 产品名称后缀为空
else()
    set(DTK_VERSION_MAJOR "6")
    set(DTK_NAME_SUFFIX "6")    # DTK6 产品名称后缀为 "6"
endif()

set(DTK_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(DTK_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(DTK_VERSION "${DTK_VERSION_MAJOR}.${DTK_VERSION_MINOR}.${DTK_VERSION_PATCH}")
set(QT_VERSION_MAJOR ${DTK_VERSION_MAJOR})

include(dtkgui.cmake)
