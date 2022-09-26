set(LIB_NAME dtkgui)
find_package(DtkCore 5.6 REQUIRED)
find_package(PkgConfig REQUIRED)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Gui)
find_package(Qt5 REQUIRED COMPONENTS DBus)
find_package(Qt5 REQUIRED COMPONENTS Network)

include(dbus/dbus.cmake)
include(filedrag/dfiledrag.cmake)
include(kernel/kernel.cmake)
include(private/private.cmake)
include(util/util.cmake)

add_library(${LIB_NAME} SHARED 
  ${dbus_SRC}
  ${filedrag_SRC}
  ${kernel_SRC}
  ${util_SRC}
  ${private_SRC}
)
if(NOT DTK_DISABLE_LIBXDG)
  target_link_libraries(
    ${LIB_NAME} PRIVATE 
    Qt5::Gui 
    Qt5::DBus 
    Qt5::Network
    Qt5XdgIconLoader
    ${librsvg_LIBRARIES}
    ${DtkCore_LIBRARIES}
  )
  
  target_include_directories(${LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../include/util/ 
    ${CMAKE_CURRENT_LIST_DIR}/../include/DtkGui/ 
    ${CMAKE_CURRENT_LIST_DIR}/../include/global/ 
    ${CMAKE_CURRENT_LIST_DIR}/../include/kernel/ 
    ${CMAKE_CURRENT_LIST_DIR}/../include/filedrag/ 
    ${CMAKE_CURRENT_LIST_DIR}/../include/
    ${CMAKE_CURRENT_LIST_DIR}/private/
    ${librsvg_INCLUDE_DIRS}
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${DtkCore_INCLUDE_DIRS}
  )
else()
  target_link_libraries(
    ${LIB_NAME} PRIVATE 
    Qt5::Gui 
    Qt5::DBus 
    Qt5::Network
    ${librsvg_LIBRARIES}
    ${DtkCore_LIBRARIES}
  )
  
  target_include_directories(${LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../include/util/ 
    ${CMAKE_CURRENT_LIST_DIR}/../include/DtkGui/ 
    ${CMAKE_CURRENT_LIST_DIR}/../include/global/ 
    ${CMAKE_CURRENT_LIST_DIR}/../include/kernel/ 
    ${CMAKE_CURRENT_LIST_DIR}/../include/filedrag/ 
    ${CMAKE_CURRENT_LIST_DIR}/../include/
    ${CMAKE_CURRENT_LIST_DIR}/private/
    ${librsvg_INCLUDE_DIRS}
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${DtkCore_INCLUDE_DIRS}
  )
endif()
set_target_properties(${LIB_NAME} PROPERTIES
  VERSION ${CMAKE_PROJECT_VERSION}
  SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
)
install(TARGETS ${LIB_NAME} DESTINATION "${LIBRARY_INSTALL_DIR}")
install(DIRECTORY ../include/util/ DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*")
install(DIRECTORY ../include/DtkGui/ DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*")
install(DIRECTORY ../include/global/ DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*")
install(DIRECTORY ../include/kernel/ DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*")
install(DIRECTORY ../include/filedrag/ DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*")
