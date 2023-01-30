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

if(DTK_DISABLE_EX_IMAGE_FORMAT OR NOT EX_IMAGE_FORMAT_LIBS_FOUND)
    add_definitions(-DDTK_DISABLE_EX_IMAGE_FORMAT)
    message("Disable extended image format!")
else()
    message("Support extended image format!")
endif()

file(GLOB UTIL_HEADER
    ${PROJECT_SOURCE_DIR}/include/util/*.h
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
