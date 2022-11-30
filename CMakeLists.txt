cmake_minimum_required(VERSION 3.5)

set (DVERSION "5.6.2" CACHE STRING "define project version")
project(DtkGui
  VERSION ${DVERSION}
  DESCRIPTION "DTK Gui module"
  HOMEPAGE_URL "https://github.com/linuxdeepin/dtkgui"
  LANGUAGES CXX C
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Set build option
option(NOTPACKAGE "to make package" ON)
option(DTK_DISABLE_LIBXDG "Disabel libxdg" OFF)
set (BUILD_DOCS ON CACHE BOOL "Generate doxygen-based documentation")

# Set install path
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()
set (INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}/libdtk-${PROJECT_VERSION}/DGui")
set (TOOL_INSTALL_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/libdtk-${PROJECT_VERSION}/DGui/bin")
set (LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_FULL_LIBDIR}")
set (MKSPECS_INSTALL_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/qt5/mkspecs/modules" CACHE STRING "INSTALL DIR FOR qt pri files")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(BUILD_TESTING ON)
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
endif ()

set(CONFIGNAME include/global/dtkgui_config.h)
file(WRITE ${CONFIGNAME} 
  "// it is auto make config\n"
)
file(GLOB CONFIGSOURCE include/DtkGui/*)

foreach(FILENAME ${CONFIGSOURCE})
  get_filename_component(thefile ${FILENAME} NAME)
  file(APPEND ${CONFIGNAME} "#define DTKGUI_CLASS_${thefile}\n")
endforeach()

configure_package_config_file(misc/DtkGuiConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DtkGuiConfig.cmake
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkGui"
    PATH_VARS INCLUDE_INSTALL_DIR LIBRARY_INSTALL_DIR TOOL_INSTALL_DIR
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/DtkGuiConfigVersion.cmake"
    VERSION ${DVERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/DtkGuiConfig.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/DtkGuiConfigVersion.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkGui"
)

configure_file(misc/dtkgui.pc.in dtkgui.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dtkgui.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig/")

configure_file(misc/qt_lib_dtkgui.pri.in qt_lib_dtkgui.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_dtkgui.pri DESTINATION "${MKSPECS_INSTALL_DIR}")

set(GUISGNAME DtkGuis)

file(WRITE ${GUISGNAME} 
  "// it is auto make config\n"
  "#ifndef DTK_GUI_MODULE_H\n"
  "#define DTK_GUI_MODULE_H\n"
)
file(GLOB FILEGRAGTOWRITSOURCE include/filedrag/*)

foreach(FILENAME ${FILEGRAGTOWRITSOURCE})
  get_filename_component(thefile ${FILENAME} NAME)
  file(APPEND ${GUISGNAME} "#include \"${thefile}\"\n")
endforeach()
file(GLOB KERNELTOWRITESOURCE include/kernel/*)
foreach(FILENAME ${KERNELTOWRITESOURCE})
  get_filename_component(thefile ${FILENAME} NAME)
  file(APPEND ${GUISGNAME} "#include \"${thefile}\"\n")
endforeach()
file(GLOB UTILTOWRITESOURCE include/util/*)
foreach(FILENAME ${UTILTOWRITESOURCE})
  get_filename_component(thefile ${FILENAME} NAME)
  file(APPEND ${GUISGNAME} "#include \"${thefile}\"\n")
endforeach()
file(APPEND ${GUISGNAME} "#endif")
install(FILES DtkGuis DESTINATION "${INCLUDE_INSTALL_DIR}")

add_subdirectory(examples)
add_subdirectory(src)
if(BUILD_TESTING)
  message("==================================")
  message("       Now Testing is enabled     ")
  message("==================================")
  enable_testing()
  add_subdirectory(tests)
endif()
add_subdirectory(tools)

if (BUILD_DOCS)
  add_subdirectory(docs)
endif ()
