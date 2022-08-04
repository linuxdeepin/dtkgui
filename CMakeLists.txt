cmake_minimum_required(VERSION 3.5)
option(DVERSION "define project version" "5.5.23")
if(DVERSION)
  project(DtkGui
   VERSION ${DVERSION}
    DESCRIPTION "DTK Gui module"
    HOMEPAGE_URL ""
    LANGUAGES CXX C
  )
else()
  project(DtkGui
    VERSION "5.5.23"
    DESCRIPTION "DTK Gui module"
    HOMEPAGE_URL ""
    LANGUAGES CXX C
  )
endif()

include(GNUInstallDirs)
option(NOTPACKAGE "to make package" ON)
option(DTK_DISABLE_LIBXDG "Disabel libxdg" OFF)
set (BUILD_DOCS ON CACHE BOOL "Generate doxygen-based documentation")

#message(${DTK_DISABLE_LIBXDG})
#set(LINUXNAME "debian")
option(LINUXNAME "linuxname" "debian")
set(SPECPATH "qt5/mkspecs/")
if(LINUXNAME)
  if (${LINUXNAME} STREQUAL "archlinux")
    set(SPECPATH "qt/mkspecs/")
    message("arch")
  endif()
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fsanitize=address -fno-omit-frame-pointer")
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


configure_file(misc/DtkGuiConfig.cmake.in DtkGuiConfig.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DtkGuiConfig.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkGui")

configure_file(misc/dtkgui.pc.in dtkgui.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dtkgui.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig/")

configure_file(misc/qt_lib_dtkgui.pri.in qt_lib_dtkgui.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_dtkgui.pri DESTINATION "${CMAKE_INSTALL_LIBDIR}/${SPECPATH}/modules")

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
install(FILES DtkGuis DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libdtk-${PROJECT_VERSION}/DGui")

add_subdirectory(examples)
add_subdirectory(src)
if(NOTPACKAGE)
add_subdirectory(tests)
endif()
add_subdirectory(tools)

if (BUILD_DOCS)
  add_subdirectory(doc)
endif ()
