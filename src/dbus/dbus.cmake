set(dbus_SRC)
set_source_files_properties(
  ${CMAKE_CURRENT_LIST_DIR}/com.deepin.api.XEventMonitor.xml
  PROPERTIES
  INCLUDE ${CMAKE_CURRENT_LIST_DIR}/arealist.h
)
qt5_add_dbus_interface(
  dbus_SRC	
  ${CMAKE_CURRENT_LIST_DIR}/com.deepin.api.XEventMonitor.xml
  xeventmonitor_interface
)
