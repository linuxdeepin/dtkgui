if(NOT DTK_DISABLE_LIBXDG)
  find_package(qt5xdgiconloader)
  add_definitions(-DXDG_ICON_VERSION_MAR=${qt5xdgiconloader_VERSION_MAJOR})
  set(UTIL_PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/private/xdgiconproxyengine_p.h
    ${CMAKE_CURRENT_LIST_DIR}/private/xdgiconproxyengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/private/dbuiltiniconengine_p.h
    ${CMAKE_CURRENT_LIST_DIR}/private/dbuiltiniconengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/private/dimagehandlerlibs_p.h
  )
else()
    add_definitions(-DDTK_DISABLE_LIBXDG)
    set(UTIL_PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/private/dbuiltiniconengine_p.h
      ${CMAKE_CURRENT_LIST_DIR}/private/dbuiltiniconengine.cpp
      ${CMAKE_CURRENT_LIST_DIR}/private/dimagehandlerlibs_p.h
    )
endif()

pkg_check_modules(librsvg REQUIRED librsvg-2.0)
#DEFINES += "XDG_ICON_VERSION_MAR=$$first(XDG_ICON_VERSION_LIST)"
file(GLOB UTIL_HEADER
  ${CMAKE_CURRENT_LIST_DIR}/../../include/util/*.h
)
file(GLOB UTIL_SOURCE 
  ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)
set(util_SRC
  ${UTIL_HEADER}
  ${UTIL_SOURCE}
  ${UTIL_PRIVATE}
  ${CMAKE_CURRENT_LIST_DIR}/icons/deepin-theme-plugin-icons.qrc
)

# include custom Modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
find_package(freeimage REQUIRED)
include_directories(${freeimage_INCLUDE_DIR})
